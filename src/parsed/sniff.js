"use strict";
var page = require('webpage').create(),
    system = require('system'),
    address;

page.settings.userAgent = 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0';

if (system.args.length === 1) {
    console.log('Usage: sniff.js <URL>');
    phantom.exit(1);
} else {
    address = system.args[1];

    page.onResourceRequested = function (req) {
		if (req.url == "https://api.47ks.com/config/webmain.php") {
			console.log(req.postData);
		}
		
		if (req.url == "https://aikan-tv.com/qq396774785.php") {
			console.log(req.postData);
		}	
    };

    page.onResourceReceived = function (res) {
        console.log("received: " + res.url)
        if (res.url == "https://aikan-tv.com/qq396774785.php") {
			console.log(JSON.stringify(res, undefined, 4));
		}
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
