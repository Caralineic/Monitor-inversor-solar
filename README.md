# Monitor de Inversor Solar via RS-485

Interface de telemetria em Arduino que lê, de forma passiva e não invasiva, os dados
operacionais de um inversor solar fotovoltaico pela porta RS-485, usando o protocolo
Modbus RTU.

Projeto de pesquisa do **IFSP – Câmpus Boituva**, com o objetivo de integrar inversores
ainda não monitorados à plataforma Campus Telemetry do câmpus.

**Status:** comunicação estabelecida e leituras validadas em campo.
Pendente: publicação via MQTT.

---

## O problema

Os inversores do câmpus geram energia há mais de uma década sem monitoramento
automatizado. Os dados existem — o inversor mede tudo internamente — mas ficam
acessíveis apenas num display dentro de uma cabine de força de acesso restrito. Sem
acompanhamento, uma queda de rendimento ou uma falha pode passar semanas despercebida.

## A solução

Um Arduino Uno com módulo conversor RS-485 atua como mestre Modbus, consultando o
inversor periodicamente e convertendo os registradores em grandezas de engenharia.
Custo total do hardware: cerca de R$ 80.

A leitura é **exclusivamente passiva**: o código implementa apenas a função Modbus
`0x04` (Read Input Registers). As funções de escrita, que poderiam alterar parâmetros do
equipamento, não existem no projeto.

---

## Hardware

| Item | Modelo |
|---|---|
| Microcontrolador | Arduino Uno |
| Conversor serial | Módulo RS-485 baseado no MAX485 |
| Inversor | Ingeteam INGECON SUN 1Play 5TL M (5 kW, monofásico, 2013) |
| Firmware do inversor | ABF1002_E |

### Ligação

| Módulo RS-485 | Arduino |
|---|---|
| `RO` | D2 |
| `DI` | D3 |
| `DE` + `RE` (unidos) | D4 |
| `VCC` | 5 V |
| `GND` | GND |
| `A` / `B` | borne RS-485 do inversor |

> **Atenção à polaridade.** A inversão dos condutores A e B foi a causa de dois meses de
> falha neste projeto. Ver a seção de diagnóstico.

O terra do inversor deve ser ligado ao terra do Arduino: o RS-485 precisa de uma
referência de modo comum para funcionar.

### Parâmetros de comunicação

```
Velocidade : 9600 bps
Formato    : 8N1
Endereço   : 5  (confirmado na IHM, menu Ajustes > ID Modbus)
Função     : 0x04 — Read Input Registers
Intervalo  : 5 s (o fabricante recomenda no máximo 2 requisições por segundo)
```

---

## Estrutura do repositório

```
Monitor-inversor-solar/
├── codigos/
│   ├── teste01_dht11/              leitura de sensor (etapa inicial)
│   ├── teste02_dois_arduinos/      comunicação mestre/escravo em bancada
│   ├── teste03_diagnostico/        diagnóstico elétrico da linha RS-485
│   └── telemetria_inversor/        leitura do inversor (versão atual)
├── docs/                           documentação e mapa de registradores
├── bibliotecas/                    bibliotecas utilizadas
└── README.md
```

---

## Etapas concluídas

O desenvolvimento seguiu metodologia incremental: cada etapa foi validada antes de
avançar para a seguinte.

- [x] Programação básica do microcontrolador
- [x] Leitura de sensor DHT11 (temperatura e umidade)
- [x] Comunicação serial entre dois Arduinos (mestre/escravo)
- [x] Comunicação RS-485 validada em bancada
- [x] Implementação do protocolo Modbus RTU com verificação por CRC16
- [x] Descoberta dos parâmetros de comunicação do inversor
- [x] Diagnóstico e correção da falha de comunicação
- [x] Leitura real do inversor, validada em campo
- [ ] Publicação dos dados via MQTT no Campus Telemetry

Essa ordem foi decisiva para o diagnóstico: como a comunicação RS-485 já estava validada
em bancada, foi possível afirmar que o problema não estava no código nem no controlador.

---

## Grandezas lidas

O mapa de registradores é tratado como **dado**, não como lógica. Para acrescentar uma
grandeza, basta incluir uma linha na tabela do código:

```cpp
const Grandeza TABELA[] PROGMEM = {
  { 29, "Corrente da rede",   "A",   100, false, 2 },
  { 30, "Tensao da rede",     "V",    10, false, 1 },
  // endereco, rotulo, unidade, divisor, tem sinal?, casas decimais
};
```

Os endereços são os **Start Address** do manual da Ingeteam, sem conversão. O divisor vem
da coluna *Magnitude* (`Vrmsx10` → 10) e o campo de sinal vem do tipo declarado
(`Int16` → `true`, `Uint16` → `false`).

| Endereço | Grandeza | Unidade |
|---|---|---|
| 0–5 | Data e hora do relógio interno | — |
| 26 | Estado da unidade | 0 a 2 |
| 29 | Corrente da rede | A |
| 30 | Tensão da rede | V |
| 32 | Frequência | Hz |
| 33–35 | Potência aparente, ativa e reativa | VA, W, VAr |
| 36 | Fator de potência | — |
| 37, 39, 41 | Corrente, tensão e potência fotovoltaica | A, V, W |
| 44 | Resistência de isolamento | kΩ |
| 49–50 | Energia do dia | kWh |

---

## Resultados validados

Leitura obtida em 17/10/2026, às 09:05:

