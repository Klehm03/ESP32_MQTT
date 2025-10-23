#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>

JsonDocument doc;
String message = "";

const String SSID = "iPhone";
const String PSWD = "123456789";

const String brokerUrl = "test.mosquitto.org";
const int port = 1883;

const byte pin_pir = 10;
const byte pin_led = 20;

int movimento = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void conexaoBroker(){                                                       
  Serial.println("Conectando ao broker");
  mqttClient.setServer(brokerUrl.c_str(),port);
  
  while(!mqttClient.connected()){
    String userId = "ESP-BANANINHA";
    userId += String(random(0xffff),HEX);
    mqttClient.connect(userId.c_str());
    Serial.println(".");
    delay(5000);
  }
  mqttClient.subscribe("osguri/servo/reset");
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

  DeserializationError error = deserializeJson(doc,mensagem);
  if(!error){

    String comando = doc["comando"];
    if(comando == "reset"){
      digitalWrite(pin_led,LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pin_led, OUTPUT);
  pinMode(pin_pir, INPUT);
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

  movimento = digitalRead(pin_pir);
  serializeJson(doc,message);


  if (movimento == 1){
    digitalWrite(pin_led, HIGH);
    mqttClient.publish("osguri/servo/comando",message.c_str());
  }

  doc["movimento"] = movimento;


  mqttClient.loop();
  delay(100);
}

