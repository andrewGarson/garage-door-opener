var mqtt = require('mqtt');
var client = mqtt.connect('mqtt://192.168.1.2');

client.on('connect', function() {
  console.log('connected');
  client.subscribe('presence');
  client.publish('presence', 'hello');
});

client.on('message', function(topic, message) {
  console.log(topic, message.toString());
});

setInterval(function() {
  client.publish('commands', 'do a barrel roll');
}, 1000);
