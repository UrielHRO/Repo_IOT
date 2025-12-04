#include <DHT.h>
#include <ESP32Servo.h>

// --- Configuração de Pinos para YD-ESP32-S3 ---
const int PIN_DHT = 4;
const int PIN_LDR = 1;      
const int PIN_LED = 5;      
const int PIN_SERVO = 36;  
const int PIN_BUTTON = 15; 

// --- Constantes de Lógica ---
const float TEMP_THRESHOLD = 28.0;                 
const int   LDR_MAX = 4095;                        
const int   LDR_THRESHOLD = LDR_MAX * 0.30;     
const unsigned long SEND_INTERVAL = 2000;           
const unsigned long BUTTON_DEBOUNCE_TIME = 250;     

// --- Objetos de Sensor e Atuador ---
DHT dht(PIN_DHT, DHT11);
Servo servoMotor;

// --- Estrutura de Dados ---
struct SensorData {
  float temperatura = NAN;
  float umidade = NAN;
  int   luminosidade = 0;
};

SensorData dados;

// --- Variáveis de Estado ---
volatile bool buttonFlag = false;
bool sistemaAtivo = true;
unsigned long lastSend = 0;
unsigned long lastToggle = 0; // Para debounce

// --- Protótipos de Funções ---
void initHardware();
void readSensors();
void controlOutputs();
void sendDataIfDue();
void handleButton();
void turnOffOutputs();
void IRAM_ATTR buttonISR(); 

void setup() {
  Serial.begin(115200);
  delay(100); 
  Serial.println("Sistema de Monitoramento Iniciado");
  initHardware();
  Serial.println("Sistema Ativo.");
}


void loop() {
  handleButton(); 
  
  if (!sistemaAtivo) {
    return;
  }

  readSensors();
  controlOutputs();
  sendDataIfDue();
}



void initHardware() {
  // Configuração do LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  analogReadResolution(12);

  // Configuração do Botão
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), buttonISR, FALLING);

  // Inicialização do DHT
  dht.begin();
  
  // Inicialização do Servo
  servoMotor.attach(PIN_SERVO);
  servoMotor.write(0); // Posição inicial (0 graus)
}


void readSensors() {
  // 1. Leitura do DHT22
  float novaTemperatura = dht.readTemperature();
  float novaUmidade = dht.readHumidity();

  if (!isnan(novaTemperatura) && !isnan(novaUmidade)) {
    dados.temperatura = novaTemperatura;
    dados.umidade = novaUmidade;
  } else {
  
    Serial.println("Falha ao ler o sensor DHT!");
  }

  //Leitura do LDR
  dados.luminosidade = analogRead(PIN_LDR);
}


void controlOutputs() {
  // Controle do LED (Luminosidade)
  if (dados.luminosidade < LDR_THRESHOLD) {
    digitalWrite(PIN_LED, HIGH);
  } else {
    digitalWrite(PIN_LED, LOW);
  }

  if (!isnan(dados.temperatura) && dados.temperatura > TEMP_THRESHOLD) {
    servoMotor.write(90); // Aciona (90 graus)
  } else {
    servoMotor.write(0); // Desliga (0 graus)
  }
}



void sendDataIfDue() {
  unsigned long now = millis();

  // Verifica se o intervalo de envio já passou
  if (now - lastSend < SEND_INTERVAL) return;
  lastSend = now; 

  if (isnan(dados.temperatura) || isnan(dados.umidade)) {
    Serial.println("Aguardando leitura válida do sensor DHT...");
    return;
  }

  // Imprime os dados
  Serial.print("Temp: ");
  Serial.print(dados.temperatura, 1); // 1 casa decimal
  Serial.print(" C  Umid: ");
  Serial.print(dados.umidade, 1);
  Serial.print(" %  LDR: ");
  Serial.println(dados.luminosidade);
}


void handleButton() {
  //Verifica se a flag da interrupção está ativa
  if (!buttonFlag) return;
  buttonFlag = false; // Limpa a flag

  unsigned long now = millis();
  
  if (now - lastToggle < BUTTON_DEBOUNCE_TIME) return;
  lastToggle = now; 

  sistemaAtivo = !sistemaAtivo;

  //Feedback e Ações
  if (!sistemaAtivo) {
    Serial.println(" SISTEMA DESATIVADO");
    turnOffOutputs();
  } else {
    Serial.println(" SISTEMA ATIVADO");
  }
}

void turnOffOutputs() {
  digitalWrite(PIN_LED, LOW); 
  servoMotor.write(0);       // Retorna o Servo para 0 graus
}


void IRAM_ATTR buttonISR() {
  buttonFlag = true;
}
