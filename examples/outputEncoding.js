"use strict";
function helloWorld() {
	console.log(chromess.outputEncoding + ": こんにちは、世界！");
}

console.log("Using default encoding...");
helloWorld();

console.log("\nUsing other encodings...");

var encodings = ["euc-jp", "sjis", "utf8", "System"];
for (var i = 0; i < encodings.length; i++) {
    chromess.outputEncoding = encodings[i];
    helloWorld();
}

chromess.exit()
