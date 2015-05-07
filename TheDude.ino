#define DEBUG 1  //Print to Serial
#define DUDEPIN 12 // Pin to Relay
#define REFRESHTIME 1 //MINUTES
#define WHILE_TO 5000 // milliseconds
#define PRINTDEBUG(STR) \
  {	\
    if (DEBUG) Serial.println(STR); \
  }
#define BIAS 1
#include <ESP8266WiFi.h>
#include "TheDudeParams.h" // Change this file params

//INIT
const char* ssid     = MY_SSID;
const char* password = MY_PWD;
const char* host = MY_HOST;
const char* hostIP = MY_HOSTIP;
String url = "/getIP.php?psswd=";
WiFiServer server(80);


//Class defintion of the water heater control
class dudeControl {
  public:
    //   dudeControl();
    void begin(int pin) {
      dudePin = pin;
      startTime = 9999;
      endTime = -9999;
      timeOfLastGet = millis() / 1000 + REFRESHTIME * 60;
      timeOfLastUpdate = millis() / 1000 + REFRESHTIME * 60;
      currentState = LOW;
      lastCurrentState = LOW;
      updateNeeded = HIGH;
      pinMode(dudePin, OUTPUT);
      digitalWrite(dudePin, currentState);
    }
    void updateTime(int dataFlag) {
      timeOfLastUpdate = (millis() / 1000); //Time in seconds of lastUpdate
      if (dataFlag == 2) {
        timeOfLastGet = timeOfLastUpdate; //Time in seconds of lastGetUrl
      }
    }
    void changeState() {
      digitalWrite(dudePin, currentState);
      updateNeeded = HIGH;
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
      if (timeOfLastGet > 0 &&  (  minutesDiff >= startTime &&  minutesDiff <= endTime))
      {
        PRINTDEBUG("TurnOn");
        PRINTDEBUG(minutesDiff);
        currentState = HIGH;
      }
      else
      {
        PRINTDEBUG("TurnOff");
        PRINTDEBUG(minutesDiff);
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
    unsigned long timeOfLastGet, timeOfLastUpdate;
    bool currentState, lastCurrentState, updateNeeded;
  private:

};

// Fucntion to connect WiFi
void connectWifi(const char* ssid, const char* password) {
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


// Inifinite loop - Causes to reset self
void resetSelf() {
  PRINTDEBUG("Reseting");
  while (1) {}
}


dudeControl dude; //define variable
unsigned long whileTimeout = 0;
////////////////////////// SETUP //////////////////////////////////////////////////////

void setup() {
  if (DEBUG) Serial.begin(115200);  //Start Serial
  delay(10);
  connectWifi(ssid, password); // Start WiFi
  server.begin();  // Start Server
  dude.begin(DUDEPIN); // Start class
  delay(10);
}


////////////////////////// LOOP //////////////////////////////////////////////////////


void loop() {

  delay(10);
  int connectFails = 0;

  while ((WiFi.status() != WL_CONNECTED)) {
    connectWifi(ssid, password);
    dude.checkState();
    connectFails++;
    if (connectFails > 4) {
      resetSelf();  // If 2 minutes passed with no connection - Reset Self
    }
  }

  WiFiClient clientS = server.available();
  unsigned long timePassed = (millis() / 1000) - dude.timeOfLastUpdate;

  if (!clientS) {
    // Skip this return only if timePassed as needed or needed upadate and timepassed more than 10 seconds
    // (less make the website not respond)
    if ( (!dude.updateNeeded && timePassed < (60 * REFRESHTIME))  || (dude.updateNeeded &&  timePassed < 10 ) ) {
      return;
    }
  }
  else // Server Commands
  {
    PRINTDEBUG("new client");
    whileTimeout = millis();
    while (!clientS.available() ) {
      if (  (millis() - whileTimeout) > WHILE_TO ) {
        clientS.stop();
        return;
      }
      delay(1);
    }
    String req = clientS.readStringUntil('\r');
    PRINTDEBUG(req);
    clientS.flush();
    int val = 1;
    if (req.indexOf("/gpio/0") != -1) // LOW
      dude.changeState(LOW);
    else if (req.indexOf("/gpio/1") != -1) //HIGH
      dude.changeState(HIGH);
    else if  (req.indexOf("/gpio/s") != -1) {} // GET STATUS
    else if  (req.indexOf("/gpio/u") != -1) {
      val = 0;  // UPDATE
    }
    // else if  (req.indexOf("/favicon.ico") != -1) {
    //   return;
    // }
    else {
      clientS.stop();
      return;
    }
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n Last GPIO state was:  ";
    s += (dude.currentState) ? "high" : "low";
    s += "</html>\n";
    clientS.print(s);
    delay(10);
    clientS.stop();
    if (val) return;
  }




  dude.updateNeeded = LOW;   // Will go to update only when state is changed (To update website)

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
  String timeFromReset = String( (int)((millis() / 1000) / 60), DEC);
  String url_myState = url + MY_WEB_PWD + "&myState=" + (dude.currentState ? "1" : "0") + "&lastReset=" + timeFromReset;
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
    whileTimeout = millis();
    while (client.available() &&  ((millis() - whileTimeout) < WHILE_TO) ) {
      if (!dataFlag) dataFlag = 1;
      String line = client.readStringUntil('\r');
      if (line.length() > 9 + BIAS && line.startsWith("NextStart:", BIAS)) {
        PRINTDEBUG("Found ON");
        dude.getStartTime(line);
      }
      if (line.length() > 9 + BIAS && line.startsWith("Next_End_:", BIAS)) {
        PRINTDEBUG("Found OFF");
        dude.getEndTime(line);
        dataFlag = 2;
      }
    }
    if  (!dataFlag) {
      dataFlagCounter++;
      if (dataFlagCounter > 20) {
        PRINTDEBUG("Connection Failed!");
        client.stop();
        dude.updateTime(dataFlag);
        return;
      }
      delay(500);
    }
  }
  client.flush();
  client.stop();

  delay(100);
  dude.updateTime(dataFlag);  // UPDATE TIME  
  dude.checkState();  // Check if GPIO needs to be changed
  PRINTDEBUG(dataFlag);
  PRINTDEBUG(dude.timeOfLastUpdate);
  PRINTDEBUG(dude.timeOfLastGet);  
  PRINTDEBUG(dude.startTime);
  PRINTDEBUG(dude.endTime);


  PRINTDEBUG();
  PRINTDEBUG("closing connection");

}

