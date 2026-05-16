// Importa a biblioteca SoftwareSerial
#include <SoftwareSerial.h>

// Cria porta serial: pino 10 (RX = recebe), pino 11 (TX = envia)
SoftwareSerial serial2(10, 11);

void setup() {
  Serial.begin(9600);   // comunicação com o computador
  serial2.begin(9600);  // comunicação com o escravo
  Serial.println("Mestre iniciado!");
}

void loop() {
  serial2.println("OI ESCRAVO");  // envia mensagem pelo fio
  Serial.println("Mensagem enviada ao escravo...");

  delay(500);  // aguarda o escravo processar e responder

  if (serial2.available()) {  // verifica se chegou resposta
    String resposta = serial2.readStringUntil('\n');  // lê até o Enter
    Serial.print("Resposta do escravo: ");
    Serial.println(resposta);
  }

  delay(2000);  // aguarda 2 segundos antes de enviar novamente
}

