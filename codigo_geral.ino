#include "DHT.h"

#define PIN_LDR        4    
#define PIN_DHT        5    
#define PIN_TRIG       6   
#define PIN_ECHO       7    

//CONFIGURAÇÃO DOS SENSORES
#define DHTTYPE DHT11      

// Inicializa o objeto DHT
DHT dht(PIN_DHT, DHTTYPE);

// Variáveis para o Ultrassônico
long duration;
float distanceCm;

void setup() {
  // Inicia a comunicação Serial (Velocidade padrão do ESP32)
  Serial.begin(115200);
  
  // Aguarda um momento para o monitor serial conectar
  delay(1000);
  Serial.println("--- Iniciando Sistema de Monitoramento ESP32-S3 ---");

  // Configuração dos Pinos
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Inicia o sensor DHT
  dht.begin();
}

void loop() {
  Serial.println("=====================================");
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Verifica se houve falha na leitura do DHT
  if (isnan(h) || isnan(t)) {
    Serial.println("Erro ao ler o sensor DHT!");
  } else {
    Serial.print("Umidade: ");
    Serial.print(h);
    Serial.print("% | Temperatura: ");
    Serial.print(t);
    Serial.println("°C");
  }

  //LEITURA DO LDR (Luminosidade)
  int ldrValue = analogRead(PIN_LDR);
  Serial.print("Luminosidade (Raw): ");
  Serial.println(ldrValue);


  //LEITURA DO ULTRASSÔNICO
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  // Lê o tempo de retorno no pino Echo
  duration = pulseIn(PIN_ECHO, HIGH);
  
  // Calcula a distância
  distanceCm = duration * 0.034 / 2;
  
  Serial.print("Distância: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  delay(2000);
}
