/*
 *******************************************************************************
 * USB-MIDI dump utility
 * Copyright 2013 Yuuichi Akagawa
 *
 * for use with USB Host Shield 2.0 from Circuitsathome.com
 * https://github.com/felis/USB_Host_Shield_2.0
 *
 * This is sample program. Do not expect perfect behavior.
 *******************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *******************************************************************************
 */

#include <Usb.h>
#include <usbh_midi.h>
#include <SPI.h>
#include <lib_aci.h>
#include <aci_setup.h>
#include <RBL_nRF8001.h>

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
uint16_t pid, vid;

/*******************************************************************************
 * INITIALIZE Internal BLE Buffer
 *******************************************************************************/
#define MAX_TX_BUFF 64
static uint8_t midiOut_buff[MAX_TX_BUFF];
static uint8_t midiOut_buff_len = 0;


/*******************************************************************************
 * Setup USB Shield and Bluetooth Chip (nRF8001)
 *******************************************************************************/
 
void setup()
{


  bFirst = true;
  vid = pid = 0;
#ifdef DEBUG 
 Serial.begin(115200); 
#endif
  // while (!Serial);
  //Workaround for non UHS2.0 Shield 
  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);

  DEBUG_PRINTLN("BLE Arduino Slave");
  Usb.Init();
  delay( 200 );

  // if (Usb.Init() == -1) {
  //   while(1); //halt
  // }//if (Usb.Init() == -1...
  SPI.begin();
  ble_begin();
  ble_do_events();
}

/*******************************************************************************
 * Main Runloop
 * 
 * Polls MIDI and BLE Data when SPI bus available
 *******************************************************************************/

void loop()
{
  unsigned long t1;
  
  // BLE not on SPI, go ahead
  if (!ble_busy()) {

    Usb.Task();
   /* usbstate = Usb.getUsbTaskState();

    if (usbstate != laststate) {
      laststate = usbstate;

      switch (usbstate) {
        case( USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE):
        E_Notify(PSTR("\r\nWaiting for device..."), 0x80);
        break;
        case( USB_ATTACHED_SUBSTATE_RESET_DEVICE):
        E_Notify(PSTR("\r\nDevice connected. Resetting..."), 0x80);
        break;
        case( USB_ATTACHED_SUBSTATE_WAIT_SOF):
        E_Notify(PSTR("\r\nReset complete. Waiting for the first SOF..."), 0x80);
        break;
        case( USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE):
        E_Notify(PSTR("\r\nSOF generation started. Enumerating device..."), 0x80);
        break;
        case( USB_STATE_ADDRESSING):
        E_Notify(PSTR("\r\nSetting device address..."), 0x80);
        break;
        case( USB_STATE_RUNNING):
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
        case( USB_STATE_ERROR):
        E_Notify(PSTR("\r\nUSB state machine reached error state"), 0x80);
        break;

      default:
        break;
      }//switch( usbstate...
    } */

    if( Usb.getUsbTaskState() == USB_STATE_RUNNING )
    {
      MIDI_poll();
     
      // TO DO
      // Send MIDI to USB Device
   
      /*   if (MIDI.read()) {
       msg[0] = MIDI.getType();
       if( msg[0] == 0xf0 ) { //SysEX
         //TODO
         //SysEx implementation
       }else{
         msg[1] = MIDI.getData1();
         msg[2] = MIDI.getData2();
         Midi.SendData(msg, 0);
       }
       */
    } 


  }
  
  
  //TO DO:
  // Omni-Directional MIDI
  /*  while(ble_available()) {
    midiOut_buff[midiOut_buff_len] = ble_read();
    midiOut_buff_len++;
  } */
  
  // Send packets
  

    ble_do_events();  
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
  
  if(Midi.vid != vid || Midi.pid != pid){
    vid = Midi.vid;
    pid = Midi.pid;
  }
    
    do {
      if( (size=Midi.RecvData(outBuf)) > 0 ){
        //MIDI Output
        //DEBUG_PRINT("MIDI SIZE: ");
        //DEBUG_PRINTLN(size);
     /*            for(int i=0; i<size; i++){
                    sprintf(buf, " %02X", outBuf[i]);
                    DEBUG_PRINT(buf);
      }*/
        parseMIDItoAppleBle(size, outBuf);
      }
    }while(size>0);

}

 
/*******************************************************************************
 * Convert MIDI Data to MIDI-BLE Packets
 * 
 * TO DO: Implement official specification once released
 * as this is based on reverse-engineering
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
  

  // 0x80 0x80 Prefix Bytes for Apple Protocol    
  // buffer empty, write apple time prefix
  // TO DO: 
  // Implement correct Apple Time Coding
  // 2nd byte (0x80) is temp workaround
  if (ble_buff_len() == 0)
  {
      ble_write(blueMidiTime >> 8);
      ble_write(0x80);
      for (int i = 0; i < size; i++)
      {
        ble_write(outBuf[i]);
      }
      
  }
    // Buffer already filled, append Packet
    // To Do: replace "80" hex seperator with relative timing data?
    // or wait for official spec to be released
  else if(ble_buff_len() >= 2)
  {
    int buffL = ble_buff_len();
      ble_write(0x80);
      for (int i = 0; i < size; i++)
      {
        ble_write(outBuf[i]);
      }
        
  }
  
          //DEBUG_PRINT("MIDI SIZE: ");
        //DEBUG_PRINTLN(size);
        DEBUG_PRINTLN("---- MIDI ----");
                 for(int i=0; i<size; i++){
                    sprintf(buf, " %02X", outBuf[i]);
                    DEBUG_PRINT(buf);
      } 
      DEBUG_PRINTLN("");
      DEBUG_PRINTLN("---- MIDI ----");
    
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



