#include <Wire.h>
#include <RtcDS3231.h> // Include RTC library by Makuna: https://github.com/Makuna/Rtc
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
//#include <time.h>
#include <ESP8266HTTPUpdateServer.h>
#include "Credentials.h"        // Create this file in the same directory as the .ino file and add your credentials (#define SID YOURSSID and on the second line #define PW YOURPASSWORD)

RtcDS3231<TwoWire> Rtc(Wire);
#include <FS.h>
#define countof(a) (sizeof(a) / sizeof(a[0]))

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdateServer;

//float timeZone = 1.0;                               // Change this value to your local timezone (in my case +1 for Amsterdam)
//const int DAY_LIGHT_OFFSET_SECONDS = 3600;          // Daylight saving
//const char *TIME_SERVER = "pool.ntp.org";
//const int EPOCH_1_1_2019 = 1546300800;
//time_t now;

// FastLED 
#include <FastLED.h>

#define NUM_LEDS 86 //  86     
#define DATA_PIN D6 // Change this if you are using another type of ESP board than a WeMos D1 Mini
CRGB LEDs[NUM_LEDS];
#define MILLI_AMPS 2400 

long numbers[] = {
  0b000111111111111111111,  // [0] 0
  0b000111000000000000111,  // [1] 1
  0b111111111000111111000,  // [2] 2
  0b111111111000000111111,  // [3] 3
  0b111111000111000000111,  // [4] 4
  0b111000111111000111111,  // [5] 5
  0b111000111111111111111,  // [6] 6
  0b000111111000000000111,  // [7] 7
  0b111111111111111111111,  // [8] 8
  0b111111111111000111111,  // [9] 9
  0b000000000000000000000,  // [10] off
  0b111111111111000000000,  // [11] degrees symbol
  0b000000111111111111000,  // [12] C(elsius)
  0b111000111111111000000,  // [13] F(ahrenheit)
};

unsigned long prevTime = 0;

const char* APssid = "CLOCK_AP";
const char* APpassword = "1234567890";
const char *ssid = SID;
const char *password = PW;

// Settings
byte r_val = 255;
byte g_val = 0;
byte b_val = 0;
bool dotsOn = true;
byte brightness = 255;
float temperatureCorrection = -3.0;
byte temperatureSymbol = 12;

byte clockMode = 0; // 0=Clock, 1=Countdown, 2=Temperature
unsigned long countdownMilliSeconds;
unsigned long endCountDownMillis;
CRGB countdownColor = CRGB::Green;

/* TODO:
 *  - add a reset button where wifiManager can be resetted
 *  done - test if it also works with a mobile phone hotspot and another phone that connects (or same?)
 *  done - upload files http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system
 *  done - fix summerTime
 *  - also add GET and implement for rgb values r_val etc 
 *  - add buzzer at end of countdown
 *  done - temp.
 */

void setup() {
  Serial.begin(115200); 
  delay(200);

  // RTC DS3231 Setup
  Rtc.Begin();    
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid()) {
      if (Rtc.LastError() != 0) {
          // we have a communications error see https://www.arduino.cc/en/Reference/WireEndTransmission for what the number means
          Serial.print("RTC communications error = ");
          Serial.println(Rtc.LastError());
      } else {
          // Common Causes:
          //    1) first time you ran and the device wasn't running yet
          //    2) the battery on the device is low or even missing
          Serial.println("RTC lost confidence in the DateTime!");
          // following line sets the RTC to the date & time this sketch was compiled
          // it will also reset the valid flag internally unless the Rtc device is
          // having an issue
          Rtc.SetDateTime(compiled);
      }
  }

  WiFi.setSleepMode(WIFI_NONE_SLEEP);  

  delay(200);
  //Serial.setDebugOutput(true);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(LEDs, NUM_LEDS);  
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalLEDStrip);
  //FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(LEDs, NUM_LEDS, CRGB::Black);
  FastLED.show();

  SPIFFS.begin();

//  // Local intialization. Once its business is done, there is no need to keep it around
//  WiFiManager wifiManager;
//  
//  // !! Uncomment and run it once, if you want to erase all the stored information
//  //wifiManager.resetSettings();
//  
//  wifiManager.autoConnect("ClockAP");
  
  // If you get here you have connected to the WiFi
//  Serial.println("Connected.");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(APssid, APpassword);    // IP will be 192.168.4.1
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
   }
  
  Serial.println("");
  Serial.println("WiFi connected");

   // Print both IP addresses, softAPIP is usually 192.168.4.1
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());

//  configTime(timeZone, DAY_LIGHT_OFFSET_SECONDS, TIME_SERVER);
//
//  while (now < EPOCH_1_1_2019)
//  {
//    now = time(nullptr);
//    delay(500);
//    Serial.print("*");
//  }
  
  httpUpdateServer.setup(&server);

