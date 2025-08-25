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
  
  AUTOR: Sistema desenvolvido para controle de alertas
  DATA: 2025
*/

// ========================================
// INCLUSÃO DAS BIBLIOTECAS
// ========================================
#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// ========================================
// CONFIGURAÇÕES DE REDE WIFI
// ========================================
const char* ssid = "SUA_REDE_WIFI";         // Substitua pelo nome da sua rede WiFi
const char* password = "SUA_SENHA_WIFI";    // Substitua pela senha da sua rede WiFi

// ========================================
// CONFIGURAÇÕES DO SISTEMA
// ========================================
const String SYSTEM_PASSWORD = "Acess0";    // Senha fixa do sistema
const int SERVER_PORT = 80;                 // Porta do servidor web

// ========================================
// DEFINIÇÃO DOS PINOS
// ========================================
const int LED_PIN = 2;                      // Pino para controle da fita LED (via BD139)
const int BUZZER_PIN = 3;                   // Pino PWM para o buzzer
const int SDA_PIN = 8;                      // Pino SDA do I2C (LCD)
const int SCL_PIN = 10;                     // Pino SCL do I2C (LCD)

// ========================================
// CONFIGURAÇÕES DE TIMING
// ========================================
const unsigned long LED_DURATION = 30000;   // Duração da fita LED: 30 segundos
const unsigned long BUZZER_DURATION = 2000; // Duração do buzzer: 2 segundos
const unsigned long ALERT_DISPLAY_TIME = 30000; // Tempo de exibição do alerta no LCD
const unsigned long BLINK_INTERVAL = 500;   // Intervalo de piscada: 500ms
const unsigned long ANIMATION_INTERVAL = 500; // Intervalo da animação do LCD

// ========================================
// OBJETOS GLOBAIS
// ========================================
LiquidCrystal_I2C lcd(0x27, 16, 2);        // Inicializa LCD I2C (endereço 0x27, 16x2)
WebServer server(SERVER_PORT);              // Servidor web na porta definida

// ========================================
// VARIÁVEIS DE CONTROLE DE TEMPO
// ========================================
unsigned long systemStartTime = 0;          // Momento de inicialização do sistema
unsigned long ledStartTime = 0;             // Momento de ativação da fita LED
unsigned long buzzerStartTime = 0;          // Momento de ativação do buzzer
unsigned long alertStartTime = 0;           // Momento de início da exibição do alerta
unsigned long lastBlinkTime = 0;            // Último momento de mudança de estado (piscar)
unsigned long lastAnimationTime = 0;        // Último momento de atualização da animação

// ========================================
// VARIÁVEIS DE ESTADO DO SISTEMA
// ========================================
bool ledActive = false;                     // Estado atual da fita LED
bool ledBlinking = false;                   // Se a fita LED está no modo piscante
bool ledState = false;                      // Estado atual do LED (para piscar)
bool buzzerActive = false;                  // Estado atual do buzzer
bool showingAlert = false;                  // Se está exibindo mensagem de alerta
int currentAlertCode = 0;                   // Código do alerta atual sendo exibido
int animationFrame = 0;                     // Frame atual da animação do LCD

// ========================================
// CARACTERES DA ANIMAÇÃO
// ========================================
const char animationChars[] = {'.', 'o', 'O', 'o', '.', ' '};
const int animationLength = 6;

// ========================================
// FUNÇÃO DE CONFIGURAÇÃO INICIAL
// ========================================
void setup() {
  // Inicializa comunicação serial para debug
  Serial.begin(115200);
  Serial.println("========================================");
  Serial.println("   INICIANDO SISTEMA DE ALERTA REMOTO");
  Serial.println("========================================");
  
  // Configura os pinos de saída
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Garante que as saídas iniciem desligadas
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Inicializa o display LCD
  initializeLCD();
  
  // Conecta à rede WiFi
  connectToWiFi();
  
  // Configura as rotas do servidor web
  setupWebServer();
  
  // Inicia o servidor
  server.begin();
  
  // Registra o momento de inicialização
  systemStartTime = millis();
  
  Serial.println("Sistema iniciado com sucesso!");
  Serial.print("IP do servidor: ");
  Serial.println(WiFi.localIP());
  Serial.println("========================================");
}

