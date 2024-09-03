
#ifndef __HIGH_TEMP_H__
#define __HIGH_TEMP_H__

class HTemp
{

public:
    HTemp(int _pinTmp, int _pinThmc);

    float getRoomTmp(); //
    float getThmc();

    void begin();

private:
    int pinRoomTmp; // pin of temperature sensor
    int pinThmc;    // pin of thermocouple

    float tempRoom; // room temperature
    float tempThmc; // thermocouple temperature

public:
    int getAnalog(int pin);
    float K_VtoT(float mV); // K type thermocouple, mv->oC
    float getThmcVol();     // get voltage of thmc in mV
};

#endif