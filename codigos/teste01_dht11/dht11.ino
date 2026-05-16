//importar a biblioteca que conversa com o senhor (DHT11)
#include <DHT.h>


//define que o sensor ta ligado no pino 2
#define PINO_DHT 2


//definindo o modelo do sensor
#define TIPO_DHT DHT11


//cria um objeto dht pra ultilizar
DHT dht(PINO_DHT, TIPO_DHT);


void setup(){
  Serial.begin(9600); //comunicação com o computador
  dht.begin(); //ligar e inicializar o sensor
  Serial.println("\nLeitura do sensor iniciada!"); // mostra que começou a leitura
}


void loop(){
  delay(5000); //delay de 2s pra esperar o sensor


  int temperatura = dht.readTemperature(); //le a temperatura e guarda na variavel
  int umidade = dht.readHumidity(); //le a umidade e guarda na variavel


  if(isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler o sensor! Verifique a ligação");
    return;
  }


    //Verifica se a leitura falhou, usa o isnan "is nota a number"
    Serial.println("----- SENSOR TEMPERATURA-----");


    Serial.print(" Temperatura ");
    Serial.print(temperatura);
    Serial.println(" °C");


    Serial.print(" Umidade ");
    Serial.print(umidade);
    Serial.println(" %");


  }


