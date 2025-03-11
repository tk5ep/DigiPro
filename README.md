# DigiPro
LoRa APRS digipeater, low cost, low power based on Arduino pro-mini

Based on an initial work of VE2YAG Rémi Bilodeau. With his kind permission, I kept the name and have reworked some parts of his work, mainly the digipeater functions.

This is a low cost, low power APRS LoRa digipeater using an Arduino pro-mini.
It can also serve as a small WX station using a SHT31 temperature and humidity sensor.

 - Support Lora carrier detect and collision avoiding using persistance and slottime, just like standard APRS
 - Digi beaconing and telemetry.
 - Digipeater support WIDEx-x and SSID digipeating for shortest packet.
 - Support ?APRS? and ?APRSS query
 - Support message ACKing, but don't use messaging for now.
 - Additional telemetry using DS18B20 or SHT31 for internal/external temperature and humidity.
 - Battery voltage monitoring and sleep mode under a certain voltage (depends of battery type)
 - PCB are low cost with or without solar panel charging.

   
