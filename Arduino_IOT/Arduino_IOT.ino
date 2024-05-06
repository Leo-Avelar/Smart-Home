#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <arduino.h>
#include <WiFi.h>

//--------- PINOS ----------
//Atuadores:
#define PIN_LED 2
#define PIN_LED_2 4
#define PIN_ALARME 16
#define PIN_SERVO 26
//Sensores:
#define PIN_FOTO 35
#define PIN_PRESENCA 19
#define PIN_CHUVA 15
  
//=============================   TOPICOS   ===================================
// LIGAR E DESLIGAR OS ATUADORES:
#define TOPICO_SUBSCRIBE_LED         "topico_liga_desliga_led_teste-1230"
#define TOPICO_SUBSCRIBE_SERVO       "topico_liga_desliga_servo_teste-1230"
#define TOPICO_SUBSCRIBE_ALARME      "topico_liga_desliga_alarme_teste-1230"

// LIGAR E DESLIGAR OS SENSORES:
#define TOPICO_SUBSCRIBE_FOTO        "topico_liga_desliga_foto_teste-12304"
#define TOPICO_SUBSCRIBE_CHUVA       "topico_liga_desliga_chuva_teste-1230"
#define TOPICO_SUBSCRIBE_PRESENCA    "topico_liga_desliga_presenca_teste-1230"

// RECEBER INFORMAÇÕES DOS SENSORES:
#define TOPICO_PUBLISH_FOTO          "topico_sensor_fotorresistor_teste-1230"
#define TOPICO_PUBLISH_CHUVA         "topico_sensor_chuva_teste-1230"
#define TOPICO_PUBLISH_PRESENCA      "topico_sensor_presenca_teste-1230"

#define ID_MQTT  "IoT_PUC_SG_mqtt2"     //id mqtt (para identificação de sessão)
//============================= INTERNET ========================================
// WIFI:
const char* SSID     = "Moto123"; // Nome da sua rede
const char* PASSWORD = "senha123";

const char* BROKER_MQTT = "test.mosquitto.org";
int BROKER_PORT = 1883; // Porta do Broker MQTT
//===============================================================================
//Variáveis e objetos globais:
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
Servo servoMotor;

bool fotoSensor =  true;
bool chuvaSensor = true;
bool presencaSensor =  true;

/* Prototypes */
void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   INICIALIZAR   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void initWiFi(void){
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}
void initMQTT(void){
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   CALLBACKS   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void mqtt_callback(char* topic, byte* payload, unsigned int length){ // LED mqtt_callback

  if(strcmp(topic, TOPICO_SUBSCRIBE_LED) == 0){
    led_callback(topic, payload, length);
  }
  else if (strcmp(topic, TOPICO_SUBSCRIBE_SERVO) == 0){
    servo_callback(topic, payload, length);
  }
  else if (strcmp(topic, TOPICO_SUBSCRIBE_ALARME) == 0){
    alarme_callback(topic, payload, length);
  }
  else if (strcmp(topic, TOPICO_SUBSCRIBE_FOTO) == 0){
    sensorFoto_callback(topic, payload, length);
  }
  else if (strcmp(topic, TOPICO_SUBSCRIBE_CHUVA) == 0){
    sensorChuva_callback(topic, payload, length);
  }
  else if (strcmp(topic, TOPICO_SUBSCRIBE_PRESENCA) == 0){  
    sensorPresenca_callback(topic, payload, length);
  }
  else{
    Serial.println(" Topico desconhecido: ");
    Serial.println(topic);
  }
}

// Ligar/Desligar Atuadores:
void led_callback(char* topic, byte* payload, unsigned int length){
  String msg;
  /* obtem a string do payload recebido */
  for (int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }
  
  Serial.print("\nChegou a seguinte string via MQTT (led): ");
  Serial.println(msg);

  if(msg.equals("L")){
    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_LED_2, HIGH);
  }
  else if (msg.equals("D")){
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_LED_2, LOW);
  }
  else{
    Serial.println("\nNão identificou comando MQTT recebido.");
  }
}
void servo_callback(char* topic, byte* payload, unsigned int length){
  String msg;
  /* obtem a string do payload recebido */
  for (int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }
  
  Serial.print("\nChegou a seguinte string via MQTT (servo): ");
  Serial.println(msg);

  int servoPosicaoMicros = servoMotor.read();

  if(msg.equals("L")){

    if(!(servoPosicaoMicros >= 80)){
      for (int pos = 0; pos <= 100; pos += 1) {
        servoMotor.write(pos);
        delay(15);
      }
    }
  }
  else if (msg.equals("D")){
    if(!(servoPosicaoMicros <= 5)){
      for (int pos = 100; pos >= 0; pos -= 1){
        servoMotor.write(pos);
        delay(15);
      }
    }
  }
  else{
    Serial.println("\nNão identificou comando MQTT recebido.");
  }
}
void alarme_callback(char* topic, byte* payload, unsigned int length){
  String msg;

  for (int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }
  
  Serial.print("\nChegou a seguinte string via MQTT (alarme): ");
  Serial.println(msg);

  if(msg.equals("L")){
    digitalWrite(PIN_ALARME, HIGH);
  }
  else if (msg.equals("D")){
    digitalWrite(PIN_ALARME, LOW);
  }
  else{
    Serial.println("\nNão identificou comando MQTT recebido.");
  }
}

