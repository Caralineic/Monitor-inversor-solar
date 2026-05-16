// Importa a biblioteca SoftwareSerial
#include <SoftwareSerial.h>

// Pino 10 recebe, pino 11 envia — mesma lógica do mestre
SoftwareSerial serial2(10, 11);

void setup() {
  Serial.begin(9600);   // comunicação com o computador
  serial2.begin(9600);  // comunicação com o mestre
  Serial.println("Escravo iniciado!");
}

void loop() {
  if (serial2.available()) {  // verifica se chegou mensagem
    String pedido = serial2.readStringUntil('\n');  // lê até o Enter
    Serial.print("Recebi do mestre: ");
    Serial.println(pedido);

    serial2.println("OI MESTRE");  // envia resposta de volta
  }
}


