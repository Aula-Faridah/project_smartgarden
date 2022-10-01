#include <WiFi.h>
#include <DHTesp.h>
#include "RTClib.h"
#include "ThingsBoard.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqttServer = "thingsboard.cloud";
int port = 1883;
String stMac;
char mac[50];
char clientId[50];

const char* topicinfo = "tsa/k1/info"; 
const char* topiccmd = "tsa/k1/command"; 
const char* topicnotif = "tsa/k1/notif"; 
const char* topicTB "v1/devices/me/telemetery";

const int DHT_PIN = 15;
const int DHT2_PIN = 13;
const int pinRelay = 3;

char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

WiFiClient espClient;
PubSubClient client(espClient);
ThingsBoard tb(espClient);
DHTesp dht, dht2;
RTC_DS1307 rtc;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "clientId-%ld", r);
    if (client.connect(clientId)) {
      Serial.print(clientId);
      Serial.println(" connected");
      client.subscribe("topicName/led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String stMessage;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    stMessage += (char)message[i];
  }
  Serial.println();

  if (String(topic) == topicinfo) {
    
    if(stMessage == "on"){
      client.publish(topicinfo, );
    }
    else if(stMessage == "off"){
      
    }
  }
}

String getValue(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length();
 
	for (int i = 0; i <= maxIndex && found <= index; i++) {
    	if (data.charAt(i) == separator || i == maxIndex) {
        	found++;
        	strIndex[0] = strIndex[1] + 1;
        	strIndex[1] = (i == maxIndex) ? i+1 : i;
    	}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String getDht() {

  TempAndHumidity lastValues = dht.getTempAndHumidity();
  TempAndHumidity lastValues2 = dht2.getTempAndHumidity();
  String result = "";

  if (isnan(lastValues.humidity) || isnan(lastValues.temperature || isnan(lastValues2.humidity) || isnan(lastValues2.temperature))) {
    result = "Failed to read from DHT sensor!";
  } else {
    result += String (lastValues.temperature)+"/"+String (lastValues.humidity) + "/" + String (lastValues2.temperature);
  }

  return result;
}

String getRTC() {
  DateTime now = rtc.now();
  String month = "";
  String result = "";

  switch (now.month()) {
    case 1 :
      month = "Januari";
      break;
    case 2 :
      month = "Februari";
      break;
    case 3 :
      month = "Maret";
      break;
    case 4 :
      month = "April";
      break;
    case 5 :
      month = "Mei";
      break;
    case 6 :
      month = "Juni";
      break;
    case 7 :
      month = "Juli";
      break;
    case 8 :
      month = "Agustus";
      break;
    case 9 :
      month = "September";
      break;
    case 10 :
      month = "Oktober";
      break;
    case 11 :
      month = "November";
      break;
    case 12 :
      month = "Desember";
      break;
  }

  result += String (daysOfTheWeek[now.dayOfTheWeek()]) + ", " + now.day() + " " + month + " " + now.year();
  return result;
}

void setup() {
  Serial.begin(115200);
  // setup_wifi();
  pinMode(pinRelay, OUTPUT);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  client.setServer(mqttServer, port);
  client.setCallback(callback);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  dht2.setup(DHT2_PIN, DHTesp::DHT22);
  }

void loop() {
  
  delay(1000);
}
