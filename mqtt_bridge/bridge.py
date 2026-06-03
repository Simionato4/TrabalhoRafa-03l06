import os
import time
import sqlite3
import logging
import paho.mqtt.client as mqtt
from dotenv import load_dotenv

load_dotenv()

# ── Configurações ────────────────────────────────────────────────────────────
MQTT_BROKER   = os.getenv("MQTT_BROKER",   "Protondata.com.br")
MQTT_PORT     = int(os.getenv("MQTT_PORT", "1884"))
MQTT_USER     = os.getenv("MQTT_USER",     "")
MQTT_PASS     = os.getenv("MQTT_PASS",     "")
MQTT_TOPIC    = os.getenv("MQTT_TOPIC",    "casa/#")
MQTT_CLIENT_ID = os.getenv("MQTT_CLIENT_ID", "mqtt-bridge-01")
SQLITE_DB     = os.getenv("SQLITE_DB",     "mqtt_mensagens.db")

SERVER_ID = f"{MQTT_BROKER}:{MQTT_PORT}"

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
log = logging.getLogger(__name__)

# ── Banco de dados ────────────────────────────────────────────────────────────

def init_db():
    """Cria a tabela se ainda não existir."""
    con = sqlite3.connect(SQLITE_DB)
    con.execute("""
        CREATE TABLE IF NOT EXISTS mqtt_mensagens (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            data_hora TEXT    NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now')),
            mensagem  TEXT,
            topico    TEXT,
            server    TEXT
        )
    """)
    con.commit()
    con.close()
    log.info(f"SQLite pronto: {SQLITE_DB}")


def insert_message(topic: str, payload: str):
    """Persiste uma mensagem no banco. Tenta até 3 vezes em caso de falha."""
    for attempt in range(1, 4):
        try:
            con = sqlite3.connect(SQLITE_DB, timeout=10)
            con.execute(
                "INSERT INTO mqtt_mensagens (mensagem, topico, server) VALUES (?, ?, ?)",
                (payload, topic, SERVER_ID),
            )
            con.commit()
            con.close()
            log.info(f"[DB] salvo  tópico='{topic}'  payload='{payload[:80]}'")
            return
        except sqlite3.Error as exc:
            log.warning(f"[DB] erro na tentativa {attempt}/3: {exc}")
            time.sleep(2)
    log.error(f"[DB] falha definitiva ao salvar mensagem do tópico '{topic}'")


# ── Callbacks MQTT ────────────────────────────────────────────────────────────

RC_MESSAGES = {
    0: "conectado com sucesso",
    1: "versão de protocolo recusada",
    2: "client ID inválido",
    3: "servidor indisponível",
    4: "usuário ou senha inválidos",
    5: "não autorizado",
}

def on_connect(client, userdata, flags, rc):
    log.info(f"[MQTT] {RC_MESSAGES.get(rc, f'rc={rc}')}")
    if rc == 0:
        client.subscribe(MQTT_TOPIC, qos=1)
        log.info(f"[MQTT] inscrito em '{MQTT_TOPIC}'")
    else:
        log.error("[MQTT] não foi possível conectar — verifique as credenciais")


def on_disconnect(client, userdata, rc):
    if rc != 0:
        log.warning(f"[MQTT] desconexão inesperada (rc={rc}), aguardando reconexão automática...")


def on_message(client, userdata, msg):
    # Decodifica o payload com segurança
    try:
        payload = msg.payload.decode("utf-8").strip()
    except UnicodeDecodeError:
        log.warning(f"[MQTT] payload indecodificável no tópico '{msg.topic}' — ignorado")
        return

    if not payload:
        log.warning(f"[MQTT] payload vazio no tópico '{msg.topic}' — ignorado")
        return

    insert_message(msg.topic, payload)


# ── Ponto de entrada ──────────────────────────────────────────────────────────

def main():
    init_db()

    client = mqtt.Client(client_id=MQTT_CLIENT_ID, clean_session=True)

    if MQTT_USER:
        client.username_pw_set(MQTT_USER, MQTT_PASS)

    client.on_connect    = on_connect
    client.on_disconnect = on_disconnect
    client.on_message    = on_message

    # Reconexão automática: tenta novamente em até 30 s
    client.reconnect_delay_set(min_delay=1, max_delay=30)

    log.info(f"[MQTT] conectando em {MQTT_BROKER}:{MQTT_PORT} ...")
    client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
    client.loop_forever()


if __name__ == "__main__":
    main()
