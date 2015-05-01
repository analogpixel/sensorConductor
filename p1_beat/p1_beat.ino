#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// Global
const int SERIALSPEED						= 9600;
boolean DEBUG                   = false;

// This code is for the mirror
const int INFM_NUMLEDS					= 152;
const int INFM_PIN							= 6;
Adafruit_NeoPixel INFM_RING			= Adafruit_NeoPixel( INFM_NUMLEDS, INFM_PIN, NEO_RGB + NEO_KHZ800);
int INFM_X											= 0;
int INFM_R1_MIN									= 0;
int INFM_R1_MAX									= 41;
int INFM_R2_MIN									= 41;
int INFM_R2_MAX									= 82;
int INFM_R3_MIN									= 82;
int INFM_R3_MAX									= 123;

// This code is for the neopixel rings
const int NEO_NUMLEDS						= 60;
const int NEO_RINGPIN						= 2;
const int NEO_FADEMAX						= 70;
const int NEO_FADEMIN						= 20;
const int NEO_FADEJUMP					= 1;
const int NEO_SWITCHES[]				= { 8, 9, 10, 11, 12};
boolean NEO_SWITCHES_STATE[]    = { 0, 0, 0, 0, 0};
const int NEO_NUMSWITCHES				= 5;
boolean NEO_ALLOFF							= false; // used to determine if all the beat are off
int NEO_I												= 0;
uint32_t NEO_C									= 0;
int NEO_FADER										= 0;
int NEO_FADEINC									= 1;
Adafruit_NeoPixel NEO_RING			= Adafruit_NeoPixel( NEO_NUMLEDS, NEO_RINGPIN, NEO_RGB + NEO_KHZ800);

// this code is for the BPM wheel
const int WHEEL_PIN							= A0; // the pin that the photcell is plugged into
const int WHEEL_TRIGGER					= 800; // what value it triggers at
boolean WHEEL_STATE							= false; // is the wheel lit or unlit
unsigned long WHEEL_TIMER				= 0;
unsigned long WHEEL_ELAPSE			= 0;
int WHEEL_BPM										= 60;
int WHEEL_CURRENTBPM						= 60;
int WHEEL_SENSORVALUE						= 0;
unsigned long WHEEL_HOLDTIME		= 0;

// working variables

void setupMirror() {
  pinMode( INFM_PIN, OUTPUT);
  INFM_RING.begin();
  INFM_RING.show();
}

void setupWheel() {
  pinMode(WHEEL_PIN, INPUT_PULLUP);
  WHEEL_TIMER = millis();
}

void setupNeopixels() {
  for (int pinset = 0; pinset < NEO_NUMSWITCHES; pinset++) {
    pinMode( NEO_SWITCHES[pinset], INPUT);
  }

  pinMode( NEO_RINGPIN, OUTPUT);
  NEO_RING.begin();
  NEO_RING.show();
}

void loopWheel() {
  WHEEL_SENSORVALUE = analogRead(WHEEL_PIN);



  if (WHEEL_SENSORVALUE > WHEEL_TRIGGER && WHEEL_STATE == false) {
		if (DEBUG) { Serial.println("Trigger On"); }
    WHEEL_TIMER = millis();
    WHEEL_STATE = true;
  }

  if (WHEEL_SENSORVALUE < WHEEL_TRIGGER && WHEEL_STATE == true) {
		if (DEBUG) { Serial.println("Trigger Off"); }
    WHEEL_ELAPSE = millis() - WHEEL_TIMER; // time it took to travel
		if ( DEBUG) { Serial.print("Elapse Time:"); Serial.println( WHEEL_ELAPSE ); }
    WHEEL_STATE = false;
    WHEEL_BPM = map( constrain( WHEEL_ELAPSE, 70, 800) , 70, 800, 180, 60);
    MIDI.sendControlChange(96, map(WHEEL_CURRENTBPM, 60, 180, 0, 127), 14);
  }

  if ( millis() > WHEEL_HOLDTIME ) {
    if (WHEEL_BPM > WHEEL_CURRENTBPM) {
      WHEEL_CURRENTBPM++;
    }
    if (WHEEL_BPM < WHEEL_CURRENTBPM) {
      WHEEL_CURRENTBPM--;
    }
    WHEEL_CURRENTBPM = constrain( WHEEL_CURRENTBPM, 60, 180);
    if ( DEBUG) { Serial.print("Current BPM:"); Serial.println( WHEEL_CURRENTBPM ); }
    if ( DEBUG) { Serial.print("Acutal  BPM:"); Serial.println( WHEEL_BPM ); }
		if (DEBUG) {Serial.println( WHEEL_SENSORVALUE );}

		WHEEL_HOLDTIME = millis() + 300;
  }

}

