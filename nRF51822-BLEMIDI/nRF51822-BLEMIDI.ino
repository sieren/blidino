/*
 *  Copyright (c) 2014 Matthias Frick
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include <pins_arduino.h>
#include <Arduino.h>
#include <BLE_API.h>
#include <nordic_common.h>
//#include <SPI.h>
#include <Usb.h>
#include <usbh_midi.h>
#include "BLEParser.h"


#define TXRX_BUF_LEN                    20
#define RX_BUF_LEN                      20 /** Overwriting RX Buf Len since we handle fragmentation **/

BLE  ble;
Ticker     sendTicker;
Ticker     usbTicker;
mfk::midi::BLEMIDIParser<256, USBH_MIDI> parser;

static uint8_t rx_buf[RX_BUF_LEN];
static int rx_buf_num, rx_state = 0;
static uint8_t rx_temp_buf[20];
uint8_t outBufMidi[128];
int outBufMidiPtr;

// MIDI BLE Service UUIDs
static const uint8_t uart_base_uuid[] = {0x03, 0xB8, 0x0E, 0x5A, 0xED, 0xE8, 0x4B, 0x33, 0xA7, 0x51, 0x6C, 0xE3, 0x4E, 0xC4, 0xC7, 0};
static const uint8_t uart_tx_uuid[]   = {0x77, 0x72, 0xE5, 0xDB, 0x38, 0x68, 0x41, 0x12, 0xA1, 0xA9, 0xF2, 0x66, 0x9D, 0x10, 0x6B, 0xF3};
static const uint8_t uart_rx_uuid[]   = {0x77, 0x72, 0xE5, 0xDB, 0x38, 0x68, 0x41, 0x12, 0xA1, 0xA9, 0xF2, 0x66, 0x9D, 0x10, 0x6B, 0xF3};
static const uint8_t uart_base_uuid_rev[] = {0, 0xC7, 0xC4, 0x4E, 0xE3, 0x6C, 0x51, 0xA7, 0x33, 0x4B, 0xE8, 0xED, 0x5A, 0x0E, 0xB8, 0x03};



uint8_t txPayload[TXRX_BUF_LEN] = {0,};
uint8_t rxPayload[TXRX_BUF_LEN] = {0,};

GattCharacteristic  txCharacteristic
  (uart_tx_uuid, txPayload, 1, 20,
  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE |
  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);

GattCharacteristic *uartChars[] = {&txCharacteristic};
GattService         uartService(uart_base_uuid, uartChars,
                                sizeof(uartChars) / sizeof(GattCharacteristic *));

/*******************************************************************************
* INITIALIZE USB MIDI Variables
*******************************************************************************/
USB  Usb;
USBH_MIDI  Midi(&Usb);
uint8_t usbstate;
uint8_t laststate;
uint8_t rcode;
USB_DEVICE_DESCRIPTOR buf;

void MIDI_poll();

boolean bFirst;
boolean isConnected;
boolean isSysex;
uint16_t pid, vid;
int bufferInUse=0;

/*******************************************************************************
* INITIALIZE Internal BLE Buffer
*******************************************************************************/
#define MAX_TX_BUFF 64
static uint8_t midiOut_buff[MAX_TX_BUFF];
static uint8_t midiOut_buff_len = 0;



/*******************************************************************************
* Send MIDI data to BLE Stack
*******************************************************************************/
void sendData(void)
{
  if(rx_buf_num > 0 && isConnected)
  {
    int bufInc = 0;
    if (rx_buf_num < 17)
    {
      bufInc = rx_buf_num % 17; }
    else
    {
      bufInc = 17;
    }
    ble.updateCharacteristicValue(txCharacteristic.getValueAttribute().getHandle(), rx_buf, bufInc);
    memmove(rx_buf, rx_buf+bufInc, rx_buf_num-bufInc); // probably not best practice? needs to be fixed
    rx_buf_num -= bufInc;
    rx_state = 0;
  }
}


