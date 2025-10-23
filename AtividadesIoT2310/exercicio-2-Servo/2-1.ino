#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>

JsonDocument doc;
String message = "";

const String SSID = "Galaxy A03s3312";
const String PSWD = "EdRp3209@@";

const String brokerUrl = "test.mosquitto.org";
const int port = 1883;

const byte pin_pot= 5;

byte val_pot = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void conexaoBroker(){
  Serial.println("Conectando ao broker");
  mqttClient.setServer(brokerUrl.c_str(),port);
  String userId = "ESP-BANANINHA123";
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

  val_pot = map(analogRead(pin_pot), 0, 4095, 0, 226);
  doc["val_pot"] = val_pot;
  serializeJson(doc,message);
  
  mqttClient.publish("osguri/servo/comando",message.c_str());
  mqttClient.loop();
  delay(100);

  
}
