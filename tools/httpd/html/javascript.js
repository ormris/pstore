//*    _                                _       _    *
//*   (_) __ ___   ____ _ ___  ___ _ __(_)_ __ | |_  *
//*   | |/ _` \ \ / / _` / __|/ __| '__| | '_ \| __| *
//*   | | (_| |\ V / (_| \__ \ (__| |  | | |_) | |_  *
//*  _/ |\__,_| \_/ \__,_|___/\___|_|  |_| .__/ \__| *
//* |__/                                 |_|         *
//===- tools/httpd/html/javascript.js -------------------------------------===//
// Copyright (c) 2017-2020 by Sony Interactive Entertainment, Inc.
// All rights reserved.
//
// Developed by:
//   Toolchain Team
//   SN Systems, Ltd.
//   www.snsystems.com
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal with the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimers.
//
// - Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimers in the
//   documentation and/or other materials provided with the distribution.
//
// - Neither the names of SN Systems Ltd., Sony Interactive Entertainment,
//   Inc. nor the names of its contributors may be used to endorse or
//   promote products derived from this Software without specific prior
//   written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
//===----------------------------------------------------------------------===//
(function () {
    'use strict';

    var uptime = new WebSocket ("ws://" + window.location.host + "/uptime");
    uptime.onerror = function (error) {
        console.error(error);
    };
    uptime.onclose = function () {
        console.log("uptime websocket closed.");
    };
    uptime.onopen = function () {
        console.log("uptime websocket open.");
    };
    uptime.onmessage = function (msg) {
        var obj = JSON.parse(msg.data);
        var el = document.getElementById("message");
        if (el !== null) {
            el.textContent = obj.uptime !== undefined ? obj.uptime : 'Unknown';
        }
    };

    window.onload = () => {
        var request = new XMLHttpRequest ();
        request.open('GET', 'http://' + window.location.host + '/cmd/version');
        request.responseType = 'json';
        request.onload = () => {
            var el = document.getElementById('version');
            if (el !== null) {
                el.textContent = request.response.version;
            }
        };
        request.send();
    };

} ());
