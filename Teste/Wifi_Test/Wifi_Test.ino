#include <WiFi.h>

void scanLocalNetWork();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Iniciando Scan de Rede Wi-Fi");
  scanLocalNetWork();
}

void loop() {
  
}

void scanLocalNetWork(){
  // put your main code here, to run repeatedly:
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