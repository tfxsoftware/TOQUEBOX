# NIOBOX V4

Dispositivo IoT baseado em ESP32 para controle de relés com recursos WiFi, Ethernet e MQTT.

## Funcionalidades

- **Conectividade Dupla**: Suporte a WiFi e Ethernet com detecção automática
- **Modo AP de Fallback**: Cria ponto de acesso quando a conexão de rede falha
- **Integração MQTT**: Controle remoto via comandos MQTT
- **Interface Web**: Configuração e controle manual
- **Atualizações OTA**: Atualizações de firmware via releases do GitHub
- **Controle de 4 Canais de Relé**: Controle até 4 relés com funcionalidade de pulso

## Hardware

- Placa de Desenvolvimento ESP32
- 4 Relés (Ativo em BAIXO nos pinos GPIO32, GPIO33, GPIO4, GPIO16)
- Botão de Reset (GPIO13)
- Módulo Ethernet (Opcional)

## Arquitetura de Software

O projeto está organizado em gerenciadores modulares:

- **Gerenciador de Rede**: Lida com conectividade WiFi e Ethernet
- **Gerenciador de Armazenamento**: Gerencia operações de arquivos SPIFFS para credenciais
- **Gerenciador de Hardware**: Controla o botão de reset e os relés
- **Gerenciador Web**: Configura e executa o servidor web
- **Gerenciador MQTT**: Lida com comunicação MQTT
- **Gerenciador OTA**: Gerencia atualizações de firmware over-the-air

## Configuração MQTT

- **Servidor**: 186.209.52.208:1883
- **Tópico**: Endereco MAC do dispotivo
- **Comandos**:
  - "RELAY1": Controla o relé 1
  - "RELAY2": Controla o relé 2
  - "RELAY3": Controla o relé 3
  - "RELAY4": Controla o relé 4


## Compilação e Gravação

Este projeto utiliza PlatformIO para compilação e gravação. Para compilar e fazer upload:

```bash
pio run -t upload
```

Para monitorar a saída serial:

```bash
pio device monitor
```

## Dependências

- ESPAsyncWebServer
- AsyncTCP
- PubSubClient
- ArduinoJson
