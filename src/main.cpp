#include <Arduino.h>
#include "internet.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

const char *mqtt_server = "broker.hivemq.com"; //link do mqtt broker gratuito
const int mqtt_port = 1883; //numero de porta padrao do broker 
const char *mqtt_client_id = "senai134_esp_juliana";
const char *mqtt_topic_sub = "senai134/devgoogle/sub"; 
const char *mqtt_topic_pub = "senai134/devgoogle/pub"; //esp publica nesse, fx ouve nesse

WiFiClient espClient;
PubSubClient client(espClient);

//-----------------Protipo das funçoes-------------------------------------------------------
void conectaMqtt();
void retornoMqtt(char *, byte *, unsigned int);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10000); //espera um tempo pra digitar

  checkWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(retornoMqtt); //configuraçao q mostra que ela (retorno Mqtt) q traz o retorno


}

void loop() {
  checkWiFi(); //ve se o wifi ta conectando

  client.loop(); //observa oq esta acontecendo com a conexao o client, atualiza o client

  if (!client.connected()) //verifica se nao esta conectado
  {
    conectaMqtt(); //executa a funçao de conectar
  }

  //-----------------Leitura Serial---------------------------------------------------------------------
  if (Serial.available() > 0) 
  {
    String textoDigitado = Serial.readStringUntil('\n');
    textoDigitado.trim();

    //------------------Nao envia texto vazio-----------------------------------------------------------
    if (textoDigitado.length() == 0)
    {
      Serial.println("Mensagem vazia");
      return; //inicia o loop novamente
    }

    //-----------------------Estrutura o Json-----------------------------------------------------------
    JsonDocument doc;
    doc["dispositivo"] = "Esp32Juliana";
    doc["mensagem"] = textoDigitado;
    doc["time"] = millis(); //leitura de sensor, tempo pode ser montado aqui dentro

    String stringJson;
    serializeJson(doc, stringJson);

    //------------------------------Publica no Mqtt-----------------------------------------------------
    client.publish(mqtt_topic_pub, stringJson.c_str()); //converteu string pra char
    Serial.println(stringJson);
    
  }
  
  

 
}


void conectaMqtt(){
  while (!client.connected())
  {
    Serial.println("Conectando ao Mqtt...");
    if (client.connect(mqtt_client_id))
    {
      Serial.println("Conectado");
      client.subscribe(mqtt_topic_sub);
    }else
    {
      Serial.print("Falha :");
      Serial.print(client.state());
      Serial.print("Tentando novamente em 5s");
      delay(5000);
    }
    
  }
  
}

void retornoMqtt(char *topic, byte *payload, unsigned int lenght){
  Serial.print("Mensagem recebida em: ");
  Serial.print(topic); //qual canal recebeu essa mensagem
  Serial.print(": ");

  //--------------------Monta a string com a mensagem recebida--------------------------------
  String mensagemRecebida = "";
  for (int i = 0; i < lenght; i++)
  {
    mensagemRecebida += (char)payload[i]; //payload é o corpo da mensage, 
  }

  Serial.print("JSON recebido: ");
  Serial.println(mensagemRecebida);

  //-------------------Interpreta o json recebido---------------------------------------------
  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagemRecebida);

  if (erro)
  {
    Serial.print("Erro de decodificar JSON: ");
    Serial.print(erro.c_str());
    return; 
  }

  //----------------------------Acessa os campos Json-----------------------------------------
  const char *dispositivo = doc ["dispositivo"];
  const char *mensagem = doc ["mensagem"];
  unsigned long time = doc ["time"];
  
  Serial.println("Dados recebidos");
  Serial.printf("Dispositivo: %s\n", dispositivo);
  Serial.printf("Mensagem: %s\n", mensagem);
  Serial.printf("Tempo: %i\n", time);
  Serial.printf("---------------------");

}