// Ligar/Desligar Sensores:
void sensorFoto_callback(char* topic, byte* payload, unsigned int length){
  String msg;
  for (int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }
  Serial.print("\nChegou a seguinte string via MQTT (sensor fotorresistor): ");
  Serial.println(msg);

  if(msg.equals("L")){
    fotoSensor = true;
  }
  else if(msg.equals("D")){
    fotoSensor = false;
  }
  else{
    Serial.println("\nNão identificou comando MQTT recebido.");
    Serial.println(msg);
  }
}
void sensorChuva_callback(char* topic, byte* payload, unsigned int length){
  String msg;
  for (int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }
  Serial.print("\nChegou a seguinte string via MQTT (sensor de chuva): ");
  Serial.println(msg);

  if(msg.equals("L")){
    chuvaSensor = true;
  }
  else if(msg.equals("D")){
    chuvaSensor = false;
  }
  else{
    Serial.println("\nNão identificou comando MQTT recebido.");
  }
}
void sensorPresenca_callback(char* topic, byte* payload, unsigned int length){
  String msg;
  for (int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }
  Serial.print("\nChegou a seguinte string via MQTT (sensor de presença): ");
  Serial.println(msg);

  if(msg.equals("L")){
    presencaSensor = true;
  }
  else if(msg.equals("D")){
    presencaSensor = false;
  }
  else{
    Serial.println("\nNão identificou comando MQTT recebido.");
  }
}


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   CONEXÕES   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void reconnectMQTT(void){
  while (!MQTT.connected())
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);

    if (MQTT.connect(ID_MQTT)){
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE_LED);
      MQTT.subscribe(TOPICO_SUBSCRIBE_SERVO);
      MQTT.subscribe(TOPICO_SUBSCRIBE_ALARME);

      MQTT.subscribe(TOPICO_SUBSCRIBE_FOTO);
      MQTT.subscribe(TOPICO_SUBSCRIBE_CHUVA);
      MQTT.subscribe(TOPICO_SUBSCRIBE_PRESENCA);    
    }
    else{
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentativa de conexao em 2s");
      delay(1000);
    }

  }  
}
void VerificaConexoesWiFIEMQTT(void){
  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita

  reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
void reconnectWiFi(void){
  //se já está conectado à rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("\nIP obtido: ");
  Serial.println(WiFi.localIP());
}


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   SETUP E LOOP   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void setup() {
  Serial.begin(9600); //Enviar e receber dados em 9600 baud

  delay(1000);
  Serial.println("Disciplina IoT: acesso a nuvem via ESP32");

  delay(1000);
  pinMode(PIN_LED, OUTPUT); // programa LED interno como saida
  pinMode(PIN_LED_2, OUTPUT);
  pinMode(PIN_ALARME, OUTPUT);
  pinMode(PIN_PRESENCA, INPUT);
  
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_LED_2, LOW);
  digitalWrite(PIN_ALARME, LOW);

  initWiFi();   /* Inicializa a conexao wi-fi */
  initMQTT();   /* Inicializa a conexao ao broker MQTT */

  servoMotor.attach(PIN_SERVO);
}

