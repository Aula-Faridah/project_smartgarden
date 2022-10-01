#include <WiFi.h>
#include <DHTesp.h>
#include "RTClib.h"
#include <ArduinoJson.h>
#include "ThingsBoard.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* token = "FVYWbihfPPbjdQhfHE9B";
const char* mqttServer = "thingsboard.cloud";
int port = 1883;
String stMac;
char mac[50];
char clientId[50];

String iddev = "lahan1";

const char* topicinfo = "tsa/k1/info"; 
const char* topiccmd = "tsa/k1/command"; 
const char* topicnotif = "tsa/k1/notif"; 
const char* topicTB = "v1/devices/me/telemetry";

const int DHT_PIN = 15;
const int DHT2_PIN = 13;
const int pinRelay = 3;

char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

WiFiClient espClient;
PubSubClient client(espClient);
ThingsBoard tb(espClient);
DHTesp dht, dhtLingkungan;
DynamicJsonDocument dataEncode(1024);
RTC_DS1307 rtc;
// X509List cert(TELEGRAM_CERTIFICATE_ROOT);
// UniversalTelegramBot bot(BOTtoken, client);

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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),token,password)) {
      Serial.println("Connected");
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
  String stMessage = "";
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    stMessage += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "tsa/k1/command") {
    String cmdid = getValue(stMessage,'/',0);
    String cmdtele = getValue(stMessage,'/',1);
    String cmd = getValue(stMessage,'/',2);
    String balasan = "";
    if(cmdid == iddev){
      if(cmd == "siram"){
        siram();
      }else if(cmd == "info"){
        balasan += getDht();
        balasan += "/";
        balasan += getRTC();
        balasan += "/";
        balasan += cmdtele;
        balasan += "/";
        balasan += iddev;
        client.publish(topicinfo, (char*) balasan.c_str());
      }
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
  TempAndHumidity lastValues2 = dhtLingkungan.getTempAndHumidity();
  String result = "";

  if (isnan(lastValues.humidity) || isnan(lastValues.temperature || isnan(lastValues2.humidity) || isnan(lastValues2.temperature))) {
    result = "Failed to read from DHT sensor!";
  } else {
    result += String (lastValues.humidity)+"/"+String (lastValues2.humidity) + "/" + String (lastValues2.temperature);
  }

  return result;
}

void siram(){
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, HIGH);
  String pesanNotif = "Penyiraman berhasil dilakukan pada\n"+getRTC();
  client.publish(topicnotif,(char*) pesanNotif.c_str());
  delay(10000);
  digitalWrite(pinRelay, LOW);
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
  setup_wifi();
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
  dhtLingkungan.setup(DHT2_PIN, DHTesp::DHT22);
  }

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Serial.println("Send data tb");
  dataEncode["kelembapan_tanah"] = getValue(getDht(),'/',0);
  dataEncode["kelembapan_lingkungan"] = getValue(getDht(),'/',1);
  dataEncode["suhu_lingkungan"] = getValue(getDht(),'/',2);
  char buffer[256];
  size_t n = serializeJson(dataEncode, buffer);
  client.publish(topicTB, buffer, n);
  delay(500);
}
