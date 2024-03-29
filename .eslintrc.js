module.exports = {
  "env": {
    "browser": true,
    "es6": true,
    "node": true,
    "mocha": true,

  },
  "extends": "eslint:recommended",
  "globals": {
    "Atomics": "readonly",
    "SharedArrayBuffer": "readonly",

  },
  "plugins": [
    "mocha",
  ],
  "parserOptions": {
    "ecmaVersion": 11,
    "sourceType": "module"
  },
  "rules": {},
  "ignorePatterns": ["assets/dependencies/**/*"]
};
