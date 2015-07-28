//  Created by Matthias Frick on 24/07/15.
/*
 BLE Parser roughly ported from ksoji's BLE MIDI for Android
 Library:
 https://github.com/kshoji/BLE-MIDI-for-Android
*/

#include "BLEParser.h"
#include <stdbool.h>
#include <string.h>
#include <SPI.h>
namespace mfk
{
namespace midi
{
// states

static int MIDI_STATE_TIMESTAMP = 0;
static int MIDI_STATE_WAIT = 1;
static int MIDI_STATE_SIGNAL_2BYTES_2 = 21;
static int MIDI_STATE_SIGNAL_3BYTES_2 = 31;
static int MIDI_STATE_SIGNAL_3BYTES_3 = 32;
static int MIDI_STATE_SIGNAL_SYSEX = 41;
int midiState;

int timestamp;
// MIDI event message
int midiEventKind;
int midiEventNote;
int midiEventVelocity;

// for Timestamp
static int MAX_TIMESTAMP = 8192;
static int BUFFER_LENGTH_MILLIS = 10;
bool useTimestamp = true;
int lastTimestamp;
long lastTimestampRecorded = 0;
int zeroTimestampCount = 0;
// for RPN/NRPN messages
static int PARAMETER_MODE_NONE = 0;
static int PARAMETER_MODE_RPN = 1;
static int PARAMETER_MODE_NRPN = 2;
int parameterMode = 0;
int parameterNumber = 0x3fff;
int parameterValue = 0x3fff;

uint8_t midiBuffer[256];
uint8_t alterSysExBuffer[256];
int midiBufferPtr = 0;
int sysExBufferPtr = 0;

void BLEMIDIParser::addByteToMidiBuffer(uint8_t midiEvent)
{
  midiBuffer[midiBufferPtr] = midiEvent;
  midiBufferPtr++;
}

void BLEMIDIParser::addByteToRecoveryBuffer(uint8_t midiEvent)
{
  alterSysExBuffer[sysExBufferPtr] = midiEvent;
  sysExBufferPtr++;
}

void BLEMIDIParser::createSysExRecovery()
{
  sysExBufferPtr = midiBufferPtr;
  memcpy(alterSysExBuffer, midiBuffer, sizeof(midiBuffer));
}

uint8_t BLEMIDIParser::replaceLastByteInMidiBuffer(uint8_t midiEvent)
{
  midiBufferPtr--;
  uint8_t lastEvt = midiBuffer[midiBufferPtr];
  midiBuffer[midiBufferPtr] = midiEvent;
  midiBufferPtr++;
  return lastEvt;
}

uint8_t BLEMIDIParser::replaceLastByteInRecoveryBuffer(uint8_t midiEvent)
{
  sysExBufferPtr--;
  uint8_t lastEvt = alterSysExBuffer[sysExBufferPtr];
  alterSysExBuffer[sysExBufferPtr] = midiEvent;
  sysExBufferPtr++;
  return lastEvt;
}

void BLEMIDIParser::resetMidiBuffer()
{
  memset(&midiBuffer[0], 0, sizeof(midiBuffer));
  midiBufferPtr = 0;
}

void BLEMIDIParser::resetRecoveryBuffer()
{
  memset(&alterSysExBuffer[0], 0, sizeof(alterSysExBuffer));
  sysExBufferPtr = 0;
}

void BLEMIDIParser::sendSysex()
{
  char debugBuf[256];
 /* Serial.println("Sending Sysex:");
  for (int i = 0; i<midiBufferPtr; i++)
  {
    sprintf(debugBuf, "%02x ", midiBuffer[i]);
    Serial.print(debugBuf);
    
  }
  Serial.print("----------\n"); */
  
  Midi->SendSysEx(midiBuffer, midiBufferPtr, 0);
}

void BLEMIDIParser::sendSysexRecovery()
{
  Serial.println("Sending ECC Sys:");
  char debugBuf[256];
  for (int i = 0; i<sysExBufferPtr; i++)
  {
    sprintf(debugBuf, "%02x ", alterSysExBuffer[i]);
    Serial.print(debugBuf);
    
  }
  Serial.print("----------\n");  
//  Midi->SendSysEx(alterSysExBuffer, sysExBufferPtr, 0);
}

void BLEMIDIParser::sendMidi()
{
  /* printf("Sending Event \n");
  char debugBuf[256];
  for (int i = 0; i<midiBufferPtr; i++)
  {
    sprintf(debugBuf, "%02x", midiBuffer[i]);
    printf(debugBuf);
    printf("\n");
  }
  printf("----------\n"); */
  Midi->SendData(midiBuffer, 0);
}

void BLEMIDIParser::setUSBMidiHandle(USBH_MIDI *MidiHdl)
{
  Midi = MidiHdl; 
}

void BLEMIDIParser::parseMidiEvent(uint8_t header, uint8_t event)
{
  uint8_t midiEvent = event & 0xff;
  uint8_t timeToWait;
  // printf((char*)midiEvent);
  if (midiState == MIDI_STATE_TIMESTAMP)
  {
   // printf("Timestamp");
    if ((midiEvent & 0x80) == 0)
    {
      // running status
      midiState = MIDI_STATE_WAIT;
    }

    if (midiEvent == 0xf7)
    {
      // make sure this is the end of sysex
      // and send alternative recovery stream
      if (sysExBufferPtr > 0)
      {
        uint8_t removed = replaceLastByteInRecoveryBuffer(midiEvent);
          Serial.println("Sysex Failed");
          sendSysexRecovery();
          resetRecoveryBuffer();

      // process next byte with state: MIDI_STATE_TIMESTAMP
      }
      midiState = MIDI_STATE_TIMESTAMP;
      return;
    }
    else
    {
      // reset alternative sysex stream
       resetRecoveryBuffer();
    }
  } // end of timestamp

  if (midiState == MIDI_STATE_TIMESTAMP)
  {
    timestamp = ((header & 0x3f) << 7) | (midiEvent & 0x7f);
    midiState = MIDI_STATE_WAIT;
  }
  else if (midiState == MIDI_STATE_WAIT)
  {
    switch (midiEvent & 0xf0) {
      case 0xf0: {
        switch (midiEvent) {
          case 0xf0:
            resetRecoveryBuffer();
            resetMidiBuffer();
            addByteToMidiBuffer(midiEvent);
            midiState = MIDI_STATE_SIGNAL_SYSEX;
            break;
          case 0xf1:
          case 0xf3:
            // 0xf1 MIDI Time Code Quarter Frame. : 2bytes
            // 0xf3 Song Select. : 2bytes
            midiEventKind = midiEvent;
            addByteToMidiBuffer(midiEvent);
            midiState = MIDI_STATE_SIGNAL_2BYTES_2;
            break;
          case 0xf2:
            // 0xf2 Song Position Pointer. : 3bytes
            midiEventKind = midiEvent;
            addByteToMidiBuffer(midiEvent);
            midiState = MIDI_STATE_SIGNAL_3BYTES_2;
            break;
          case 0xf6:
            // 0xf6 Tune Request : 1byte
            addByteToMidiBuffer(midiEvent);
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xf8:
            // 0xf8 Timing Clock : 1byte
#pragma mark send timeclock
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xfa:
            // 0xfa Start : 1byte
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xfb:
            // 0xfb Continue : 1byte
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xfc:
            // 0xfc Stop : 1byte
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xfe:
            // 0xfe Active Sensing : 1byte
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xff:
            // 0xff Reset : 1byte
            midiState = MIDI_STATE_TIMESTAMP;
            break;

          default:
            break;
        }
      }
        break;
      case 0x80:
      case 0x90:
      case 0xa0:
      case 0xb0:
      case 0xe0:
        // 3bytes pattern
        midiEventKind = midiEvent;
        midiState = MIDI_STATE_SIGNAL_3BYTES_2;
        break;
      case 0xc0: // program change
      case 0xd0: // channel after-touch
        // 2bytes pattern
        midiEventKind = midiEvent;
        midiState = MIDI_STATE_SIGNAL_2BYTES_2;
        break;
      default:
        // 0x00 - 0x70: running status
        if ((midiEventKind & 0xf0) != 0xf0) {
          // previous event kind is multi-bytes pattern
          midiEventNote = midiEvent;
          midiState = MIDI_STATE_SIGNAL_3BYTES_3;
        }
        break;
    }
  }
  else if (midiState == MIDI_STATE_SIGNAL_2BYTES_2)
  {
    switch (midiEventKind & 0xf0)
    {
        // 2bytes pattern
      case 0xc0: // program change
        midiEventNote = midiEvent;
        midiState = MIDI_STATE_TIMESTAMP;
        break;
      case 0xd0: // channel after-touch
        midiEventNote = midiEvent;
        midiState = MIDI_STATE_TIMESTAMP;
        break;
      case 0xf0:
      {
        switch (midiEventKind)
        {
          case 0xf1:
            // 0xf1 MIDI Time Code Quarter Frame. : 2bytes
            midiEventNote = midiEvent;
            addByteToMidiBuffer(midiEventNote);
            sendMidi();
            resetMidiBuffer();
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xf3:
            // 0xf3 Song Select. : 2bytes
            midiEventNote = midiEvent;
            addByteToMidiBuffer(midiEventNote);
            sendMidi();
            resetMidiBuffer();
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          default:
            // illegal state
            Serial.println("Illegal");
            midiState = MIDI_STATE_TIMESTAMP;
            break;
        }
      }
        break;
      default:
        // illegal state
        Serial.println("Illegal");
        midiState = MIDI_STATE_TIMESTAMP;
        break;
    }
  }
  else if (midiState == MIDI_STATE_SIGNAL_3BYTES_2)
  {
    switch (midiEventKind & 0xf0)
    {
      case 0x80:
      case 0x90:
      case 0xa0:
      case 0xb0:
      case 0xe0:
      case 0xf0:
        // 3bytes pattern
        midiEventNote = midiEvent;
        midiState = MIDI_STATE_SIGNAL_3BYTES_3;
        break;
      default:
        // illegal state
        Serial.println("Illegal");
        midiState = MIDI_STATE_TIMESTAMP;
        break;
    }
  }
  else if (midiState == MIDI_STATE_SIGNAL_3BYTES_3)
  {
    switch (midiEventKind & 0xf0)
    {
        // 3bytes pattern
      case 0x80: // note off
        midiEventVelocity = midiEvent;
        addByteToMidiBuffer(midiEventKind);
        addByteToMidiBuffer(midiEventNote);
        addByteToMidiBuffer(midiEventVelocity);
        sendMidi();
        resetMidiBuffer();
        midiState = MIDI_STATE_TIMESTAMP;
        break;
      case 0x90: // note on
        midiEventVelocity = midiEvent;
        //timeToWait = calculateTimeToWait(timestamp);
        addByteToMidiBuffer(midiEventKind);
        addByteToMidiBuffer(midiEventNote);
        addByteToMidiBuffer(midiEventVelocity);
        sendMidi();
        resetMidiBuffer();
        midiState = MIDI_STATE_TIMESTAMP;
        break;
      case 0xa0: // control polyphonic key pressure
        midiEventVelocity = midiEvent;
        addByteToMidiBuffer(midiEventKind);
        addByteToMidiBuffer(midiEventNote);
        addByteToMidiBuffer(midiEventVelocity);
        sendMidi();
        resetMidiBuffer();
        midiState = MIDI_STATE_TIMESTAMP;
        break;
      case 0xb0: // control change
        midiEventVelocity = midiEvent;
        switch (midiEventNote & 0x7f)
        {
          case 98:
            // NRPN LSB
            parameterNumber &= 0x3f80;
            parameterNumber |= midiEventVelocity & 0x7f;
            parameterMode = PARAMETER_MODE_NRPN;
            break;
          case 99:
            // NRPN MSB
            parameterNumber &= 0x007f;
            parameterNumber |= (midiEventVelocity & 0x7f) << 7;
            parameterMode = PARAMETER_MODE_NRPN;
            break;
          case 100:
            // RPN LSB
            parameterNumber &= 0x3f80;
            parameterNumber |= midiEventVelocity & 0x7f;
            parameterMode = PARAMETER_MODE_RPN;
            break;
          case 101:
            // RPN MSB
            parameterNumber &= 0x007f;
            parameterNumber |= (midiEventVelocity & 0x7f) << 7;
            parameterMode = PARAMETER_MODE_RPN;
            break;
          case 38:
            // data LSB
            Serial.println("Entry LSB");
            parameterValue &= 0x3f80;
            parameterValue |= midiEventVelocity & 0x7f;

            if (parameterNumber != 0x3fff) {
              if (parameterMode == PARAMETER_MODE_RPN)
              {
                addByteToMidiBuffer(midiEventKind);
                addByteToMidiBuffer(parameterNumber);
                addByteToMidiBuffer(parameterValue);
                sendMidi();
                resetMidiBuffer();
              }
              else if (parameterMode == PARAMETER_MODE_NRPN)
              {
                addByteToMidiBuffer(midiEventKind);
                addByteToMidiBuffer(parameterNumber);
                addByteToMidiBuffer(parameterValue);
                sendMidi();
                resetMidiBuffer();
              }
            }
            break;
          case 6:
            // data MSB
            parameterValue &= 0x007f;
            parameterValue |= (midiEventVelocity & 0x7f) << 7;

            if (parameterNumber != 0x3fff)
            {
              if (parameterMode == PARAMETER_MODE_RPN)
              {
                addByteToMidiBuffer(midiEventKind);
                addByteToMidiBuffer(parameterNumber);
                addByteToMidiBuffer(parameterValue);
                sendMidi();
                resetMidiBuffer();
              }
              else if (parameterMode == PARAMETER_MODE_NRPN)
              {
                addByteToMidiBuffer(midiEventKind);
                addByteToMidiBuffer(parameterNumber);
                addByteToMidiBuffer(parameterValue);
                sendMidi();
                resetMidiBuffer();
              }
            }
            break;
          default:
            // do nothing
            break;
        }
        addByteToMidiBuffer(midiEventKind);
        addByteToMidiBuffer(midiEventNote);
        addByteToMidiBuffer(midiEventVelocity);
        sendMidi();
        resetMidiBuffer();
        midiState = MIDI_STATE_TIMESTAMP;
        break;
      case 0xe0: // pitch bend
        midiEventVelocity = midiEvent;
        addByteToMidiBuffer(midiEventKind);
        addByteToMidiBuffer(midiEventNote);
        addByteToMidiBuffer(midiEventVelocity);
        sendMidi();
        resetMidiBuffer();
        midiState = MIDI_STATE_TIMESTAMP;
        break;
      case 0xf0: // Song Position Pointer.
        midiEventVelocity = midiEvent;
        addByteToMidiBuffer(midiEventKind);
        addByteToMidiBuffer(midiEventNote);
        addByteToMidiBuffer(midiEventVelocity);
        sendMidi();
        resetMidiBuffer();
        midiState = MIDI_STATE_TIMESTAMP;
        break;
      default:
        // illegal state
        midiState = MIDI_STATE_TIMESTAMP;
        break;
    }
  }
  else if (midiState == MIDI_STATE_SIGNAL_SYSEX)
  {
    if (midiEvent == 0xf7)
    {
      uint8_t repEvt = replaceLastByteInMidiBuffer(midiEvent);
      
      resetRecoveryBuffer();
      createSysExRecovery();
      replaceLastByteInRecoveryBuffer(repEvt);
      addByteToRecoveryBuffer(midiEvent);
      sendSysex();
      resetMidiBuffer();
      midiState = MIDI_STATE_TIMESTAMP;
    }
    else
    {
      addByteToMidiBuffer(midiEvent);
    }

  }
}
  
} // midi
} // mfk
