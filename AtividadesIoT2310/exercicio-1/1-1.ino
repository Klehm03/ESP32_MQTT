#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

JsonDocument doc;


const String SSID = "iPhone";
const String PSWD = "123456789";

const String brokerUrl = "test.mosquitto.org";
const int port = 1883;


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_SDA 5
#define I2C_SCK 6


WiFiClient espClient;
PubSubClient mqttClient(espClient);
Adafruit_SSD1306 tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void mostrarNaTela(int distancia, bool movimento, int limiar) {
  tela.clearDisplay();
  tela.setTextSize(1);
  tela.setTextColor(SSD1306_WHITE);
  tela.setCursor(0, 0);
  tela.println("Dados Recebidos");
  tela.println("----------------");
  tela.setTextSize(1);
  tela.print("Distancia: ");
  tela.println(distancia);
  tela.print("Status: ");
  tela.println(movimento);
  tela.print("Limiar: ");
  tela.println(limiar);
  tela.display();
  Serial.println(">>> Dados exibidos no display");
}


void callback(char* topic, byte* payload, unsigned long length) {
  String mensagem = "";
  for (int i = 0; i < length; i++) mensagem += (char)payload[i];

  Serial.print("Mensagem recebida do MQTT: ");
  Serial.println(mensagem);

  DeserializationError error = deserializeJson(doc, mensagem);
  if (!error) {
    int distancia = doc["distancia"];
    bool movimento = doc["movimento"];
    int limiar = doc["val_pot"];

    Serial.println(distancia);
    Serial.println(movimento);
    Serial.println(limiar);
    // Serial.println("Movimento: " + (char)movimento);
    // Serial.println("Limiar: " + (char)limiar);

    mostrarNaTela(distancia, movimento, limiar);
  }
}


void conexaoBroker() {
  mqttClient.setServer(brokerUrl.c_str(), port);
  mqttClient.setCallback(callback);

  Serial.print("Conectando ao broker MQTT...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect("MonitorCentral")) {
      Serial.println(" Conectado!");
    } else {
      Serial.print(".");
      delay(2000);
    }
  }

  mqttClient.subscribe("osguri/estacao/dados");
  Serial.println("Inscrito no tópico: osguri/estacao/dados");
}


void conexaoWifi() {
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(SSID, PSWD);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 50) {
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
  Wire.begin(I2C_SDA, I2C_SCK);

  if (!tela.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha ao inicializar o display OLED!");
    while (true)
      ;
  }

  tela.clearDisplay();
  tela.setTextSize(1);
  tela.setTextColor(SSD1306_WHITE);
  tela.setCursor(10, 25);
  tela.println("Conectando...");
  tela.display();

  conexaoWifi();
  conexaoBroker();
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    conexaoWifi();
  }

  if (!mqttClient.connected()) {
    Serial.println("MQTT desconectado. Tentando reconectar...");
    conexaoBroker();
  }

  mqttClient.loop();
}