# ESP8266-Water-Heater-Control
This is a code for a project controlling the water heater via the web using ESP8266 programmed with arduino IDE


In order to make it work you'll need 

1. Build a sketch as in the JPG file.

2. Build a web site with PHP and SQL support.

3. create file "TheDudeParams.h", add it to your workspace in the arduino IDE. The file should consist the next code:

#define MY_SSID "YOUR_WIFI_SSID"
#define MY_PWD "YOUR_WIFI_PASSWORD"
#define MY_HOST "YOUR_WEBSITE"
#define MY_HOSTIP "YOUR_WEBSITE_IP"
#define MY_WEB_PWD "REQUEST_PASSWORD_IN_YOUR_PHP_FILES"

4. create file 
