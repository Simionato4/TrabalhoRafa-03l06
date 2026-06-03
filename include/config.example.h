#pragma once

// =============================================================================
// Copie este arquivo para include/config.h e preencha com seus dados reais.
// O arquivo config.h está no .gitignore e NUNCA deve ser enviado ao GitHub.
// =============================================================================

// --- Rede Wi-Fi ---------------------------------------------------------------
#define WIFI_SSID      "nome_da_sua_rede"
#define WIFI_PASS      "senha_da_sua_rede"

// --- Broker MQTT --------------------------------------------------------------
#define MQTT_BROKER    "Protondata.com.br"
#define MQTT_PORT      1884
#define MQTT_USER      "seu_usuario_mqtt"
#define MQTT_PASS      "sua_senha_mqtt"
#define MQTT_CLIENT_ID "nodemcu-01"

// --- Tópicos: controle do LED -------------------------------------------------
#define TOPIC_LED_CMD    "casa/led/set"     // recebe "1" (ligar) ou "0" (desligar)
#define TOPIC_LED_STATUS "casa/led/status"  // publica "ligado" ou "desligado"

// --- Tópicos: monitoramento do dispositivo ------------------------------------
#define TOPIC_IP     "casa/nodemcu/ip"
#define TOPIC_STATUS "casa/nodemcu/status"
#define TOPIC_UPTIME "casa/nodemcu/uptime"

// --- Hardware -----------------------------------------------------------------
// LED interno do NodeMCU ESP8266: pino GPIO2 (D4), lógica invertida (LOW = aceso)
#define LED_PIN LED_BUILTIN
