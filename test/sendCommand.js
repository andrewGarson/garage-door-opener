var CryptoJS = require('crypto-js');
var net = require('net');
var Hex = CryptoJS.enc.Hex;

var key = Hex.parse('3f422756a527be913c5655390f231190b881650e765e48b79992e0bc32ffbe12194a8b6ff078dfcbf7de3633cfa050b4a8b48dc0976201389194ec11c5654f');

var command = '00';
var nonce = '1b98f060299b7413f79ef069cf4a67d8b788c83a'; //'c46821e7a8582d2c3e9133ab92f172df9d45f0fb');                        
var unitSeparator = 0x1F;
var recordSeparator = 0x1E;

var hmac = Hex.stringify(CryptoJS.HmacSHA1( 
      Hex.parse(command).concat(Hex.parse(nonce)), 
      key));

var bytes = ((command.length + nonce.length + hmac.length) / 2) + 3;
var messageBuffer = new Buffer(bytes);

var bufferIndex = 0;

[command, nonce, hmac].forEach(function(component) {
  // write each component to the buffer followed by a unit separator
  // maintain an index into the buffer the whole time
  for(var i = 0; i < component.length; i+=2) {
    messageBuffer[bufferIndex] = Number.parseInt(component.substr(i, 2), 16);
    bufferIndex++;
  }
  messageBuffer[bufferIndex] = unitSeparator;
  bufferIndex++;
});
// the last unitSeparator should really be a recordSeparator because that's 
// the command terminator
messageBuffer[bufferIndex - 1] = recordSeparator;

var socket = new net.Socket();
socket.setTimeout(1000);

socket.on('data', function(data) {
  console.log('Got Data: ', data.toString('ascii'));
  if(data.toString('ascii') === 'HELLO\n') {
    socket.write(messageBuffer);
  } else {
    console.log('got: ', data.toString('ascii'));
    socket.end();
  }
});

socket.on('connect', function() { });

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
