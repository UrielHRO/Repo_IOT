#include "DHT.h"

// --- Definição dos Pinos ---
#define PIN_LDR 36 
#define PIN_DHT 4
#define PIN_RED 19
#define PIN_GREEN 18
#define PIN_BLUE 5
#define PIN_BTN 27

// --- Configuração DHT ---
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// --- Configuração PWM ---
const int freq = 5000;
const int resolution = 8;
const int chRed = 0;
const int chGreen = 1;
const int chBlue = 2;

// --- Variáveis de Controle ---
volatile bool sistemaAtivo = true;
volatile unsigned long lastDebounce = 0;

void IRAM_ATTR trataBotao() {
  if ((millis() - lastDebounce) > 300) {
    sistemaAtivo = !sistemaAtivo;
    lastDebounce = millis();
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(PIN_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN), trataBotao, FALLING);

  // Configuração PWM
  ledcSetup(chRed, freq, resolution);
  ledcSetup(chGreen, freq, resolution);
  ledcSetup(chBlue, freq, resolution);

  ledcAttachPin(PIN_RED, chRed);
  ledcAttachPin(PIN_GREEN, chGreen);
  ledcAttachPin(PIN_BLUE, chBlue);
}

void loop() {
  if (!sistemaAtivo) {
    setColor(0, 0, 0);
    delay(100); 
    return;
  }

  // Leitura do VP (GPIO 36)
  int ldrValue = analogRead(PIN_LDR);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Erro DHT!");
    return;
  }

  // Lógica de cores
  if (ldrValue < 1000) {
    setColor(0, 0, 255); // Azul
  } else if (ldrValue < 2500) {
    setColor(0, 255, 0); // Verde
  } else {
    setColor(255, 0, 0); // Vermelho
  }

  Serial.printf("Luz: %d | Temp: %.1fC | Umid: %.1f%%\n", ldrValue, t, h);
  delay(1000);
}

void setColor(int r, int g, int b) {
  ledcWrite(chRed, r);
  ledcWrite(chGreen, g);
  ledcWrite(chBlue, b);
}
