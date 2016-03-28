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

#ifndef ____BLEParser__
#define ____BLEParser__
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

namespace mfk
{
namespace midi
{
  static int MIDI_STATE_TIMESTAMP = 0;
  static int MIDI_STATE_WAIT = 1;
  static int MIDI_STATE_SIGNAL_2BYTES_2 = 21;
  static int MIDI_STATE_SIGNAL_3BYTES_2 = 31;
  static int MIDI_STATE_SIGNAL_3BYTES_3 = 32;
  static int MIDI_STATE_SIGNAL_SYSEX = 41;

  // for Timestamp
  static int MAX_TIMESTAMP = 8192;
  static int BUFFER_LENGTH_MILLIS = 10;


  // for RPN/NRPN messages
  static int PARAMETER_MODE_NONE = 0;
  static int PARAMETER_MODE_RPN = 1;
  static int PARAMETER_MODE_NRPN = 2;
  int parameterMode = 0;
  int parameterNumber = 0x3fff;
  int parameterValue = 0x3fff;

  template <int kMaxBufferSize, typename Receiver>
  class BLEMIDIParser
  {
  public:

    void parseMidiEvent(uint8_t header, const uint8_t event)
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
          if (sysExRecBufferPtr > 0)
          {
            uint8_t removed = replaceLastByteInRecoveryBuffer(midiEvent);
            sendSysexRecovery();
            resetRecoveryBuffer();
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
                resetSysExBuffer();
                addByteToSysExBuffer(midiEvent);
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
                sendMidi(2);
                resetMidiBuffer();
                midiState = MIDI_STATE_TIMESTAMP;
                break;
              case 0xf3:
                // 0xf3 Song Select. : 2bytes
                midiEventNote = midiEvent;
                addByteToMidiBuffer(midiEventNote);
                sendMidi(2);
                resetMidiBuffer();
                midiState = MIDI_STATE_TIMESTAMP;
                break;
              default:
                // illegal state
                midiState = MIDI_STATE_TIMESTAMP;
                break;
            }
          }
            break;
          default:
            // illegal state
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
            sendMidi(3);
            resetMidiBuffer();
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0x90: // note on
            midiEventVelocity = midiEvent;
            //timeToWait = calculateTimeToWait(timestamp);
            addByteToMidiBuffer(midiEventKind);
            addByteToMidiBuffer(midiEventNote);
            addByteToMidiBuffer(midiEventVelocity);
            sendMidi(3);
            resetMidiBuffer();
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xa0: // control polyphonic key pressure
            midiEventVelocity = midiEvent;
            addByteToMidiBuffer(midiEventKind);
            addByteToMidiBuffer(midiEventNote);
            addByteToMidiBuffer(midiEventVelocity);
            sendMidi(3);
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
                parameterValue &= 0x3f80;
                parameterValue |= midiEventVelocity & 0x7f;

                if (parameterNumber != 0x3fff) {
                  if (parameterMode == PARAMETER_MODE_RPN)
                  {
                    addByteToMidiBuffer(midiEventKind);
                    addByteToMidiBuffer(parameterNumber);
                    addByteToMidiBuffer(parameterValue);
                    sendMidi(3);
                    resetMidiBuffer();
                  }
                  else if (parameterMode == PARAMETER_MODE_NRPN)
                  {
                    addByteToMidiBuffer(midiEventKind);
                    addByteToMidiBuffer(parameterNumber);
                    addByteToMidiBuffer(parameterValue);
                    sendMidi(3);
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
                    sendMidi(3);
                    resetMidiBuffer();
                  }
                  else if (parameterMode == PARAMETER_MODE_NRPN)
                  {
                    addByteToMidiBuffer(midiEventKind);
                    addByteToMidiBuffer(parameterNumber);
                    addByteToMidiBuffer(parameterValue);
                    sendMidi(3);
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
            sendMidi(3);
            resetMidiBuffer();
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xe0: // pitch bend
            midiEventVelocity = midiEvent;
            addByteToMidiBuffer(midiEventKind);
            addByteToMidiBuffer(midiEventNote);
            addByteToMidiBuffer(midiEventVelocity);
            sendMidi(3);
            resetMidiBuffer();
            midiState = MIDI_STATE_TIMESTAMP;
            break;
          case 0xf0: // Song Position Pointer.
            midiEventVelocity = midiEvent;
            addByteToMidiBuffer(midiEventKind);
            addByteToMidiBuffer(midiEventNote);
            addByteToMidiBuffer(midiEventVelocity);
            sendMidi(3);
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
          uint8_t repEvt = replaceLastByteInSysExBuffer(midiEvent);

          resetRecoveryBuffer();
          createSysExRecovery();
          replaceLastByteInRecoveryBuffer(repEvt);
          addByteToRecoveryBuffer(midiEvent);
          sendSysex();
          resetSysExBuffer();
          midiState = MIDI_STATE_TIMESTAMP;
        }
        else
        {
          addByteToSysExBuffer(midiEvent);
        }

      }
    }
    void setUSBMidiHandle(Receiver *MidiHdl)
    {
      midiRecv = MidiHdl;
    }

