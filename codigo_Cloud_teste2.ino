#include "thingProperties.h"
#include "DHT.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Pinos
#define PIN_LDR 36   
#define PIN_DHT 4    
#define PIN_RED 19
#define PIN_GREEN 18
#define PIN_BLUE 5
#define PIN_BTN 27

//Configurações
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// PWM
const int freq = 5000;
const int resolution = 8;

unsigned long ultimoTempo = 0;
const long intervalo = 2000; 

// Controle do Botão Físico 
volatile bool botaoAcionado = false;
volatile unsigned long lastDebounce = 0;

void IRAM_ATTR trataBotao() {
  if ((millis() - lastDebounce) > 300) {
    botaoAcionado = true; 
    lastDebounce = millis();
  }
}

void setup() {
  // Desativa Brownout
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  dht.begin();
  delay(1500); 

  //Inicialização do Arduino Cloud
  initProperties(); 
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Pinos
  pinMode(PIN_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN), trataBotao, FALLING);

  //PWM 
  ledcAttach(PIN_RED, freq, resolution);
  ledcAttach(PIN_GREEN, freq, resolution);
  ledcAttach(PIN_BLUE, freq, resolution);

  // Valor inicial
  estado_sistema = true; 
}

void loop() {
  ArduinoCloud.update();

  //Trata Botão Físico
  if (botaoAcionado) {
    estado_sistema = !estado_sistema; // Inverte o valor
    botaoAcionado = false;
    onEstadoSistemaChange(); 
  }

  //Timer
  if (millis() - ultimoTempo >= intervalo) {
    ultimoTempo = millis();
    
    if (estado_sistema) {
      lerSensores();
    }
  }
}

void lerSensores() {
  int ldrRaw = analogRead(PIN_LDR);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Atualiza variáveis da Nuvem
  luminosidade = ldrRaw;
  temperatura = t;
  umidade = h;

  // Lógica RGB
  if (luminosidade < 1000) {
    setColor(0, 0, 255); // Azul
    coresRGB = 1;       
  } else if (luminosidade < 2500) {
    setColor(0, 255, 0); // Verde
    coresRGB = 2;       
  } else {
    setColor(255, 0, 0); // Vermelho
    coresRGB = 3;        
  }

  Serial.printf("Luz: %d | Temp: %.1f | Umid: %.1f\n", luminosidade, temperatura, umidade);
}


void onEstadoSistemaChange() {
  if (estado_sistema) {
    Serial.println("Sistema REATIVADO");
    lerSensores();
  } else {
    Serial.println("sistema desativado pelo botão/nuvem");
    setColor(0, 0, 0);
    coresRGB = 0; 
  }
}

void setColor(int r, int g, int b) {
  ledcWrite(PIN_RED, r);
  ledcWrite(PIN_GREEN, g);
  ledcWrite(PIN_BLUE, b);
}