// ========================================
// LOOP PRINCIPAL DO PROGRAMA
// ========================================
void loop() {
  // Processa requisições do servidor web
  server.handleClient();
  
  // Atualiza o estado dos dispositivos baseado no tempo
  updateDeviceStates();
  
  // Atualiza a exibição do LCD
  updateDisplay();
  
  // Pequeno delay para evitar sobrecarga do processador
  delay(10);
}

// ========================================
// INICIALIZAÇÃO DO DISPLAY LCD
// ========================================
void initializeLCD() {
  Serial.println("Inicializando display LCD...");
  
  // Inicializa o LCD com os pinos I2C personalizados
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  
  // Exibe mensagem de inicialização
  lcd.setCursor(0, 0);
  lcd.print("SISTEMA ALERTA");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  
  delay(2000);
  lcd.clear();
  
  Serial.println("LCD inicializado com sucesso!");
}

// ========================================
// CONEXÃO COM A REDE WIFI
// ========================================
void connectToWiFi() {
  Serial.print("Conectando ao WiFi: ");
  Serial.print(ssid);
  
  // Exibe status no LCD
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi conectado com sucesso!");
    Serial.print("IP obtido: ");
    Serial.println(WiFi.localIP());
    
    // Exibe IP no LCD por alguns segundos
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Conectado!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(3000);
    lcd.clear();
  } else {
    Serial.println();
    Serial.println("ERRO: Falha ao conectar WiFi!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERRO WiFi!");
    lcd.setCursor(0, 1);
    lcd.print("Verifique config");
    while(1); // Para o programa em caso de erro
  }
}

// ========================================
// CONFIGURAÇÃO DAS ROTAS DO SERVIDOR WEB
// ========================================
void setupWebServer() {
  Serial.println("Configurando rotas do servidor web...");
  
  // Rota principal da API - POST /alert
  server.on("/alert", HTTP_POST, handleAlertRequest);
  
  // Rota para verificar status - GET /status
  server.on("/status", HTTP_GET, handleStatusRequest);
  
  // Rota para página inicial - GET /
  server.on("/", HTTP_GET, handleRootRequest);
  
  // Manipulador para rotas não encontradas
  server.onNotFound(handleNotFound);
  
  Serial.println("Rotas configuradas com sucesso!");
}

// ========================================
// MANIPULADOR DA REQUISIÇÃO DE ALERTA
// ========================================
void handleAlertRequest() {
  Serial.println("Recebida requisição de alerta");
  
  // Verifica se o content-type é JSON
  if (!server.hasHeader("Content-Type") || 
      server.header("Content-Type") != "application/json") {
    sendErrorResponse(400, "Content-Type deve ser application/json");
    return;
  }
  
  // Obtém o corpo da requisição
  String requestBody = server.arg("plain");
  Serial.print("Corpo da requisição: ");
  Serial.println(requestBody);
  
  // Parse do JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, requestBody);
  
  if (error) {
    Serial.print("Erro no parse JSON: ");
    Serial.println(error.c_str());
    sendErrorResponse(400, "JSON inválido");
    return;
  }
  
  // Extrai os parâmetros
  String receivedPassword = doc["senha"];
  int alertCode = doc["codigo"];
  
  // Validação da senha
  if (receivedPassword != SYSTEM_PASSWORD) {
    Serial.println("Senha incorreta fornecida");
    sendErrorResponse(401, "Senha incorreta");
    return;
  }
  
  // Validação do código
  if (alertCode < 0 || alertCode > 4) {
    Serial.println("Código de alerta inválido");
    sendErrorResponse(400, "Código deve estar entre 0 e 4");
    return;
  }
  
  // Processa o código de alerta
  processAlertCode(alertCode);
  
  // Resposta de sucesso
  StaticJsonDocument<100> responseDoc;
  responseDoc["status"] = "sucesso";
  responseDoc["codigo_executado"] = alertCode;
  responseDoc["timestamp"] = millis();
  
  String response;
  serializeJson(responseDoc, response);
  
  server.send(200, "application/json", response);
  
  Serial.print("Alerta processado com sucesso. Código: ");
  Serial.println(alertCode);
}

