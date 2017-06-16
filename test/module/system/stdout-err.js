//! no-harness
//! expect-stdout: Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich
//! expect-stderr: いろはにほへとちりぬるをわかよたれそつねならむうゐのおくやまけふこえてあさきゆめみしゑひもせす

//^ stdout: pangram in German
//^ stderr: pan+isogram in hiragana (the Iroha)

chromess.onError = function () { chromess.exit(1); };

var sys = require("system");

sys.stdout.write("Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich\n");
sys.stderr.write("いろはにほへとちりぬるをわかよたれそつねならむうゐのおくやまけふこえてあさきゆめみしゑひもせす");
chromess.exit(0);
