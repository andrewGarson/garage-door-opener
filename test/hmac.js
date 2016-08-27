var CryptoJS = require('crypto-js');
var net = require('net');

var key = '3f422756a527be913c5655390f231190b881650e765e48b79992e0bc32ffbe12194a8b6ff078dfcbf7de3633cfa050b4a8b48dc0976201389194ec11c5654f';
var message = 'HI';

var expectedHmac = CryptoJS.enc.Hex.stringify(
  CryptoJS.HmacSHA1( 
    CryptoJS.enc.Utf8.parse(message),
    CryptoJS.enc.Hex.parse(key)));



var socket = new net.Socket();
socket.setTimeout(1000);

socket.on('data', function(data) {
  var message = data.toString('utf8');
  if(message === 'HELLO\n') {
    console.log('got hello');
    socket.write(message + '\n');
  } else {
    console.log('got: ', message);
    if(message === expectedHmac + '\n') {
      console.log('Thats a match');
    } else {
      console.log('Not a match');
    }
    socket.end();
  }
});

socket.on('connect', function() {
});

socket.on('timeout', function() {
  console.log('TIMEOUT');
  socket.end();
});
socket.on('error', function() {
  console.log('ERROR');
  socket.end();
});

socket.connect({
  port: 23,
  host: '192.168.1.11',
});


