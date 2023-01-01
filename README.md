# HelloRestCar
Requirements:
Arduino Uno R3
Elecrow ACS17101S - Wifi Shield based on RN171 module.
4tronix Initio 4WD Robocar
Library:  https://github.com/Seeed-Studio/WiFi_Shield.git
Modify WiFlyClieng.h to use:
#define PIN_TX			5
#define PIN_RX			6
Description:
Car makes http-requests to webserver for fetching actions it's going to execute next.
It must implement all actions which are defined on web server backend client api.
Default implementation fo webserver is visible on https://robo.sukelluspaikka.fi
