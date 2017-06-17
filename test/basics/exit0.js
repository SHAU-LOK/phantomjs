//! no-harness
//! expect-exit: 0
//! expect-stdout: "we are alive"
//! expect-stdout-fails

var sys = require('system');
sys.stdout.write("we are alive\n");
chromess.exit();
sys.stdout.write("ERROR control passed beyond chromess.exit");
