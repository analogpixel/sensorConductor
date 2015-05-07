
/*
  
 Reads analog A0 sends midi pitch bend & shifts raibow on LED strand
 Uses microsmooth to reduce nose on A0
 
 Reads analog A1 sends midi note on velocity, pressure & number of LEDs on strand 
 
 This example code is in the public domain.
 
 Added microsmooth lib   https://github.com/AsheeshR/Microsmooth#microsmooth
 Added Ardumidi lib
 Added NeoPixel lib  https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library
 , Matt Collier
 3-23-2015
 
 */

//#include <autotune.h>
#include <microsmooth.h>
uint16_t *history = ms_init(SMA);

#include <ardumidi.h>  //Midi Lib

#include <Adafruit_NeoPixel.h>  //LED control lib

#define PIN 6    // LED control pin
#define LED_COUNT 100

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

int ledPin = 13;
int note_on = 0;
int n = 1;

int lastAnalog[2] = {
  0, 0};

long int sensorValue[2] = {
  0, 0};        // value read from the pot

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'


  // initialize serial communications 
  Serial.begin(9600); 
  pinMode(ledPin, OUTPUT);

}

void loop() {
  //colorWipe(strip.Color(255, 0, 0), 0, sensorValue); // Red

  //unsigned long pepe1=millis();  // takes the time before the library begins

  // read analog pins, over sample for more resulotion and toss noise spikes
  sensorValue[0] = analogReadOverSample(A0, 190, 3, sensorValue[0]);

  // Call Microsmooth average lib
  // unsigned int pitchBend  = sma_filter(sensorValue[0], history);

  //unsigned long pepe2=millis()-pepe1;  // the following gives you the time taken to get the measurement

  sensorValue[1] = analogReadOverSample(A1, 5, 4, sensorValue[1]);

  // map pitch bend, Pich value range: 0 - 16383 (0x3FFF)
  // int pitchBend = map(sensorValue[0], 0, 4000, 500, 14000);
  unsigned int pitchBend = sensorValue[0];


  // map sensor 2 to note velocity (volume)  values 0-127
  int noteVelocity = map(sensorValue[1], 0, 170, 0, 127);

  if (noteVelocity > 25) {
    if (note_on == 0) {
      //Send note on 
      midi_note_on(0, MIDI_C, noteVelocity);
      note_on = 1;
    }
    if (noteVelocity != lastAnalog[1])  {
      //midi_note_on(0, MIDI_C, noteVelocity);
      //
      midi_key_pressure(0, MIDI_C, noteVelocity);
      //midi_controller_change(1, 0x07, noteVelocity);
      lastAnalog[1] = noteVelocity;
    }
    if (pitchBend > 16380) {
      pitchBend = 16383;
    }
    if (pitchBend != lastAnalog[0]) {

      // call ArduMIDI lib
      midi_pitch_bend(0, pitchBend); 
      lastAnalog[0] = pitchBend;

    }

    // Send Sensor data to Rainbow function LED
    rainbow(map(sensorValue[1], 0, 170, 0, LED_COUNT), map(sensorValue[0], 0, 15000, 0, 255));

  }
  else {
    //Send midi note off
    if (note_on == 1) {
      //Send note off
      midi_note_on(0, MIDI_C, 0);
      note_on = 0;
    }
    // Send Sensor data to Rainbow function LED
    rainbow(0, map(sensorValue[0], 0, 15000, 0, 255));

  }

  /*
  // print the results to the serial monitor:
   Serial.print("sensor = " );                       
   Serial.print(sensorValue[0]);      
   Serial.print("\t processed = ");      
   Serial.print(processed_value);
   Serial.print("\t \t proess time = ");      
   Serial.print(pepe2);       
   Serial.print("\t sensor 2 = ");      
   Serial.println(sensorValue[1]);   
   */

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(20);                     
}


// read the analog in value: use for loop to oversample
// call analogInPin: Analog pin
//      samples: sample itterations
//      bitshift: devisor used in averanging ( 1 - devides by 2, 2 - devides by 4, 3 - devides by 8 ect...
//      sum: previous reading to be averaged in
int analogReadOverSample(int analogInPin, int samples, int bitshift, unsigned long sum) {  
  unsigned int maxValue = 0 ;
  unsigned int sample = 0;
  for (int i = 0; i < samples; i++) {

    // read analog pin
    sample = analogRead(analogInPin); 
    // idetifyes max value to be removed
    if (sample > maxValue) {
      maxValue = sample;     
    }

    // sum readings
    sum += sample;   
  }  
  sample = (sum - maxValue) >> bitshift;  // toss out highest sample and devide with bit shift

    return sample;
} 

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void rainbow(int outValue, uint16_t j) {
  uint16_t i;

  for(i=0; i< strip.numPixels(); i++) {
    if(i < outValue){
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    else{
      strip.setPixelColor(i, (strip.getPixelColor(i-1) >> 1)& 0x7F7F7F);
    }
  }
  //Serial.print(outValue);
  //Serial.print("\t j: ");
  //Serial.println(j);

  strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait, uint8_t MaxCount) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    if(i > MaxCount){
      c=0;
    }

    delay(wait);
  }

}









