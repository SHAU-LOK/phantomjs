// List all the files in a Tree of Directories

"use strict";
var system = require('system');

if (system.args.length !== 2) {
    console.log("Usage: chromessjs scandir.js DIRECTORY_TO_SCAN");
    chromess.exit(1);
}

var scanDirectory = function (path) {
    var fs = require('fs');
    if (fs.exists(path) && fs.isFile(path)) {
        console.log(path);
    } else if (fs.isDirectory(path)) {
        fs.list(path).forEach(function (e) {
            if ( e !== "." && e !== ".." ) {    //< Avoid loops
                scanDirectory(path + '/' + e);
            }
        });
    }
};
scanDirectory(system.args[1]);
chromess.exit();
