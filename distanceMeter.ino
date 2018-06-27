#include <Arduino.h>
#include <U8x8lib.h>
#include <NewPing.h>
#include <Timers.h>

#define TRIGGER_PIN  10
#define ECHO_PIN     11
#define MAX_DISTANCE 200
#define US_ROUNDTRIP_CM 57.0

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#define LED_PIN 13
#define BUTTON_PIN 2

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 12, /* data=*/ A3, /* reset=*/ U8X8_PIN_NONE);

float measure = 0;
static unsigned long last_interrupt_time = 0;
int elapsedSeconds = 0;
String lines[] = {
      "1. -",
      "2. -",
      "3. -",
      "4. -"
  };
int currentLine = 1;
int numberOfMeasurment = 0;

Timer timer;
Timer measureTime;

void timerInterrupt() {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  elapsedSeconds++;
  if(elapsedSeconds > 999) elapsedSeconds = 1;
}

void start() {
  u8x8.drawString(3, 1, "Miernik");
  u8x8.drawString(2, 2, "Odleglosci");
  u8x8.drawString(2, 4, "Wlaczanie");

  for (int i = 0; i < 3; i++) {
    u8x8.drawString(11, 4, ".  ");
    for (int j = 11; j < 11 + 3; j++) {
      u8x8.drawString(j, 4, ".");
      delay(250);
    }
  }
}

float measureDistance() {
  float measure = sonar.ping() / US_ROUNDTRIP_CM;
  if(measure > MAX_DISTANCE) measure = 0;
  
  return measure;
}

char* floatToString(float x, byte precision = 2) {
  char tmp[8];
  dtostrf(x, 0, precision, tmp);
  sprintf(tmp, "%scm ", tmp);
  return tmp;
}

char* elapsedTimeChar() {
  volatile char tmp[7];    
  itoa((int)elapsedSeconds, tmp, 10);
  sprintf(tmp, "%ss  ", tmp);
  return tmp;
}

void buttonPushed() {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      numberOfMeasurment++;
      lines[currentLine - 1] = String(numberOfMeasurment) + "." + String(measure) + "cm T:" + String(elapsedSeconds) + "s  ";
      currentLine++;
      if(currentLine > 4) currentLine = 1;
      elapsedSeconds = 0;
    }
  }
  last_interrupt_time = interrupt_time;
}

void drawDestriptions() {
  uint8_t tiles[128];
  for (int i = 0; i < 128; i++) tiles[i] = 16;
  u8x8.drawTile(0, 1, 16, tiles);
  u8x8.drawString(0, 2, "Measures:");
  u8x8.setInverseFont(1);
  u8x8.drawString(0, 0, "Dist:");
  u8x8.setInverseFont(0);
  u8x8.drawString(0, 7, "Timer:");
}

void setup(void)
{
  Serial.begin(9600);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  NewPing::timer_ms(1000, timerInterrupt);
  
  measureTime.begin(200);

  u8x8.begin();
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
  u8x8.setPowerSave(0);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPushed, CHANGE);
  start();
  u8x8.clear();
  elapsedSeconds = 0;
  
  drawDestriptions();
}

void loop(void)
{
    if (measureTime.available()) {
      measure = measureDistance();
      u8x8.setInverseFont(1);
      u8x8.drawString(5, 0, floatToString(measure));
      u8x8.setInverseFont(0);
      u8x8.drawString(6, 7, elapsedTimeChar());
      for(int i = 0; i < 4; i++) {
        char tmp[16];
        lines[i].toCharArray(tmp, 16);
        u8x8.drawString(0, i + 3, tmp);
      }
      measureTime.restart();
    }
}
