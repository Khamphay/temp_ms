
#ifndef __HIGH_TEMP_H__
#define __HIGH_TEMP_H__

class HTemp
{

public:
    HTemp(int _pinTmp, int _pinThmc);

    double getRoomTmp(); //
    double getThmc();

    void begin();

private:
    int pinRoomTmp; // pin of temperature sensor
    int pinThmc;    // pin of thermocouple

    double tempRoom; // room temperature
    double tempThmc; // thermocouple temperature

public:
    int getAnalog(int pin);
    double K_VtoT(double mV); // K type thermocouple, mv->oC
    double getThmcVol();      // get voltage of thmc in mV
};

#endif