#include <DHT.h>

//Definição dos Pinos
#define PIN_TRIG 5
#define PIN_ECHO 18
#define PIN_LDR 34     
#define PIN_DHT 4      
#define PIN_BUTTON 14 

// Pinos do Módulo RGB
#define PIN_RED   19
#define PIN_GREEN 21
#define PIN_BLUE  22

//Configurações dos Sensores
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

//Configurações do PWM 
const int freq = 5000;
const int resolution = 8;
const int channelRed = 0;
const int channelGreen = 1;
const int channelBlue = 2;

//Configurações de Distância
const int minDist = 5;  
const int maxDist = 40; 

// Variáveis de Controle do Sistema 
volatile bool sistemaAtivo = true;       
volatile unsigned long ultimoDebounce = 0; 
const unsigned long tempoDebounce = 250; 

//Função de Interrupção (ISR)
void IRAM_ATTR isrBotao() {
  if ((millis() - ultimoDebounce) > tempoDebounce) {
    sistemaAtivo = !sistemaAtivo; 
    ultimoDebounce = millis();
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LDR, INPUT);
  
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), isrBotao, FALLING);

  ledcAttachChannel(PIN_RED, freq, resolution, channelRed);
  ledcAttachChannel(PIN_GREEN, freq, resolution, channelGreen);
  ledcAttachChannel(PIN_BLUE, freq, resolution, channelBlue);
}

void loop() {
  if (!sistemaAtivo) {
    // Apaga o LED
    ledcWrite(PIN_RED, 0);
    ledcWrite(PIN_GREEN, 0);
    ledcWrite(PIN_BLUE, 0);
    
    Serial.println("SISTEMA DESLIGADO");
    delay(500); 
    return;     
  }

  int ldrValue = analogRead(PIN_LDR); 
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Leitura do Ultrassônico
  long duration;
  float distance;
  
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  duration = pulseIn(PIN_ECHO, HIGH);
  distance = duration * 0.034 / 2;

  // Lógica de Cores
  float constrainedDist = constrain(distance, minDist, maxDist);
  int mapVal = map(constrainedDist, minDist, maxDist, 0, 510);
  int r = 0, g = 0, b = 0;

  if (mapVal < 255) {
    r = 255 - mapVal;
    g = mapVal;
    b = 0;
  } else {
    r = 0;
    g = 255 - (mapVal - 255);
    b = (mapVal - 255);
  }

  ledcWrite(PIN_RED, r);
  ledcWrite(PIN_GREEN, g);
  ledcWrite(PIN_BLUE, b);

  // Print Serial
  Serial.print("Status: LIGADO | Dist: ");
  Serial.print(distance);
  Serial.print(" cm | LDR: ");
  Serial.print(ldrValue);
  Serial.print(" | Temp: ");
  Serial.println(t);
  
  delay(100);
}
