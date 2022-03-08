// Example script that opens the CAN connection and
// sends a message.
// The example also sets up a filter to catch a response message
// (obviously you need a compatible remote device on the bus in order to get a
// response )


const Can = require('../..');

let board = new Can({
  canRate: 250000,
  filters: [ { ext: true, id: '10EF0000 10EFFFFF' } ]
});


// Look for compatible CAN adapters
board.list()
  .then(function(ports) {

    // got a list of the ports, try to open the last one which is likely
    // the USB cable
    ports = ports.slice(-1);

    board.on('write', function(msg) {
      console.log('Write: ', msg);
    });

    // Event handler for each incoming message
    board.on('data', function(msg) {
      console.log('Msg: ', msg.id.toString(16), msg.buf);
    });

    // Open the COM port and initialize the USBCAN device...
    console.log('opening ', ports[0].path);
    return board.open(ports[0].path);

  })
  .then(function() {

    // send a command
    board.write({ id: 0x10EF8001, ext: true, buf: Buffer.from([0x45, 0x00, 0x00, 0x04, 0x00, 0x00]) });


  })
  .catch(function(err) {
    // Something went wrong...
    console.error(err);
    board.close();
    process.exit(-1);
  });