// ========================================
// PROCESSAMENTO DOS CÓDIGOS DE ALERTA
// ========================================
void processAlertCode(int code) {
  Serial.print("Processando código de alerta: ");
  Serial.println(code);
  
  // Para todos os dispositivos antes de aplicar novo comando
  stopAllDevices();
  
  unsigned long currentTime = millis();
  
  switch (code) {
    case 0:
      // Código 0: Desativa tudo (já feito no stopAllDevices)
      Serial.println("Todos os dispositivos desativados");
      break;
      
    case 1:
      // Código 1: Ativa LED por 30 segundos
      ledActive = true;
      ledBlinking = false;
      ledStartTime = currentTime;
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ativado por 30 segundos");
      break;
      
    case 2:
      // Código 2: Ativa LED piscando por 30 segundos
      ledActive = true;
      ledBlinking = true;
      ledStartTime = currentTime;
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      lastBlinkTime = currentTime;
      Serial.println("LED ativado (piscando) por 30 segundos");
      break;
      
    case 3:
      // Código 3: Ativa buzzer por 2 segundos
      buzzerActive = true;
      buzzerStartTime = currentTime;
      tone(BUZZER_PIN, 1000); // Tom de 1000Hz
      Serial.println("Buzzer ativado por 2 segundos");
      break;
      
    case 4:
      // Código 4: LED piscando por 30s + buzzer por 2s
      ledActive = true;
      ledBlinking = true;
      ledStartTime = currentTime;
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      lastBlinkTime = currentTime;
      
      buzzerActive = true;
      buzzerStartTime = currentTime;
      tone(BUZZER_PIN, 1000);
      
      Serial.println("LED piscando (30s) + Buzzer (2s) ativados");
      break;
  }
  
  // Configura exibição do alerta no LCD
  showingAlert = true;
  currentAlertCode = code;
  alertStartTime = currentTime;
}

// ========================================
// PARADA DE TODOS OS DISPOSITIVOS
// ========================================
void stopAllDevices() {
  // Para LED
  ledActive = false;
  ledBlinking = false;
  digitalWrite(LED_PIN, LOW);
  
  // Para buzzer
  buzzerActive = false;
  noTone(BUZZER_PIN);
  
  Serial.println("Todos os dispositivos foram parados");
}

// ========================================
// ATUALIZAÇÃO DO ESTADO DOS DISPOSITIVOS
// ========================================
void updateDeviceStates() {
  unsigned long currentTime = millis();
  
  // Controle do LED
  if (ledActive) {
    // Verifica se o tempo de ativação expirou
    if (currentTime - ledStartTime >= LED_DURATION) {
      ledActive = false;
      ledBlinking = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED desativado automaticamente (tempo expirado)");
    } else if (ledBlinking) {
      // Controla o piscar do LED
      if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        lastBlinkTime = currentTime;
      }
    }
  }
  
  // Controle do buzzer
  if (buzzerActive) {
    if (currentTime - buzzerStartTime >= BUZZER_DURATION) {
      buzzerActive = false;
      noTone(BUZZER_PIN);
      Serial.println("Buzzer desativado automaticamente (tempo expirado)");
    }
  }
  
  // Controle da exibição de alerta
  if (showingAlert) {
    if (currentTime - alertStartTime >= ALERT_DISPLAY_TIME) {
      showingAlert = false;
      Serial.println("Exibição de alerta finalizada");
    }
  }
}

