#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// ── Objetos globais ───────────────────────────────────────────────────────────
WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

static const unsigned long PUBLISH_INTERVAL = 30000; // publica info a cada 30s
unsigned long lastPublish = 0;

// ── Wi-Fi ─────────────────────────────────────────────────────────────────────
void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;

    Serial.printf("\n[WiFi] Conectando a '%s'", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[WiFi] Conectado! IP: %s\n", WiFi.localIP().toString().c_str());
}

// ── MQTT: callback de mensagens recebidas ─────────────────────────────────────
void onMessage(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

    Serial.printf("[MQTT] recebido  topico='%s'  payload='%s'\n", topic, msg.c_str());

    if (String(topic) == TOPIC_LED_CMD) {
        if (msg == "1") {
            digitalWrite(LED_PIN, LOW);          // LOW = aceso (lógica invertida)
            mqtt.publish(TOPIC_LED_STATUS, "ligado");
            Serial.println("[LED] ligado");
        } else if (msg == "0") {
            digitalWrite(LED_PIN, HIGH);         // HIGH = apagado
            mqtt.publish(TOPIC_LED_STATUS, "desligado");
            Serial.println("[LED] desligado");
        } else {
            Serial.printf("[LED] payload invalido: '%s' (use '1' ou '0')\n", msg.c_str());
        }
    }
}

// ── MQTT: conexão e reconexão ─────────────────────────────────────────────────
void connectMQTT() {
    while (!mqtt.connected()) {
        Serial.printf("[MQTT] conectando como '%s'...\n", MQTT_CLIENT_ID);

        // Last Will: publica "offline" automaticamente se o dispositivo cair
        bool ok = mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS,
                               TOPIC_STATUS, 1, true, "offline");

        if (ok) {
            Serial.println("[MQTT] conectado!");
            mqtt.subscribe(TOPIC_LED_CMD, 1);
            Serial.printf("[MQTT] inscrito em '%s'\n", TOPIC_LED_CMD);
            mqtt.publish(TOPIC_STATUS, "online", true);
            mqtt.publish(TOPIC_IP, WiFi.localIP().toString().c_str(), true);
        } else {
            Serial.printf("[MQTT] falhou (rc=%d), tentando em 5s...\n", mqtt.state());
            delay(5000);
        }
    }
}

// ── Publicação periódica dos dados do dispositivo ─────────────────────────────
void publishDeviceInfo() {
    String ip     = WiFi.localIP().toString();
    String uptime = String(millis() / 1000);

    mqtt.publish(TOPIC_IP,     ip.c_str(),     true);
    mqtt.publish(TOPIC_STATUS, "online",        true);
    mqtt.publish(TOPIC_UPTIME, uptime.c_str(), true);

    Serial.printf("[INFO] ip=%s  uptime=%ss\n", ip.c_str(), uptime.c_str());
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // apagado por padrão

    connectWiFi();

    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setCallback(onMessage);

    connectMQTT();
    publishDeviceInfo();
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] conexao perdida, reconectando...");
        connectWiFi();
    }

    if (!mqtt.connected()) {
        connectMQTT();
    }

    mqtt.loop();

    if (millis() - lastPublish >= PUBLISH_INTERVAL) {
        lastPublish = millis();
        publishDeviceInfo();
    }
}
