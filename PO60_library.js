



const nextPacket = new Uint8Array([0x99, 0x01, 0x1A]).buffer;
const deleteRecordCMD = new Uint8Array([0x99, 0x7F, 0x18]).buffer;

const doorbell = new Uint8Array([0x99, 0x00, 0x19]).buffer;

let getStorageCMD = new Uint8Array([0x90, 0x05, 0x15]);



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



let endOfData = false;


class PO {

    constructor() {
      this.device = null;
      this.onDisconnected = this.onDisconnected.bind(this);
    }
    
    async request() {
      let options = {
        "filters": [{
          "name": "PO60",
          "services": ["0000ff12-0000-1000-8000-00805f9b34fb"]
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
      case 0x99:
        // code for message type 0x99
        console.log('Message Type: Get Results');
        break;
      default:
        // code for unknown message type
        console.log('Unknown Message Type: ' + messageType.toString(16));
    }

    let messageLength = ResultArray[1];
    let messageData = ResultArray.slice(2, messageLength + 2);
    console.log('Message Type: ' + messageType.toString(16));
    console.log('Message Length: ' + messageLength);
    console.log('Message Data: ' + messageData);
  }
