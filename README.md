# 🚀 Sistema de Biometria para Turismo Espacial - IoT

Este projeto consiste em um sistema de monitoramento de sinais vitais e condições do traje espacial, desenvolvido para garantir a segurança e o bem-estar de turistas espaciais. Utilizando um ESP32 simulado no Wokwi, o sistema capta dados biométricos e ambientais, processa alertas locais e transmite as informações via protocolo MQTT (seguro) para uma Dashboard interativa no Node-RED.

## 📋 Arquitetura do Sistema

O sistema é dividido em três camadas principais: **Edge** (Captura e atuação local com ESP32), **Fog/Cloud** (Mensageria com HiveMQ) e **Aplicação** (Dashboard no Node-RED).

### 🔌 Entradas (Sensores)
* **Sensor DHT22:** Monitoramento em tempo real da temperatura (°C) e umidade relativa (%) do interior do traje espacial.
* **Potenciômetro:** Atuando como um simulador de sensor de batimentos cardíacos (BPM), mapeando valores analógicos para uma escala fisiológica realista (40 a 160 BPM) (Usado por limitações dos sensores no Wokwi).
* **Pushbutton (Botão de Pânico):** Acionamento manual de emergência médica pelo turista.

### 💡 Saídas (Atuadores e Interface)
* **LED RGB:** Indicador visual de status de saúde.
* **Buzzer:** Simula um motor de vibração no traje para alertas hápticos.
* **Display OLED SSD1306 (I2C):** Interface local de telemetria, exibindo as leituras e o status de forma offline e instantânea.

## ⚙️ Métodos e Lógica de Negócio

O microcontrolador avalia os dados continuamente e define o estado do paciente através da seguinte lógica:
* **🟢 NORMAL (Verde):** Temperatura entre 35.0°C e 37.5°C, e BPM entre 50 e 100. Sem alertas.
* **🟡 ALERTA (Amarelo):** Temperatura superior a 37.5°C ou BPM acima de 100. O LED muda para amarelo, indicando necessidade de atenção, mas sem acionamento do motor de vibração.
* **🔴 PERIGO (Vermelho):** Temperatura extrema (>38.5°C ou <35.0°C), BPM crítico (>130 ou <50) ou Botão de Pânico acionado. O LED fica vermelho e o Buzzer (vibração) é acionado para alerta imediato.

Todos os dados lidos são empacotados no formato JSON antes do envio via MQTT, otimizando o tráfego de rede.

## 📚 Bibliotecas Utilizadas (Wokwi)
O firmware do ESP32 depende das seguintes bibliotecas:
* `WiFi.h` e `WiFiClientSecure.h` (Conectividade e criptografia TLS)
* `PubSubClient` (Comunicação MQTT)
* `Wire.h`, `Adafruit_GFX`, e `Adafruit_SSD1306` (Controle do Display OLED)
* `DHT sensor library` (Leitura do DHT22)
* `ArduinoJson` (Serialização dos dados em formato JSON)

## 🌐 Comunicação MQTT (Broker Privado)
Para garantir a segurança dos dados médicos, utilizamos um cluster privado na nuvem com TLS:
* **Broker URL:** `4ffe8925948f46d5883ca30dad59a71f.s1.eu.hivemq.cloud`
* **Porta:** `8883` (Requer TLS/SSL)
* **Usuário:** `user1`
* **Senha:** `Fiapiot2026`

**Tópicos Utilizados:**
1. `gs/espaco/turismo/sensores`: Envio do JSON com Temp, Umidade e BPM.
2. `gs/espaco/turismo/status`: Envio do estado sistêmico atual e cor da indicação visual.
3. `gs/espaco/turismo/panico`: Envio de alerta crítico quando o botão é pressionado.

## 📊 Dashboard Interativa (Node-RED)
O Node-RED atua como a central de controle médico. Ele se conecta ao mesmo broker seguro e se inscreve nos tópicos acima.

* **Dependência:** Requer o pacote `node-red-dashboard` instalado na paleta.
* **Funcionamento:** O fluxo recebe as strings JSON, converte em objetos JavaScript (Nós de Parse/Function) e alimenta componentes de UI (Gauges para BPM e Temperatura, e textos dinâmicos para status e pânico).

## 🚀 Como Executar o Projeto

Todos os arquivos fonte estão disponíveis neste repositório. Siga os passos abaixo para testar:

**1. Simulador ESP32 (Wokwi):**
* Acesse o Wokwi e crie um novo projeto ESP32.
* Substitua o conteúdo do arquivo `diagram.json` do Wokwi pelo arquivo `diagram.json` deste repositório para carregar o circuito (componentes e fios) automaticamente.
* Cole o código contido no arquivo `sketch.ino` na aba de código do simulador.
* Adicione as bibliotecas citadas acima no "Library Manager" do simulador.
* Dê "Play" para iniciar a simulação e conectar ao broker.

**2. Central de Monitoramento (Node-RED):**
* Com seu Node-RED rodando, vá até `Menu > Manage palette` e instale o `node-red-dashboard`.
* Vá em `Menu > Import` e cole o conteúdo do arquivo `flow.json` disponibilizado neste repositório.
* Clique em "Deploy".
* Acesse a interface gráfica adicionando `/ui` no final do endereço local do seu Node-RED (Ex: `http://127.0.0.1:1880/ui`).

---

**Arquivos do Repositório:**
* `sketch.ino`: Firmware C++ do ESP32.
* `diagram.json`: Estrutura de ligação elétrica para o Wokwi.
* `flow.json`: Código de nós para importação na plataforma Node-RED.