/*******************************************************************************
* Initialize USB Communication
*******************************************************************************/
void m_status_check_handle(void)
{

  Usb.Task();
  usbstate = Usb.getUsbTaskState();

  if (usbstate != laststate)
  {
    laststate = usbstate;

    switch (usbstate)
    {
      case ( USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE):
        E_Notify(PSTR("\r\nWaiting for device..."), 0x80);
        break;
      case ( USB_ATTACHED_SUBSTATE_RESET_DEVICE):
        E_Notify(PSTR("\r\nDevice connected. Resetting..."), 0x80);
        break;
      case ( USB_ATTACHED_SUBSTATE_WAIT_SOF):
        E_Notify(PSTR("\r\nReset complete. Waiting for the first SOF..."), 0x80);
        break;
      case ( USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE):
        E_Notify(PSTR("\r\nSOF generation started. Enumerating device..."), 0x80);
        break;
      case ( USB_STATE_ADDRESSING):
        E_Notify(PSTR("\r\nSetting device address..."), 0x80);
        break;
      case ( USB_STATE_RUNNING):
        E_Notify(PSTR("\r\nGetting device descriptor"), 0x80);
        rcode = Usb.getDevDescr(1, 0, sizeof (USB_DEVICE_DESCRIPTOR), (uint8_t*) & buf);

        if (rcode)
        {
          E_Notify(PSTR("\r\nError reading device descriptor. Error code "), 0x80);
          // print_hex(rcode, 8);
        }
        else
        {
          E_Notify(PSTR("\r\nDescriptor Length:\t"), 0x80);
          print_hex(buf.bLength, 8);
          E_Notify(PSTR("\r\nDescriptor type:\t"), 0x80);
          print_hex(buf.bDescriptorType, 8);
          E_Notify(PSTR("\r\nUSB version:\t\t"), 0x80);
          print_hex(buf.bcdUSB, 16);
          E_Notify(PSTR("\r\nDevice class:\t\t"), 0x80);
          print_hex(buf.bDeviceClass, 8);
          E_Notify(PSTR("\r\nDevice Subclass:\t"), 0x80);
          print_hex(buf.bDeviceSubClass, 8);
          E_Notify(PSTR("\r\nDevice Protocol:\t"), 0x80);
          print_hex(buf.bDeviceProtocol, 8);
          E_Notify(PSTR("\r\nMax.packet size:\t"), 0x80);
          print_hex(buf.bMaxPacketSize0, 8);
          E_Notify(PSTR("\r\nVendor  ID:\t\t"), 0x80);
          print_hex(buf.idVendor, 16);
          E_Notify(PSTR("\r\nProduct ID:\t\t"), 0x80);
          print_hex(buf.idProduct, 16);
          E_Notify(PSTR("\r\nRevision ID:\t\t"), 0x80);
          print_hex(buf.bcdDevice, 16);
          E_Notify(PSTR("\r\nMfg.string index:\t"), 0x80);
          print_hex(buf.iManufacturer, 8);
          E_Notify(PSTR("\r\nProd.string index:\t"), 0x80);
          print_hex(buf.iProduct, 8);
          E_Notify(PSTR("\r\nSerial number index:\t"), 0x80);
          print_hex(buf.iSerialNumber, 8);
          E_Notify(PSTR("\r\nNumber of conf.:\t"), 0x80);
          print_hex(buf.bNumConfigurations, 8);

        }
        break;
      case ( USB_STATE_ERROR):
        E_Notify(PSTR("\r\nUSB state machine reached error state"), 0x80);
        break;

      default:
        break;
    }//switch( usbstate...
  }

  if (Usb.getUsbTaskState() == USB_STATE_RUNNING)
  {
    MIDI_poll();
  }
}


/*******************************************************************************
* Callback for Bluetooth disconnection
* Restart advertising and stop the timers.
*******************************************************************************/
void disconnectionCallback(Gap::Handle_t handle,
  Gap::DisconnectionReason_t reason)
{
    Serial.println("Disconnected ");
    Serial.println("Restart advertising ");
    isConnected = false;
    ble.startAdvertising();
}


/*******************************************************************************
* Callback for incoming connections
* Start the timers.
*******************************************************************************/
void connectionCallback(const Gap::ConnectionCallbackParams_t* params)
{
  isConnected = true;
  Serial.println("Connected");
  sendTicker.attach(sendData, 0.01);
}


/*******************************************************************************
* Callback for INCOMING MIDI BLE Data
*******************************************************************************/
void onDataWritten(const GattWriteCallbackParams *Handler)
{
  uint8_t buf[TXRX_BUF_LEN];
  uint16_t bytesRead;
  if (Handler->handle == txCharacteristic.getValueAttribute().getHandle())
  {
    ble.readCharacteristicValue(txCharacteristic.getValueAttribute().getHandle(),
      buf, &bytesRead);
    parseIncoming(buf, bytesRead);
  }
}


