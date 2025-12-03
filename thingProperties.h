#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char DEVICE_LOGIN_NAME[]  = "c839465d-d201-4b71-a044-6d42d7bdc5c5";

const char SSID[]               = "Galaxy S23 FE";    
const char PASS[]               = "goue0800";
const char DEVICE_KEY[]         = "z?g!M!P5WlU3Z2gGbGzRQBir0";

// Declaração das funções de Callback (geradas automaticamente pelo Cloud)
void onEstadoSistemaChange();
void onLedVerdeChange();
void onLedVermelhoChange();

// Definição das variáveis EXATAS da sua imagem
bool estado_sistema;
bool led_Verde;
bool led_Vermelho;
int potenciometro;
float tensao;

void initProperties(){
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);

  // Mapeamento correto com suas variáveis
  ArduinoCloud.addProperty(estado_sistema, READWRITE, ON_CHANGE, onEstadoSistemaChange);
  ArduinoCloud.addProperty(led_Verde, READWRITE, ON_CHANGE, onLedVerdeChange);
  ArduinoCloud.addProperty(led_Vermelho, READWRITE, ON_CHANGE, onLedVermelhoChange);
  ArduinoCloud.addProperty(potenciometro, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(tensao, READ, ON_CHANGE, NULL);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
