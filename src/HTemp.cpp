
#include <Arduino.h>
#include "HTemp.h"

const double VOL_OFFSET = 350; // offset voltage, mv
const double AMP_AV = 54.16;   // Av of amplifier
const double Var_VtoT_K_1[9] = {0, 2.5173462e1, -1.1662878, -1.0833638, -8.9773540 / 1e1, -3.7342377 / 1e1, -8.6632643 / 1e2, -1.0450598 / 1e2, -5.1920577 / 1e4};
const double Var_VtoT_K_2[10] = {0, 2.508355e1, 7.860106 / 1e2, -2.503131 / 1e1, 8.315270 / 1e2, -1.228034 / 1e2, 9.804036 / 1e4, -4.413030 / 1e5, 1.057734 / 1e6, -1.052755 / 1e8};
const double Var_VtoT_K_3[7] = {-1.318058e2, 4.830222e1, -1.646031, 5.464731 / 1e2, -9.650715 / 1e4, 8.802193 / 1e6, -3.110810 / 1e8};

HTemp::HTemp(int _pinTmp, int _pinThmc)
{
    pinRoomTmp = _pinTmp;
    pinThmc = _pinThmc;
}

void HTemp::begin()
{
    tempRoom = getRoomTmp();
}

double HTemp::getThmc()
{
    double vol = getThmcVol();
    tempThmc = K_VtoT(vol) + tempRoom;

    return tempThmc;
}

int HTemp::getAnalog(int pin)
{
    long sum = 0;

    for (int i = 0; i < 32; i++)
    {
        sum += analogRead(pin);
    }

    return ((sum >> 5)); // 3.3V supply
}

double HTemp::getRoomTmp()
{
    int a = getAnalog(pinRoomTmp) * 50 / 33;                                         // 3.3V supply
    double resistance = (double)(1023 - a) * 10000 / a;                              // get the resistance of the sensor;
    double temperature = 1 / (log(resistance / 10000) / 3975 + 1 / 298.15) - 273.15; // convert to temperature via datasheet ;

    tempRoom = temperature;
    return temperature;
}

double HTemp::getThmcVol() // get voltage of thmc in mV
{
    double vout = (double)getAnalog(pinThmc) / 1023.0 * 5.0 * 1000;
    double vin = (vout - VOL_OFFSET) / AMP_AV;
    return (vin);
}

double HTemp::K_VtoT(double mV)
{
    int i = 0;
    double value = 0;
    if (mV >= -6.478 && mV < 0)
    {
        value = Var_VtoT_K_1[8];
        for (i = 8; i > 0; i--)
            value = mV * value + Var_VtoT_K_1[i - 1];
    }
    else if (mV >= 0 && mV < 20.644)
    {
        value = Var_VtoT_K_2[9];
        for (i = 9; i > 0; i--)
            value = mV * value + Var_VtoT_K_2[i - 1];
    }
    else if (mV >= 20.644 && mV <= 54.900)
    {
        value = Var_VtoT_K_3[6];
        for (i = 6; i > 0; i--)
            value = mV * value + Var_VtoT_K_3[i - 1];
    }

    return value;
}
