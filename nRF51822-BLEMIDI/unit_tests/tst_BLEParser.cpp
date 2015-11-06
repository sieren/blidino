#include <stdint.h>
#include <inttypes.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../BLEParser.h"
#include <iostream>
#include <vector>

typedef uint8_t byte;

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

// Mocking used USBMidi_H Functions
class USBMidiMock
{
  public:
  USBMidiMock() = default;
  // Ommiting byte nCable=0 overload
  MOCK_METHOD3(SendSysEx, void(
    uint8_t *dataptr,
    unsigned int datasize,
    byte nCable));

  // everything GEQ 3 is ignored by USBH_MIDI
  MOCK_METHOD2(SendData, void(
    (const uint8_t (&dataptr)[3], byte nCable)));
};

struct Fixture
{
  Fixture()
  {
    mParser.setUSBMidiHandle(&mUSBMock);
  }

  void parseDataArray(uint8_t *data, uint8_t length)
  {
    for (int i = 1; i < length; i++)
    {
      mParser.parseMidiEvent(data[0], data[i]);
    }
  }

  void parseMidiEvent(uint8_t header, uint8_t event)
  {
    mParser.parseMidiEvent(header, event);
  }

  USBMidiMock& getMock()
  {
    return mUSBMock;
  }
  
  mfk::midi::BLEMIDIParser<256, USBMidiMock> mParser;
  ::testing::NiceMock<USBMidiMock> mUSBMock;
};


TEST(BLEReturnMIDIData, SingleNote)
{
  using testing::ElementsAreArray;
  using testing::_;
  Fixture f;
  static uint8_t testdata[5] =
    { 0x80, 0x80, 0x90, 0x80, 0x45 };
  static uint8_t midiResult[3] =
    { 0x90, 0x80, 0x45 };

  EXPECT_CALL(f.getMock(), SendData(ElementsAreArray(midiResult),0));
  f.parseDataArray(testdata, 5);
}


TEST(BLEReturnRunningMIDIData, TwoNotes)
{
  using testing::ElementsAreArray;
  using testing::_;
  Fixture f;
  static uint8_t testdata[8] =
  { 0x80, 0x80, 0x90, 0x45, 0x45, 0x80, 0x22, 0x45 };
  static uint8_t midiResult1[3] =
  { 0x90, 0x45, 0x45 };
  static uint8_t midiResult2[3] =
  { 0x90, 0x22, 0x45 };



  EXPECT_CALL(f.getMock(), SendData(ElementsAreArray(midiResult1),0));
  EXPECT_CALL(f.getMock(), SendData(ElementsAreArray(midiResult2),0));
  f.parseDataArray(testdata, 8);
}


TEST(BLEReturnSysExData, SingleData)
{
  using testing::ElementsAreArray;
  using testing::_;
  using testing::Args;
  Fixture f;
  static uint8_t testdata[8] =
  { 0x80, 0x80, 0xf0, 0x90, 0x80, 0x45, 0x88, 0xf7};
  static uint8_t sysExResult[5] =
  { 0xf0, 0x90, 0x80, 0x45, 0xf7 }; 

  EXPECT_CALL(f.getMock(), SendSysEx(_,_,0)).With(Args<0,1>(ElementsAreArray(sysExResult)));
  f.parseDataArray(testdata, 8);
}


TEST(BLEReturnLongSysExData, MultipleData)
{
  using testing::ElementsAreArray;
  using testing::_;
  using testing::Args;
  Fixture f;
  static uint8_t longSysEx1[9] =
  {
    0x80, 0x80, 0xf0, 0x47, 0x7f, 0x15, 0x5c, 0x00, 0x01
  };
  static uint8_t longSysEx2[9] =
  {
    0x80, 0x21, 0x46, 0x47, 0x7f, 0x15, 0x5c, 0x80, 0xf7
  };
 
  static uint8_t sysExResult[14] =
  { 0xf0, 0x47, 0x7f, 0x15, 0x5c, 0x00, 0x01, 0x21, 0x46,
    0x47, 0x7f, 0x15, 0x5c,  0xf7
  }; 

  EXPECT_CALL(f.getMock(), SendSysEx(_,_,0)).With(Args<0,1>(ElementsAreArray(sysExResult)));
  f.parseDataArray(longSysEx1, 9);
  f.parseDataArray(longSysEx2, 9);
}

TEST(BLEReturnSysExRecoveryBuffer, ReturnTwice)
{
  using testing::ElementsAreArray;
  using testing::_;
  using testing::Args;
  Fixture f;
  static uint8_t longSysEx1[9] =
  {
    0x80, 0x80, 0xf0, 0x47, 0x7f, 0x15, 0x5c, 0x00, 0x01
  };
  static uint8_t longSysEx2[9] =
  {
    0x80, 0x21, 0x46, 0x47, 0x7f, 0x15, 0x5c, 0xf7, 0xf7
  };
 
  static uint8_t sysExFalseResult[13] =
  { 0xf0, 0x47, 0x7f, 0x15, 0x5c, 0x00, 0x01, 0x21, 0x46,
    0x47, 0x7f, 0x15, 0xf7
  };

  static uint8_t sysExRecoveryResult[14] =
  { 0xf0, 0x47, 0x7f, 0x15, 0x5c, 0x00, 0x01, 0x21, 0x46,
    0x47, 0x7f, 0x15, 0x5c, 0xf7
  }; 

  EXPECT_CALL(f.getMock(), SendSysEx(_,_,0)).With(Args<0,1>(
    ElementsAreArray(sysExFalseResult))).Times(1);
  EXPECT_CALL(f.getMock(), SendSysEx(_,_,0)).With(Args<0,1>(
    ElementsAreArray(sysExRecoveryResult))).Times(1);

  f.parseDataArray(longSysEx1, 9);
  f.parseDataArray(longSysEx2, 9);
}

