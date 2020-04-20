/*
The rpm sensor monitor the time elapsed between two adjacent magnet detection.
The elapsed time is then converted to the revolution speed and display on both the serial port and the LCD.
If there is no magnet being detected over a certain period of time, the rpm reading is set to zero.

The code can read up to approximately 1200RPM (tested on Arduino NANO).
The minimum reading is approximately 30RPM which is defined by time_resetIfNoReading.
Increase time_resetIfNoReading to reduce the minimum reading range, but this will lengthen the reset time.

More magnets may be distributed evenly on the rotating device in order to get more accurate reading.
In this regard, the number of the magnet in the code needs to be specified accordingly.
*/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define sensorPin 2
bool pinStatus_last = HIGH;
unsigned long time_ref = micros();
unsigned long time_resetIfNoReading = 2;  // [second] the frequency is set to zero if no reading is detected withing this many seconds
float HZ_now = 0.0;
float HZ_filtered = 0.0;
int N_magnet = 1;  // number of the magnets on the rotating device.

float display_interval = 1;  // [sec]
unsigned long time_ref_display = millis();

// for filtering the reading
float alpha = 0.2;  // \in[0,1] the gain for a low-pass filter

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(sensorPin, INPUT_PULLUP);

  // initialize the LCD
  lcd.begin();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Mag. Number: ");
  lcd.setCursor(13, 0);
  lcd.print(N_magnet);
  lcd.setCursor(0, 1);
  lcd.print("RPM: 0");
}

void loop() {
  bool pinStatus = digitalRead(sensorPin);

  // detect the pin status change and measure the frequency of magnet detection
  if ((pinStatus != pinStatus_last) && pinStatus==LOW){
    unsigned long time_detected = micros();
    unsigned long time_elapsed = time_detected - time_ref;
    time_ref = time_detected;
    HZ_now = 1/(float(time_elapsed)/1000000);
  }
  pinStatus_last = pinStatus;
  // if no magnet is detected over a period of time, then set the frequency to zero
  if(micros()-time_ref > time_resetIfNoReading*1000000)
  {
    HZ_now = 0.0;
    HZ_filtered = 0.0;
  }

  // Apply a low-pass filter to the frequency reading and compute the RPM accordingly
  HZ_filtered = alpha*HZ_now + (1-alpha)*HZ_filtered;

  float RPM = HZ_filtered * float(60);

  // display the RPM on the serial monitor at a specific interval
  // this section may be altered with teh LCD display
  if (millis()-time_ref_display > display_interval*1000)
  {
    time_ref_display = millis();
    Serial.println(RPM);

    lcd.setCursor(5, 1);
    lcd.print("      ");
    lcd.setCursor(5, 1);
    lcd.print(RPM);
  }

}