```
Estado              : CONECTADO A REDE
Corrente da rede    : 8.93 A
Tensao da rede      : 230.2 V
Frequencia          : 59.98 Hz
Potencia ativa      : 2013 W
Fator de potencia   : 1.000
Tensao FV           : 277 V
Potencia FV         : 2074 W
Energia do dia      : 2.25 kWh
```

A correção das leituras foi verificada por três caminhos independentes:

1. **Coerência elétrica** — 230,2 V × 8,93 A = 2.056 VA, contra 2.013 VA informados pelo
   equipamento. A diferença de 2% corresponde ao arredondamento das escalas.
2. **Rendimento** — 2.074 W no lado fotovoltaico contra 2.013 W entregues à rede, ou
   97,1%, valor plausível que só se obtém se ambas as leituras estiverem corretas.
3. **Identificação do equipamento** — a função `0x11` (Report Server ID) devolveu o
   número de série `130617282026`, idêntico ao da etiqueta física do inversor.

---

## Diagnóstico: dois meses até a primeira leitura

Esta seção documenta o processo, porque ele foi a maior parte do trabalho.

### Sintoma inicial

Chegavam bytes pela serial, mas nenhum quadro Modbus válido. A análise estatística de 363
bytes capturados mostrou que não era dado mal interpretado:

| Indicador | Observado | Esperado em quadro real |
|---|---|---|
| Bits em nível alto | 85,9% | ~50% |
| Bytes com 6+ bits altos | 93,8% | ~14% |
| Ocorrências de `0x00` | nenhuma | várias |
| Quadros válidos por CRC16 | 0 | todos |

Os bytes mais frequentes eram `0xFF` com exatamente um bit apagado — assinatura de linha
em repouso captando ruído, em que a interferência gera falsos bits de início. A ausência
total do byte `0x00` foi decisiva: em Modbus ele é frequente, pois ocupa a parte alta de
quase todo endereço.

### Hipóteses eliminadas

| Hipótese | Como foi descartada |
|---|---|
| Erro de escala ou offset | Produziria valores errados porém estáveis; não havia quadro válido algum |
| Ruído por falta de referência | Resolvido com aterramento comum e remoção de derivações |
| Endereço incorreto | ID 5 confirmado na IHM; varredura de IDs 1 a 32 |
| Velocidade incorreta | Varredura de 1200 a 115200 bps |
| Paridade incorreta | 8N1, 8E1 e 8O1 testados |
| Função inadequada | Nem o eco `0x08` nem o `0x11` respondiam |
| Modbus desabilitado | A série 1Play TL M não possui essa opção; RS-485 é padrão |

Ao todo, 310 combinações de velocidade, endereço, paridade e comando, todas sem resposta.

### A virada

Esgotadas as hipóteses de configuração, a pergunta mudou de *"o inversor responde?"* para
*"em que estado elétrico está a linha?"*.

Um sketch de diagnóstico passou a amostrar o pino `RO` diretamente, mais de 240 mil vezes
em dois segundos, com o receptor habilitado e desabilitado. O resultado mostrou uma linha
sem nível de repouso definido, que permanecia no estado em que a última atividade a havia
deixado — sintoma de **inversão dos condutores A e B**. Após a troca, a comunicação
funcionou na primeira tentativa.

### Aprendizados

- Com A e B invertidos, a `SoftwareSerial` não entrega byte algum em vez de entregar
  lixo: sem transição de descida na linha, a interrupção que inicia a recepção nunca
  dispara.
- Uma resposta de exceção Modbus, com CRC válido, é prova de link funcionando tanto
  quanto um dado. Durante o diagnóstico, vale buscá-la.
- Implementar o protocolo manualmente, em vez de usar biblioteca pronta, foi o que
  permitiu enxergar os bytes crus — e foram eles que revelaram o problema.

---

## Bibliotecas

| Biblioteca | Uso |
|---|---|
| `SoftwareSerial` | porta serial em pinos comuns, para o módulo RS-485 |
| `DHT sensor library` + `Adafruit Unified Sensor` | apenas na etapa inicial, com o DHT11 |

O protocolo Modbus foi implementado manualmente, sem biblioteca, incluindo a montagem dos
quadros e o cálculo do CRC16.

---

## Como usar

1. Monte o circuito conforme a tabela de ligação.
2. Abra `codigos/telemetria_inversor/` na IDE do Arduino.
3. Ajuste o bloco de configuração no topo: pinos, endereço Modbus e velocidade.
4. Grave na placa e abra o monitor serial em **115200 bps**.

Se nada aparecer, verifique nesta ordem: a velocidade do monitor serial, a porta
selecionada na IDE e a polaridade dos fios A e B.

---

## Próximas etapas

- [ ] Publicação dos dados via MQTT na API do Campus Telemetry
- [ ] Leitura dos registradores de alarme (18 a 22) para detecção de falhas
- [ ] Leitura do segundo canal MPPT
- [ ] Tratamento de reconexão automática em caso de perda de comunicação

---

## Referências

- Ingeteam. *INGECON SUN 1Play — Functionality Specifications*, mapa de registradores
  Modbus (ABF1000), rev. 03/05/2024.
- MODBUS. *Modbus Application Protocol Specification V1.1b3*. Modbus Organization, 2012.
- TRAN, D. L. Develop an RS-485 Protocol for Arduino Boards. *Journal of Technical
  Education Science*, v. 19, n. SI03, p. 80-89, 2024.

## Autoria

**Caroline Inacio Campos** — desenvolvimento e documentação
**Prof. Daniel Cintra Cugler** — orientação

IFSP – Câmpus Boituva · Análise e Desenvolvimento de Sistemas