// ========================================
// ATUALIZAÇÃO DO DISPLAY LCD
// ========================================
void updateDisplay() {
  unsigned long currentTime = millis();
  
  // Primeira linha: IP + animação
  lcd.setCursor(0, 0);
  
  String ipString = WiFi.localIP().toString();
  lcd.print(ipString);
  
  // Adiciona espaços para limpar caracteres antigos se necessário
  int ipLength = ipString.length();
  for (int i = ipLength; i < 15; i++) {
    lcd.print(" ");
  }
  
  // Atualiza animação no último caractere da primeira linha
  if (currentTime - lastAnimationTime >= ANIMATION_INTERVAL) {
    lcd.setCursor(15, 0);
    lcd.print(animationChars[animationFrame]);
    animationFrame = (animationFrame + 1) % animationLength;
    lastAnimationTime = currentTime;
  }
  
  // Segunda linha: Tempo de funcionamento ou alerta
  lcd.setCursor(0, 1);
  
  if (showingAlert) {
    // Exibe informação do alerta
    lcd.print("ALERTA: ");
    lcd.print(currentAlertCode);
    lcd.print("       "); // Limpa caracteres extras
  } else {
    // Exibe tempo de funcionamento
    unsigned long uptime = (currentTime - systemStartTime) / 1000; // em segundos
    
    lcd.print("Ativo: ");
    if (uptime < 60) {
      lcd.print(uptime);
      lcd.print("s    ");
    } else if (uptime < 3600) {
      lcd.print(uptime / 60);
      lcd.print("m");
      lcd.print(uptime % 60);
      lcd.print("s   ");
    } else {
      lcd.print(uptime / 3600);
      lcd.print("h");
      lcd.print((uptime % 3600) / 60);
      lcd.print("m  ");
    }
  }
}

// ========================================
// MANIPULADOR DA REQUISIÇÃO DE STATUS
// ========================================
void handleStatusRequest() {
  StaticJsonDocument<300> doc;
  
  doc["sistema"] = "Sistema de Alerta Remoto";
  doc["versao"] = "1.0";
  doc["ip"] = WiFi.localIP().toString();
  doc["uptime_segundos"] = (millis() - systemStartTime) / 1000;
  doc["led_ativo"] = ledActive;
  doc["led_piscando"] = ledBlinking;
  doc["buzzer_ativo"] = buzzerActive;
  doc["exibindo_alerta"] = showingAlert;
  
  if (showingAlert) {
    doc["codigo_alerta_atual"] = currentAlertCode;
  }
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
  Serial.println("Status do sistema enviado");
}

// ========================================
// MANIPULADOR DA PÁGINA INICIAL
// ========================================
void handleRootRequest() {
  String html = "<html><head><title>Sistema de Alerta Remoto</title></head>";
  html += "<body><h1>Sistema de Alerta Remoto - ESP32-C3</h1>";
  html += "<p>Sistema funcionando normalmente!</p>";
  html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Tempo ativo: " + String((millis() - systemStartTime) / 1000) + " segundos</p>";
  html += "<h2>Rotas disponíveis:</h2>";
  html += "<ul><li>POST /alert - Enviar comando de alerta</li>";
  html += "<li>GET /status - Verificar status do sistema</li></ul>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  Serial.println("Página inicial acessada");
}

// ========================================
// MANIPULADOR PARA ROTAS NÃO ENCONTRADAS
// ========================================
void handleNotFound() {
  String message = "Rota não encontrada\n\n";
  message += "URI: " + server.uri() + "\n";
  message += "Método: " + String((server.method() == HTTP_GET) ? "GET" : "POST") + "\n";
  message += "Argumentos: " + String(server.args()) + "\n";
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
  Serial.println("Rota não encontrada acessada: " + server.uri());
}

// ========================================
// FUNÇÃO PARA ENVIAR RESPOSTA DE ERRO
// ========================================
void sendErrorResponse(int statusCode, String message) {
  StaticJsonDocument<100> doc;
  doc["erro"] = message;
  doc["codigo"] = statusCode;
  
  String response;
  serializeJson(doc, response);
  
  server.send(statusCode, "application/json", response);
  
  Serial.print("Erro enviado - Código: ");
  Serial.print(statusCode);
  Serial.print(", Mensagem: ");
  Serial.println(message);
}
