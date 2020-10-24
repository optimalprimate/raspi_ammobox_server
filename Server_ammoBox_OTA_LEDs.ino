#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

//currently ip .75
// TEMP wire to pin:
#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);

//LEDs on pin:
#define LED_PIN    14
//How many LEDs:
#define LED_COUNT 60
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


const char* ssid = "***";
const char* password =  "***";
const char* mqttServer = "192.168.1.**";
const int mqttPort = 1883;
int holding_var = 0;
unsigned long time_now = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
strip.show();            // Turn OFF all strip ASAP
strip.setBrightness(50); //birghtness has to be set in advance
Serial.begin(9600); 

  sensors.begin(); 
WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
 client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
 // Create a random client ID
    String clientId = "ammobox_";
    clientId += String(random(0xffff), HEX);
if (client.connect(clientId.c_str())) {  

 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  client.subscribe("esp/ammo");
//OTS stuff ------------------------------->
ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //end of OTA stuff ------------------------->


pinMode(5, OUTPUT); //NPN for fan2, 1= ON, 2 = OFF
pinMode(4, OUTPUT); //NPN for fan1, 1= ON, 2 = OFF
client.publish("esp/test", "Hello from AMMO_BOX");
}

void reconnect() {
  Serial.println("Reconnect activated");
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");


// Create a random client ID
    String clientId = "Ammo_Box_";
    clientId += String(random(0xffff), HEX);
if (client.connect(clientId.c_str())) {  
      Serial.println("connected");
       delay(1000);
        client.publish("esp/test", "Hello from AMMO_Box(recon)");
        client.subscribe("esp/ammo");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Callback function for recieving messages ------------------->
void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    holding_var = (payload[i]);
     Serial.println(holding_var);
      if(holding_var == 65) { //set to what inject is 
      digitalWrite(5,HIGH);
    }
          if(holding_var == 66) { //set to what inject is 
      digitalWrite(5,LOW);
    }
          if(holding_var == 67) { //set to what inject is 
      digitalWrite(4,HIGH);
    }
          if(holding_var == 68) { //set to what inject is 
      digitalWrite(4,LOW);
    }
          if(holding_var == 69) { //set to what inject is 
          //LEDs GREEN
          for(int i=0;i<LED_COUNT;i++){
            strip.setPixelColor(i, strip.Color(0,255,0));
          }
           strip.show();
    }
          if(holding_var == 70) { //set to what inject is 
          //LEDs RED
          for(int i=0;i<LED_COUNT;i++){
            strip.setPixelColor(i, strip.Color(255,0,0));
          }
           strip.show();
    }

 
  
  
  
  
  }
}

//end of callback ----------------------------------------->

void loop() {
 ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }


//temp read
 if(millis() > time_now + 90000){
      sensors.requestTemperatures(); 
      client.publish("esp/ammo", String(sensors.getTempCByIndex(0)).c_str());
      
    time_now = millis();
    
   }
  client.loop();

}
