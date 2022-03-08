/**
 * Tests receive filtering
 *
 */
const CsPcanUsb = require('..');
const chai = require('chai');
const expect = chai.expect;
const should = chai.should();
chai.use(require('chai-events'));

const CAN_OPTIONS = {
  canRate: 250000,
  loopback: true,
};


const MSG1 = {
  id: 0x1,
  ext: false,
  buf: Buffer.from([])
};

const MSG2 = {
  id: 0x1,
  ext: true,
  buf: Buffer.from([])
};

const MSG3 = {
  id: 0x7FF,
  ext: false,
  buf: Buffer.from([1, 2, 3])
};

const MSG4 = {
  id: 0x1FFFFFFF,
  ext: true,
  buf: Buffer.from([3, 2, 1])
};

const MSG5 = {
  id: 0x10EF8081,
  ext: true,
  buf: Buffer.from([5, 4, 3])
};

describe('No Filters', () => {

  let can = null;

  // before all tests in this block, get an open port
  before(async () => {

    // set and keep for later tests
    can = new CsPcanUsb(Object.assign(CAN_OPTIONS, { filters: [] }));

    let result = await can.list();

    let p = can.should.emit('open');

    can.open(result[0].path);

    return p;
  })

  it('should receive MSG1', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG1);

      done();

    });

    can.write(MSG1);
  });

  it('should receive MSG2', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG2);

      done();

    });

    can.write(MSG2);
  });

  it('should receive MSG3', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG3);

      done();

    });

    can.write(MSG3);
  });

  it('should receive MSG4', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG4);

      done();

    });

    can.write(MSG4);
  });

  // after all tests in this block
  after(async () => {

    await can.close();

  })
});

describe('Only Standard IDs', () => {

  let can = null;

  // before all tests in this block, get an open port
  before(async () => {

    // set and keep for later tests
    can = new CsPcanUsb(Object.assign(CAN_OPTIONS, {
      filters: [{
        ext: false,

        id: '0 7FF'
      }]
    }));

    let result = await can.list();

    let p = can.should.emit('open');

    can.open(result[0].path);

    return p;
  })

  it('should receive MSG1', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG1);

      done();

    });

    can.write(MSG1);
  });

  it('should not receive MSG2', async () => {

    let p = can.should.not.emit('data');

    can.write(MSG2);

    return p;
  });

  it('should receive MSG3', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG3);

      done();

    });

    can.write(MSG3);
  });

  it('should not receive MSG4', async () => {

    let p = can.should.not.emit('data');

    can.write(MSG4);

    return p;
  });

  // after all tests in this block
  after(async () => {

    await can.close();

  })
});


describe('Only Extended IDs', () => {

  let can = null;

  // before all tests in this block, get an open port
  before(async () => {

    // set and keep for later tests
    can = new CsPcanUsb(Object.assign(CAN_OPTIONS, {
      filters: [{
        ext: true,

        id: '0 1FFFFFFF'
      }]
    }));

    let result = await can.list();

    let p = can.should.emit('open');

    can.open(result[0].path);

    return p;
  })

  it('should not receive MSG1', async () => {

    let p = can.should.not.emit('data');

    can.write(MSG1);

    return p;
  });

  it('should receive MSG2', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG2);

      done();

    });

    can.write(MSG2);
  });


  it('should not receive MSG3', async () => {

    let p = can.should.not.emit('data');

    can.write(MSG3);

    return p;
  });

  it('should receive MSG4', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG4);

      done();

    });

    can.write(MSG4);
  });


  // after all tests in this block
  after(async () => {

    await can.close();

  })
});

describe('Only a single ID', () => {

  let can = null;

  // before all tests in this block, get an open port
  before(async () => {

    // set and keep for later tests
    can = new CsPcanUsb(Object.assign(CAN_OPTIONS, {
      filters: [{
        ext: true,

        id: MSG5.id.toString(16)
      }]
    }));

    let result = await can.list();

    let p = can.should.emit('open');

    can.open(result[0].path);

    return p;
  })

  it('should not receive MSG1', async () => {

    let p = can.should.not.emit('data');

    can.write(MSG1);

    return p;
  });

  it('should not receive MSG2', async () => {

    let p = can.should.not.emit('data');

    can.write(MSG2);

    return p;
  });


  it('should not receive MSG3', async () => {

    let p = can.should.not.emit('data');

    can.write(MSG3);

    return p;
  });

  it('should not receive MSG4', async () => {

    let p = can.should.not.emit('data');

    can.write(MSG4);

    return p;
  });

  it('should receive MSG5', (done) => {

    can.once('data', (msg) => {

      expect(msg).to.deep.eq(MSG5);

      done();

    });

    can.write(MSG5);
  });


  // after all tests in this block
  after(async () => {

    await can.close();

  })
});
