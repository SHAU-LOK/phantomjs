"use strict";
var t = 10,
    interval = setInterval(function(){
        if ( t > 0 ) {
            console.log(t--);
        } else {
            console.log("BLAST OFF!");
            chromess.exit();
        }
    }, 1000);
