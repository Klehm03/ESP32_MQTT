#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>

JsonDocument doc;
String message = "";

const String SSID = "iPhone";
const String PSWD = "123456789";

const String brokerUrl = "test.mosquitto.org";
const int port = 1883;

const byte pin_echo = 5;
const byte pin_trig = 6;

unsigned long duracao = 0;
int distancia = 0;

unsigned long temp_init = 0;
unsigned long temp = 0;

bool proximidade = false;

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
  pinMode(pin_trig,OUTPUT);
  pinMode(pin_echo,INPUT);
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

  digitalWrite(pin_trig, LOW);
  delayMicroseconds(5);
  digitalWrite(pin_trig, HIGH);
  delayMicroseconds(5);
  digitalWrite(pin_trig, LOW);

  duracao = pulseIn(pin_echo,HIGH);
  distancia = duracao * 0.034/2;
  if(distancia > 15){
    temp_init = millis();
    temp = millis();
    while(temp - temp_init < 3000 ){
      temp = millis();
      digitalWrite(pin_trig, LOW);
      delayMicroseconds(5);
      digitalWrite(pin_trig, HIGH);
      delayMicroseconds(5);
      digitalWrite(pin_trig, LOW);

      duracao = pulseIn(pin_echo,HIGH);
      distancia = duracao * 0.034/2;
      
      if(distancia < 15){
        break;
      }
    }

    if(temp - temp_init > 3000){
      proximidade = true;
    }else{
      proximidade = false;
    }
  }else{
    proximidade = false;
  }
  
  doc["proximidade"] = proximidade;
  doc["distancia"] = distancia;
  serializeJson(doc,message);
  mqttClient.publish("osguri/servo/comando",message.c_str());

  mqttClient.loop();
  delay(100);

  
}

