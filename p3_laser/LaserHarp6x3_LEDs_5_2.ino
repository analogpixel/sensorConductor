/* Laser Harp
 
 for the Longmont Museum sensor conductor
 
 Matt Collier &
 Luke Collier
 
 4-29-2014
 
 */
#include <ardumidi.h>

#include <Adafruit_NeoPixel.h>

#define NeoPixelPIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(123, NeoPixelPIN, NEO_GRB + NEO_KHZ800);


// Variables will change:
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
long interval = 50;           // interval at which to update chase speed (milliseconds)

int threshold = 325;             // sensor note on threashold
boolean sensorState[18] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};    // array to  store the note on state
int drivePinArray[] = {
  13, 10, 12, 9, 11, 8};   // sensor metrix drive pins in order

// Notes to play when sensors are activated. First colum is the first row of sensors. +1 is sharp & -1 is flat
int note[18] = {
  MIDI_E - 24,      MIDI_A - MIDI_OCTAVE,      MIDI_B - MIDI_OCTAVE,       MIDI_C - MIDI_OCTAVE,     MIDI_B,     MIDI_E -1,
  MIDI_G + 1 - 24,  MIDI_C + 1 - MIDI_OCTAVE,  MIDI_D + 1 - MIDI_OCTAVE,   MIDI_E - 1 - MIDI_OCTAVE, MIDI_D,     MIDI_G -1,
  MIDI_B - 12,      MIDI_E - MIDI_OCTAVE,      MIDI_F + 1 - MIDI_OCTAVE,   MIDI_G - MIDI_OCTAVE,     MIDI_F + 1, MIDI_B -1}; 
int noteVelocity = 80;

// LED on the NeoPixle to light up when Sensors are activated
int activatePixel[18] = {
  11, 25, 52, 65, 92, 105,
  13, 27, 54, 67, 94, 107,
  15, 29, 56, 69, 96, 109};

long activatePixelColor[18] = {
  0x0000ff, 0xff00ff, 0xff0000, 0xffff00, 0x00ff00, 0x888888,
  0x0000ff, 0xff00ff, 0xff0000, 0xffff00, 0x00ff00, 0x888888,
  0x0000ff, 0xff00ff, 0xff0000, 0xffff00, 0x00ff00, 0x888888};


void setup() {

  // initilize LED strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'


  // set the digital pin as output
  for(int i = 0; i <= 5; i++) {
    pinMode(drivePinArray[i], OUTPUT);  
  }
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

}

void loop()
{

  //increment thru each DigiOut Pins
  for(int driveCount = 0; driveCount <= 5; driveCount++) {
    digitalWrite(drivePinArray[driveCount], HIGH);

    // comment out all MIDI commands and uncomment all Serial commands for troubleshooting
    //Serial.print(driveCount);      //
    //Serial.print("\t");

    //Read analog pints A0, A1 & A2
    for(int readCount = 0; readCount < 3; readCount++) {
      int sensorValue = analogRead(readCount);
      int currentState = LOW;
      int sensor = driveCount * 3 + readCount;

      if (sensorValue > threshold){
        currentState = HIGH;
      }

      if (sensorState[sensor] != currentState) {   
        sensorState[sensor] = currentState;

        if(currentState == LOW) {
          // when voltage is low, the laser is blocked and note is played
          midi_note_on(6, note[sensor], noteVelocity);
          //Serial.print("o");
          
          // Sets a pixel and color based on the sensors that are blocked
          //int setThisLED = (strip.numPixels() / 3) * readCount + driveCount * 2;  //gets a pixel to set the color to based on sensor

          strip.setPixelColor(activatePixel[sensor], activatePixelColor[sensor]); // simple way to set a differant colors for each sensor that at most  
          strip.show();   // Display pixels

        }
        else {
          // voltage is high with laser is not blocked, set note off
          midi_note_off(6, note[sensor], noteVelocity);
          //Serial.print("x");
        }

        //Serial.print(sensorState[driveCount * 3 + readCount]);
        //Serial.print(" ");

      }

      // print out the value and state:
      //Serial.print(sensorValue);
      //Serial.print("\t");
    }

    delay(1);        // delay in between reads for stability

    digitalWrite(drivePinArray[driveCount], LOW);  
  }

  ////////////////////////////////
  // Prints a carage returen. comment for MIDI mode
  // Serial.println("");
  ////////////////////////////////


  // check to see if it's time to update the pixels LED strip
  // difference between the current time and last time y
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    // save the last time updated the LED
    previousMillis = currentMillis;   

    unsigned long lastPixel = strip.getPixelColor(strip.numPixels() - 1);  //saves last pixel for chase will rap around

    LEDChase(); 

    // Set the first pixel to last pixel
    strip.setPixelColor(0, (lastPixel));
    
    // ever so often devide the RGB values by 2 for a fade away effect
    for(int fadeNth = 0 ; fadeNth < strip.numPixels(); fadeNth += 30){  
      //bitshift >> 1 same as /2.  AND value with 7f7f7f to keep red bits out of green bits and green bits out of blue bits 
      strip.setPixelColor(fadeNth, (strip.getPixelColor(fadeNth) >> 1) &  0x7F7F7F); 
    } 

    strip.show();

  }

}

void   LEDChase() {
  for(int i = strip.numPixels() - 1; i >= 0; i--) {
    strip.setPixelColor(i , strip.getPixelColor(i - 1));
  }

}








