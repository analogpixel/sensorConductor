
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include <HSBColor.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define PIN 6 // data pin
#define STRIPLEN 152 //total strip length

// Adafruit Neopixel library
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLEN, PIN, NEO_GRB + NEO_KHZ800);

int knob1 = A4;
int knob2 = A5;
int slide1 = A3;
int slide2 = A2;
int slide3 = A1;
int slide4 = A0;

int sound1Prev = 0;
int sound2Prev = 0;
int sound3Prev = 0;
int sound4Prev = 0;
int sound5Prev = 0;
int sound6Prev = 0;

// RGB array and pointer for HSB library
int rgbArray[3];
int *rgb = rgbArray;

int pos = 0; // postion of "lead" pixel
long prevMillis = 0;

void setup() {
  Serial.begin(115200);
  pinMode(knob1, INPUT_PULLUP);
  pinMode(knob2, INPUT_PULLUP);
  pinMode(slide1, INPUT_PULLUP);
  pinMode(slide2, INPUT_PULLUP);
  pinMode(slide3, INPUT_PULLUP);
  pinMode(slide3, INPUT_PULLUP);
  strip.begin();
  strip.show();
}

void loop() {
  
  int k1 = analogRead(knob1);
  int k2 = analogRead(knob2);
  int s1 = analogRead(slide1);
  int s2 = analogRead(slide2);
  int s3 = analogRead(slide3);
  int s4 = analogRead(slide4);
  
  /* Mappings for sensors --> color changes */
  unsigned long del = map(k2, 0, 1023, 100, 1);   // k2 =  Delay in milliseconds. Max 100, min 1 (Knob)
  int sat = map(s1, 62, 968, 0, 99);              // s1 = Saturation. Max: 99 (Slide)
  int len = map(s3, 60, 976, 0, STRIPLEN);        // s3 = Tail length. Max: Striplen (Slide)
  int hue = map(s2, 55, 979, 0, 360);             // s4 = Hue. Max: 360 (Slide)
  int bowy = map(s4, 47, 970, 0, 360);            // s2 = Rainbowyness. Max: 360 (Slide)
  
/* Mappings for MIDI */
  int sound1 = map(k2, 0, 1024, 0, 127);
  int sound2 = map(k1, 0, 1024, 0, 127);
  int sound3 = map(s1, 0, 1024, 0, 127);
  int sound4 = map(s2, 0, 1024, 0, 127);
  int sound5 = map(s3, 0, 1024, 0, 127);
  int sound6 = map(s4, 0, 1024, 0, 127);
  
  unsigned long currMillis = millis();
  
  currMillis = millis();
  if (currMillis - prevMillis > 30) {
    if (sound1 > sound1Prev + 1 || sound1 < sound1Prev - 1) {
      MIDI.sendControlChange(97, sound1, 3);
      sound1Prev = sound1;
    }
    if (sound2 > sound2Prev + 1 || sound2 < sound2Prev - 1) {
     MIDI.sendControlChange(98, sound2, 3);
     sound2Prev = sound2;
    }
    if (sound3 > sound3Prev + 1 || sound3 < sound3Prev - 1) {
      MIDI.sendControlChange(99, sound3, 3);
      sound3Prev = sound3;
    }
    if (sound4 > sound4Prev + 1 || sound4 < sound4Prev - 1) {
      MIDI.sendControlChange(100, sound4, 3);
      sound4Prev = sound4;
    }
    if (sound5 > sound5Prev + 1 || sound5 < sound5Prev - 1) {
      MIDI.sendControlChange(101, sound5, 3);
      sound5Prev = sound5;
    }
    if (sound6 > sound6Prev + 1 || sound6 < sound6Prev - 1) {
      MIDI.sendControlChange(102, sound6, 3);
      sound6Prev = sound6;
    }
  }
  
  

  /* SPEED MODE 
   If del is short, pos becomes automatic with K2 controlling speed */
  if (del < 99) {
      currMillis = millis();
      
      if (currMillis - prevMillis > del) {
        prevMillis = currMillis;
         if (pos < STRIPLEN) {
             pos++;
         }
         else {
             pos = 0;
         }
        lightShow(hue, sat, bowy, pos, len);
        strip.show();
     }
   }
   
   /* MANUAL MODE
   If del is high (K2 at min), K1 controls position*/
   else {
     int usrPos = map(k1, 0, 1023, pos, pos + STRIPLEN);
     if (usrPos > STRIPLEN) {
       usrPos = usrPos - STRIPLEN;
     }
     lightShow(hue, sat, bowy, usrPos, len);
     strip.show();
  }
     
     clearPixels();
}

// This function takes positions from all the knobs and sliders and creates
// a "comet" with several adjustable features
void lightShow(int hue, int sat, int bowy, int pos, int len) {
  // Grab the position when the function was called
  int tail = pos;
  // Bri is not user controlled, the comet will always have a faded tail
  // relative to its length
  float bri = 99;
  // Break the 99 brightness settings into chunks depending on len
  // Since len can be more than 99, it's a float!
  float briOffset = bri/len;
  // Break the 360 hue settings into chunks depending on len
  float bowyOffset = bowy/len;
  
  for (int l = 0; l < len; l++) {
     // Make sure we count from the end of the strip when we pass 0
     if (tail < 0) {
       tail = STRIPLEN;
     }
    // Convert this pixel's hues from HSB to RGB
     H2R_HSBtoRGB(hue, sat, bri, rgb);
     strip.setPixelColor(tail, rgbArray[0], rgbArray[1], rgbArray[2]);
     //bri value of each pixel is reduced for each pixel in the tail
     bri = bri-briOffset;

     // Wrap hue around
     if (hue >= 360) {
        hue = 0;
     }
     // Offset hue as needed
     hue = hue + bowyOffset;
     
     // Offset pixel by one position
     tail--;
  }
}

void clearPixels() {
  for (int i=0; i < STRIPLEN; i++) {
          strip.setPixelColor(i, 0);
        }
}
