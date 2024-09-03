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
const char *fileSetting = "setting.txt";
unsigned long drMills = 60000; // 60s;
unsigned long lastMills = 0;
char date[21];
char buffer[61];
char c[14];

bool onSerial = false;

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

void writeSetting(unsigned long ms)
{
  if (SD.exists(fileName))
    SD.remove(fileSetting);

  file = SD.open(fileSetting, FILE_WRITE);
  if (!file)
    return;

  file.println(ms);
  file.close();
}

void readSetting()
{
  if (!SD.exists(fileName))
    return;

  file = SD.open(fileSetting, FILE_READ);
  if (!file)
    return;

  char ch;
  String intval = "";
  while (file.available())
  {
    ch = file.read();
    if (ch == '\n')
      break;

    intval += ch;
  }
  if (intval != "")
    drMills = atoi(intval.c_str());

  file.close();
}

void showMenus()
{
  Serial.println("\t\t* Menus * ");
  Serial.println("\t p: Show all menus.");
  Serial.println("\t r: Read data logs.");
  Serial.println("\t eY: Erase data logs.");
  Serial.println("\t t: Set the RTC datetime (tDDMMYYhhmmss)");
  Serial.println("\t d: Show the RTC datetime.");
  Serial.println("\t mSSS: Set delay for record data log (SS or SSS as seconds).");
}

void getDateTime()
{
  DateTime now = rtc.now();
  sprintf(date, "%02d/%02d/%d %02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
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

  file = SD.open(fileName, FILE_WRITE);
  if (!file)
  {
    // Serial.println("Not found file");
    initSDCard();
    return;
  }
  file.println(buffer);
  file.close();

  Serial.println(buffer);
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
    String s = Serial.readStringUntil('\n');
    s.toCharArray(c, 14);
    Serial.println(c);
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
      char d[3] = {c[1], c[2], '\0'};
      char m[3] = {c[3], c[4], '\0'};
      char y[3] = {c[5], c[6], '\0'};
      char h[3] = {c[7], c[8], '\0'};
      char mn[3] = {c[9], c[10], '\0'};
      char sc[3] = {c[11], c[12], '\0'};

      rtc.adjust(DateTime(atoi(y), atoi(m), atoi(d), atoi(h), atoi(mn), atoi(sc)));
      delay(500);
      getDateTime();
      Serial.println(date);
      break;
    }
    case 'm':
    {
      int digi = 60;
      if (c[3] != '\0')
      {
        char concat[4] = {c[1], c[2], c[3], '\0'};
        digi = atoi(concat);
      }
      else if (c[3] == '\0')
      {
        char concat[3] = {c[1], c[2], '\0'};
        digi = atoi(concat);
      }

      drMills = digi * 1000; // 60 s=1000ms
      writeSetting(drMills);
      Serial.print("Save data every ");
      Serial.print(digi);
      Serial.println(" seconds");
      break;
    }
    case 'd':
    {
      getDateTime();
      Serial.println(date);
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

  delay(1000);

  initSDCard();
  readSetting();
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
