#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>

// Configurações Wi-Fi
const char* ssid = "CLARO_71BFC6";    // Substitua pelo nome da sua rede Wi-Fi
const char* password = "gTMkWRUZUY";  // Substitua pela senha da sua rede Wi-Fi

// Configurações MQTT
const char* mqtt_server = "test.mosquitto.org";  // Endereço do servidor Mosquitto
const int mqtt_port = 1883;                     // Porta padrão MQTT

WiFiClient espClient;
PubSubClient client(espClient);

// Pinos do sensor e relé
int PinoAnalogico = A0;
int Rele = D1;

// Variáveis de estado
int ValAnalogIn;
int UmidadeAtual = -1;  // Valor atual da umidade (inicializado como -1 para garantir envio inicial)
int UmidadeAnterior = -1; // Valor anterior da umidade
const int THRESHOLD = 5;  // Diferença mínima para enviar uma nova leitura (%)

// Limites para calibração do sensor
const int SENSOR_SECO = 244;  // Valor analógico com solo seco
const int SENSOR_UMIDO = 223; // Valor analógico com solo totalmente molhado

unsigned long lastCheckTime = 0;       // Timestamp para última verificação
const unsigned long checkInterval = 5000; // Intervalo de leitura do sensor (em milissegundos)

// Função para obter o timestamp absoluto
unsigned long getUnixTimestamp() {
  time_t now;
  time(&now); // Obtém o tempo atual em segundos desde 1970
  if (now < 1609459200) { // Verifica se o timestamp é válido (após 01/01/2021)
    Serial.println("Erro: Tempo não sincronizado corretamente.");
    return 0; // Retorna 0 se o tempo não estiver sincronizado
  }
  unsigned long timestamp = now * 1000; // Converte para milissegundos
  Serial.println("Timestamp sincronizado (ms): " + String(timestamp));
  return timestamp; // Retorna o valor em milissegundos
}

void setup() {
  // Configuração Serial e Wi-Fi
  Serial.begin(9600);
  setup_wifi();

  // Configuração do tempo NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // Sincroniza com servidores NTP
  Serial.println("Sincronizando o tempo...");
  while (time(nullptr) < 1609459200) { // Aguarda até que o timestamp seja válido (após 01/01/2021)
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nTempo sincronizado com sucesso!");

  // Configuração MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  // Define o callback para comandos

  // Configuração dos pinos
  pinMode(Rele, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentTime = millis();

  // Verifica o sensor a cada intervalo definido
  if (currentTime - lastCheckTime >= checkInterval) {
    lastCheckTime = currentTime;

    // Leitura do sensor de umidade
    ValAnalogIn = analogRead(PinoAnalogico);
    UmidadeAtual = map(ValAnalogIn, SENSOR_SECO, SENSOR_UMIDO, 0, 100); // Ajuste de calibração
    UmidadeAtual = constrain(UmidadeAtual, 0, 100); // Garante que o valor fique entre 0 e 100

    // Obtem o timestamp atual
    unsigned long timestamp = getUnixTimestamp();

    // Publica no broker MQTT apenas se o timestamp for válido
    if (timestamp != 0) {
      // Envia para o broker MQTT apenas se a diferença de umidade for maior que o threshold
      if (abs(UmidadeAtual - UmidadeAnterior) >= THRESHOLD) {
        String payload = "{\"umidade\": " + String(UmidadeAtual) + ", \"timestamp\": " + String(timestamp) + "}";
        client.publish("sensor/umidade", payload.c_str());
        Serial.println("Publicando no MQTT: " + payload);

        // Atualiza o valor anterior
        UmidadeAnterior = UmidadeAtual;
      } else {
        Serial.println("Nenhuma alteração significativa na umidade detectada.");
      }
    } else {
      Serial.println("Erro: Tempo inválido, dados não foram publicados.");
    }

    // Debug no Serial Monitor
    Serial.print("Valor Analógico: ");
    Serial.print(ValAnalogIn);
    Serial.print(" | Umidade Atual: ");
    Serial.print(UmidadeAtual);
    Serial.print("% | Umidade Anterior: ");
    Serial.println(UmidadeAnterior);
  }
}


// Configuração da conexão Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado!");
}

// Reconexão ao MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentando se conectar ao MQTT...");
    if (client.connect("NodeMCU_Irrigador_154_fhc")) {
      Serial.println("Conectado ao MQTT!");
      // Assina o tópico de controle
      client.subscribe("irrigador/controle");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

// Callback para processar comandos MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensagem);

  // Processa comandos no tópico "irrigador/controle"
  if (String(topic) == "irrigador/controle") {
    if (mensagem == "ligar") {
      digitalWrite(Rele, HIGH);  // Liga a bomba
      client.publish("irrigador/status", "Relé acionado manualmente");
      Serial.println("Relé ligado manualmente.");
    } else if (mensagem == "desligar") {
      digitalWrite(Rele, LOW);  // Desliga a bomba
      client.publish("irrigador/status", "Relé desligado manualmente");
      Serial.println("Relé desligado manualmente.");
    }
  }
}
