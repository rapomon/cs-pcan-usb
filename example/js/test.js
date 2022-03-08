const Can = require('../..');

let can = new Can({
  canRate: 250000,
});

can.list()
  .then((ports) => {
    console.log(ports);
    return can.open(ports[0].path)
      .catch((err) => {
        console.error(err);
        process.exit(1);
      });
  })
  .then(() => {
    can.write({ id: 0x10EF8001,
                ext: true,
                buf: Buffer.from([0x45, 0x00, 0x00, 0x04, 0x00, 0x00]) });
  })
//  .then(() => {
//    can.close();
//  })
  .catch((err) => {
    console.error(err);
    can.close();
    process.exit(1);
  });


can.on('data', function(msg) {
  console.log(JSON.stringify(msg, (key, value) => {
    if ((typeof value === 'number') && (key == "id")) {
      return value.toString(16).toUpperCase().padStart(8, '0');
    } else if (typeof value === 'number') {
      return value.toString(16).toUpperCase().padStart(2, '0');
    } else {
      return value;
    }
  }));
});

