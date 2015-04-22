#define DEBUG 0
#define BIAS 1
#define DUDEPIN 0
#define REFRESHTIME 1 //MINUTES
#define PRINTDEBUG(STR) \
{	\
  if (DEBUG) Serial.println(STR); \
}
#include <ESP8266WiFi.h>
#include "TheDudeParams.h"

const char* ssid     = MY_SSID;
const char* password = MY_PWD;
const char* host = MY_HOST;
const char* hostIP = MY_HOSTIP;
String url = "/getIP.php?psswd=";
WiFiServer server(80);

class dudeControl {
  public:
    //   dudeControl();
    void begin(int pin) {
      dudePin = pin;
      startTime = 9999;
      endTime = -9999;
      timeOfLastGet = millis()/1000 + REFRESHTIME*60;
      currentState = LOW;
      lastCurrentState = LOW;
      pinMode(dudePin, OUTPUT);
      digitalWrite(dudePin, currentState);
    }
    void updateTime() {
      timeOfLastGet = (millis() / 1000); //Time in seconds of lastGetUrl
    }
    void changeState() {
      digitalWrite(dudePin, currentState);
    }
    void changeState(bool stt) {
      currentState = stt;
      changeState();
    }
    void getStartTime(String stringOne) { // Function to get Time in minutes to start
      int strIdx = 10 + BIAS;
      int lastIdx = stringOne.length();
      int sign = 1;
      if (stringOne.charAt(strIdx) == '-')
      {
        sign = -1;
        strIdx++;
      }
      int startTimeTMP = 0;
      while (strIdx < lastIdx) {
        if ((-48 + (int)stringOne.charAt(strIdx)) < 0 || (-48 + (int)stringOne.charAt(strIdx)) > 9 || (lastIdx - strIdx > 4) )
        {
          PRINTDEBUG("Error!");
          startTimeTMP = 9999;
          break;
        }
        startTimeTMP = startTimeTMP * 10 + (-48 + (int)stringOne.charAt(strIdx));
        strIdx++;
      }
      startTime = (int)(sign * startTimeTMP);
    }
    void getEndTime(String stringOne) { // Function to get Time in minutes to end
      int strIdx = 10 + BIAS;
      int lastIdx = stringOne.length();
      int sign = 1;
      if (stringOne.charAt(strIdx) == '-')
      {
        sign = -1;
        strIdx++;
      }
      int endTimeTMP = 0;
      while (strIdx < lastIdx) {
        if ((-48 + (int)stringOne.charAt(strIdx)) < 0 || (-48 + (int)stringOne.charAt(strIdx)) > 9 || (lastIdx - strIdx > 4) )
        {
          PRINTDEBUG("Error!");
          endTimeTMP = -9999;
          break;
        }
        endTimeTMP = endTimeTMP * 10 + (-48 + (int)stringOne.charAt(strIdx));
        strIdx++;
      }

      endTime = (int)(sign * endTimeTMP);
    }

    void checkState() {   // Check if GPIO need to be changed
      int minutesDiff = (int)(((millis() / 1000) - timeOfLastGet) / 60);
      if (timeOfLastGet > 0 &&  (  minutesDiff > startTime &&  minutesDiff < endTime))
      {
        PRINTDEBUG("TurnOn");
        currentState = HIGH;
      }
      else
      {
        PRINTDEBUG("TurnOff");
        currentState = LOW;
      }
      if (lastCurrentState != currentState) {
        changeState();
      }
      lastCurrentState = currentState;
    }
    int dudePin;
    int startTime;
    int endTime;
    unsigned long timeOfLastGet;
    bool currentState, lastCurrentState;
  private:

};

void connectWifi(const char* ssid,const char* password) {
      int WiFiCounter = 0;
      // We start by connecting to a WiFi network
      PRINTDEBUG("Connecting to ");
      PRINTDEBUG(ssid);
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED && WiFiCounter < 30) {
          delay(1000);
          WiFiCounter++;
          PRINTDEBUG(".");
      }
    
      PRINTDEBUG("");
      PRINTDEBUG("WiFi connected");
      PRINTDEBUG("IP address: ");
      PRINTDEBUG(WiFi.localIP());
}





dudeControl dude;

void setup() {
  if (DEBUG) Serial.begin(115200);
  delay(10);

  
  connectWifi(ssid, password);
  server.begin();
  dude.begin(DUDEPIN);
  delay(10);
}

void loop() {

  delay(10);
  
  while ((WiFi.status() != WL_CONNECTED)) {
      connectWifi(ssid, password);
      dude.checkState();
  }
  
WiFiClient clientS = server.available();
  if (!clientS) {
    if ((millis()/1000) - dude.timeOfLastGet < (60*REFRESHTIME) ) {
      return;
      }
  }
  else
  {
    PRINTDEBUG("new client");
    while (!clientS.available()) {
      delay(1);
    }
    String req = clientS.readStringUntil('\r');
    PRINTDEBUG(req);
    clientS.flush();
    int val = 1;
    if (req.indexOf("/gpio/0") != -1)
      dude.changeState(LOW);
    else if (req.indexOf("/gpio/1") != -1)
      dude.changeState(HIGH);
    else if  (req.indexOf("/gpio/s") != -1) {}
    else if  (req.indexOf("/favicon.ico") != -1) {
      return;
    }
    else {
      val = 0;
    }
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n Last GPIO state was:  ";
    s += (dude.currentState) ? "high" : "low";
    s += "</html>\n";
    clientS.print(s);
    delay(10);
    if (val) return;
    clientS.stop();
  }





  // Use WiFiClient class to create TCP connections
  PRINTDEBUG("connecting to ");
  PRINTDEBUG(host);
  delay(10);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(hostIP, httpPort)) {
    PRINTDEBUG("connection failed");
    return;
  }





  // This will send the request to the server
  PRINTDEBUG("Requesting URL: ");
  delay(10);
  String url_myState = url + MY_WEB_PWD + "&myState=" + (dude.currentState ? "1" : "0");
  PRINTDEBUG(url_myState);
  delay(10);
  client.print(String("GET ") + url_myState + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(100);

  // Read all the lines of the reply from server and find time
  int dataFlag = 0;
  int dataFlagCounter = 0;
  while (!dataFlag) {
    while (client.available()) {
      dataFlag = 1;
      String line = client.readStringUntil('\r');
      if (line.length() > 9 + BIAS && line.startsWith("NextStart:", BIAS)) {
        PRINTDEBUG("Found");
        dude.getStartTime(line);
      }
      if (line.length() > 9 + BIAS && line.startsWith("Next_End_:", BIAS)) {
        dude.getEndTime(line);
      }
    }
    client.stop();
    if  (!dataFlag) {
      dataFlagCounter++;
      if (dataFlagCounter > 10) {
        dude.updateTime();
        return;
      }
      delay(2000);
    }
  }

  delay(100);
  dude.checkState();


  if (dataFlag) {
    dude.updateTime();
    PRINTDEBUG(dude.startTime);
    PRINTDEBUG(dude.endTime);
  }
  PRINTDEBUG();
  PRINTDEBUG("closing connection");
  
}

