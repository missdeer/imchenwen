"use strict";
var page = require('webpage').create(),
    system = require('system'),
    address;

page.settings.userAgent = 'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2062.120 Safari/537.36';

if (system.args.length === 1) {
    console.log('Usage: api.47ks.com.js <some URL>');
    phantom.exit(1);
} else {
    address = system.args[1];

    page.onResourceRequested = function (req) {
		if (req.url == "https://api.47ks.com/config/webmain.php") {
			console.log(req.postData);
			phantom.exit();
		}
		
		if (req.url == "https://aikan-tv.com/qq396774785.php") {
			console.log(req.postData);
			phantom.exit();
		}	
    };

    page.onResourceReceived = function (res) {
        for (var i in res.headers) {
            if (res.headers[i].name == "Set-Cookie") {
                console.log("Set-Cookie: " + res.headers[i].value);
            }
        }
    };

    page.open(address, function (status) {
        if (status !== 'success') {
            console.log('FAIL to load the address');
        }
        phantom.exit();
    });
}
