const noble = require('noble');

const deviceName = 'FT95';
const ThermometerService = '00001809-0000-1000-8000-00805f9b34fb';
const TemperatureCharacteristic = '2a1c';

let bluetoothDeviceDetected;
let gattCharacteristic;

noble.on('stateChange', state => {
  if (state === 'poweredOn') {
    noble.startScanning([ThermometerService], false);
  } else {
    noble.stopScanning();
  }
});

noble.on('discover', peripheral => {
  if (peripheral.advertisement.localName === deviceName) {
    noble.stopScanning();
    bluetoothDeviceDetected = peripheral;
    connect();
  }
});

function connect() {
  bluetoothDeviceDetected.connect(error => {
    if (error) {
      console.error('Error connecting to device:', error);
      return;
    }

    bluetoothDeviceDetected.discoverServices([ThermometerService], (error, services) => {
      const thermometerService = services[0];
      thermometerService.discoverCharacteristics([TemperatureCharacteristic], (error, characteristics) => {
        gattCharacteristic = characteristics[0];
        gattCharacteristic.on('data', (data, isNotification) => {
          console.log('Temperature:', calculateValue(data));
        });
        gattCharacteristic.subscribe(error => {
          if (error) console.error('Error subscribing to characteristic:', error);
        });
      });
    });
  });
}

function calculateValue(bytes) {
  if (bytes.length !== 4) {
    throw new Error('Input must be an array of 4 bytes');
  }

  let exponent = 256 - bytes[0];
  let mantissa = (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
  let value = mantissa * Math.pow(2, -exponent);

  return value;
}