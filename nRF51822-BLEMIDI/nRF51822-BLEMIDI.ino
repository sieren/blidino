
/*
 *   Copyright (c) 2014 Matthias Frick, All rights reserved.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <BLE_API.h>
#include <pins_arduino.h>
#include <SPI.h>
#include <Usb.h>
#include <usbh_midi.h>
#define __CORE_CM0_H_GENERIC
#define BLE_UUID_TXRX_SERVICE            0x0000 /**< The UUID of the Nordic UART Service. */
#define BLE_UUID_TX_CHARACTERISTIC       0x0001 /**< The UUID of the TX Characteristic. */
#define BLE_UUID_RX_CHARACTERISTIC      0x0002 /**< The UUID of the RX Characteristic. */

#define TXRX_BUF_LEN                     100
#define UART_RX_TIME                     APP_TIMER_TICKS(10, 0)


#define STATUS_CHECK_TIME                APP_TIMER_TICKS(20, 0)


// Activate to debug
// this increases delay significantly due to increased I/Os
#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(x)     Serial.print (x)
#define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
#define DEBUG_PRINTHEX(x)     Serial.print (x, HEX)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTHEX(x)
#define DEBUG_PRINTLN(x)
#endif



BLEDevice  ble;

static app_timer_id_t                    m_uart_rx_id;
static app_timer_id_t                    m_status_check_id;
static uint8_t rx_buf[TXRX_BUF_LEN];
static int rx_buf_num, rx_state = 0;
static uint8_t rx_temp_buf[20];


// TXRX Service
static const uint8_t uart_base_uuid[] = {0x03, 0xB8, 0x0E, 0x5A, 0xED, 0xE8, 0x4B, 0x33, 0xA7, 0x51, 0x6C, 0xE3, 0x4E, 0xC4, 0xC7, 0};
static const uint8_t uart_tx_uuid[]   = {0x77, 0x72, 0xE5, 0xDB, 0x38, 0x68, 0x41, 0x12, 0xA1, 0xA9, 0xF2, 0x66, 0x9D, 0x10, 0x6B, 0xF3};
static const uint8_t uart_rx_uuid[]   = {0x77, 0x72, 0xE5, 0xDB, 0x38, 0x68, 0x41, 0x12, 0xA1, 0xA9, 0xF2, 0x66, 0x9D, 0x10, 0x6B, 0xF3};
static const uint8_t uart_base_uuid_rev[] = {0, 0xC7, 0xC4, 0x4E, 0xE3, 0x6C, 0x51, 0xA7, 0x33, 0x4B, 0xE8, 0xED, 0x5A, 0x0E, 0xB8, 0x03};

uint8_t txPayload[TXRX_BUF_LEN] = {0,};
uint8_t rxPayload[TXRX_BUF_LEN] = {0,};

GattCharacteristic  txCharacteristic (uart_tx_uuid, txPayload, 1, 20,
                                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);

*/
GattCharacteristic *uartChars[] = {&txCharacteristic};
GattService         uartService(uart_base_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));



/*******************************************************************************
 * INITIALIZE USB MIDI Variables
 *******************************************************************************/
USB  Usb;
USBH_MIDI  Midi(&Usb);
uint8_t usbstate;
uint8_t laststate;
uint8_t rcode;
//uint8_t buf[sizeof(USB_DEVICE_DESCRIPTOR)];
USB_DEVICE_DESCRIPTOR buf;

void MIDI_poll();
void doDelay(unsigned long t1, unsigned long t2, unsigned long delayTime);

boolean bFirst;
boolean isConnected;
uint16_t pid, vid;


/*******************************************************************************
* INITIALIZE Internal BLE Buffer
*******************************************************************************/
#define MAX_TX_BUFF 64
static uint8_t midiOut_buff[MAX_TX_BUFF];
static uint8_t midiOut_buff_len = 0;



/*******************************************************************************
* Send MIDI data to BLE Stack
*
* BLE Packet is capped to 17 bytes (2 bytes prefix + 3 bytes MIDI + 3 * 4 bytes)
*
* Right now this is periodically called by the app_timer, to make sure the buffer clears
* out fast enough as calling this through MIDI_poll() caused more delay. 
* TO DO: Use MIDI-BLE Objects instead of byte array
*
*******************************************************************************/
void m_uart_rx_handle(void * p_context)
{
  if(rx_buf_num > 0 && isConnected) {
 int bufInc = 0;
 if (rx_buf_num < 17) {
  bufInc = rx_buf_num % 17; }
  else { bufInc = 17; }
  ble.updateCharacteristicValue(txCharacteristic.getHandle(), rx_buf, bufInc);
  memmove(rx_buf, rx_buf+bufInc, rx_buf_num-bufInc); // probably not best practice, needs to be fixed
  rx_buf_num -= bufInc;
  rx_state = 0;
  }
}

void uartCallBack(void)
{
  uint32_t err_code = NRF_SUCCESS;

  if (rx_state == 0)
  {
    rx_state = 1;
    err_code = app_timer_start(m_uart_rx_id, UART_RX_TIME, NULL);
    APP_ERROR_CHECK(err_code);
    rx_buf_num = 0;
  }
  while ( Serial.available() )
  {
    rx_buf[rx_buf_num] = Serial.read();
    rx_buf_num++;
  }
}

