// Ultrasonic sensor pins
const int trigPin = A5;
const int echoPin = A4;

long duration;
int distance;

#include <IRremote.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
////////// IR REMOTE CODES //////////
#define F 16736925 // FORWARD
#define B 16754775 // BACK
#define L 16720605 // LEFT
#define R 16761405 // RIGHT
#define S 16712445 // STOP
#define K1 16738455 // 1
#define K2 16750695 // 2
#define K3 16756815 // 3
#define K4 16724175 // 4
#define K5 16718055 // 5
#define K6 16743045 // 6
#define K7 16716015 // 7
#define K8 16726215 // 8
#define K9 16734885 // 9
#define K_STAR 16728765 // *
#define K0 16730805 // 0
#define K_HASH 16732845 // #

#define UNKNOWN_F 5316027    // FORWARD
#define UNKNOWN_B 2747854299 // BACK
#define UNKNOWN_L 1386468383 // LEFT
#define UNKNOWN_R 553536955  // RIGHT
#define UNKNOWN_S 3622325019 // STOP

#define RECV_PIN 12
#define PIN 10
#define NUMPIXELS 3
Adafruit_NeoPixel pixels =
    Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define PIN 10
/*define channel enable output pins*/
#define ENA 5 // Left  wheel speed
#define ENB 6 // Right wheel speed
/*define logic control output pins*/
#define IN1 7        // Left  wheel forward
#define IN2 8        // Left  wheel reverse
#define IN3 9        // Right wheel reverse
#define IN4 11       // Right wheel forward
#define carSpeed 255 // initial speed of car >=0 to <=255

IRrecv irrecv(RECV_PIN);
decode_results results;
unsigned long val;
unsigned long preMillis;

/**
 * BEGIN DEFINE FUNCTIONS
 */

void forward() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // Serial.println("go forward!");
}
void back() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Serial.println("go back!");
}
void left() {
  analogWrite(ENA, 255);
  analogWrite(ENB, 255);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // Serial.println("go left!");
}
void right() {
  analogWrite(ENA, 255);
  analogWrite(ENB, 255);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Serial.println("go right!");
}
void stop() {
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  // Serial.println("STOP!");
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  stop();
  irrecv.enableIRIn();
#if defined(__AVR_ATtiny85__)
  if (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
}

// #define BRIGHTNESS 150
#define BRIGHTNESS 3
const int blinkPeriod = 1000;
bool isPolice = false;

int nearby_seq_len = 0;

void loop() {
  if ((millis() % blinkPeriod) == 0) {
    if (isPolice) {
      pixels.setPixelColor(0, pixels.Color(0, 0, BRIGHTNESS));
      pixels.setPixelColor(1, pixels.Color(BRIGHTNESS, 0, 0));
    } else {
      pixels.setPixelColor(0, pixels.Color(0, BRIGHTNESS, 0));
      pixels.setPixelColor(1, pixels.Color(0, BRIGHTNESS, 0));
    }

    pixels.show(); // This sends the updated pixel color to the hardware.
  } else if ((millis() % blinkPeriod) == (blinkPeriod / 2)) {
    if (isPolice) {
      pixels.setPixelColor(1, pixels.Color(0, 0, BRIGHTNESS));
      pixels.setPixelColor(0, pixels.Color(BRIGHTNESS, 0, 0));
    } else {
      pixels.setPixelColor(0, pixels.Color(0, BRIGHTNESS, 0));
      pixels.setPixelColor(1, pixels.Color(0, BRIGHTNESS, 0));
    }

    pixels.show(); // This sends the updated pixel color to the hardware.
  }

  if (irrecv.decode(&results)) {
    preMillis = millis();
    val = results.value;
    irrecv.resume();
    if (isPolice) {
      // Usual buttons
      switch (val) {
      case F:
      case UNKNOWN_F:
        forward();
        break;
      case B:
      case UNKNOWN_B:
        back();
        break;
      case L:
      case UNKNOWN_L:
        left();
        break;
      case R:
      case UNKNOWN_R:
        right();
        break;
      case S:
        stop();
        break;
      default:
        break;
      }
    } else {
      // Shift to the bottom 4 buttons: 5 -> forward, 0 -> back, etc.
      switch (val) {
      case K5:
        forward();
        break;
      case K0:
        back();
        break;
      case K6:
        left();
        break;
      case K8:
        right();
        break;
      case K7:
        stop();
        break;
      default:
        break;
      }
    }
  } else {
    if (millis() - preMillis > 500) {
      stop();
      preMillis = millis();
    }
  }
  if (isPolice)
    return;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;
  if (distance < 40) {
    nearby_seq_len++;
  }
  /* Requires the cars to be close for 500 loop ticks before becoming a police
   * car, This fixes a bug where it would become a police car when reversing its
   * direction
   */
  if (nearby_seq_len > 500) {
    isPolice = true;
  }
}
