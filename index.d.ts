interface Filter {
  ext?: boolean;
  id?: string;
  fromID?: number;
  toID?: number;
  code?: number;
  mask?: number;
}

interface Options {
  canRate?: number;
  loopback?: boolean;
  filters?: Array<Filter>
}

declare class Can {
  constructor(options: Options);
  addListener: Function;
  removeListener: Function;
  removeAllListeners: Function;
  setOptions: Function;
  list: Function;
  open: Function;
  close: Function;
  write: Function;
  status: Function;
  isOpen: Function;
  isConnected: Function;
  emit: Function;
}

export = Can;
