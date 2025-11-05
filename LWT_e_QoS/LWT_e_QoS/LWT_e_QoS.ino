#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  

#define I2C_SCK 6
#define I2C_SDA 5

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, -10800);


Adafruit_SSD1306 tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const String SSID = "Dudunet";
const String PSWD = "dudu1234";
const String brokerUrl = "test.mosquitto.org";
const int port = 1883;

const char* Topic_LWT = "Aula/Boards/Izacky/Status";
const int QoS_LWT = 1;
const bool Retain_LWT = true;

String status_esp2 = "Offline";
String status_esp1 = "Offline";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

JsonDocument doc;
String message = "";

void ConectarWifi(){
  Serial.println("Iniciando conaxão com rede WiFi");
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
   status_esp2 = doc["status_esp2"].as<String>();
   
  }
}

void ConectarBroker(){
  Serial.println("Conectando ao broker...");
  mqttClient.setServer(brokerUrl.c_str(),port);
  String userId = "ESP-GURIS";
  userId += String(random(0xffff), HEX);
  while(!mqttClient.connected()){
    status_esp1 = "Offline";
    doc["status_esp1"] = status_esp1;
    serializeJson(doc, message);

    mqttClient.connect(userId.c_str(),
                       "", 
                       "",
                       Topic_LWT, 
                       QoS_LWT, 
                       Retain_LWT, 
                       message.c_str());
    Serial.print(".");
    delay(2000);
  }
  status_esp1 = "Online";
  doc["status_esp1"] = status_esp1;
  serializeJson(doc, message);

  mqttClient.publish(Topic_LWT,message.c_str(), Retain_LWT);
  mqttClient.setCallback(callback);
  mqttClient.subscribe("Aula/Boards/DUDUDAMASSA/Status");
  Serial.println("Conectado com Sucesso!");
}

void setup() {
  Serial.begin(115200);
  ConectarWifi();
  ConectarBroker();
  Wire.begin(I2C_SDA, I2C_SCK);
  tela.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  tela.setTextColor(SSD1306_WHITE);
  tela.clearDisplay();
  timeClient.begin();
  // timeClient.setTimeOffset(61200);
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

  tela.clearDisplay();
  tela.setTextSize(0);
  tela.setCursor(0,0);
  tela.println("SISTEMA DE MONITORAMENTO");
  tela.println("---------------------");
  tela.print("PLACA 1: ");
  tela.println(status_esp1);
  tela.print("PLACA 2: ");
  tela.println(status_esp2);
  tela.println("---------------------");
  tela.println("Ultima atualizacao");
  tela.print(timeClient.getFormattedTime());
  tela.display();

  mqttClient.loop();
  delay(100);
}