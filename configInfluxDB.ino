#include <Arduino.h>
#include <WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h> 

// --- CORREÇÃO DE BROWNOUT ----
#include "soc/rtc_cntl_reg.h"

// --- CONFIGURAÇÕES DE WI-FI ---
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// --- SUAS CREDENCIAIS DO INFLUXDB ---
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "2tIiaU_y_mbg8HK3YBfw7ML9bIsxAPzVccnYm0Ilen9XDESvmH-gx7wqYMNK6gW_RH8KVh-SoL9b2yJlo9f0sQ=="
#define INFLUXDB_ORG "866bf123095fb2ee"
#define INFLUXDB_BUCKET "monitoramento_simuladoesp32"

// Fuso horário (UTC-3 para o Brasil)
#define TZ_INFO "UTC-3"

// --- Objetos InfluxDB ---
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Criação do ponto de dados
Point sensor("status_esp32");

// --- Definições de Pinos ---
const int PIN_LED_RED   = 5;
const int PIN_LED_GREEN = 4;
const int PIN_BTN_LEDS  = 21; 
const int PIN_BTN_SYS   = 23; 
const int PIN_POT       = 34; 

// --- Variáveis Globais ---
volatile bool systemOn = true;       
volatile bool toggleRedGreen = true;  

volatile unsigned long lastDebounceTime1 = 0; 
volatile unsigned long lastDebounceTime2 = 0; 
const unsigned long debounceDelay = 250;      

// --- Interrupções (ISRsonf) ---
void IRAM_ATTR isr_toggleLeds() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime1) > debounceDelay) {
    toggleRedGreen = !toggleRedGreen; 
    lastDebounceTime1 = currentTime;
  }
}

void IRAM_ATTR isr_toggleSystem() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime2) > debounceDelay) {
    systemOn = !systemOn; 
    lastDebounceTime2 = currentTime;
  }
}

// --- Função Auxiliar de Conexão Wi-Fi ---
void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.printf("Conectado IP: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  // 1. Desabilita o detector de Brownout (Correção de Reinicialização)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(1000); 
  Serial.println("--- Iniciando Setup ---");

  // 2. Configuração dos Pinos
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BTN_LEDS, INPUT_PULLUP); 
  pinMode(PIN_BTN_SYS, INPUT_PULLUP);
  pinMode(PIN_POT, INPUT);
  analogReadResolution(12);

  attachInterrupt(digitalPinToInterrupt(PIN_BTN_LEDS), isr_toggleLeds, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_SYS), isr_toggleSystem, FALLING);

  // 3. Conexões de Rede
  setupWiFi();

  // 4. Sincronia de Tempo (Obrigatório para Cloud)
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // 5. Validar conexão InfluxDB
  if (client.validateConnection()) {
    Serial.print("Conectado ao InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("Erro de conexao InfluxDB: ");
    Serial.println(client.getLastErrorMessage());
  }

  // 6. Configurar TAGS Fixas (APENAS AQUI NO SETUP)
  sensor.addTag("device", "ESP32_Bancada"); 
  sensor.addTag("location", "Laboratorio");

  Serial.println("--- Sistema Pronto ---");
}

void loop() {
  // Se sistema OFF, não envia dados e espera
  if (!systemOn) {
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    Serial.println("Sistema OFF (Standby)");
    delay(1000); 
    return; 
  }

  // Reconecta WiFi se cair
  if(WiFi.status() != WL_CONNECTED) {
      setupWiFi();
  }

  // Leitura Sensores
  int potValue = analogRead(PIN_POT);
  float voltage = (potValue * 3.3) / 4095.0; 

  String ledStatus = "";
  if (toggleRedGreen) {
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_LED_GREEN, LOW);
    ledStatus = "RED";
  } else {
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, HIGH);
    ledStatus = "GREEN";
  }

  // --- ENVIO PARA INFLUXDB ---
  sensor.clearFields(); 

  // Adiciona os valores dinâmicos
  sensor.addField("voltage", voltage);
  sensor.addField("pot_raw", potValue);
  sensor.addField("active_led", ledStatus);
  sensor.addField("system_status", systemOn);

  // Debug no Serial
  Serial.print("Enviando: ");
  // Mostra a string exata que vai para o banco
  Serial.println(client.pointToLineProtocol(sensor)); 

  // Envia para a nuvem
  if (!client.writePoint(sensor)) {
    Serial.print("Falha ao enviar: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Aguarda 1s
  delay(1000); 
}