//  server.on("/", HTTP_GET, []() {
//    String s = "<html>test</html>"; //MAIN_page; 
//    server.send(200, "text/html", s);
//  });

  server.on("/color", HTTP_POST, []() {    
    r_val = server.arg("r").toInt();
    g_val = server.arg("g").toInt();
    b_val = server.arg("b").toInt();
    server.send(200, "text/json", "{ok}");
  });

  server.on("/setdate", HTTP_POST, []() { 
    // Sample input: date = "Dec 06 2009", time = "12:34:56"
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    String datearg = server.arg("date");
    String timearg = server.arg("time");
    Serial.println(datearg);
    Serial.println(timearg);    
    char d[12];
    char t[9];
    datearg.toCharArray(d, 12);
    timearg.toCharArray(t, 9);
    RtcDateTime compiled = RtcDateTime(d, t);
    Rtc.SetDateTime(compiled);   
    clockMode = 0;     
    server.send(200, "text/json", "{ok}");
  });

  server.on("/brightness", HTTP_POST, []() {    
    brightness = server.arg("brightness").toInt();    
    server.send(200, "text/json", "{ok}");
  });
  
  server.on("/countdown", HTTP_POST, []() {    
    countdownMilliSeconds = server.arg("ms").toInt();     
    byte cd_r_val = server.arg("r").toInt();
    byte cd_g_val = server.arg("g").toInt();
    byte cd_b_val = server.arg("b").toInt();
    countdownColor = CRGB(cd_r_val, cd_g_val, cd_b_val); 
    endCountDownMillis = millis() + countdownMilliSeconds;
    allBlank();

    Serial.print("CountdownMilliSeconds: ");
    Serial.println(countdownMilliSeconds);
    
    clockMode = 1;     
    server.send(200, "text/json", "{ok}");
  });

  server.on("/temperature", HTTP_POST, []() {   
    temperatureCorrection = server.arg("correction").toInt();
    temperatureSymbol = server.arg("symbol").toInt();
    clockMode = 2;     
    server.send(200, "text/json", "{ok}");
  });  

  server.on("/clock", HTTP_POST, []() {       
    clockMode = 0;     
    server.send(200, "text/json", "{ok}");
  });  
  
  // Before uploading the files with the "ESP8266 Sketch Data Upload" tool, zip the files with the command "gzip -r ./data/" (on Windows I do this with a Git Bash)
  // *.gz files are automatically unpacked and served from your ESP (so you don't need to create a handler for each file).
  server.serveStatic("/", SPIFFS, "/", "max-age=86400");
  server.begin();     
    
  Serial.println("SPIFFS contents:");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
  }
  Serial.printf("\n"); 

  // Simulate/test countdown mode
  //countdownMilliSeconds = 3660000;
  //endCountDownMillis = millis() + countdownMilliSeconds;
  //clockMode = 1;
}


CRGB colors[] = {CRGB::Red,CRGB::Chocolate,CRGB::Blue,CRGB::Green,CRGB::Orange,CRGB::White,CRGB::Yellow,CRGB::DeepSkyBlue,CRGB::FireBrick};
byte colorIndex = 0;

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}

void loop(){

  server.handleClient(); 
  
  unsigned long currentMillis = millis();  
  if (currentMillis - prevTime >= 1000) {
    prevTime = currentMillis;

    if (clockMode == 0) {
      updateClock();
    } else if (clockMode == 1) {
      updateCountdown();
    } else if (clockMode == 2) {
      updateTemperature();      
    }

    FastLED.setBrightness(brightness);
    FastLED.show();
  }   
}

void allBlank() {
  for (int i=0; i<NUM_LEDS; i++) {
    LEDs[i] = CRGB::Black;
  }
  FastLED.show();
}

void updateTemperature() {
  RtcTemperature temp = Rtc.GetTemperature();
  float ftemp = temp.AsFloatDegC();
  float ctemp = ftemp + temperatureCorrection;
  Serial.print("Sensor temp: ");
  Serial.print(ftemp);
  Serial.print(" Corrected: ");
  Serial.println(ctemp);

  if (temperatureSymbol == 13)
    ctemp = (ctemp * 1.8000) + 32;

  byte t1 = int(ctemp) / 10;
  byte t2 = int(ctemp) % 10;
  CRGB color = CRGB(r_val, g_val, b_val);
  displayNumber(t1,3,color);
  displayNumber(t2,2,color);
  displayNumber(11,1,color);
  displayNumber(temperatureSymbol,0,color);
  hideDots();
}

