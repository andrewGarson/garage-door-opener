const readline = require('readline');
const mqtt = require('mqtt');
const client = mqtt.connect('mqtt://192.168.1.2');

client.on('connect', function() {
  console.log('connected');
  client.subscribe('presence');
  client.publish('presence', 'controller');
});

client.on('message', function(topic, message) {
  console.log(topic, message.toString());
});

var rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

function handleCommand(command) {
  if(command === 'quit') {
    rl.close();
    client.end();
    console.log('Bye');
    return;
  } else if(command) {
    console.log('Sending Command: ', command);
    client.publish('commands', command + '\0');
  }
  rl.question('Enter Command: ', handleCommand);
}

handleCommand(null);

