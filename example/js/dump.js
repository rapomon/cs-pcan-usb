// Example that listens to all PGNs and dump them
// to the console.
// CTRL-C stops the example

const Can = require('../..');

let board = new Can({
  canRate: 250000,
});

// Event handler
board.on('open', function() {
  console.info('Port Open');
});

// Event handler
board.on('error', function(err) {
  console.error('Port Error:', err);
});

// Event handler - port closed or disconnected
board.on('close', function(err) {
  if (err && err.disconnected) {
    console.log('Port Disconnected');
  } else {
    console.log('Port Closed by application');
  }
});

// Look for compatible CAN adapters
board.list()
  .then(function(ports) {

    console.log(ports);

    if (ports.length > 0) {
      // got a list of the ports, try to open the last one which is likely
      // the USB cable
      ports = ports.slice(-1);

      // Event handler for each incoming message
      board.on('data', function(msg) {

        console.log('Msg: ', msg.id.toString(16), msg.buf);

      });

      console.log('Opening ', ports[0].path);

      // Open the COM port and initialize the USBCAN device...
      return board.open(ports[0].path);
    } else {
      console.error('No CAN-USB-COM Devices found');
    }

  })
  .catch(function(err) {
    // Something went wrong...
    console.error(err);
    board.close();
    process.exit(-1);
  });
