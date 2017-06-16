// Use 'page.injectJs()' to load the script itself in the Page context

"use strict";
if ( typeof(chromess) !== "undefined" ) {
    var page = require('webpage').create();

    // Route "console.log()" calls from within the Page context to the main chromess context (i.e. current "this")
    page.onConsoleMessage = function(msg) {
        console.log(msg);
    };

    page.onAlert = function(msg) {
        console.log(msg);
    };

    console.log("* Script running in the chromess context.");
    console.log("* Script will 'inject' itself in a page...");
    page.open("about:blank", function(status) {
        if ( status === "success" ) {
            console.log(page.injectJs("injectme.js") ? "... done injecting itself!" : "... fail! Check the $PWD?!");
        }
        chromess.exit();
    });
} else {
    alert("* Script running in the Page context.");
}
