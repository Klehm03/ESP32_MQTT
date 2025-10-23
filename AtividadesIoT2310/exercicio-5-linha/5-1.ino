#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

JsonDocument doc;
String message = "";

const String SSID = "iPhone";
const String PSWD = "123456789";

const String brokerUrl = "test.mosquitto.org";
const int port = 1883;

const byte pin_line = 6;

int estado = 0;

int movimento = 0;

#define SERVO_PIN 5
Servo servo;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void conexaoBroker(){
  Serial.println("Conectando ao broker");
  mqttClient.setServer(brokerUrl.c_str(),port);
  String userId = "ESP-BANANINHA";
  while(!mqttClient.connected()){
    mqttClient.connect(userId.c_str());
    Serial.println(".");
    delay(5000);
  }
  mqttClient.subscribe("osguri/servo/comando");
  mqttClient.setCallback(callback);
  Serial.println("mqtt Conectado com sucesso!");
}

void conexaoWifi() {
  Serial.println("Iniciando conexão com rede Wi-Fi");
  Serial.print("Conectando");

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

void callback(char* topico, byte* payload, unsigned long length){
  String mensagem = "";
  for(int i=0; i < length; i++){
    mensagem += (char) payload[i];
  }
  Serial.println(mensagem);
}

void setup() {
  Serial.begin(115200);
  pinMode(pin_line, INPUT);
  servo.attach(SERVO_PIN);
  conexaoWifi();
  conexaoBroker();
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Conexão Wi-Fi perdida");
    conexaoWifi();
  }

  if(!mqttClient.connected()){
    Serial.println("Conexão Broker perdida");
    conexaoBroker();
  }

  estado = digitalRead(pin_line);

  if(estado == 0){
    servo.write(90);
  } else if(estado == 1) {
    servo.write(0);
  }

  doc["estado"] = estado;

  serializeJson(doc,message);
  mqttClient.publish("osguri/servo/comando",message.c_str());
  mqttClient.loop();
  delay(500);
}