/*******************************************************************************
* Setup
* Initialize USB Port, set Characteristics and Callbacks for BLE Stack.
*******************************************************************************/
void setup(void)
{
  bFirst = true;
  vid = pid = 0;
  isConnected = false;

  //Workaround for non UHS2.0 Shield
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  Usb.Init();
  delay(500);
  Serial.begin(9600);

  parser.setUSBMidiHandle(&Midi);

  ble.init();
  ble.onDisconnection(disconnectionCallback);
  ble.onConnection(connectionCallback);
  ble.onDataWritten(onDataWritten);

  /* setup advertising */
  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
  ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
    (const uint8_t *)"BLIDIno", sizeof("BLIDIno") - 1);
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
    (const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid_rev));

  ble.accumulateScanResponse(GapAdvertisingData::SHORTENED_LOCAL_NAME,
    (const uint8_t *)"BLIDIno", sizeof("BLIDIno") - 1);
  ble.accumulateScanResponse(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
    (const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid_rev));
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);

  /* 100ms; in multiples of 0.625ms. */
  ble.setAdvertisingInterval(160);

  // set adv_timeout, in seconds
  ble.setAdvertisingTimeout(0);
  ble.addService(uartService);

  //Set Device Name
  ble.setDeviceName((const uint8_t *)"Blidino");

  ble.startAdvertising();
  usbTicker.attach(m_status_check_handle, 0.01);
}


/*******************************************************************************
* BLE Loop
*******************************************************************************/
void loop(void)
{
  ble.waitForEvent();
}


/*******************************************************************************
* Poll MIDI DATA from USB Port
*******************************************************************************/
void MIDI_poll()
{
  uint8_t bufMidi[64];
  uint16_t  rcvd;
  uint8_t size;
  byte outBuf[ 3 ];
  char buf[20];

  if (Midi.vid != vid || Midi.pid != pid)
  {
    vid = Midi.vid;
    pid = Midi.pid;
  }

  do
  {
    if ((size = Midi.RecvData(outBuf)) > 0)
    {
      // Send data to parser
      parseMIDItoAppleBle(size, outBuf);
    }
  } while (size > 0);
}


/*******************************************************************************
* Convert MIDI Data to MIDI-BLE Packets
*******************************************************************************/
void parseMIDItoAppleBle(int size, byte outBuf[3])
{
  char time[2];
  char buf[20];
  unsigned long timer = 0;
  int lastPos;
  timer = millis();
  uint16_t blueMidiTime = 0;
  blueMidiTime = 32768 + (timer % 16383); // TODO Proper Bitshifting

  // TODO
  // This section is a total mess and is in urgent need of
  // a rewrite.

  if(rx_buf_num <= 100) // arbitrary high number
  {
    if (rx_buf_num % 17 == 0) // End of packet, start a new one
    {
      rx_buf[rx_buf_num] = blueMidiTime >> 8;
      rx_buf_num++;
      rx_buf[rx_buf_num] = 0x80;
      rx_buf_num++;
    }
    else
    {
      rx_buf[rx_buf_num] = 0x80;
      rx_buf_num++;
    }
    for (int i = 0; i < size; i++)
    {
      rx_buf[rx_buf_num] = outBuf[i];
      rx_buf_num++;
    }
  }
}


/*******************************************************************************
* Convert MIDI BLE to MIDI USB
*******************************************************************************/
void parseIncoming(uint8_t *buffer, uint16_t bytesRead)
{
  for (int i = 1; i < bytesRead; i++)
  {
    parser.parseMidiEvent(buffer[0], buffer[i]);
  }
}


/*******************************************************************************
* Convert USB Host debug info to Hex
*******************************************************************************/
void print_hex(int v, int num_places)
{
  int mask = 0, n, num_nibbles, digit;
  for (n = 1; n <= num_places; n++)
  {
    mask = (mask << 1) | 0x0001;
  }
  v = v & mask; // truncate v to specified number of places

  num_nibbles = num_places / 4;
  if ((num_places % 4) != 0)
  {
    ++num_nibbles;
  }
  do
  {
    digit = ((v >> (num_nibbles - 1) * 4)) & 0x0f;
    Serial.println(digit, HEX);
  }
  while (--num_nibbles);
}

