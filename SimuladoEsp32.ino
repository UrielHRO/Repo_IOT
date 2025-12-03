#include "thingProperties.h"
#include <WiFi.h>
#include <Arduino.h>

// --- Definições de Pinos ---
const int PIN_LED_RED   = 5;
const int PIN_LED_GREEN = 4;
const int PIN_BTN_LEDS  = 21; 
const int PIN_BTN_SYS   = 23; 
const int PIN_POT       = 34; 

// --- Variáveis de Controle Local ---
volatile bool localSystemOn = true;       
volatile bool localRedSelected = true; // true = Vermelho selecionado, false = Verde

volatile unsigned long lastDebounceTime1 = 0; 
volatile unsigned long lastDebounceTime2 = 0; 
const unsigned long debounceDelay = 250;      

// --- Interrupções (ISRs) ---

//  Alterna entre Vermelho e Verde
void IRAM_ATTR isr_toggleLeds() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime1) > debounceDelay) {
    localRedSelected = !localRedSelected; 
    lastDebounceTime1 = currentTime;
  }
}

// Liga/Desliga Sistema
void IRAM_ATTR isr_toggleSystem() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime2) > debounceDelay) {
    localSystemOn = !localSystemOn; 
    lastDebounceTime2 = currentTime;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  // Inicializa propriedades e conecta ao Cloud
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Configuração de Hardware
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BTN_LEDS, INPUT_PULLUP); 
  pinMode(PIN_BTN_SYS, INPUT_PULLUP);
  pinMode(PIN_POT, INPUT);
  analogReadResolution(12);

  attachInterrupt(digitalPinToInterrupt(PIN_BTN_LEDS), isr_toggleLeds, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_SYS), isr_toggleSystem, FALLING);
}

void loop() {
  ArduinoCloud.update(); // Mantém a conexão viva

  // --- Sincronização: Variáveis Locais -> Variáveis da Nuvem ---
  
  //Atualiza o status do sistema na nuvem
  if (estado_sistema != localSystemOn) {
      estado_sistema = localSystemOn;
  }

  //Atualiza os LEDs na nuvem 
  if (localSystemOn) {
      if (localRedSelected) {
          // Se o físico está vermelho:
          if (!led_Vermelho) led_Vermelho = true; // Liga switch vermelho na tela
          if (led_Verde)     led_Verde = false;   // Desliga switch verde na tela
      } else {
          // Se o físico está verde:
          if (led_Vermelho)  led_Vermelho = false; // Desliga switch vermelho na tela
          if (!led_Verde)    led_Verde = true;     // Liga switch verde na tela
      }
  } else {
      // Se sistema desligado, desliga tudo na tela para visualização correta
      led_Vermelho = false;
      led_Verde = false;
  }

  // --- Controle do Hardware Físico ---
  
  if (!localSystemOn) {
    // Sistema OFF
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    
    // Zera leituras na nuvem para indicar inatividade
    tensao = 0.0;
    potenciometro = 0;
  } 
  else {
    // Sistema ON
    
    // Leitura Sensores
    int rawPot = analogRead(PIN_POT);
    float calcVolts = (rawPot * 3.3) / 4095.0; 

    // Envia para Nuvem
    potenciometro = rawPot;
    tensao = calcVolts;

    // Aciona LEDs
    if (localRedSelected) {
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);
    } else {
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, HIGH);
    }
  }
}

/*
  FUNÇÕES DE CALLBACK (Acionadas quando você clica na Dashboard)
*/

void onEstadoSistemaChange() {
  // Atualiza a variável local de controle
  localSystemOn = estado_sistema;
}

void onLedVerdeChange() {
  if (led_Verde) {
    localRedSelected = false; // Muda lógica interna para Verde
    // O loop cuidará de desligar o switch vermelho na tela
  }
}

void onLedVermelhoChange() {
  if (led_Vermelho) {
    localRedSelected = true; // Muda lógica interna para Vermelho
  }
}
