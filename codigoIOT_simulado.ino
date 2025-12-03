#include "DHT.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- Definição dos Pinos ---
#define PIN_LDR 36  
#define PIN_DHT 4   
#define PIN_RED 19
#define PIN_GREEN 18
#define PIN_BLUE 5
#define PIN_BTN 27

// --- Configuração ---
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// PWM
const int freq = 5000;
const int resolution = 8;

// Variáveis de Controle
volatile bool sistemaAtivo = true;
volatile unsigned long lastDebounce = 0;
bool ultimoEstado = true; 

// Interrupção
void IRAM_ATTR trataBotao() {
  if ((millis() - lastDebounce) > 300) {
    sistemaAtivo = !sistemaAtivo;
    lastDebounce = millis();
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

  Serial.begin(115200);
  delay(1000);
  dht.begin();

  pinMode(PIN_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN), trataBotao, FALLING);

  //
  ledcAttach(PIN_RED, freq, resolution);
  ledcAttach(PIN_GREEN, freq, resolution);
  ledcAttach(PIN_BLUE, freq, resolution);

  Serial.println("--- Sistema Iniciado ---");
}

void loop() {
  if (sistemaAtivo != ultimoEstado) {
    if (sistemaAtivo == false) {
      Serial.println("sistema OFF");
      setColor(0, 0, 0);
    } else {
      Serial.println("Sistema REATIVADO");
    }
    ultimoEstado = sistemaAtivo; 
  }

  if (!sistemaAtivo) {
    delay(100);
    return;
  }

  // --- Leituras ---
  int ldrValue = analogRead(PIN_LDR);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Lógica de Cores
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

// Função auxiliar PWM 
void setColor(int r, int g, int b) {
  ledcWrite(PIN_RED, r);
  ledcWrite(PIN_GREEN, g);
  ledcWrite(PIN_BLUE, b);
}
