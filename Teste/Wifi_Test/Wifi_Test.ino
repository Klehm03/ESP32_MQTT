#include <WiFi.h>
const String SSID = "iPhone";
const String PSW = "iot_sul_123";

void scanLocalNetWork();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  scanLocalNetWork();
  Serial.println("Iniciando conexão com rede WiFi");
  WiFi.begin(SSID, PSW);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado");
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void scanLocalNetWork(){
  Serial.println("Iniciando Scan de Rede Wi-Fi");
  int number = WiFi.scanNetworks();
  delay(500);
  if (number == -1){
    Serial.println("ERRO!");
  }
  else{
    for(int net = 0; net < number; net++){
      Serial.printf("%d - %s | %d db\n", net ,WiFi.SSID(net), WiFi.RSSI(net));
    }
    //Serial.printf("Número de redes encontradas: %d\n",number);
  }
}