#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>

JsonDocument doc;
String message = "";


const String SSID = "iPhone";
const String PSWD = "123456789";

const String brokerUrl = "test.mosquitto.org";
const int port = 1883;

const byte pin_pot = 5;
const byte pin_echo = 22;
const byte pin_trigg = 21;
const byte pin_pir = 10;

int val_pot = 0;

unsigned long tempo = 0;
int distancia = 0;

int movimento = 0;

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
  mqttClient.subscribe("osguri/estacao/dados");
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
  pinMode(pin_pot,INPUT);
  pinMode(pin_echo,INPUT);
  pinMode(pin_trigg,OUTPUT);
  pinMode(pin_pir,INPUT);
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

  val_pot = map(analogRead(pin_pot), 0, 4095, 0, 100);

  digitalWrite(pin_trigg, LOW);
  delayMicroseconds(5);
  digitalWrite(pin_trigg, HIGH);
  delayMicroseconds(5);
  digitalWrite(pin_trigg, LOW);

  tempo = pulseIn(pin_echo, HIGH);
  distancia = tempo * 0.034/2;

  movimento = digitalRead(pin_pir);

  doc["val_pot"] = val_pot;
  doc["movimento"] = movimento;
  doc["distancia"] = distancia;


  serializeJson(doc, message);
  mqttClient.publish("osguri/estacao/dados", message.c_str());
  mqttClient.loop();
  delay(5000);  
}