  private:

    void addByteToMidiBuffer(uint8_t midiEvent)
    {
      midiBuffer[midiBufferPtr] = midiEvent;
      midiBufferPtr++;
    }

    void addByteToSysExBuffer(uint8_t midiEvent)
    {
      sysExBuffer[sysExBufferPtr] = midiEvent;
      sysExBufferPtr++;
    }

    uint8_t replaceLastByteInSysExBuffer(uint8_t midiEvent)
    {
      sysExBufferPtr--;
      uint8_t lastEvt = sysExBuffer[sysExBufferPtr];
      sysExBuffer[sysExBufferPtr] = midiEvent;
      sysExBufferPtr++;
      return lastEvt;
    }

    void sendSysex()
    {
      midiRecv->SendSysEx(sysExBuffer, sysExBufferPtr, 0);
    }

    void createSysExRecovery()
    {
      sysExRecBufferPtr = sysExBufferPtr;
      memcpy(alterSysExBuffer, sysExBuffer, sysExBufferPtr);
    }

    void sendSysexRecovery()
    {
      midiRecv->SendSysEx(alterSysExBuffer, sysExRecBufferPtr, 0);
    }

    uint8_t replaceLastByteInRecoveryBuffer(uint8_t midiEvent)
    {
      sysExRecBufferPtr--;
      uint8_t lastEvt = alterSysExBuffer[sysExRecBufferPtr];
      alterSysExBuffer[sysExRecBufferPtr] = midiEvent;
      sysExRecBufferPtr++;
      return lastEvt;
    }

    void addByteToRecoveryBuffer(uint8_t midiEvent)
    {
      alterSysExBuffer[sysExRecBufferPtr] = midiEvent;
      sysExRecBufferPtr++;
    }

    void resetMidiBuffer()
    {
      memset(&midiBuffer[0], 0, sizeof(midiBuffer));
      midiBufferPtr = 0;
    }

    void resetSysExBuffer()
    {
      memset(&sysExBuffer[0], 0, kMaxBufferSize);
      sysExBufferPtr = 0;
    }

    void resetRecoveryBuffer()
    {
      memset(&alterSysExBuffer[0], 0, sizeof(alterSysExBuffer));
      sysExRecBufferPtr = 0;
    }

    void sendMidi(uint8_t size)
    {
      midiRecv->SendData(midiBuffer, 0);
    }

    uint8_t midiBuffer[3];
    uint8_t sysExBuffer[kMaxBufferSize];
    uint8_t alterSysExBuffer[kMaxBufferSize];
    int midiBufferPtr = 0;
    int sysExRecBufferPtr = 0;
    int sysExBufferPtr = 0;


    // MIDI event messages, state & stamps
    int midiEventKind;
    int midiEventNote;
    int midiEventVelocity;
    int midiState = MIDI_STATE_TIMESTAMP;
    int timestamp;


    bool useTimestamp = true;
    int lastTimestamp;
    long lastTimestampRecorded = 0;
    int zeroTimestampCount = 0;
    Receiver *midiRecv;
  };
}
}

#endif /* defined(____BLEParser__) */
