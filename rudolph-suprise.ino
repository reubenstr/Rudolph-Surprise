/* 
	Rudolph Surprise
	
	Rudolph sings a songs, get's sick, the barfs Christmas presents!
	
	
	MCU:
		Arduino Nano		
	
	5x Servos:
		Servo's moves ear, two eye lids, mouth, and barfing motion.
	
	1x LED:	
		LED in nose.
	
	Sound and melody playback provided by: https://www.arduino.cc/en/Tutorial/PlayMelody
	
	Hardware work around:
	Some servos are powered on using a relay due to overcurrent condition, 
	when attempting to provide power to all the servo's all at once.
*/

// Single in degrees relative to hardware position, requires physical calibration.
#define PRESENTS_STOP 71
#define PRESENTS_START 65//64

#define MOUTH_OPEN 120
#define MOUTH_CLOSE 164
#define MOUTH_TALK_OPEN 140
#define MOUTH_TALK_CLOSE 158

#define EAR_UP 65
#define EAR_MIDDLE 100
#define EAR_MIDDLE_TO_LOW 115
#define EAR_DOWN 145

#define EYES_TOP_OPEN 73
#define EYES_TOP_MIDDLE 82
#define EYES_TOP_CLOSE 96
#define EYES_BOTTOM_OPEN 90
#define EYES_BOTTOM_MIDDLE 85
#define EYES_BOTTOM_CLOSE 76

#define LIGHT_SENSOR_ON_STATE 100
#define BUTTON_PRESSED 0

#include <Servo.h>

Servo servoEyeTop;
Servo servoEyeBottom;
Servo servoMouth;
Servo servoEar;
Servo servoPresents;

const int lightSensorPin = A0;
const int servoActivatePin = A4;
const int nosePin = A3;
const int buttonPin = 2;

enum States {WAIT_FOR_START = 0, PRE_PLAY_MUSIC, PLAY_MUSIC, PRE_BARF, BARF};
States state = 0;

// TONES  ==========================================
// Start by defining the relationship between
//       note, period, &  frequency.
#define  c     3830    // 261 Hz 
#define  d     3400    // 294 Hz 
#define  e     3038    // 329 Hz 
#define  f     2864    // 349 Hz 
#define  g     2550    // 392 Hz 
#define  a     2272    // 440 Hz 
#define  b     2028    // 493 Hz 
#define  C     1912    // 523 Hz 
#define  R     0       // Rest

int speakerOut = A1;
int melody[] = {
  g, a, g, e, C, a, g,
  g, a, g, a, g, C, b,
  f, g, f, d, b, a, g,
  g, a, g, a, g, a, e,
};

#define EIGHTH 32
#define QUARTER 64
#define QUARTERPLUS 80
#define HALF 128
#define HALFPLUS 160 //192
#define WHOLE 192 //256

int beats[]  = {
  EIGHTH, QUARTER, EIGHTH, QUARTER, QUARTER, QUARTER, HALFPLUS,
  EIGHTH, EIGHTH, EIGHTH, EIGHTH, QUARTER, QUARTER, WHOLE,
  EIGHTH, QUARTERPLUS, EIGHTH, QUARTER, QUARTER, QUARTER, HALFPLUS,
  EIGHTH, EIGHTH, EIGHTH, EIGHTH, QUARTER, QUARTER, WHOLE,
};
int MAX_COUNT = sizeof(melody) / 2; // Melody length, for looping.

long tempo = 7500;
int pause = 1000;
int rest_count = 100;
int tone_ = 0;
int beat = 0;
long duration  = 0;

void playTone() 
{
  long elapsed_time = 0;
  if (tone_ > 0) 
  {
    while (elapsed_time < duration) 
	{
      digitalWrite(speakerOut, HIGH);
      delayMicroseconds(tone_ / 2);
      digitalWrite(speakerOut, LOW);
      delayMicroseconds(tone_ / 2);
      elapsed_time += (tone_);
    }
  }
  else // Rest beat; loop times delay
  { 
    for (int j = 0; j < rest_count; j++) 
	{
      delayMicroseconds(duration);
    }
  }
}


void setup() 
{

  // Allow the servos to settle.
  delay(1000); 

  // Relay for servo power (only power a few to prevent startup over current of system).
  pinMode(servoActivatePin, OUTPUT);
  digitalWrite(servoActivatePin, LOW);

  // Input button.
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);       // turn on pullup resistors

  // Nose led.
  pinMode(nosePin, OUTPUT);
  digitalWrite(nosePin, LOW);

  // Speaker.
  pinMode(speakerOut, OUTPUT);

  Serial.begin(9600);

  // Move servo elements to starting positions.
  digitalWrite(servoActivatePin, HIGH);
  servoMouth.write(MOUTH_CLOSE);
  servoEyeTop.write(EYES_TOP_MIDDLE);
  servoEyeBottom.write(EYES_BOTTOM_MIDDLE);
  servoEar.write(EAR_MIDDLE);
  servoPresents.write(PRESENTS_STOP);

  servoEyeTop.attach(11);
  servoEyeBottom.attach(10);
  servoMouth.attach(9);
  servoEar.attach(8);
  delay(1000);


  //  Index presents.
  while (analogRead(lightSensorPin) < LIGHT_SENSOR_ON_STATE)
  {
    servoPresents.write(PRESENTS_START);
    servoPresents.attach(12);
  }
  servoPresents.write(PRESENTS_STOP);
  servoPresents.detach();

  // Turn on nose LED.
  digitalWrite(nosePin, HIGH);

}

