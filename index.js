'use strict';

// const binary = require('node-pre-gyp');
// const path = require('path');
// const pcan_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
// const pcan = require(pcan_path);
const pcan = require('./binding/Release/node-v93-win32-x64/cs_pcan_usb.node');

const { Duplex } = require('stream');
const tpcan = require('./lib/tpcan');

// Default configuration used unless overridden by caller
const DEFAULT_OPTIONS = {
  canRate: 250000,
  filters: [],
  loopback: false,
};

const PCAN_RECEIVE_STATUS = 0x0F;
const PCAN_LISTEN_ONLY = 0x08;

module.exports = class PcanUsb extends Duplex {

  constructor(options) {
    super({ objectMode: true });

    let me = this;

    this.setOptions(options);
    this.requestQueue = [];
    this.port = null;
    this.isReady = false;

    this.status = {
      code: undefined,
      string: "",
      isError: false,
      notify() {
        let status = { code: this.code, string: this.string, isError: this.isError };
        me.emit('status', status );
        console.debug(status);
      },
      get statusCode() { return this.code; },
      set statusCode(code) {
        let same = (this.code === code) ? true : false;
        if (!same) {
          this.code = code;
          if (this.code == 0) {
            this.string = "Bus OK";
            this.isError = false;
          } else {
            this.isError = true;
            try {
              this.string = pcan.GetErrorText(code, 0);
            } catch(err) {
              console.error(err);
            }
          }
          this.notify();
        }
      }
    }
  }

  // Sets (or re-sets) the configuration options
  setOptions(options) {
    // Save for later use
    this.options = Object.assign(DEFAULT_OPTIONS, options);
  }

  // Returns a promise that resolves to a list of available CAN ports
  list() {
    let me = this;
    return new Promise(function(resolve, reject) {
      if (process.platform == "win32") {
        // If running on Windows, get channel data through the ChannelInfo method
        try {
          resolve(pcan.ChannelInfo());
        } catch(err) {
          if (err.code == "PCAN_ERROR_NOCHANNELS") {
            reject(err);
          }
        }
      } else if (process.platform == "darwin") {
        const usb  = require('usb');
        // If running on macOS, we fake it by resolving with an array of objects
        // we create, based on the number of detected PCAN-USB devices. We assume
        // that the first handle is always 0x51.
        let devices = usb.getDeviceList().filter((dev) => {
          return ((dev.deviceDescriptor.idVendor == 0x0C72) &&
                  (dev.deviceDescriptor.idProduct == 0x000C));
        });

        let channels = [];

        for (let index in devices) {
          channels.push({ channel_handle: (0x51 + Number(index)),
                          device_type: 5,
                          controller_number: 0,
                          device_features: 0,
                          device_id: 0,
                          channel_condition: 0 })
        }

        resolve(channels);
      } else {
        // If running on Linux or any other platform, we return an empty channel
        // list in an attempt to fail gracefully
        resolve([]);
      }

    })
      .then(function(channels) {
        // For compatibility, create a new property 'path' that is the
        // identifier for the CAN device in the system and can be passed
        // to the open() method
        for (let idx in channels) {
          channels[idx].path = channels[idx].channel_handle;
        }
        return channels;
      })
      .catch(function(err) {
        me.emit('error', err);
        throw err;
      });
  }

  open(port) {
    let me = this;

    return new Promise(function(resolve, reject) {

      try {

        // Ensure listen-only mode is turned off
        pcan.SetValue(port, PCAN_LISTEN_ONLY, Buffer.from([0]));

        // Windows only: Start with message reception off - failure to do this
        // has been observed to cause inexplicable timing problems in apps
        if (process.platform == "win32") {
          pcan.SetValue(port, PCAN_RECEIVE_STATUS, Buffer.from([0]));
        }

        // Initialize CAN port (aka "channel")
        pcan.Initialize(port, pcan.TranslateBaud(me.options.canRate));

        me.port = port;
        me._configure();
        me.isReady = true;
        me.emit('open');

        // Windows only: Turn on message reception
        if (process.platform == "win32") {
          pcan.SetValue(port, PCAN_RECEIVE_STATUS, Buffer.from([1]));
        }

        resolve();
      } catch(err) {
        reject(err);
      }
    })
      .then(function() {
        // Close event handler
        me.on('close', function() {
          me.close();
        });

        // Data receive event handler
        me.on('_data', me._onData.bind(me));

        // Enable data event
        pcan.EnableEvent(port, function() {
          me.emit('_data');
        });

        // Enable busoff auto-reset
        if (process.platform == "win32") {
          pcan.SetValue(port, 0x07, Buffer.from([0x01]));
        }
      })
      .catch(function(err) {
        me.emit('error', err);
        throw err;
      });
  }

  close() {
    let me = this;

    return new Promise(function(resolve, reject) {
      if (me.port === undefined) {
        reject(new Error("CAN port is undefined"));
      } else {
        // Remove listeners
        me.removeAllListeners();
        if (me.isOpen()) {
          pcan.DisableEvent(me.port);
          pcan.Reset(me.port);
          pcan.Uninitialize(me.port);
        }
        // Close stream
        me.emit('close');
        resolve();
      }
    })
      .then(function() {
        me.port = undefined;
        me.isReady = false;
        me.push(null);
      })
      .catch(function(err) {
        me.emit('error', err);
        throw err;
      });
  }

  write(msg) {
    let me = this;

    return new Promise(function(resolve, reject) {
      if (me.port === undefined) {
        reject(new Error("CAN port is undefined"));
      } else {
        if (msg.buf.length <= 8) {
          let tpcan_buffer = tpcan.toTPCANMsg(msg);

          // Write message to bus
          try {
            pcan.Write(me.port, tpcan_buffer);
          } catch(err) {
            reject(err);
          }

          me.emit('write', msg);

          // If loopback is enabled, push message back into receive stream
          if (me.options.loopback) {
            me.push(tpcan.toMsg(tpcan_buffer));
          }

          resolve();
        } else {
          reject(new Error("Tried to send invalid CAN data"));
        }
      }
    })
    .catch(function(err) {
      me.emit('error', err);
      throw err;
    });
  }

  status() {
    let me = this;

    return new Promise(function(resolve, reject) {
      if (me.port === undefined) {
        reject(new Error("CAN port is undefined"));
      } else {
        let status = pcan.GetStatus(me.port);
        resolve(status);
      }
    })
    .catch(function(err) {
      me.emit('error', err);
      throw err;
    });
  }

  // Required function for cs-modbus GenericConnection
  isOpen() {
    return this.port && this.isReady;
  }

  // Required function for cs-modbus GenericConnection
  isConnected() {
    return this.isOpen();
  }

  _onData() {
    let me = this;

    if (me.port === undefined) {
      let err = new Error("CAN port is undefined");
      me.emit('error', err);
      throw err;
    } else {
      while (true) {
        try {
          let read_obj = pcan.Read(me.port);
          me.push(tpcan.toMsg(read_obj.message)); // Emits 'data' event
        } catch(err) {
          if (err.code != "PCAN_ERROR_QRCVEMPTY") {
            console.debug(err.code);
          }
          break;
        }
      }
      // If status changed, the following will emit a 'status' event
      this.status.statusCode = pcan.GetStatus(me.port);
    }
  }

  // Required for a readable stream; we don't need to do anything
  _read() {}

  // Required for a writable stream; we don't need to do anything
  _write() {}

  _configure() {
    let me = this;

    // Set filters
    if (me.options.filters.length > 0) {
      // Ensure extended/standard filters aren't being mixed, because this
      // isn't supported by the PCAN-USB hardware.
      if ((me.options.filters.some((f) => { return (f.ext == true); })) &&
          (me.options.filters.some((f) => { return (f.ext == false); }))) {
        let err = new Error("PCAN-USB does not support mixing extended and standard filters.");
        me.emit('error', err);
        throw err;
      }

      for (let i in me.options.filters) {
        const filter = me.options.filters[i];
        if (filter.code && filter.mask) {
          if (filter.ext) {
            pcan.AcceptanceFilter29Bit(me.port, filter.code, filter.mask);
          } else {
            pcan.AcceptanceFilter11Bit(me.port, filter.code, filter.mask);
          }
        } else if (filter.fromID && filter.toID) {
          pcan.FilterMessages(me.port, filter.fromID, filter.toID, 0x02);
        } else if (filter.id) {

          // Split start/stop range and convert them to acceptance code + mask
          const parts = filter.id.split(' ');
          let fromID = parseInt(parts[0], 16);
          let toID = parseInt(parts[1], 16);
          pcan.FilterMessages(me.port, fromID, toID, 0x02);
        } else {
          let err = new Error("Unknown or unspecified filter format.");
          me.emit('error', err);
          throw err;
        }
      }
    }
  }
};

