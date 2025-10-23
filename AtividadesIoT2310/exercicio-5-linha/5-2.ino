#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  

#define I2C_SCK 5
#define I2C_SDA 6

Adafruit_SSD1306 tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const byte pin_led_g = 11;
const byte pin_led_y = 10;

String comando = "";

JsonDocument doc;
String message = "";

const String SSID = "iPhone";
const String PSWD = "123456789";

const String brokerUrl = "test.mosquitto.org";
const int port = 1883;

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

  DeserializationError error = deserializeJson(doc,mensagem);
  if(!error){
    int fora_da_linha = doc["estado"];

    if(fora_da_linha == 1){
      tela.clearDisplay();
      tela.setTextSize(1);
      tela.setCursor(1,0);
      tela.println("Status: Robo fora da linha!");
      tela.display();
      digitalWrite(pin_led_g,LOW);
      digitalWrite(pin_led_y,HIGH);
    }else{
      tela.clearDisplay();
      tela.setTextSize(1);
      tela.setCursor(1,0);
      tela.println("Status: Robo na linha!");
      tela.display();
      digitalWrite(pin_led_g,HIGH);
      digitalWrite(pin_led_y,LOW);
      // serializeJson(doc,messi/servo/comando",message.c_str());
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCK);
  pinMode(pin_led_g, OUTPUT);
  pinMode(pin_led_y, OUTPUT);

  tela.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  tela.setTextColor(SSD1306_WHITE);
  tela.clearDisplay();
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
  // tela.clearDisplay();
  // tela.setTextSize(1);
  // tela.setCursor(1,0);
  // tela.println("Esperando registro..");
  // tela.display();

  mqttClient.loop();
  delay(100);
}