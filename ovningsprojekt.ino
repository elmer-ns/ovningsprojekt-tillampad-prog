/*
* Name: övningsprojekt
* Author: Elmer Nolgren Saldert
* Date: 2025-11-11
* Description: This project uses a ds3231 to measure time and displays the time to an 1306 oled display in the form of an analog clock, 
* Further, it measures temperature with ds3231 and displays a mapped value to a 9g-servo-motor, together with the raw value on the oled display. 
* It also writes the time to an 8 segment display in the format hh:mm:ss.
*/

// Include Libraries
#include <RTClib.h>
#include <Wire.h>
#include <U8glib.h>
#include <Servo.h>
#include <LedControl.h>

// Construct objects
RTC_DS3231 rtc;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
LedControl lc = LedControl(9, 10, 11, 2);
Servo servo;

void setup() {
  // init communication
  Serial.begin(9600);
  Wire.begin();

  // Init Hardware
  rtc.begin();
  lc.shutdown(0, false);

  // Settings
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  u8g.setFont(u8g_font_6x12);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  servo.attach(6);  
}

void loop() {
  // Get time and temperature from DS3231
  DateTime now = rtc.now();
  float temp = getTemp();

  // Write time and temperature to screen, servo and 8 segment display
  drawClock(now, temp);
  servoWrite(temp);
  writeTime(now);

  // Print time to serial monitor
  Serial.println(formatTime(now));

  delay(200);
}

//This function takes a DateTime object and return in formated as hh:mm:ss as a String
//Parameters: DateTime time
//Returns: time in hh:mm:ss as String
String formatTime(DateTime time) {
  return (
    (time.hour() < 10 ? "0" : "") + String(time.hour())) + ":" 
    + ((time.minute() < 10 ? "0" : "") + String(time.minute())) + ":" 
    + ((time.second() < 10 ? "0" : "") + String(time.second()));
}

/*
* This function takes temperature from ds3231 and returns as a float
*Parameters: Void
*Returns: temperature as float 
*/
float getTemp() {
  return rtc.getTemperature();
}

/*
* This function takes a DateTime object and a temperature, and draws an analog clock together with the temperature on the u8g display
*Parameters: - DateTime time, float temp
*Returns: void
*/
void drawClock(DateTime time, float temp) {
  // Define constants
  const int CLOCK_SIZE = 24;
  const int CLOCK_X = 32;
  const int CLOCK_Y = 32;

  const int HOUR_HAND = 14;
  const int MIN_HAND = 16;
  const int SEC_HAND = 18;

  // Get components of DateTime

  float second = time.second();
  float minute = time.minute() + second / 60.0;
  float hour = time.hour() + minute / 60.0;

  // Calculate hand angles

  float hours_angle = (2.0 * PI * hour / 12.0) - HALF_PI;
  float minutes_angle = (2.0 * PI * minute / 60.0) - HALF_PI;
  float seconds_angle = (2.0 * PI * second / 60.0) - HALF_PI;

  // Draw on the screen
  u8g.firstPage();
  do {
    {  // Draw clock
      u8g.drawCircle(CLOCK_X, CLOCK_Y, CLOCK_SIZE);
      u8g.drawCircle(CLOCK_X, CLOCK_Y, 1);

      // Hour hand
      u8g.drawLine(CLOCK_X, CLOCK_Y, CLOCK_X + cos(hours_angle) * HOUR_HAND, CLOCK_Y + sin(hours_angle) * HOUR_HAND);

      // Minute hand
      u8g.drawLine(CLOCK_X, CLOCK_Y, CLOCK_X + cos(minutes_angle) * MIN_HAND, CLOCK_Y + sin(minutes_angle) * MIN_HAND);

      // Second hand
      u8g.drawLine(CLOCK_X, CLOCK_Y, CLOCK_X + cos(seconds_angle) * SEC_HAND, CLOCK_Y + sin(seconds_angle) * SEC_HAND);

      for (int i = 0; i < 12; i++) {
        float angle = 2.0 * PI * i / 12.0;

        u8g.drawLine(CLOCK_X + sin(angle) * (CLOCK_SIZE - 1), CLOCK_Y - cos(angle) * (CLOCK_SIZE - 1), CLOCK_X + sin(angle) * (CLOCK_SIZE + 1), CLOCK_Y - cos(angle) * (CLOCK_SIZE + 1));
      }
    }
    {  // Draw temp
      String str = String(temp, INT1) + char(176) + "C";
      u8g.drawStr(72, 32, str.c_str());
    }
  } while (u8g.nextPage());
}

/*
* takes a temperature value and maps it to corresppnding degree on a servo
*Parameters: - value: temperature
*Returns: void
*/
void servoWrite(float value) {
  servo.write(179 - map(value, 0, 35, 0, 179));
}

/*
* Takes a DateTime object and writes it to the 8 segment display
*Parameters: - DateTime time: 
*Returns: void
*/
void writeTime(DateTime time) {
  lc.setDigit(0, 0, time.second() % 10, false);
  lc.setDigit(0, 1, time.second() / 10, false);

  lc.setDigit(0, 3, time.minute() % 10, false);
  lc.setDigit(0, 4, time.minute() / 10, false);

  lc.setDigit(0, 6, time.hour() % 10, false);
  lc.setDigit(0, 7, time.hour() / 10, false);
}