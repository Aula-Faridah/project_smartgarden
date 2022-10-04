#include <WiFi.h>
#include <DHTesp.h>
#include "RTClib.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* passwordmqtt = "objSNrgXqMXM0fMF8uxRjhy92i6fFKcT";
const char* token = "kzntxixa:kzntxixa";
const char* mqttServer = "armadillo.rmq.cloudamqp.com";
int port = 1883;
String stMac;
char mac[50];
char clientId[50];

String iddev = "lahan1";

const char* topicinfo = "tsa/k1/info";
const char* topiccmd = "tsa/k1/command";
const char* topicmode = "tsa/k1/mode";
const char* topicpersonal = "tsa/k1/personal";
const char* topicnotif = "tsa/k1/notif";
const char* topicTB = "v1/devices/me/telemetry";
const char* topicstream = "tsa/k1/stream";

const int DHT_PIN = 15;
const int DHT2_PIN = 13;
const int pinRelay = 2;

int status = 0;
int durasi = 20000;
int statussiram = 0;
bool isAuto = 0;


char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

WiFiClient espClient;
PubSubClient client(espClient);
DHTesp dht, dhtLingkungan;
DynamicJsonDocument dataEncode(1024);
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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), token, passwordmqtt)) {
      Serial.println("Connected");
      client.subscribe(topiccmd);
      client.subscribe(topicmode);
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
  Serial.println(stMessage);

  if (String(topic) == "tsa/k1/command") {
    String cmdid = getValue(stMessage, '/', 0);
    String cmdtele = getValue(stMessage, '/', 1);
    String cmd = getValue(stMessage, '/', 2);
    String balasan = "";
    if (cmdid == iddev) {
      if (cmd == "siram") {
        siramnormal();
      } else if (cmd == "info") {
        balasan += getDht();
        balasan += "/";
        balasan += getRTC();
        balasan += "/";
        balasan += cmdtele;
        balasan += "/";
        balasan += iddev;
        client.publish(topicinfo, (char*) balasan.c_str());
      } else if (cmd == "siram") {
        manual();
      }
    }
  }else if(String(topic) == "tsa/k1/mode"){
    String modetele = getValue(stMessage, '/', 0);
    String modedev = getValue(stMessage, '/', 1);
    if(modedev == "auto"){
      if(isAuto == 1){
        String balasan = "Device sudah dalam keadaan otomatis/"+modetele;
        client.publish(topicpersonal,(char*) balasan.c_str());
      }else{
        isAuto = 1;
        String balasan = "Device berhasil diganti ke mode otomatis";
        client.publish(topicnotif,(char*) balasan.c_str());
      }
      
      
    }else if(modedev == "manual"){
      if(isAuto == 0){
        String balasan = "Device sudah dalam keadaan manual/"+modetele;
        client.publish(topicpersonal,(char*) balasan.c_str());
      }else{
        isAuto = 0;
        String balasan = "Device berhasil diganti ke mode manual";
        client.publish(topicnotif,(char*) balasan.c_str());
      }
    }
  }
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length();

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
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
    result += String (lastValues.humidity) + "/" + String (lastValues2.humidity) + "/" + String (lastValues2.temperature);
  }

  return result;
}

int jadwal() {
  DateTime now = rtc.now();
  int result;

  if ((now.hour() == 8 || now.hour() == 13  || now.hour() == 17)) {

    result = 1;
  } else {
    result = 0;
  }

  return result;
}

void manual(){
  String SKUTemp = "";
  TempAndHumidity lastValues = dht.getTempAndHumidity();
  TempAndHumidity lastValues2 = dhtLingkungan.getTempAndHumidity();

  float KT = lastValues.humidity;
  float SU = lastValues2.temperature;
  float KU = lastValues2.humidity;
  SKUTemp = getSKU();
  int SKU = SKUTemp.toInt();
  if(KT<40){
      switch (SKU){
        case 11:
          siramlama();
          break;
        case 12:
          siramlama();
          break;
        case 13:
          siramlama();
          break;
        case 21:
          siramlama();
          break;
        case 22:
          siramlama();
          break;
        case 23:
          siramnormal();
          break;
        case 31:
          siramlama();
          break;
        case 32:
          siramnormal();
          break;
        case 33:
          siramsebentar();
          break;
      }
    }else{
      switch (SKU){
        case 11:
          siramlama();
          break;
        case 12:
          siramnormal();
          break;
        case 13:
          siramnormal();
          break;
        case 21:
          siramnormal();
          break;
        case 22:
          siramsebentar();
          break;
        case 23:
          siramsebentar();
          break;
        case 31:
          siramsebentar();
          break;
        case 32:
          break;
        case 33:
          break;
      }
    }
  }


