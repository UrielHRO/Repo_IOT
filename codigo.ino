// Inclui as bibliotecas necessárias
#include <DHT.h>
#include <Servo.h>

// --- Definição de Pinos ---
#define DHTPIN 7       // Pino digital onde o pino de dados do DHT11 está conectado
#define DHTTYPE DHT11  // Tipo de sensor de umidade e temperatura

#define LDR_PIN A0     // Pino analógico onde o LDR está conectado (em um divisor de tensão)
#define LED_PIN 4      // Pino digital para o LED
#define SERVO_PIN 9    // Pino digital PWM para o Servo Motor
#define BUTTON_PIN 2   // Pino digital para o botão (deve ser um pino de Interrupção Externa, como o 2 ou 3 no Uno)

// --- Constantes de Limite ---
const int LDR_MAX_VALUE = 1023; // Valor máximo de leitura analógica do LDR (para 5V)
const int LDR_THRESHOLD = LDR_MAX_VALUE * 0.30; // 30% do valor máximo para ligar o LED
const float TEMP_THRESHOLD = 26.0; // Limite de temperatura para acionar o Servo (°C)

// --- Variáveis Globais ---
DHT dht(DHTPIN, DHTTYPE); // Cria o objeto DHT
Servo myServo;            // Cria o objeto Servo
volatile bool systemActive = true; // Flag de estado do sistema, volátil para uso em interrupção
unsigned long lastMeasurementTime = 0; // Para controle do intervalo de medição
const long MEASUREMENT_INTERVAL = 2000; // Intervalo de 2 segundos (2000 ms)

// --- Protótipo da Função de Interrupção ---
void toggleSystem();

// --- Setup ---
void setup() {
  Serial.begin(9600);
  dht.begin();
  myServo.attach(SERVO_PIN);
  myServo.write(0); // Posição inicial do servo (0 graus)

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Usa INPUT_PULLUP: o pino fica HIGH, e LOW quando pressionado.

  // Configura a interrupção externa
  // BUTTON_PIN (2) -> Interrupção 0
  // CHANGE: aciona a interrupção na borda de subida e descida do sinal.
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggleSystem, CHANGE);

  Serial.println("Sistema Inicializado. Pressione o botao para Ativar/Desativar.");
  Serial.print("Estado Inicial: ");
  Serial.println(systemActive ? "ATIVO" : "INATIVO");
}

// --- Loop Principal ---
void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastMeasurementTime >= MEASUREMENT_INTERVAL) {
    lastMeasurementTime = currentTime; // Atualiza o tempo da última medição

    // 1. Leitura de Sensores
    // Se o sistema estiver INATIVO, apenas imprime o estado e espera a interrupção.
    if (!systemActive) {
      Serial.println("\n------------------------------------");
      Serial.println("Sistema INATIVO por interrupcao externa.");
      Serial.println("------------------------------------");
      // Desativa todos os atuadores e sai do loop de medição
      digitalWrite(LED_PIN, LOW);
      myServo.write(0);
      return; // Sai da função loop para esperar o próximo intervalo ou interrupção.
    }
    
    // Se o sistema estiver ATIVO, realiza as medições e ações.
    Serial.println("\n------------------------------------");
    Serial.println("Sistema ATIVO - Lendo Sensores:");
    
    // Leitura DHT11 (demora cerca de 2 segundos, mas estamos forçando a leitura a cada 2s)
    float h = dht.readHumidity();
    float t = dht.readTemperature(); // Temperatura em Celsius
    
    // Leitura LDR (0 a 1023)
    int ldrValue = analogRead(LDR_PIN);
    
    // 2. Envio de Medições (a cada 2 segundos)
    Serial.print("Luminosidade (0-1023): ");
    Serial.println(ldrValue);
    
    // Checa se as leituras do DHT11 foram bem-sucedidas
    if (isnan(h) || isnan(t)) {
      Serial.println("Falha na leitura do sensor DHT11!");
    } else {
      Serial.print("Umidade: ");
      Serial.print(h);
      Serial.println(" %");
      Serial.print("Temperatura: ");
      Serial.print(t);
      Serial.println(" *C");

      // 3. Controle do LED (Luminosidade)
      if (ldrValue < LDR_THRESHOLD) {
        digitalWrite(LED_PIN, HIGH); // Liga o LED
        Serial.println("Luminosidade BAIXA: LED LIGADO");
      } else {
        digitalWrite(LED_PIN, LOW); // Desliga o LED
        Serial.println("Luminosidade OK: LED DESLIGADO");
      }

      // 4. Controle do Servo Motor (Temperatura)
      if (t > TEMP_THRESHOLD) {
        myServo.write(90); // Move o servo para 90 graus
        Serial.println("Temperatura ALTA: Servo para 90 graus");
      } else {
        myServo.write(0); // Move o servo para 0 graus
        Serial.println("Temperatura OK: Servo para 0 graus");
      }
    }
  }

  // Pequeno delay para debounce interno do loop, embora o controle de tempo seja principal.
  // Poderia ser removido, pois o controle de tempo do `lastMeasurementTime` já limita a execução.
  delay(10);
}

// --- Função de Interrupção Externa ---
// Esta função é chamada automaticamente quando o estado do pino do botão muda (CHANGE).
void toggleSystem() {
  // Pequeno delay de debounce para a interrupção
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  
  if (interruptTime - lastInterruptTime > 50) { // Debounce de 50ms
    systemActive = !systemActive; // Inverte o estado do sistema (ATIVO <-> INATIVO)
    
    if (!systemActive) {
      // Garante que os atuadores sejam desligados imediatamente ao desativar
      digitalWrite(LED_PIN, LOW);
      myServo.write(0);
      Serial.println(">>> INTERRUPCAO: Sistema DESATIVADO! <<<");
    } else {
       Serial.println(">>> INTERRUPCAO: Sistema ATIVADO! <<<");
       // Reseta o timer para a próxima leitura começar logo após a ativação
       lastMeasurementTime = 0; 
    }
    
    lastInterruptTime = interruptTime;
  }
}
