#include <WiFi.h>
#include <WiFiClientSecure.h> // Biblioteca para conexão segura (TLS)
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações do HiveMQ Cloud Privado
const char* mqtt_server = "4ffe8925948f46d5883ca30dad59a71f.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "user1"; // <-- ALTERE AQUI
const char* mqtt_pass = "Fiapiot2026"; // <-- ALTERE AQUI

// Instancia o cliente seguro
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Pinos
#define DHTPIN 15
#define DHTTYPE DHT22
#define POT_PIN 34       // Simula Batimentos
#define BOTAO_PIN 14     // Botão de pânico
#define BUZZER_PIN 13    // Motor de vibração
#define LED_R 25
#define LED_G 26
#define LED_B 27

// Display OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;
bool panicoAtivo = false;

void setup() {
  Serial.begin(115200);
  
  pinMode(BOTAO_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  dht.begin();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha no display OLED");
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

  setup_wifi();
  
  // Ignora a validação do certificado SSL (ideal para simulações acadêmicas)
  espClient.setInsecure(); 
  
  client.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "GS_Espaco_Turista_";
    clientId += String(random(0xffff), HEX);
    Serial.print("Tentando conexao MQTT...");
    
    // Conecta passando o usuário e senha do HiveMQ Cloud
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado com sucesso!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setCorLed(int r, int g, int b) {
  digitalWrite(LED_R, r > 0 ? HIGH : LOW);
  digitalWrite(LED_G, g > 0 ? HIGH : LOW);
  digitalWrite(LED_B, b > 0 ? HIGH : LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Leitura do Botão de Pânico
  if (digitalRead(BOTAO_PIN) == LOW) {
    panicoAtivo = true;
    String panicoMsg = "{\"alerta_medico\": true, \"mensagem\": \"PÂNICO ACIONADO\"}";
    client.publish("gs/espaco/turismo/panico", panicoMsg.c_str());
    delay(200); // Debounce
  } else {
    panicoAtivo = false;
  }

  unsigned long now = millis();
  if (now - lastMsg > 500) { 
    lastMsg = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int potValue = analogRead(POT_PIN);
    
    int bpm = map(potValue, 0, 4095, 40, 160); 

    String estado = "NORMAL";
    String cor = "VERDE";
    
    if (panicoAtivo || bpm > 130 || bpm < 50 || t > 38.5 || t < 35.0) {
      setCorLed(255, 0, 0); // Vermelho
      tone(BUZZER_PIN, 1000, 500); // Apita Buzzer (Vibração)
      estado = "PERIGO";
      cor = "VERMELHO";
    } 
    else if (bpm > 100 || t > 37.5) {
      setCorLed(255, 255, 0); // Amarelo
      noTone(BUZZER_PIN);
      estado = "ALERTA";
      cor = "AMARELO";
    } 
    else {
      setCorLed(0, 255, 0); // Verde
      noTone(BUZZER_PIN);
    }

    // Atualiza Display OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Status: "); display.println(estado);
    display.println(" ");
    display.setTextSize(2);
    display.print("BPM: "); display.println(bpm);
    display.print("Temp: "); display.print(t, 1); display.println("C");
    display.display();

    // Cria JSON e Publica via MQTT
    StaticJsonDocument<200> docSensores;
    docSensores["temperatura"] = t;
    docSensores["umidade"] = h;
    docSensores["bpm"] = bpm;
    char bufferSensores[256];
    serializeJson(docSensores, bufferSensores);
    client.publish("gs/espaco/turismo/sensores", bufferSensores);

    StaticJsonDocument<200> docStatus;
    docStatus["estado"] = estado;
    docStatus["cor_led"] = cor;
    char bufferStatus[256];
    serializeJson(docStatus, bufferStatus);
    client.publish("gs/espaco/turismo/status", bufferStatus);
  }
}