var SerialPort = require("serialport").SerialPort;
var serialPortExists = false;
var serialPort;
var update;
var settings;

exports.init = function(newSettings, newUpdate) {
	settings = newSettings;
	update = newUpdate;

	serialPortExists = true;

	serialPort = new SerialPort(settings.serial.port, {
		baudrate: settings.serial.baudrate
	}, false);

	serialPort.on('error', function(err){
		console.log("error: " + err);
		serialPortExists = false;
	});

	setTimeout(function() {
		if (serialPortExists)
			serialPort.open(function () {
				if (settings.debug) console.log(serialPort.comName + ' opened');
			});
	}, 1);
};

if (serialPortExists)
	serialPort.on('data', function(data) {
		if (settings.debug) console.log('data received: ' + data);
		dataArray[0] = parseFloat(data);
		update(dataArray);
	});

exports.write = function (address, state) {
	if (serialPortExists)
		serialPort.write(address + ":" + (state ? 1 : 0) + "\n");
	if (settings.debug) console.log("serial write: " + address + ":" + state);
};

var lastPulse = false;
exports.pulse = function(address, wavelength){
	var interval = setInterval(function(){
		exports.write(address, !lastPulse);
		lastPulse = !lastPulse;
	}, wavelength);
	return interval;
};

//test code via arg -test
if (process.argv.slice(2)[0] == '-test') {
	exports.init({serial: {port: process.argv.slice(2)[1], baudrate: 9600}});
	var last = 0;
	setInterval(function() {
		if (serialPortExists) {
			if (last)
				serialPort.write("0:0\n");
			else
				serialPort.write("0:1\n");
			last = !last;
		} else
			console.log("serial port not open!!!");
	}, 500);
}