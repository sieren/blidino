//  Created by Matthias Frick on 24/07/15.

#ifndef ____BLEParser__
#define ____BLEParser__
#include <BLE_API.h>
#include <stdio.h>
#include <usbh_midi.h>
namespace mfk
{
namespace midi
{
  class BLEMIDIParser
  {
  public:
    void parseMidiEvent(uint8_t header, uint8_t event);
    void setUSBMidiHandle(USBH_MIDI *MidiHdl);
  private:
    void addByteToMidiBuffer(uint8_t midiEvent);
    uint8_t replaceLastByteInMidiBuffer(uint8_t midiEvent);
    void sendSysex();
    void createSysExRecovery();
    void sendSysexRecovery();
    uint8_t replaceLastByteInRecoveryBuffer(uint8_t midiEvent);
    void addByteToRecoveryBuffer(uint8_t midiEvent);
    void resetMidiBuffer();
    void resetRecoveryBuffer();
    void sendMidi();
    USBH_MIDI *Midi;
  };
}
}

#endif /* defined(____BLEParser__) */
