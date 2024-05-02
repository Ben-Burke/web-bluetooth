

// Bluetooth UUIDs and device names
let bluetoothDevicePO60 = 'PO60';
let bleServiceSPo2 = '0000ff12-0000-1000-8000-00805f9b34fb'



//PO60 Device Commands
const nextPacket = new Uint8Array([0x99, 0x01, 0x1A]).buffer;
const deleteRecordCMD = new Uint8Array([0x99, 0x7F, 0x18]).buffer;
const doorbell = new Uint8Array([0x99, 0x00, 0x19]).buffer;
let getStorageCMD = new Uint8Array([0x90, 0x05, 0x15]);
const resetCMD = new Uint8Array([0xFF]).buffer;



// Run time persistent variables
let ResultArray = new Uint8Array();
let PreviousResultArray = new Uint8Array();

let minSpo02 = 0;
let maxSpo02 = 0;
let avgSpo02 = 0;
let minPulseRate = 0;
let maxPulseRate = 0;
let avgPulseRate = 0;

let countOfStaleResults = 0;

const resultsValuesArray = [minSpo02, maxSpo02, avgSpo02, minPulseRate, maxPulseRate, avgPulseRate];

let debug = true;


class PO {

    constructor() {
      this.device = null;
      this.onDisconnected = this.onDisconnected.bind(this);
    }
    
    async request() {
      let options = {
        "filters": [{
          "name": bluetoothDevicePO60,
          "services": [bleServiceSPo2]
        }]
      };
      this.device = await navigator.bluetooth.requestDevice(options);
      if (!this.device) {
        throw "No device selected";
      }
      this.device.addEventListener('gattserverdisconnected', this.onDisconnected);
      console.log('BLE Device selected:', this.device);
    }
    
    
    async connect() {
      if (!this.device) {
        return Promise.reject('Device is not connected.');
      }
      await this.device.gatt.connect();
      console.log('Connected to Device.');
    }
    
    async readCustomService() {
      debugPrint('Reading value from 0xFF02.');
      const service = await this.device.gatt.getPrimaryService("0000ff12-0000-1000-8000-00805f9b34fb");
      const characteristic = await service.getCharacteristic(0xFF02);
      await characteristic.readValue();
    }
  
    async writeCustomService(data) {
      debugPrint('Write value to 0xFF01.');
      const service = await this.device.gatt.getPrimaryService("0000ff12-0000-1000-8000-00805f9b34fb");
      const characteristic = await service.getCharacteristic(0xFF01);
      await characteristic.writeValue(data);
    }
  
    async resetDevice(data) {
      debugPrint('Resetting device.');
      const service = await this.device.gatt.getPrimaryService("0000ff12-0000-1000-8000-00805f9b34fb");
      const characteristic = await service.getCharacteristic(0xFFF1);
      await characteristic.writeValue(data);
    }

    async startCustomServiceNotifications(listener) {
      console.log('Starting notifications for 0xFF02.');
      const service = await this.device.gatt.getPrimaryService("0000ff12-0000-1000-8000-00805f9b34fb");
      const characteristic = await service.getCharacteristic(0xFF02);
      await characteristic.startNotifications();
      characteristic.addEventListener('characteristicvaluechanged', listener);
    }
  
    async stopCustomServiceNotifications(listener) {
      const service = await this.device.gatt.getPrimaryService("0000ff12-0000-1000-8000-00805f9b34fb");
      const characteristic = await service.getCharacteristic(0xFF02); // this was FF01
      await characteristic.stopNotifications();
      characteristic.removeEventListener('characteristicvaluechanged', listener);
    }

    

    listener (event) {
      let value = event.target.value;
      value = value.buffer ? value : new DataView(value);
      ResultArray = new Uint8Array(value.buffer)
      // make a copy of ResultArray into PreviousResultArray including all the values
      console.log('Received ' + ResultArray.length + ' bytes of data.');
      console.log ('Data: ' + ResultArray);
      DecodeNotificationMessageType(ResultArray);
     
    }

   
    disconnect() {
      if (!this.device) {
        return Promise.reject('Device is not connected.');
      }
      return this.device.gatt.disconnect();
    }
  
    onDisconnected() {
      console.log('Device is disconnected.');
    }
  }

  function isWebBluetoothEnabled() {
    if (!navigator.bluetooth) {
      console.log('Web Bluetooth API is not available in this browser!')
      return false
    }

    return true
  }

 

// this function will take the passed parameter and pass to console.log IF the debug variable is true
function debugPrint(message) {
  debug = true;
  if (debug) {
    console.log(message);
  }
}

// This function calculates the checksum for a PO60 packet
// The checksum is calculated by adding all the bytes in the packet (including the header) and then returning the lowest 7 bits of the
// result
function calculateChecksum(packet) {
    let sum = 0;
    console.log('Calculating checksum for packet');
    debugPrint('Calculating checksum for packet: ' + packet.buffer);
    for (let i = 0; i < packet.byteLength; i++) {
        sum += packet[i];
    }
    return sum & 0x7F;
}

function DecodeNotificationMessageType(ResultArray) {
    let messageType = ResultArray[0];
    // case statement to determine the message type
    switch (messageType) {
      case 0xE0:
        // code for message type 0xE0
        console.log('Message Type: Get Storage Information');
        countOfStaleResults = ResultArray[2];
        console.log('Count of Stale Results: ' + countOfStaleResults);
        break;
      case 0xE9:
        // code for message type 0x99
        console.log('Message Type: Results Block Data');
        maxSpo02 = ResultArray[17];
        minSpo02 = ResultArray[18];
        avgSpo02 = ResultArray[19];
        break;
      default:
        // code for unknown message type
        if ( ResultArray.length == 4) {
          console.log('Message Type: Final Bytes');
          maxPulseRate = ResultArray[0];
          minPulseRate = ResultArray[1];
          avgPulseRate = ResultArray[2];
        }
        console.log('Unknown Message Type: ' + messageType.toString(16));
    }

    // let messageLength = ResultArray[1];
    // let messageData = ResultArray.slice(2, messageLength + 2);
    // console.log('Message Type: ' + messageType.toString(16));
    // console.log('Message Length: ' + messageLength);
    // console.log('Message Data: ' + messageData);
  }

  // This function keeps calling "toTry" until promise resolves or has
// retried "max" number of times. First retry has a delay of "delay" seconds.
// "success" is called upon success.
async function exponentialBackoff(max, delay, toTry, success, fail) {
    try {
        const result = await toTry();
        success(result);
    } catch(error) {
        if (max === 0) {
            return fail();
        }
        time('Retrying in ' + delay + 's... (' + max + ' tries left)');
        setTimeout(function() {
            exponentialBackoff(--max, delay * 2, toTry, success, fail);
        }, delay * 1000);
    }
}

function time(text) {
    log('[' + new Date().toJSON().substr(11, 8) + '] ' + text);
}