void m_status_check_handle(void * p_context)
{

  Usb.Task();
  usbstate = Usb.getUsbTaskState();

  if (usbstate != laststate) {
    laststate = usbstate;

    switch (usbstate) {
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

        if (rcode) {
          E_Notify(PSTR("\r\nError reading device descriptor. Error code "), 0x80);
          // print_hex(rcode, 8);
        }
        else {

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

  if ( Usb.getUsbTaskState() == USB_STATE_RUNNING )
  {

    MIDI_poll();
  }

}
void disconnectionCallback(void)
{
  Serial.println("Disconnected! \r\n");
  Serial.println("Restarting the advertising process \r\n");
  isConnected = false;
  ble.startAdvertising();
}

void connectionCallback(void) {
 isConnected = true; 
 uint32_t err_code = NRF_SUCCESS;
  err_code = app_timer_create(&m_uart_rx_id, APP_TIMER_MODE_REPEATED, m_uart_rx_handle);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_start(m_uart_rx_id, STATUS_CHECK_TIME, NULL);
  APP_ERROR_CHECK(err_code);
  
  
}


/*******************************************************************************
* Callback for INCOMING MIDI BLE Data
*
*
*******************************************************************************/

void onDataWritten(uint16_t charHandle)
{
  uint8_t buf[TXRX_BUF_LEN];
  uint16_t bytesRead;
 Serial.println("Something");

  if ( charHandle == txCharacteristic.getHandle() )
  {
    ble.readCharacteristicValue(txCharacteristic.getHandle(), buf, &bytesRead);
    parseBLEtoMIDI(buf, bytesRead);
  }
}

void setup(void)
{
  bFirst = true;
  vid = pid = 0;
  isConnected = false;
  uint32_t err_code = NRF_SUCCESS;
  uart_callback_t uart_cb;
  Serial.begin(9600);
  // while (!Serial); // activate to wait for attached serial

  //Workaround for non UHS2.0 Shield
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  DEBUG_PRINTLN("BLE Arduino Slave");
  Usb.Init();
  delay(500);
  Serial.begin(9600);

  ble.init();
  ble.onDisconnection(disconnectionCallback);
  ble.onConnection(connectionCallback);
  ble.onDataWritten(onDataWritten);

  /* setup advertising */
  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME, (const uint8_t *)"BLIDIno", sizeof("BLIDIno") - 1);
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS, (const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid));

  /* 100ms; in multiples of 0.625ms. */
  ble.setAdvertisingInterval(160);

  ble.addService(uartService);
  SPI.begin();
  //Set Dev_Name
  err_code = RBL_SetDevName("BLIDino");
  APP_ERROR_CHECK(err_code);

  ble.startAdvertising();
 // err_code = app_timer_create(&m_uart_rx_id, APP_TIMER_MODE_SINGLE_SHOT, m_uart_rx_handle);
 
  err_code = app_timer_create(&m_status_check_id, APP_TIMER_MODE_REPEATED, m_status_check_handle);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_start(m_status_check_id, STATUS_CHECK_TIME, NULL);
  APP_ERROR_CHECK(err_code);

}

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

  if (Midi.vid != vid || Midi.pid != pid) {
    vid = Midi.vid;
    pid = Midi.pid;
  }

  do {
    if ( (size = Midi.RecvData(outBuf)) > 0 ) {
      // Send data to parser
     parseMIDItoAppleBle(size, outBuf);

    }
  } while (size > 0);

 uint32_t err_code = NRF_SUCCESS;

}

/*******************************************************************************
 * Convert MIDI BLE to MIDI USB
 *
 * TO DO: 
 * - Currently stubbed out, the nRF51288 SoftDevice S110 does not support MTU and
 * FAR (Fragmentation and Assembly) which is being used for Bluetooth Packets > 20 Bytes
 * 
 *******************************************************************************/
void parseBLEtoMIDI(uint8_t *dataptr, uint16_t bytesRead)
{
  // remove time prefix
  // uint8_t bufMidi[128]; 
  // Remove first 4 Bytes, rest should be fine
 /*  memcpy(bufMidi, dataptr+4, bytesRead-4);
  
  if (bufMidi[0] == 0xF0)
  {
    Serial.println("Received Sysex");
  }
  else
  {
     Serial.println("Received MIDI");
     
  } */
//  if (!Midi.SendData(bufMidi
}



/*******************************************************************************
 * Convert MIDI Data to MIDI-BLE Packets
 *
 * TO DO: 
 * - Jitter / Time Coding is not yet fully reverse-engineered
 * - 1 Byte between MIDI Packet seems to be some relative time-offset
 *
 * - Implement official specification once released
 * as this is based on reverse-engineering.
 *
 *******************************************************************************/
void parseMIDItoAppleBle(int size, byte outBuf[3]) {
  char time[2];
  char buf[20];

  unsigned long timer = 0;
  int lastPos;
  timer = millis();

  uint16_t blueMidiTime = 0;
  blueMidiTime = 32768 + (timer % 16383);

  uint32_t err_code = NRF_SUCCESS;
  int localBufNum = rx_buf_num;

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
 * Convert USB Host debug info to Hex
 *
 *******************************************************************************/
void print_hex(int v, int num_places) {
  int mask = 0, n, num_nibbles, digit;

  for (n = 1; n <= num_places; n++) {
    mask = (mask << 1) | 0x0001;
  }
  v = v & mask; // truncate v to specified number of places

  num_nibbles = num_places / 4;
  if ((num_places % 4) != 0) {
    ++num_nibbles;
  }
  do {
    digit = ((v >> (num_nibbles - 1) * 4)) & 0x0f;
    DEBUG_PRINTHEX(digit);
  }
  while (--num_nibbles);
}
