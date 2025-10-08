#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // Largura da tela
#define SCREEN_HEIGHT 64  // Altura da tela

#define I2C_SCK 6
#define I2C_SDA 5

Adafruit_SSD1306 tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCK); //Inicia a comunicação I2C
  tela.begin(SSD1306_SWITCHCAPVCC, 0x3C); // SSD1306_SWITCHCAPVCC Vai usar a alimentação da placa pra conectar e td mais
  tela.clearDisplay(); // Apaga a tela
  tela.setTextSize(2); // Seta o tamanho da letra
  tela.setTextColor(SSD1306_WHITE); // Colocar este comando, se não não funciona
  tela.setCursor(1,1);
  tela.println("Hello World!");
  tela.display(); // display mostra na tela
}

void loop() {
  // put your main code here, to run repeatedly:
  tela.clearDisplay();
  tela.setCursor(40, 20); // Coluna x Linha
  tela.print(millis()/1000);
  tela.println("s");
  tela.display();
}
