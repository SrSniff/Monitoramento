# Monitoramento
Sistema de alarme para monitoramento
/*
  ========================================
  SISTEMA DE ALERTA REMOTO - ESP32-C3 SUPERMINI
  ========================================
  
  Desenvolvido para Arduino IDE 2.0
  
  HARDWARE NECESSÁRIO:
  - ESP32-C3 Supermini
  - Display LCD 16x2 com módulo I2C
  - Transistor BD139 (para controle da fita LED)
  - Buzzer PWM
  - Fita de LED (pequena)
  - Resistores de pull-up/pull-down conforme necessário
  
  CONEXÕES:
  - LCD I2C: SDA = GPIO8, SCL = GPIO10
  - Fita LED (via BD139): GPIO2
  - Buzzer PWM: GPIO3
  
  FUNCIONALIDADES:
  - API REST para controle remoto
  - Autenticação por senha
  - Múltiplos tipos de alerta
  - Display informativo com animação
  - Controle temporal automático
  
  AUTOR: Daniel Scherer - Canal Eletrônica e Tecnologia
  DATA: 2025
*/

Sistema de Alerta Remoto ESP32-C3 Supermini
Sistema completo de alerta remoto.
Principais Características:

API REST com autenticação por senha
Display LCD com animação e informações em tempo real
Controle temporal automático dos dispositivos
Múltiplos tipos de alerta (0-4)
Logging serial para debug
Interface web básica para verificação

Configurações Necessárias:
Antes de carregar o código, ajuste estas configurações no início do arquivo:
cppconst char* ssid = "SUA_REDE_WIFI";         // Nome da sua rede WiFi
const char* password = "SUA_SENHA_WIFI";    // Senha da sua rede WiFi
Bibliotecas Necessárias:
Instale estas bibliotecas no Arduino IDE:

LiquidCrystal_I2C (para o display LCD)
ArduinoJson (para processamento JSON)
WiFi e WebServer (já incluídas no core ESP32)

Exemplo de Chamada da API:
URL: POST http://[IP_DO_ESP32]/alert
Headers:
Content-Type: application/json
Corpo da Requisição (JSON):
json{
  "senha": "Acess0",
  "codigo": 2
}
Exemplos práticos usando curl:
bash# Ativar LED piscando por 30 segundos
curl -X POST http://192.168.1.100/alert \
  -H "Content-Type: application/json" \
  -d '{"senha":"Acess0","codigo":2}'

# Ativar buzzer por 2 segundos
curl -X POST http://192.168.1.100/alert \
  -H "Content-Type: application/json" \
  -d '{"senha":"Acess0","codigo":3}'

# Desativar todos os dispositivos
curl -X POST http://192.168.1.100/alert \
  -H "Content-Type: application/json" \
  -d '{"senha":"Acess0","codigo":0}'
Outras Rotas Disponíveis:

GET / - Página inicial com informações básicas
GET /status - Status completo do sistema em JSON

Display LCD:

Linha 1: IP do ESP32 + animação (. o O o . [espaço])
Linha 2: Tempo ativo OU "ALERTA: [código]" por 30 segundos

Códigos de Alerta:

0: Desativa tudo
1: LED fixo por 30 segundos
2: LED piscando por 30 segundos
3: Buzzer por 2 segundos
4: LED piscando (30s) + buzzer (2s)

Conexões Sugeridas:

LCD I2C: SDA→GPIO8, SCL→GPIO10
LED (via BD139): Coletor→+V, Base→GPIO2 (com resistor), Emissor→LED→GND
Buzzer: GPIO3
