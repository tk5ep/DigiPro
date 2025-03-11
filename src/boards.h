//Boards definition
#include "settings.h"

// original board
#if defined BOARD_ORIGINAL
    #define RXD_GPS    0  // UBlox GPS (Only with tracker)
    #define TXD_GPS    1
    #define LORA_CS    2  // Lora radio module
    #define LORA_DIO   3  // Lora radio module
    #define DS_SENSOR  4  // DS18B20 external temp sensor   
    #define LORA_MOSI  11 // Lora radio module
    #define LORA_MISO  12
    #define LORA_SCLK  13
    #define LORA_RESET 17 
    #define I2C_SDA    18 // BMP180 sensor 
    #define I2C_SCL    19
    #define BATT_VOLT  A0 // (A0 for DIP28 prototype, else A7 for TQFP-32 PCB) Cell voltage, 15k/47k 5v = 1.22v (internal 1.2v ref)
#endif

// INTEL-IIRIS V4.1
#if defined BOARD_IIRIS_V4
    #define SENSORS_PWR A1
    #define I2C_SDA A4
    #define I2C_SCL A5
    #define SOLAR_VOLTAGE_PIN A7
    #define SOLAR_POWER_PIN 5
    #define DS_SENSOR 6         // 18B20 sensor
    #define HUMIDITY_PWR A0
    #define WATERMARK2_PWR 7
    #define WATERMARK1_PWR 8
    // LoRa
    #define LORA_DIO   2        // Lora radio module
    #define LORA_CS    10       // Lora radio module NSS
    #define LORA_MOSI  11       // Lora radio module
    #define LORA_MISO  12
    #define LORA_SCLK  13
    #define LORA_RESET 4
#endif    

#if defined BOARD_IIRIS_V3
    #define SENSORS_PWR A1
    #define SDA A4
    #define SCL A5
    #define SOLAR_PIN A7
    #define SOLAR_PWR 5
    #define DS_SENSOR 6         // 18B20 sensor
    #define HUMIDITY_PWR A0
    #define WATERMARK2_PWR 7
    #define WATERMARK1_PWR 8
    // LoRa
    #define LORA_DIO   2  // Lora radio module
    #define LORA_CS    10  // Lora radio module NSS
    #define LORA_MOSI  11 // Lora radio module
    #define LORA_MISO  12
    #define LORA_SCLK  13
    #define LORA_RESET 4

    #define I2C_SDA    18 // BMP180 sensor 
    #define I2C_SCL    19
#endif    

