#include <Time.h>
#include <TimeLib.h>
#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include<LiquidCrystal.h>

// LCD
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);  
int contrast=0;

// GPS
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
SoftwareSerial gps_ss(RXPin, TXPin);

// Time
int hourSetup    = 0;
int minuteSetup  = 0;
int secondSetup  = 0;
int daySetup     = 1;
int monthSetup   = 1;
int yearSetup    = 1970;

// Support Char
char buf1[20];
char buf2[20];
time_t prevDisplay = 0; // when the digital clock was displayed

// Print 
static const unsigned long REFRESH_INTERVAL = 2000; // ms
static unsigned long lastRefreshTime = 0;

bool printTime = true;
bool printPos  = false;
bool printStat = false;

double divideHdopByOneHundred(double hdopIn)
{
  return hdopIn/100;
}

void lcd_print_time()
{
  sprintf(buf1, "%02d:%02d:%02d  ", hour(), minute(), second());
  sprintf(buf2, "%02d/%02d/%02d", day(),month(), year());
  lcd.setCursor(0, 0);
  lcd.print(F("Time: "));
  lcd.println(buf1);
 
  lcd.setCursor(0, 1);
  lcd.print(F("Date: "));
  lcd.println(buf2);
  
}
void lcd_print_coord(double latitude, 
                     double longitude,
                     double altitude)
{
  sprintf(buf1, "Lat:%s| Alt: ", String(latitude,2).c_str());
  sprintf(buf2, "Lon:%s| %s ", String(longitude,2).c_str(),
                              String(altitude,1).c_str());
   
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(buf1);
  
  lcd.setCursor(0, 1);
  lcd.print(buf2);
  return;
}

void lcd_gps_status()
{
  sprintf(buf1, "Alt: %s", String(gps.altitude.meters(),1).c_str());
  sprintf(buf2, "HDOP: %s, Sat:%s", String(divideHdopByOneHundred(gps.hdop.value()),1).c_str(), String(gps.satellites.value()).c_str());
   
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(buf1);
  
  lcd.setCursor(0, 1);
  lcd.print(buf2);
  return;
}

void setUpdateTime(int hourSetup,
                   int minuteSetup,
                   int secondSetup,
                   int daySetup,
                   int monthSetup,
                   int yearSetup)
{
  setTime(hourSetup, 
          minuteSetup, 
          secondSetup, 
          daySetup, 
          monthSetup, 
          yearSetup);
  //adjustTime(offset * SECS_PER_HOUR);
}

void lcd_status()
{
  lcd.clear();
  lcd.print("Message Sent");
  delay(REFRESH_INTERVAL);
  lcd.clear();
  lcd.print("System Ready");
  return;
}

void setup(){
  Serial.begin(9600);
  gps_ss.begin(GPSBaud);
  // Setup Time
  setUpdateTime(hourSetup, 
                minuteSetup, 
                secondSetup, 
                daySetup, 
                monthSetup, 
                yearSetup);
  
  // LCD init
  lcd.begin(16,2);
  pinMode(13,OUTPUT);
  analogWrite(6,contrast);
  lcd.setCursor(0, 0);
  lcd.print("Vehicle Tracking");
  lcd.setCursor(0,1);
  lcd.print("    System  Init");
  delay(REFRESH_INTERVAL/2);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("by Matteo");
  lcd.setCursor(0,1);
  lcd.print("       Bartolini");
  delay(REFRESH_INTERVAL/2);
  
  // GSM init
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GSM Init");
  delay(REFRESH_INTERVAL/2);
  lcd.setCursor(0, 1);
  lcd.print("GSM Init - DONE");
  delay(REFRESH_INTERVAL/2);
 
  
  // GPS init
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.println("AT+CNMI=2,2,0,0,0");
  lcd.print("GNSS Init");
  delay(REFRESH_INTERVAL/2);
  lcd.setCursor(0, 1);
  lcd.print("GNSS Range Found");
  lcd.setCursor(0,1);
  lcd.print("GNSS Init - DONE");
  delay(REFRESH_INTERVAL/2);
  
  lcd.print("System Ready");
}

void adjustContrast()
{
  //Lets setup the contrast depending on the time
  if ((hour() >= 18) or (hour() < 07))
  {
    contrast = 35;  
  } 
  else
  {
    contrast = 0;
  }
  analogWrite(6,contrast);
}

void loop()
{
  // put your main code here, to run repeatedly:  
  // Displays information when new sentence is available.
  while (gps_ss.available() > 0)
  {
    gps.encode(gps_ss.read());

    // Let's display the new location and altitude
    // whenever either of them have been updated.
    if (gps.location.isUpdated() || gps.altitude.isUpdated())
    {
      Serial.print("Location: "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(",");
      Serial.print(gps.location.lng(), 6);
      Serial.print("  Altitude: ");
      Serial.println(gps.altitude.meters());
      Serial.print("  HDOP: ");
      Serial.println(divideHdopByOneHundred(gps.hdop.value()));
      Serial.print("  Satellites: ");
      Serial.println(gps.satellites.value());
      setUpdateTime(gps.time.hour(), 
                    gps.time.minute(), 
                    gps.time.second(), 
                    gps.date.day(), 
                    gps.date.month(), 
                    gps.date.year());
    }  
  }



  if(millis() - lastRefreshTime >= REFRESH_INTERVAL)
  {
    lastRefreshTime += REFRESH_INTERVAL;
    adjustContrast();
    //lcd_print_time();
    //lcd_print_coord(gps.location.lat(), gps.location.lng());
    //lcd_gps_status();
    if ( printTime == true)
    {
      lcd_print_time();
      printTime = false;
      printPos  = true;
      printStat = false;
    }
    else if ( printPos == true)
    {
      lcd_print_coord(gps.location.lat(), 
                      gps.location.lng(),
                      gps.altitude.meters());
      printTime = false;
      printPos  = false;
      printStat = true;
    }
    else if ( printStat == true)
    {
      lcd_gps_status();
      printTime = true;
      printPos  = false;
      printStat = false;
    }  
  }
}
