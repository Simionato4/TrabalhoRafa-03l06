# TrabalhoRafa-03l06

Integração entre Broker MQTT, banco de dados SQLite e NodeMCU ESP8266.

## Arquitetura

```
NodeMCU ESP8266  ←──►  Broker MQTT (Protondata.com.br:1884)  ──►  SQLite
   (hardware)              (mensagens em tempo real)            (histórico)
```

---

## Estrutura do repositório

```
TrabalhoRafa-03l06/
├── platformio.ini              # Configuração do projeto NodeMCU (PlatformIO)
├── .gitignore
├── include/
│   ├── config.example.h        # Template de configuração (sem credenciais)
│   └── config.h                # Credenciais reais — NÃO está no GitHub
├── src/
│   └── main.cpp                # Código do NodeMCU ESP8266
└── mqtt_bridge/
    ├── bridge.py               # Bridge MQTT → SQLite (Python)
    ├── requirements.txt
    ├── .env.example            # Template de configuração (sem credenciais)
    ├── .env                    # Credenciais reais — NÃO está no GitHub
    └── sql/
        └── create_table.sql    # Script SQL da tabela
```

---

## Task 1 — Bridge MQTT → SQLite (Python)

Escuta mensagens publicadas no broker e persiste no banco de dados SQLite.

### Pré-requisitos

- Python 3.8+
- Acesso ao broker MQTT

### Configuração

```bash
cd mqtt_bridge
cp .env.example .env
```

Edite o arquivo `.env` com suas credenciais:

```
MQTT_BROKER=Protondata.com.br
MQTT_PORT=1884
MQTT_USER=seu_usuario
MQTT_PASS=sua_senha
MQTT_TOPIC=turma/#
MQTT_CLIENT_ID=mqtt-bridge-01
SQLITE_DB=mqtt_mensagens.db
```

### Execução

```bash
pip install -r requirements.txt
python bridge.py
```

### Verificar dados salvos

```bash
python -c "
import sqlite3
con = sqlite3.connect('mqtt_mensagens.db')
rows = con.execute('SELECT * FROM mqtt_mensagens ORDER BY id DESC LIMIT 20').fetchall()
for r in rows:
    print(r)
"
```

### Campos da tabela `mqtt_mensagens`

| Campo      | Tipo    | Descrição                              |
|------------|---------|----------------------------------------|
| `id`       | INTEGER | Chave primária autoincrementada        |
| `data_hora`| TEXT    | Timestamp ISO 8601 da inserção         |
| `mensagem` | TEXT    | Payload recebido                       |
| `topico`   | TEXT    | Tópico MQTT de origem                  |
| `server`   | TEXT    | Endereço do broker (host:porta)        |

---

## Task 2 — NodeMCU ESP8266 com MQTT

Controla o LED interno do NodeMCU via mensagens MQTT e publica dados do dispositivo.

### Pré-requisitos

- [PlatformIO](https://platformio.org/) instalado no VS Code
- NodeMCU ESP8266
- Rede Wi-Fi 2.4GHz

### Configuração

Copie o template e preencha com seus dados:

```bash
cp include/config.example.h include/config.h
```

Edite `include/config.h`:

```cpp
#define WIFI_SSID      "sua_rede"
#define WIFI_PASS      "sua_senha"

#define MQTT_BROKER    "Protondata.com.br"
#define MQTT_PORT      1884
#define MQTT_USER      "seu_usuario"
#define MQTT_PASS      "sua_senha"
#define MQTT_CLIENT_ID "nodemcu-01"
```

### Upload

```bash
pio run --target upload
```

### Monitorar Serial

```bash
pio device monitor --baud 115200
```

### Tópicos MQTT

| Tópico              | Direção  | Payload          | Descrição                  |
|---------------------|----------|------------------|----------------------------|
| `casa/led/set`      | Recebe   | `1` ou `0`       | Liga ou desliga o LED      |
| `casa/led/status`   | Publica  | `ligado` / `desligado` | Confirmação do comando |
| `casa/nodemcu/ip`   | Publica  | ex: `192.168.0.25` | IP do dispositivo        |
| `casa/nodemcu/status` | Publica | `online` / `offline` | Estado do dispositivo  |
| `casa/nodemcu/uptime` | Publica | ex: `15320`      | Segundos desde o boot      |

> O status `offline` é publicado automaticamente pelo broker caso o dispositivo perca conexão (Last Will).

### Comportamento

- Ao ligar, o NodeMCU conecta ao Wi-Fi e ao broker, e publica `online` e seu IP
- A cada 30 segundos publica IP, status e uptime
- Ao receber `1` em `casa/led/set`, acende o LED e confirma com `ligado`
- Ao receber `0` em `casa/led/set`, apaga o LED e confirma com `desligado`

---

## Segurança

Os arquivos abaixo contêm credenciais reais e estão listados no `.gitignore`:

| Arquivo              | Contém                          |
|----------------------|---------------------------------|
| `include/config.h`   | Wi-Fi e credenciais MQTT do ESP |
| `mqtt_bridge/.env`   | Credenciais MQTT e caminho do banco |

Use os templates `.env.example` e `config.example.h` como base ao clonar o projeto.