String getSKU() {
  TempAndHumidity lastValues = dht.getTempAndHumidity();
  TempAndHumidity lastValues2 = dhtLingkungan.getTempAndHumidity();

  float KT = lastValues.humidity;
  float SU = lastValues2.temperature;
  float KU = lastValues2.humidity;
  String result = "";

  if (SU < 25) {
    result += "3";
  } else if (SU >= 25 && SU <= 30) {
    result += "2";
  } else {
    result += "1";
  }
  
  if (KU < 45) {
    result += "1";
  } else if (KU >= 45 && KU <= 65) {
    result += "2";
  } else {
    result += "3";
  }
  
  return result;
}

void penyiraman(int persen) {
  digitalWrite(pinRelay, HIGH);
  delay(round(durasi * (persen / 100)));
  digitalWrite(pinRelay, LOW);
  String pesanNotif = "Penyiraman berhasil dilakukan pada " + getRTC();
  pesanNotif += "\n\nCatatan: device tidak akan menerima command selama 1 jam setelah penyiraman";
  pesanNotif += "\nCatatan: status lahan akan diinformasikan 1 jam setelah penyiraman";
  pesanNotif += "\nstatus: ";
  if(isAuto){
    pesanNotif+="otomatis";
  }else{
    pesanNotif+="manual";
  }
  client.publish(topicnotif, (char*) pesanNotif.c_str());
  // delay(3600000);
  delay(5000);
  String balasan ="";
  String statusdht = getDht();
  balasan += "Pada lahan "+iddev+" waktu:\n";
  balasan += getRTC();
  balasan += "\nKelembapan tanah: "+getValue(statusdht,'/',0);
  balasan += "\nKelembapan udara: "+getValue(statusdht,'/',1);
  balasan += "\nSuhu udara: "+getValue(statusdht,'/',2);
  client.publish(topicnotif, (char*) balasan.c_str());
}

void siramnormal() {
  penyiraman(100);
}
void siramsebentar() {
  penyiraman(60);
}
void siramlama() {
  penyiraman(150);
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

String gettimeRTC() {
  DateTime now = rtc.now();
  String timeHour = "";
  String timeMinute = "";
  String result = "";

  if (now.hour() < 10 ) {
    if (now.minute() < 10) {
      timeMinute += '0' + String(now.minute());
    } else {
      timeMinute = now.minute();
    }
    timeHour += '0' + String(now.hour());
  } else {
    if (now.minute() < 10 && now.minute() >= 0) {
      timeMinute += '0' + String(now.minute());
    } else {
      timeMinute = now.minute();
    }
    timeHour = now.hour();
    
  }

  result += timeHour + ":" + timeMinute;
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

  dataEncode["kelembapan_tanah"] = getValue(getDht(), '/', 0);
  dataEncode["kelembapan_lingkungan"] = getValue(getDht(), '/', 1);
  dataEncode["suhu_lingkungan"] = getValue(getDht(), '/', 2);
  char buffer[256];
  size_t n = serializeJson(dataEncode, buffer);
  client.publish(topicTB, buffer, n);

  TempAndHumidity lastValues = dht.getTempAndHumidity();
  TempAndHumidity lastValues2 = dhtLingkungan.getTempAndHumidity();

  if (isAuto) {
    String SKUTemp = "";
    TempAndHumidity lastValues = dht.getTempAndHumidity();
    TempAndHumidity lastValues2 = dhtLingkungan.getTempAndHumidity();

    float KT = lastValues.humidity;
    float SU = lastValues2.temperature;
    float KU = lastValues2.humidity;
    SKUTemp = getSKU();
    int SKU = SKUTemp.toInt();
    if (KT < 40) {
      switch (SKU) {
        case 11:
          siramlama();
          break;
        case 12:
          siramlama();
          break;
        case 13:
          siramlama();
          break;
        case 21:
          siramlama();
          break;
        case 22:
          siramlama();
          break;
        case 23:
          siramnormal();
          break;
        case 31:
          siramlama();
          break;
        case 32:
          siramnormal();
          break;
        case 33:
          siramsebentar();
          break;
      }
    }
  } else {
    if(jadwal()){
      manual();
    }
  }
}