void loopMirror() {
  // set left side
  int t = map( WHEEL_CURRENTBPM, 60, 180, INFM_R2_MIN, INFM_R2_MAX);
  for (int i=INFM_R2_MIN ; i < INFM_R2_MAX  ; i++) {
		if (i < t) {
			INFM_RING.setPixelColor( i, INFM_RING.Color(255,0,0));
		} else {
			INFM_RING.setPixelColor(i, INFM_RING.Color(0,255,0) );
		}
	}

  // set right side
  t = int(map( WHEEL_CURRENTBPM, 60, 180, INFM_R3_MAX, INFM_R3_MIN));

  for (int i=INFM_R3_MIN ; i < INFM_R3_MAX  ; i++) {
		if (i > t) {
			INFM_RING.setPixelColor( i, INFM_RING.Color(255,0,0));
		} else {
			INFM_RING.setPixelColor(i, INFM_RING.Color(0,255,0) );
		}
	}

	// set the bottom left half
  t = int(map( WHEEL_CURRENTBPM, 60, 180, INFM_R1_MAX, INFM_R1_MAX/2));

  for (int i=int(INFM_R1_MAX/2) ; i < INFM_R1_MAX  ; i++) {
		if (i > t) {
			INFM_RING.setPixelColor( i, INFM_RING.Color(255,0,0));
		} else {
			INFM_RING.setPixelColor(i, INFM_RING.Color(0,255,0) );
		}
	}

	// set the bottom right half
  t = int(map( WHEEL_CURRENTBPM, 60, 180, INFM_R1_MIN, INFM_R1_MAX/2));

  for (int i=INFM_R1_MIN ; i < int(INFM_R1_MAX/2)  ; i++) {
		if (t > i) {
			INFM_RING.setPixelColor( i, INFM_RING.Color(255,0,0));
		} else {
			INFM_RING.setPixelColor(i, INFM_RING.Color(0,255,0) );
		}
	}


	INFM_RING.show();

}

void loopNeopixels() {
  NEO_I++;
  NEO_C++;

  if (NEO_C > 255) {
    NEO_C = 0;
    NEO_FADER += 1;
  }
  if (NEO_I > 11) {
    NEO_I = 0;
  }
  NEO_ALLOFF = true;

  // loop through all the switches and find out if any are on
  // if one is on, then light it up
  for (int z = 0; z < NEO_NUMSWITCHES; z++) {
    if ( digitalRead( NEO_SWITCHES[z] )) {
      NEO_RING.setPixelColor( (z * 12) + NEO_I, Wheel(NEO_C));
      NEO_ALLOFF = false;

			if (NEO_SWITCHES_STATE[z] == 0) {
				MIDI.sendControlChange(10 + z , 127 , 14);
				NEO_SWITCHES_STATE[z] = 1;
			}

    } else {
			if (NEO_SWITCHES_STATE[z] == 1) {
				MIDI.sendControlChange(10 + z, 0, 14);
				NEO_SWITCHES_STATE[z] = 0;
			}
		}
  }

  if (NEO_ALLOFF) {
    NEO_FADER += (NEO_FADEINC * NEO_FADEJUMP);
    if (NEO_FADER > NEO_FADEMAX) {
      NEO_FADER = NEO_FADEMAX;
      NEO_FADEINC = -1;
    }
    if (NEO_FADER < NEO_FADEMIN) {
      NEO_FADER = NEO_FADEMIN;
      NEO_FADEINC = 1;
    }
    NEO_RING.setBrightness(int (NEO_FADER));
    solidFill( NEO_RING.Color(255, 100, 255), 0, 60);
    delay(20);
  } else {
    NEO_RING.setBrightness(255);
    NEO_RING.show();
    delay(20);
    solidFill( NEO_RING.Color( 0, 0, 0), 0, 60);
  }

}


void solidFill( uint32_t c, int pixStart, int pixEnd) {
  for (int i = pixStart; i < pixEnd; i++) {
    NEO_RING.setPixelColor(i, c);
  }

  NEO_RING.show();
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return NEO_RING.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return NEO_RING.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return NEO_RING.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}


void setup() {
  MIDI.begin();
  Serial.begin(SERIALSPEED);
  setupNeopixels() ;
  setupWheel();
  setupMirror();
}


void loop() {
  loopNeopixels();
  loopWheel();
  loopMirror();
}
