/*Home appliance control with tv remote and web controlled interface*/
/*By Malomo Oluwadamlare Oluwapelumi*/


/* remote librries */
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

/* Library for Mqtt connection */
#include <PubSubClient.h>


/*wiFi libraries*/
#include "ESP8266WiFi.h"

// Wifi network it connecting to
const char* ssid = "Xender_APdb74"; //Enter SSID
const char* password = "password"; //Enter Password

// Online Mqtt Server to connect to
const char* mqtt_server = "broker.mqttdashboard.com";

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
const uint16_t kRecvPin = D3;

IRrecv irrecv(kRecvPin);

decode_results results;

WiFiClient espClient;
PubSubClient client(espClient);

#define button_1 0xFF30CF
#define button_2 0xFF18E7
#define button_3 0xFF7A85
#define button_4 0xFF10EF
#define button_5 0xFF38C7
#define button_6 0xFF5AA5
#define button_7 0xFF42BD
#define button_8 0xFF4AB5
#define button_9 0xFF52AD

#define relay_1 D4
#define relay_2 D7
#define relay_3 D8

uint32_t Previous; 

/* Function to handle messages that arrived from mqtt broker */
void messageArrived(char* topic, byte* payload, unsigned int length, unsigned int pin) {
  Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      char state = (char)payload[i];
      if (state == '0') {
        digitalWrite(pin, LOW);   // Turn the LED on (Note that LOW is the voltage level
        Serial.println("LED off");
        Serial.println(digitalRead(pin));
       } 
       else if(state == '1') {
        digitalWrite(pin, HIGH);  // Turn the LED off by making the voltage HIGH
        Serial.println("LED on");
        Serial.println(digitalRead(pin));
       }
      }
}

/* Mqtt callback function once connected successfully */
void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,"relay_1") == 0) {
    messageArrived(topic, payload, length, D4);
   }
   if(strcmp(topic,"relay_2") == 0) {
    messageArrived(topic, payload, length, D7);
   }
   if(strcmp(topic,"relay_3") == 0) {
    messageArrived(topic, payload, length, D8);
   }
  Serial.println();

}

/* Function to reconnect to mqtt broker if not connected */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    remoteControl();
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("relay_1", "0", true);
      //client.publish("relay_2", "0", true);
      //client.publish("relay_3", "0", true);
      // ... and resubscribe
      client.subscribe("relay_1");
      client.subscribe("relay_2");
      client.subscribe("relay_3");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* Function for controlling switches with remote */
void remoteControl() {
 if (irrecv.decode(&results)) {
    // print() & println() can't handle printing long longs. (uint64_t)
    if(results.value == 0xFFFFFFFF) {
       results.value = Previous;
    }
    switch (results.value){
      // button 1 on remote should toggle switch 1
      case button_1 : 
        if(digitalRead(relay_1) == 0) {
          digitalWrite(relay_1, HIGH);
          client.publish("relay_1", "1", true);
        }
        else {
          digitalWrite(relay_1, LOW);
          client.publish("relay_1", "0", true);
        }
        break;

      // button 2 on remote should toggle switch 2 
      case button_2 : 
        if(digitalRead(relay_2) == 0) {
          digitalWrite(relay_2, HIGH);
          client.publish("relay_2", "1", true);
        }
        else {
          digitalWrite(relay_2, LOW);
          client.publish("relay_2", "0", true);
        }
        break;

      // button 3 on remote should toggle switch 3   
      case button_3 : 
        if(digitalRead(relay_3) == 0) {
          digitalWrite(relay_3, HIGH);
          client.publish("relay_3", "1", true);
        }
        else {
          digitalWrite(relay_3, LOW);
          client.publish("relay_3", "0", true);
        }
        break;
      
      case button_4 : 
        digitalWrite(relay_1, HIGH); 
        digitalWrite(relay_2, HIGH); 
        digitalWrite(relay_3, HIGH); 
        client.publish("relay_1", "1", true);
        client.publish("relay_2", "1", true);
        client.publish("relay_3", "1", true);
        break;
      case button_5 : 
        digitalWrite(relay_1, LOW); 
        digitalWrite(relay_2, LOW); 
        digitalWrite(relay_3, LOW); 
        client.publish("relay_1", "0", true);
        client.publish("relay_2", "0", true);
        client.publish("relay_3", "0", true);
        break;
    }
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();  // Receive the next value
  }
   
}

void setup() {
  // Setting pins
  pinMode(D3, INPUT);
  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  pinMode(relay_3, OUTPUT);

  // writing pins
  digitalWrite(relay_1, LOW);
  digitalWrite(relay_2, LOW);
  digitalWrite(relay_3, LOW);
  
  Serial.begin(115200);
  
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println("Setting up Ir remote");
  irrecv.enableIRIn();  // Start the receiver
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);

  // Setting up wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
     remoteControl();
     delay(500);
     Serial.print("*");
  }
  Serial.println("");
  Serial.println("WiFi connection Successful");
  Serial.print("The IP Address of Switching system is: ");
  Serial.print(WiFi.localIP());// Print the IP address

  // Connect to Mqtt broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
} 

void loop() {
  // publishStates();
  remoteControl();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(100);
}  
