#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

JsonDocument doc;

const String SSID = "A55 de Isaque";
const String PSWD = "12345678";

const String brokerUrl = "test.mosquitto.org";
const int port = 1883;
const char* TOPICO_SUBSCRIBE = "osguri/servo/comando";

//OLED//
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_SDA 8
#define I2C_SCL 10

//servo//
#define SERVO_PIN 5
Servo meuServo;
int anguloAtual = 61;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
Adafruit_SSD1306 tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void mostrarNaTela(byte angulo) {
  tela.clearDisplay();
  tela.setTextSize(2);
  tela.setTextColor(SSD1306_WHITE);
  tela.setCursor(0, 10);
  tela.println("Servo em:");
  tela.setTextSize(3);
  tela.setCursor(20, 35);
  tela.print(angulo);
  tela.println("°");
  tela.display();
  meuServo.write(angulo);
}

void callback(char* topic, byte* payload, unsigned long length) {
  String mensagem = "";
  for (int i = 0; i < length; i++)
    mensagem += (char)payload[i];

  Serial.print("Mensagem recebida: ");
  Serial.println(mensagem);

  DeserializationError erro = deserializeJson(doc, mensagem);
  if (!erro){
    byte angulo = doc["val_pot"];
    Serial.println(angulo);
    mostrarNaTela(angulo);
  }


  // if (doc.containsKey("angulo")) {
  //   int novoAngulo = doc["angulo"];
  //   novoAngulo = constrain(novoAngulo, 0, 180);
  //   meuServo.write(novoAngulo);
  //   anguloAtual = novoAngulo;
  //   Serial.printf("Servo movido para %d graus\n", novoAngulo);
  //   mostrarNaTela(anguloAtual);
  // }
}

void conectarBroker() {
  mqttClient.setServer(brokerUrl.c_str(), port);
  mqttClient.setCallback(callback);

  Serial.print("Conectando ao broker MQTT...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ControladorServo")) {
      Serial.println(" Conectado!");
      mqttClient.subscribe(TOPICO_SUBSCRIBE);
      Serial.print("Inscrito no tópico: ");
      Serial.println(TOPICO_SUBSCRIBE);
    } else {
      Serial.print(".");
      delay(2000);
    }
  }
}

void conectarWiFi() {
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(SSID, PSWD);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha ao conectar no Wi-Fi!");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  meuServo.attach(SERVO_PIN);
  meuServo.write(anguloAtual);

  if (!tela.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha ao inicializar o display!");
    while (true);
  }

  tela.clearDisplay();
  tela.setTextSize(1);
  tela.setTextColor(SSD1306_WHITE);
  tela.setCursor(10, 25);
  tela.println("Conectando Wi-Fi...");
  tela.display();

  conectarWiFi();
  conectarBroker();

  mostrarNaTela(anguloAtual);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    conectarWiFi();
  }

  if (!mqttClient.connected()) {
    Serial.println("MQTT desconectado. Tentando reconectar...");
    conectarBroker();
  }

  mqttClient.loop();
}