void loop() {

  VerificaConexoesWiFIEMQTT(); /* garante funcionamento das conexões WiFi e ao broker MQTT */

  Serial.print("\n");

  if(fotoSensor == true){
    sensorLuminosidade();
    Serial.print("  |  ");
  }

  if(presencaSensor == true){
    sensorPresenca();
    Serial.print("  |  ");
  }

  if(chuvaSensor == true){
    sensorChuva();
    Serial.print("  |  ");
  }

  MQTT.loop();  /* keep-alive da comunicação com broker MQTT */
  delay(1000);
}


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   SENSORES   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void sensorLuminosidade(){
 
  int sensorValue = analogRead(PIN_FOTO);
  char mensagem[30] = {0};

  if (sensorValue < 40) {
    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_LED_2, HIGH);
    sprintf(mensagem, "%d => Dark", sensorValue);
    //sprintf(mensagem,"Escuro", sensorValue);

  } else if (sensorValue < 300) {//700
    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_LED_2, HIGH);
    sprintf(mensagem, "%d => Dim", sensorValue);
    //sprintf(mensagem,"Claro", sensorValue);

  } else if (sensorValue < 2000) {
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_LED_2, LOW);
    sprintf(mensagem, "%d => Light", sensorValue);
    //sprintf(mensagem,"Claro", sensorValue);

  } else if (sensorValue < 3200) {
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_LED_2, LOW);
    sprintf(mensagem, "%d => Bright", sensorValue);
    //sprintf(mensagem,"Claro", sensorValue);
  } else {
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_LED_2, LOW);
    sprintf(mensagem, "%d => Very bright", sensorValue);
    //sprintf(mensagem,"Claro", sensorValue);
  }

  Serial.print("Valor de luminosidade = ");
  Serial.print(mensagem);

  MQTT.publish(TOPICO_PUBLISH_FOTO, mensagem);
}
void sensorPresenca(){
  int sensorValue = digitalRead(PIN_PRESENCA);
  char mensagem[30] = {0};

  if (sensorValue == HIGH) {
    sprintf(mensagem," Movimento detectado ! ");
    digitalWrite(PIN_ALARME, HIGH);
  } else {
    sprintf(mensagem," Nenhum movimento detectado. ");
    digitalWrite(PIN_ALARME, LOW);
  }

  Serial.print(mensagem);
  MQTT.publish(TOPICO_PUBLISH_PRESENCA, mensagem);
}
void sensorChuva(){

  int sensorValue = digitalRead(PIN_CHUVA);
  int servoPosicaoMicros = servoMotor.read();
  char mensagem[50] = {0};

  if(sensorValue == LOW){
    if(!(servoPosicaoMicros >= 80)){
      for (int pos = 0; pos <= 100; pos += 1) {
        servoMotor.write(pos);
        delay(15);
      }
      sprintf(mensagem,"Sem sinal de chuva, a janela foi aberta");
    }
    else{
      sprintf(mensagem,"Sem sinal de chuva, a janela já está aberta");
    }
  }
  
  else{
    if(!(servoPosicaoMicros <= 5)){
      for (int pos = 100; pos >= 0; pos -= 1){
        servoMotor.write(pos);
        delay(15);
      }
      sprintf(mensagem,"Chuva detectada, a janela foi fechada");
    }
    else{
      sprintf(mensagem,"Chuva detectada, a janela já está fechada");
    }
  }

  Serial.print(mensagem);
  MQTT.publish(TOPICO_PUBLISH_CHUVA, mensagem);
}