/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <SPI.h>
#include <Adafruit_WINC1500.h>
#include <PubSubClient.h>

// Define the WINC1500 board connections below.
// If you're following the Adafruit WINC1500 board
// guide you don't need to modify these:
#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
//#define WINC_EN   2     // or, tie EN to VCC and comment this out
// The SPI pins of the WINC1500 (SCK, MOSI, MISO) should be
// connected to the hardware SPI port of the Arduino.
// On an Uno or compatible these are SCK = #13, MISO = #12, MOSI = #11.
// On an Arduino Zero use the 6-pin ICSP header, see:
//   https://www.arduino.cc/en/Reference/SPI

/************ LAMP & SWITCH PINS *********/
#define LAMP_ONE       9 // input pin  
#define SWITCH_ONE     10 // output pin
#define LAMP_TWO       6 // input pin
#define SWITCH_TWO     5 // output pin
#define LAMP_THREE     2 // input for the switch
#define SWITCH_THREE   3 // switch fot the lamp 2

int lastSwitchOneState;
int lastLampOneState;

int lastSwitchTwoState;
int lastLampTwoState;

int lastSwitchThreeState;
int lastLampThreeState;

// Setup the WINC1500 connection with the pins above and the default hardware SPI.
Adafruit_WINC1500 WiFi(WINC_CS, WINC_IRQ, WINC_RST);

// Or just use hardware SPI (SCK/MOSI/MISO) and defaults, SS -> #10, INT -> #7, RST -> #5, EN -> 3-5V
//Adafruit_WINC1500 WiFi;


char ssid[] = "";     //  your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Update these with values suitable for your network.
const char* mqtt_server = "";

// Initialize the WIFI client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
Adafruit_WINC1500Client wifiClient;

PubSubClient client(wifiClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
/*#ifdef WINC_EN
  pinMode(WINC_EN, OUTPUT);
  digitalWrite(WINC_EN, HIGH);
#endif*/
  Serial.begin(9600);
  // define the pin modes
  pinMode(LAMP_ONE, INPUT);
  pinMode(SWITCH_ONE, OUTPUT);
  
  pinMode(LAMP_TWO, INPUT);
  pinMode(SWITCH_TWO, OUTPUT);
  
  pinMode(LAMP_THREE, INPUT);
  pinMode(SWITCH_THREE, OUTPUT);
  
  // initialize the switch
  digitalWrite(SWITCH_ONE, LOW);
  digitalWrite(SWITCH_TWO, LOW);
  digitalWrite(SWITCH_THREE, LOW);
  lastSwitchOneState = LOW;
  lastSwitchTwoState = LOW;
  lastSwitchThreeState = LOW;
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      delay(1000);
    }
  }

  Serial.println("Connected to wifi");
  printWifiStatus();

  // initialize server
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\nMessage arrived [");
  Serial.print(topic);
  Serial.print("]\n");
  char *value = (char*)payload;
  Serial.println(value);
  String strTopic = topic;
  // change the lamp status according to the broker message
  if(strTopic == "josleugim/groundfloor/frontyard/lamp1") {
    ChangeLamp1Status(atoi(value));
  }
  if(strTopic == "josleugim/groundfloor/frontyard/lamp2") {
    ChangeLamp2Status(atoi(value));
  }
  /*for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }*/
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClientWinc1500", "", "")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      // suscribe to the corresponding topics
      client.subscribe("josleugim/groundfloor/frontyard/lamp2");
      client.subscribe("josleugim/groundfloor/frontyard/lamp1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // publish the status of the lamp to the broker
  publishLamp1Status(digitalRead(LAMP_ONE));
  publishLamp2Status(digitalRead(LAMP_TWO));
}

void publishLamp1Status(int lampState) {
  if((lampState == LOW) && (lastLampOneState != lampState)) {
    client.publish("josleugim/groundfloor/frontyard/lamp1state","0");
    lastLampOneState = LOW;
  }
  if((lampState == HIGH) && (lastLampOneState != lampState)) {
    client.publish("josleugim/groundfloor/frontyard/lamp1state","1");
    lastLampOneState = HIGH;
  }
}

void publishLamp2Status(int lampState) {
  if((lampState == LOW) && (lastLampTwoState != lampState)) {
    client.publish("josleugim/groundfloor/frontyard/lamp2state","0");
    lastLampTwoState = LOW;
  }
  if((lampState == HIGH) && (lastLampTwoState != lampState)) {
    client.publish("josleugim/groundfloor/frontyard/lamp2state","1");
    lastLampTwoState = HIGH;
  }
}

// change the lamp status according to the broker
void ChangeLamp2Status(int switchState) {
  if((digitalRead(LAMP_TWO) == LOW) && (switchState == HIGH)) {
      if(lastSwitchTwoState == HIGH) {
        digitalWrite(SWITCH_TWO, LOW);
        lastSwitchTwoState = LOW;
      } else {
        digitalWrite(SWITCH_TWO, HIGH);
        lastSwitchTwoState = HIGH;
      }
      client.publish("josleugim/groundfloor/frontyard/lamp2state","1");
    }
    if((digitalRead(LAMP_TWO) == HIGH) && (switchState == LOW)) {
      if(lastSwitchTwoState == HIGH) {
        digitalWrite(SWITCH_TWO, LOW);
        lastSwitchTwoState = LOW;
      } else {
        digitalWrite(SWITCH_TWO, HIGH);
        lastSwitchTwoState = HIGH;
      }
      client.publish("josleugim/groundfloor/frontyard/lamp2state","0");
    }
}


// change the lamp status according to the broker
void ChangeLamp1Status(int switchState) {
  Serial.print("\nReceiving message from the broker");
  
  if((digitalRead(LAMP_ONE) == LOW) && (switchState == HIGH)) {
      if(lastSwitchOneState == HIGH) {
        digitalWrite(SWITCH_ONE, LOW);
        lastSwitchOneState = LOW;
      } else {
        digitalWrite(SWITCH_ONE, HIGH);
        lastSwitchOneState = HIGH;
      }
      client.publish("josleugim/groundfloor/frontyard/lamp1state","1");
    }
    if((digitalRead(LAMP_ONE) == HIGH) && (switchState == LOW)) {
      if(lastSwitchOneState == HIGH) {
        digitalWrite(SWITCH_ONE, LOW);
        lastSwitchOneState = LOW;
      } else {
        digitalWrite(SWITCH_ONE, HIGH);
        lastSwitchOneState = HIGH;
      }
      client.publish("josleugim/groundfloor/frontyard/lamp1state","0");
    }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