void updateClock() {  
  RtcDateTime now = Rtc.GetDateTime();
  //printDateTime(now);    

  int hour = now.Hour();
  int mins = now.Minute();
  int secs = now.Second();
  
  byte h1 = hour / 10;
  byte h2 = hour % 10;
  byte m1 = mins / 10;
  byte m2 = mins % 10;  
  byte s1 = secs / 10;
  byte s2 = secs % 10;
  
  CRGB color = CRGB(r_val, g_val, b_val);

  if (h1 > 0)
    displayNumber(h1,3,color);
  else 
    displayNumber(10,3,color);  // Blank
    
  displayNumber(h2,2,color);
  displayNumber(m1,1,color);
  displayNumber(m2,0,color); 

  displayDots(color);  
}

void updateCountdown() {
  unsigned long restMillis = endCountDownMillis - millis();
  unsigned long hours   = ((restMillis / 1000) / 60) / 60;
  unsigned long minutes = (restMillis / 1000) / 60;
  unsigned long seconds = restMillis / 1000;
  int remSeconds = seconds - (minutes * 60);
  int remMinutes = minutes - (hours * 60); 
  
  Serial.print(restMillis);
  Serial.print(" ");
  Serial.print(hours);
  Serial.print(" ");
  Serial.print(minutes);
  Serial.print(" ");
  Serial.print(seconds);
  Serial.print(" | ");
  Serial.print(remMinutes);
  Serial.print(" ");
  Serial.println(remSeconds);

  byte h1 = hours / 10;
  byte h2 = hours % 10;
  byte m1 = remMinutes / 10;
  byte m2 = remMinutes % 10;  
  byte s1 = remSeconds / 10;
  byte s2 = remSeconds % 10;

  CRGB color = countdownColor;
  if (restMillis <= 60000) {
    color = CRGB::Red;
  }

  if (hours > 0) {
    // hh:mm
    displayNumber(h1,3,color); 
    displayNumber(h2,2,color);
    displayNumber(m1,1,color);
    displayNumber(m2,0,color);  
  } else {
    // mm:ss   
    displayNumber(m1,3,color);
    displayNumber(m2,2,color);
    displayNumber(s1,1,color);
    displayNumber(s2,0,color);  
  }

  displayDots(color);  


  if (hours <= 0 && remMinutes <= 0 && remSeconds <= 0) {
    Serial.println("Countdown timer ended.");
    endCountdown();
    countdownMilliSeconds = 0;
    endCountDownMillis = 0;
    clockMode = 0;
    return;
  }  
}

void endCountdown() {
  allBlank();
  for (int i=0; i<NUM_LEDS; i++) {
    if (i>0)
      LEDs[i-1] = CRGB::Black;
    
    LEDs[i] = CRGB::Red;
    FastLED.show();
    delay(25);
  }  
}

void displayDots(CRGB color) {
  if (dotsOn) {
    LEDs[42] = color;
    LEDs[43] = color;
  } else {
    LEDs[42] = CRGB::Black;
    LEDs[43] = CRGB::Black;
  }

  dotsOn = !dotsOn;  
}

void hideDots() {
  LEDs[42] = CRGB::Black;
  LEDs[43] = CRGB::Black;
}

void displayNumber(byte number, byte segment, CRGB color) {
  /*
   * 
      __ __ __        __ __ __          __ __ __        12 13 14  
    __        __    __        __      __        __    11        15
    __        __    __        __      __        __    10        16
    __        __    __        __  42  __        __    _9        17
      __ __ __        __ __ __          __ __ __        20 19 18  
    __        65    __        44  43  __        21    _8        _0
    __        __    __        __      __        __    _7        _1
    __        __    __        __      __        __    _6        _2
      __ __ __       __ __ __           __ __ __       _5 _4 _3   

   */
 
  // segment from left to right: 3, 2, 1, 0
  byte startindex = 0;
  switch (segment) {
    case 0:
      startindex = 0;
      break;
    case 1:
      startindex = 21;
      break;
    case 2:
      startindex = 44;
      break;
    case 3:
      startindex = 65;
      break;    
  }

  for (byte i=0; i<21; i++){
    yield();
    LEDs[i + startindex] = ((numbers[number] & 1 << i) == 1 << i) ? color : CRGB::Black;
  } 
}

//boolean summerTime() {
//  struct tm *timeinfo;
//  time(&now);
//  timeinfo = localtime(&now);
//
//  int y = timeinfo->tm_year;
//  int m = timeinfo->tm_mon;
//  int d = timeinfo->tm_mday;
//  int h = timeinfo->tm_hour;  
//
//  if (m < 3 || m > 10) return false;  // No summer time in Jan, Feb, Nov, Dec
//  if (m > 3 && m < 10) return true;   // Summer time in Apr, May, Jun, Jul, Aug, Sep
//  if (m == 3 && (h + 24 * d) >= (3 +  24 * (31 - (5 * y / 4 + 4) % 7)) || m == 10 && (h + 24 * d) < (3 +  24 * (31 - (5 * y / 4 + 1) % 7)))
//  return true;
//    else
//  return false;
//}
