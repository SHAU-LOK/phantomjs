"use strict";
var p = require("webpage").create();

p.onConsoleMessage = function(msg) { console.log(msg); };

// Calls to "callchromess" within the page 'p' arrive here
p.onCallback = function(msg) {
    console.log("Received by the 'chromess' main context: "+msg);
    return "Hello there, I'm coming to you from the 'chromess' context instead";
};

p.evaluate(function() {
    // Return-value of the "onCallback" handler arrive here
    var callbackResponse = window.callchromess("Hello, I'm coming to you from the 'page' context");
    console.log("Received by the 'page' context: "+callbackResponse);
});

chromess.exit();
