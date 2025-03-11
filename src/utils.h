#ifndef UTILS_H 
#define UTILS_H

uint32_t readVcc();
uint32_t readVoltage(uint8_t analog_pin);
void solarPanel_ON();
void solarPanel_OFF();
void sensors_ON();
void sensors_OFF();
void checkBattery();
void init_SHT31();
void read_SHT31();
void sleepfor(uint32_t nbs);

#endif