/**
 * Implements a pingpong test using 2 CAN-USB-COM modules
 *
 * Note you must
 *   npm install can-usb-com
 * in order for this to work.to
 *
 * The two devices must be connected together and properly terminated (so that
 * CANBUS communication is physically possible).physically
 *
 * This script opens both devices, and each device sends a message to the other.
 * When a device receives a message from the other, it immediately sends it back.it
 *
 * We use the 'priority' field of the message ID to keep track of who originated
 * the message.
 *
 * So the two devices continually send bi-directional messages to each other.
 *
 */

const CanPCAN = require('../..');
const CanUSBCOM = require('can-usb-com');

const J1939 = require('@csllc/j1939');


let can1 = new CanPCAN({ // Device 1
  canRate: 250000,
  //canRate: 1000000,


  filters: [{
    ext: true,

    // make sure to include any possible incoming messages we might get
    id: '08000000 1CFFFFFF'
  }]

});

let can2 = new CanUSBCOM({ // Device 2
  baudRate: 460800,
  canRate: 250000,
  //canRate: 250000,


  filters: [{
    ext: true,

    // make sure to include any possible incoming messages we might get
    id: '08000000 1CFFFFFF'
  }]

});


// A class to hold the state information for the pinging
class Pinger extends J1939 {

  constructor(bus, options) {
    super(bus, options)

    this.priority = options.priority;
    this.fill = options.fill;

    this.msgSize = 1;

    // bus.on('data', (msg) => {
    //   console.log(msg.id.toString(16), msg.buf);
    // });
  }

  start(target) {

    this.target = target;

    this.on('data', this.onMsg.bind(this));

    this.sendNext();

  }


  sendNext() {

    console.log(this.address + ' sending ' + this.msgSize + ' bytes');

    this.write({
      pgn: 0xEF00,
      dst: this.target,
      priority: this.priority,
      buf: Buffer.alloc(this.msgSize).fill(this.fill)
    });

  }

  onMsg(msg) {

    let me = this;

    if (msg.pgn === 0xEF00) {


      // If it originated from us, this is a response so go to the next message
      // size
      if (msg.buf[0] === me.fill) {

        console.log(this.address + ' Received ' + msg.buf.length + ' byte response from ' + msg.src);

        me.msgSize++;
        if (me.msgSize > 1785) {
          me.msgSize = 1;
          process.exit(0);
        }

        me.sendNext();
      } else {
        // it's from the other guy, just send it back
        this.write({
          pgn: msg.pgn,
          dst: msg.src,
          priority: msg.pri,
          buf: msg.buf
        });
      }
    }
  }

}

// The two instances of pingers

let device1 = new Pinger(can1, { //PCAN-USB
  address: 0x80,
  priority: 4,
  fill: 0x55,
});

let device2 = new Pinger(can2, { //CAN-USB-COM
  address: 0x81,
  priority: 5,
  fill: 0xAA,
});




// Find the device COM ports
can1.list()
  .then((ports) => {
    // open the last ports.  When successful, the J1939 instances will
    // emit 'open'
    return Promise.all([
      can1.open(ports[0].path),
    ]);

  })

  .catch((err) => {

    console.error(err);
    process.exit(-1);
  });

can2.list()
  .then((ports) => {
    // open the last ports.  When successful, the J1939 instances will
    // emit 'open'
    return Promise.all([
      can2.open(ports[0].path),
    ]);

  })

  .catch((err) => {

    console.error(err);
    process.exit(-1);
  });


device1.on('open', (address) => {

  console.log('Device 1 ready with address ', address);

  // if both are ready, start
  if (device2.isReady()) {
    console.log("Device 2 appears to be ready.");
    device1.start(device2.address);
    device2.start(device1.address);
  }
});


device2.on('open', (address) => {

  console.log('Device 2 ready with address ', address);
  // if both are ready, start
  if (device1.isReady()) {
    console.log("starting device 1...")
    device1.start(device2.address);
    console.log("starting device 2...");
    device2.start(device1.address);
  }

});
