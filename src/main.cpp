#include <Arduino.h>
#include "RTClib.h"
#include <SPI.h>
#include <max6675.h>
#include "HTemp.h"
#include "SdFat.h"

#define SDPin 10
#define thCLK 9
#define thCS 8
#define thDO 7

const char *fileName = "temp_log.txt";
unsigned long drMills = 60000; // 60s;
unsigned long lastMills = 0;
char date[21];
char buffer[60];
boolean onSerial = false;

SdFat SD;
File file;
RTC_PCF8523 rtc;
HTemp ht1(A1, A0);
HTemp ht2(A3, A2);
MAX6675 thermocouple(thCLK, thCS, thDO);

void initSDCard()
{
  if (!SD.begin(SDPin))
  {
    // Serial.println("Initailize SD Card failed");
    while (1)
    {
      // Serial.println("Re-init SD Card...");
      delay(1000);
      SD.begin(SDPin);
    };
  }

  if (!SD.exists(fileName))
    SD.open(fileName, FILE_WRITE);
  // Serial.println("Initailize SD Card done!");
}

void showMenus()
{
  Serial.println("\t\t* Menus * ");
  Serial.println("\t p: Show all menus.");
  Serial.println("\t r: Read data logs.");
  Serial.println("\t eY: Erase data logs.");
  Serial.println("\t t: Set the RTC datetime.");
  Serial.println("\t d: Show the RTC datetime.");
  Serial.println("\t mXX: Set delay for record data log (XX as seconds).");
}

char *getDateTime()
{
  DateTime now = rtc.now();
  sprintf(date, "%02d/%02d/%02d %02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
  return date;
}

void writeEraseAndHeader()
{
  SD.remove(fileName);
  file = SD.open(fileName, FILE_WRITE);
  if (!file)
  {
    // Serial.println("Not found file");
    initSDCard();
    return;
  }

  file.println("DateTime, MaxTempC, MaxTemF, HTemp1, HTemp2");
  file.close();
  Serial.println("Erase data log done!");
}

void saveData()
{
  getDateTime();
  float temp1 = ht1.getThmc();
  float temp2 = ht2.getThmc();
  float temp3C = thermocouple.readCelsius();
  float temp3F = thermocouple.readFahrenheit();

  sprintf(buffer, "%s,%s,%s,%s,%s", date, String(temp3C).c_str(), String(temp3F).c_str(), String(temp1).c_str(), String(temp2).c_str());
  if (Serial.available())
    Serial.println(buffer);

  file = SD.open(fileName, FILE_WRITE);
  if (!file)
  {
    // Serial.println("Not found file");
    initSDCard();
    return;
  }
  file.println(buffer);
  file.close();
  // Serial.println("Write done!");
}

void readData()
{
  // Serial.flush();
  file = SD.open(fileName, FILE_READ);
  if (!file)
  {
    Serial.println("Not found SD Card");
    return;
  }

  while (file.available())
    Serial.write(file.read());

  file.close();
  Serial.println("Read done!");
}

void setting()
{
  if (Serial.available() > 0)
  {
    onSerial = true;
    String s = Serial.readString();
    char c[sizeof(s)];
    s.toCharArray(c, sizeof(s));
    switch (c[0])
    {
    case 'p':
    {
      showMenus();
      break;
    }
    case 'r':
    {
      readData();
      break;
    }
    case 'e':
    {
      if (c[1] == 'Y')
        writeEraseAndHeader();

      break;
    }
    case 't':
    {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      delay(500);
      Serial.println(getDateTime());
      break;
    }
    case 'm':
    {
      if (sizeof(c) >= 3)
      {
        char concat[2];
        concat[0] = c[1];
        concat[1] = c[2];

        int digi = atoi(concat);
        drMills = digi * 1000; // 60 s=1000ms
        Serial.print("Save data every: ");
        Serial.print(digi);
        Serial.println(" seconds");
      }
      break;
    }
    case 'd':
    {
      Serial.println(getDateTime());
      break;
    }
    default:
      break;
    }

    onSerial = false;
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(SDPin, OUTPUT);

  ht1.begin();
  ht2.begin();

  if (!rtc.begin())
  {
    // Serial.println("Couldn't find RTC");
    while (1)
      delay(100);
  }

  // TODO: Set default date (This line will clone the date from your computer date and set to RTC)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // rtc.start();
  delay(1000);
  // Serial.println("Initailize RTC done!");

  initSDCard();
  if (Serial.available())
    showMenus();
}

void loop()
{
  unsigned long currMills = millis();
  if (currMills - lastMills >= drMills && onSerial == false)
  {
    saveData();
    lastMills = currMills;
  }
  setting();
}
