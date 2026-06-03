-- Tabela para persistência das mensagens MQTT
-- SQLite: execute uma única vez ou deixe o bridge.py criar automaticamente

CREATE TABLE IF NOT EXISTS mqtt_mensagens (
    id        INTEGER  PRIMARY KEY AUTOINCREMENT,
    data_hora TEXT     NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now')),
    mensagem  TEXT,
    topico    TEXT,
    server    TEXT
);
