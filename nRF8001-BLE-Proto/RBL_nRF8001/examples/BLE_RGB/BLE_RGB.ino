#include "Adafruit_NeoPixel.h"
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 7, NEO_GRB + NEO_KHZ800);

int R = 0;
int G = 0;
int B = 0;

void setup() 
{
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  ble_begin();
}

void loop() 
{
  if(3 == ble_available())
  {
    R = ble_read();
    G = ble_read();
    B = ble_read();
    colorWipe( strip.Color(R, G, B) );
  }
  ble_do_events();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c) 
{
  for(uint16_t i=0; i<strip.numPixels(); i++) 
  {
      strip.setPixelColor(i, c);
      strip.show();
  }
}
