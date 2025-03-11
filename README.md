# DigiPro

This is a low cost, low power APRS LoRa digipeater using an Arduino pro-mini.
It can also serve as a small WX station using a SHT31 temperature and humidity sensor.

Based on an initial work of VE2YAG Rémi Bilodeau. With his kind permission, I kept the name and parts of the software, mainly the digipeater functions.
I've removed somes parts, simplified the programming and added some functions, like battery charging, SHT31 sensor, etc...

Main features are :
 - Lora carrier detect and collision avoiding using persistance and slottime, just like standard APRS
 - Digi beaconing and telemetry.
 - Digipeater support WIDEx-x and SSID digipeating for shortest packet.
 - Support ?APRS? and ?APRSS query
 - Support message ACKing, but don't use messaging for now.
 - Additional telemetry using DS18B20 or SHT31 for internal/external temperature and humidity.
 - Status messages with number of received/digipeated/transmitted packets.
 - Battery voltage monitoring and sleep mode under a certain voltage (depends of battery type)
 - Only the LoRa module is kept alive to keep the power consumption as low as possible.
 - PCB are low cost with/without solar panel charging.
