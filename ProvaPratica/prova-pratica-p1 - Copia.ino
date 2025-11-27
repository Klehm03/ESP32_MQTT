#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>
#include <WiFiClientSecure.h>

#define LED_PIN 8
#define NUMPIXELS 1

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define I2C_SCK 21
#define I2C_SDA 20

#define DHTPIN 9
#define DHTTYPE DHT11

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

DHT dht11(DHTPIN, DHTTYPE);
Adafruit_SSD1306 tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const String SSID = "Dudunet";
const String PSWD = "dudu1234";

const char* brokerUser = "dududamassa";
const char* brokerPass = "Dudu1234";
const String brokerUrl = "224a031c7e714f00b6f5367683fd460c.s1.eu.hivemq.cloud";
const int port = 8883;

const char* Topic_LWT = "ProvaPratica/IoT/Status1";
const int QoS_LWT = 1;
const bool Retain_LWT = true;

String status_esp1 = "Offline";
String status_esp2 = "Offline";

int pessoas = 0;
int nivel_op = 0;

int temperature = 0;
int humidity = 0;

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);


String message = "";
JsonDocument doc;

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
    JsonDocument doc;
    if(strcmp(topico, "ProvaPratica/IoT/QntdPessoas") == 0){
      DeserializationError error = deserializeJson(doc,mensagem);
      if(!error){
      int quantidade = doc["quantidade"];
      pessoas = quantidade;  

      Serial.print(quantidade);

      if(quantidade <= 10){
        pixels.setPixelColor(0, pixels.Color(0, 255, 0));
        pixels.setBrightness(255);
        pixels.show();
      } else if(quantidade > 10 && quantidade <= 15){
        pixels.setPixelColor(0, pixels.Color(255, 200, 0));
        pixels.setBrightness(255);
        pixels.show();
      } else{
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
        pixels.setBrightness(255);
        pixels.show();
      }
    }  
  }else if(strcmp(topico, "ProvaPratica/IoT/QntdPessoasMax") == 0){
    DeserializationError error = deserializeJson(doc,mensagem);
    if(!error){
      int max = doc["max"];
      nivel_op = max;

      Serial.print(max);
    }
  }
}

void ConectarBroker(){
  Serial.println("Conectando ao broker...");
  mqttClient.setServer(brokerUrl.c_str(),port);
  String userId = "ESP-SLA";
  JsonDocument doc;
  while(!mqttClient.connected()){
    userId += String(random(0xffff), HEX);
    status_esp1 = "Offline";
    doc["status_esp1"] = status_esp1;
    serializeJson(doc, message);
    mqttClient.connect(userId.c_str(), //erro de conexão
                       brokerUser, 
                       brokerPass,
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

  mqttClient.publish(Topic_LWT, message.c_str(), Retain_LWT);
  mqttClient.setCallback(callback);
  mqttClient.subscribe("ProvaPratica/IoT/QntdPessoas");
  mqttClient.subscribe("ProvaPratica/IoT/QntdPessoasMax");
  Serial.println("Conectado com Sucesso!");
}

void setup() {
  Serial.begin(115200);
  dht11.begin();
  espClient.setInsecure();
  ConectarWifi();
  ConectarBroker();
  pixels.begin();
  pinMode(LED_PIN, OUTPUT);
  Wire.begin(I2C_SDA, I2C_SCK);
  tela.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  tela.setTextColor(SSD1306_WHITE);
  tela.clearDisplay();
}

long start = 0;

void loop() {
  start = millis();
  JsonDocument doc;
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Conexão Wi-Fi perdida");
    ConectarWifi();
  }
  
  if(!mqttClient.connected()){
    Serial.println("Conexão Broker perdida");
    ConectarBroker();
  }
  
  Serial.println("1");
  
  // int result = dht11.readTemperatureHumidity(temperature, humidity);
  float temperature = dht11.readTemperature();
  float humidity = dht11.readHumidity();

  tela.clearDisplay();
  tela.setTextSize(0);
  tela.setCursor(0,0);
  tela.println("Dados de Sala");
  tela.println("---------------------");
  tela.print("Qntd de pessoas: ");
  tela.println(pessoas);//quantidade de pessoas
  tela.print("Temperatura: ");
  tela.println(temperature);//temperatura
  tela.print("Humidade: ");
  tela.println(humidity);//humidade
  tela.print("Nivel de OP: ");
  if (pessoas > nivel_op * 1/*porcentagem desejada*/){
    tela.println("Livre");
  } else if(pessoas >= nivel_op * 1/*porcentagem desejada*/ &&  pessoas <= nivel_op * 1/*porcentagem desejada*/){
    tela.println("Atencao");
  } else{
    tela.println("Alerta");
  }
  tela.print("Quant Max de P: ");
  tela.println(nivel_op);
  tela.println("---------------------");
  tela.display();

  Serial.println("2");
  
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  serializeJson(doc, message);
  mqttClient.publish("ProvaPratica/IoT/DHT", message.c_str());

  pixels.show();

  Serial.println("3");

  mqttClient.loop();
  Serial.println(millis()- start);
  delay(2500);
}