#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>



const String SSID = "Dudunet";
const String PSWD = "dudu1234";

const char* brokerpass = "Dudu1234";
const char* brokeruser = "dududamassa2";
const String brokerUrl = "224a031c7e714f00b6f5367683fd460c.s1.eu.hivemq.cloud";
const int port = 8883;

const char* Topic_LWT = "ProvaPratica/IoT/Status2";
const int QoS_LWT = 1;
const bool Retain_LWT = true;

String status_esp2 = "Offline";

const byte pin_echo1 = 6;
const byte pin_trig1 = 7;
const byte pin_echo2 = 1;
const byte pin_trig2 = 8;

unsigned long duracao = 0;
int distancia1 = 0;
int distancia2 = 0;
bool entrada = false;

unsigned long Temp_init = 0;
unsigned long Temp_finish = 0;

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

JsonDocument doc;
String message = "";

void ConectarWifi(){
  Serial.print("Iniciando conaxão com rede WiFi");
  WiFi.begin(SSID, PSWD);
  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED && retry_count < 20) {
    Serial.print(".");
    delay(500);
    retry_count++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado");
  } else {
    Serial.println("\nFalha ao conectar");
  }
}

void ConectarBroker(){
  Serial.print("Conectando ao broker");
  mqttClient.setServer(brokerUrl.c_str(),port);
  String userId = "ESP-SLA";
  userId += String(random(0xffff), HEX);
  while(!mqttClient.connected()){
    status_esp2 = "Offline";
    doc["status_esp2"] = status_esp2;
    serializeJson(doc,message);
    mqttClient.connect(userId.c_str(),
                       brokeruser, 
                       brokerpass,
                       Topic_LWT, 
                       QoS_LWT, 
                       Retain_LWT, 
                       message.c_str());

    Serial.print(".");
    delay(2000);
  }
  status_esp2 = "Online";
  doc["status_esp2"] = status_esp2;
  serializeJson(doc, message);

  mqttClient.publish(Topic_LWT,message.c_str(), Retain_LWT);
  Serial.println("\nConectado com Sucesso!");
}

void setup() {
  Serial.begin(115200);
  espClient.setInsecure();
  pinMode(pin_echo1, INPUT);
  pinMode(pin_trig1, OUTPUT);
  pinMode(pin_echo2, INPUT);
  pinMode(pin_trig2, OUTPUT);
  ConectarWifi();
  ConectarBroker();
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Conexão Wi-Fi perdida");
    ConectarWifi();
  }
  if(!mqttClient.connected()){
    Serial.println("Conexão Broker perdida");
    ConectarBroker();
  }
  digitalWrite(pin_trig1, HIGH);
  delayMicroseconds(100);
  digitalWrite(pin_trig1, LOW);
  duracao = pulseIn(pin_echo1,HIGH);
  distancia1 = (duracao*(340.29/10000))/2;

  digitalWrite(pin_trig2, HIGH);
  delayMicroseconds(100);
  digitalWrite(pin_trig2, LOW);
  duracao = pulseIn(pin_echo2,HIGH);
  distancia2 = (duracao*(340.29/10000))/2;

  if(distancia1 < 50 ){
    entrada = true;
    doc["entrada"] = entrada;
    serializeJson(doc,message);
    Temp_init = millis();
    Temp_finish = millis();
    while(Temp_init < Temp_finish+2000){
      mqttClient.loop();
      digitalWrite(pin_trig1, HIGH);
      delayMicroseconds(100);
      digitalWrite(pin_trig1, LOW);
      duracao = pulseIn(pin_echo1,HIGH);
      distancia1 = (duracao*(340.29/10000))/2;

      digitalWrite(pin_trig2, HIGH);
      delayMicroseconds(100);
      digitalWrite(pin_trig2, LOW);
      duracao = pulseIn(pin_echo2,HIGH);
      distancia2 = (duracao*(340.29/10000))/2;
      if(distancia2 > 50 || distancia1 > 50){
        Temp_init = millis();
      }
    }
    mqttClient.publish("ProvaPratica/IoT/Sensor",message.c_str());

  } else if(distancia2 < 50){
      entrada = false;
      doc["entrada"] = entrada;
      serializeJson(doc,message);

      Temp_init = millis();
      Temp_finish = millis();
      while(Temp_init < Temp_finish+2000 ){
        mqttClient.loop();
        digitalWrite(pin_trig1, HIGH);
        delayMicroseconds(100);
        digitalWrite(pin_trig1, LOW);
        duracao = pulseIn(pin_echo1,HIGH);
        distancia1 = (duracao*(340.29/10000))/2;

        digitalWrite(pin_trig2, HIGH);
        delayMicroseconds(100);
        digitalWrite(pin_trig2, LOW);
        duracao = pulseIn(pin_echo2,HIGH);
        distancia2 = (duracao*(340.29/10000))/2;
        if(distancia2 > 50 || distancia1 > 50){
          Temp_init = millis();
        }
      }
      mqttClient.publish("ProvaPratica/IoT/Sensor",message.c_str());
    

  }



  mqttClient.loop();
  delay(100);
}