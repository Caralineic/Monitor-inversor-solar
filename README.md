
# ⚡ Monitor de Inversor Solar via RS485

Projeto de monitoramento de inversor solar usando Arduino e protocolo Modbus RTU via RS485.

## 📋 Sobre o projeto

Este projeto tem como objetivo captar dados de um inversor de placa solar
através do Arduino, utilizando o módulo RS485 e o protocolo Modbus RTU.

## 🔧 Materiais utilizados

- 2x Arduino Uno
- Módulo sensor DHT11
- Módulo RS485 (MAX485) — aguardando chegada
- Fios de conexão

## 📚 Bibliotecas necessárias

- **DHT sensor library** (Adafruit)
- **Adafruit Unified Sensor** (Adafruit)
- **SoftwareSerial** (Arduino)
- **ArduinoRS485** (Arduino)
- **ArduinoModbus** (Arduino)

## ✅ Testes realizados

- [x] Pisca LED no Arduino IDE e PictoBlox
- [x] Leitura do sensor DHT11 (temperatura e umidade)
- [x] Comunicação serial entre dois Arduinos (mestre/escravo)
- [ ] Teste com módulo RS485
- [ ] Leitura real do inversor solar

## 📁 Estrutura do projeto
monitor-inversor-solar/
codigos/
teste01_dht11/
dht11.ino
teste02_dois_arduinos/
mestre.ino
escravo.ino
docs/
Documentacao_Arduino.docx
bibliotecas/
bibliotecas.md
README.md

## 📖 Documentação

A documentação completa do projeto está na pasta `/docs`.

## 🚀 Próximos passos

- Receber o módulo RS485 (MAX485)
- Testar comunicação com módulo RS485
- Descobrir os registradores Modbus do inversor solar
- Implementar leitura real dos dados do inversor