void loop() 
{
  /*
    // TEST CODE FOR PRESENT SERVO CONFIGURATION   
    if (digitalRead(buttonPin) == 0)
    {
		digitalWrite(servoActivatePin, HIGH);
		servoPresents.write(PRESENTS_START);
		servoPresents.attach(12);
	} 
	else 
	{
		servoPresents.write(PRESENTS_STOP);
		servoPresents.detach();
    }

    // TEST CODE TO INDEX CONFIGURATION
    if (analogRead(lightSensorPin) > LIGHT_SENSOR_ON_STATE)
    {
		tone_ = 3038;
		beat = 40; // 64 = QUARTER
		duration = beat * tempo;
		playTone();
    }
  */

	// TODO: state machine should be broken into methods();
  switch (state) 
  {
    case WAIT_FOR_START:
      
	  if (digitalRead(buttonPin) == BUTTON_PRESSED)
      {
        digitalWrite(servoActivatePin, HIGH);

        delay(250);

        state = PRE_PLAY_MUSIC;
      }
      else
      {
        // State location after barf.
        //digitalWrite(servoActivatePin, LOW);
        servoPresents.write(PRESENTS_STOP);
        servoPresents.detach();
        servoEyeTop.detach();
        servoEyeBottom.detach();
        servoMouth.detach();
        servoEar.detach();
      }

      break;

    case PRE_PLAY_MUSIC:

      // Move expression into talking position.
      servoEar.write(EAR_UP);
      servoEyeTop.write(EYES_TOP_OPEN);
      servoEyeBottom.write(EYES_BOTTOM_OPEN);
      servoMouth.write(MOUTH_CLOSE);
      servoEyeTop.attach(11);
      servoEyeBottom.attach(10);
      servoMouth.attach(9);
      servoEar.attach(8);
      delay(500); // allow servos to settle before turning on the others
      digitalWrite(servoActivatePin, HIGH);

      state = PLAY_MUSIC;
      break;

    case PLAY_MUSIC:

      for (int i = 0; i < MAX_COUNT; i++) 
	  {
        tone_ = melody[i];
        beat = beats[i];
        duration = beat * tempo;
        playTone();
        delayMicroseconds(pause);
        static bool talkState = true;

        // Move mouth as singing.
        if (talkState) 
		{
          talkState = false;
          servoMouth.write(MOUTH_TALK_OPEN);
        } 
		else 
		{
          talkState = true;
          servoMouth.write(MOUTH_TALK_CLOSE);
          // close mouth all the way on last note.
          if (i == MAX_COUNT - 1) 
		  {
			  servoMouth.write(MOUTH_CLOSE); 
		  }
        }

        // Move ear to new position on matching note.
        if (i == 13) servoEar.write(EAR_MIDDLE_TO_LOW);

      }

      state = PRE_BARF;
      break;

    case PRE_BARF:

      for (int i = 0; i < 3; i++)
      {
        tone_ = 3038;
        beat = 40; // 64 = QUARTER
        duration = beat * tempo;
        playTone();
		
        // Delay for effect.
        if (i == 1) servoEar.write(EAR_DOWN);
        if (i == 1) servoEyeTop.write(EYES_TOP_CLOSE);
        if (i == 1) servoEyeBottom.write(EYES_BOTTOM_CLOSE);
        if (i == 1) servoMouth.write(MOUTH_OPEN);
      }

      state = BARF;
      break;

    case BARF:      

      unsigned long offset = 0;
      unsigned long startTime = millis();
      int barfCount = 0;

      while (1)
      {
        duration = 10000;
        tone_ = 3038 + offset;
        playTone();
        offset = offset + 200;

        if (offset > 4000) offset = 4000;

        // Delay start barfing.
        if (offset > 600)
        {
          servoPresents.write(PRESENTS_START);
          servoPresents.attach(12);
        }

        // Give time for presents servo to move before checking light sensor state.
        if (millis() > startTime + 1000) // ~one second delay
        {
          // Check for presents to reach stoping index (light sensor).
		  // Check if barf action performed twice.
          int adcRead = analogRead(lightSensorPin); 
          if (adcRead > LIGHT_SENSOR_ON_STATE) 
          {
			if (barfCount < 2) 
			{
			  barfCount++;
			  startTime = millis();
			} 
			else
			{            
				Serial.println("Barf over");
				servoPresents.write(PRESENTS_STOP);
				servoPresents.detach();
				
				// Return expression back to normal.
				servoMouth.write(MOUTH_CLOSE);
				servoEyeTop.write(EYES_TOP_MIDDLE);
				servoEyeBottom.write(EYES_BOTTOM_MIDDLE);
				servoEar.write(EAR_MIDDLE);
				digitalWrite(nosePin, LOW);
				delay(500); // Allow time for servos to move before detaching in next state.
				digitalWrite(nosePin, HIGH);
				state = WAIT_FOR_START;
				break;
			}
          }
        }
      }
      break;
  }
}