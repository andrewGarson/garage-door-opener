const dgram = require('dgram');

const host = '0.0.0.0';
const port = 41234;
const multicastGroup = '239.255.1.1';

const server = dgram.createSocket('udp4');

server.on('error', (err) => {
    console.log(`server error:\n${err.stack}`);
      server.close();
});

server.on('message', (msg, rinfo) => {
    console.log(`server got: ${msg} from ${rinfo.address}:${rinfo.port}`);
});

server.on('listening', () => {
    var address = server.address();
    console.log(`server listening ${address.address}:${address.port}`);

    server.setBroadcast(true);
    server.setMulticastTTL(128);
    server.addMembership(multicastGroup);
});

server.bind(port, host);
