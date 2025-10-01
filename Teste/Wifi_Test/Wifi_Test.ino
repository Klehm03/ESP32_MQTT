#include <WiFi.h> 
const String SSID = "A55 de Isaque";
const String PSWD = "12345678";


void scanLocalNetworks();

void conexaoWifi(){
  WiFi.begin(SSID,PSWD);
  Serial.println("Iniciando conexão com rede Wi-Fi");
  Serial.print("Conectando");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado!");
}

void reconexaoWifi(){
  
  Serial.println("Iniciando reconexão com rede Wi-Fi");
  Serial.print("Reconectando");
  while(WiFi.status() != WL_CONNECTED){
    WiFi.begin(SSID,PSWD);
    Serial.print(".");
    delay(2000);
  }
  Serial.println("\nReconectado!");
}


void setup() {
  Serial.begin(115200);
  scanLocalNetworks();
  conexaoWifi();
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    reconexaoWifi();
  }
}

void scanLocalNetworks(){
  Serial.println("Iniciando o Scan de redes Wi-Fi");
  int number = WiFi.scanNetworks();
    delay(500);
    if(number == -1){
      Serial.println("ERRO!, problema em encontrar alguma rede");
    }
    else{
      for(int net = 0; net < net; net++){
        Serial.printf("%d - %s | %d db\n",net,WiFi.SSID(net),WiFi.RSSI(net));
      }
      Serial.printf("Número de redes encontradas: %d\n", number);

    }
}