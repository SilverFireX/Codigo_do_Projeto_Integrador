#include <PID_v1.h>             // Controle PID
#include <Servo.h>              // Servo Motor
#include "Ultrasonic.h"         // Sensor Ultrasom
#include <Wire.h>               // Tela
#include <LiquidCrystal_I2C.h>  // Tela LCD I2C


#define motoD1 5
#define motoD2 4
#define motoE1 3
#define motoE2 2
//#define enA ?
//#define enB ?
#define refletancia_pin 11 // DEFINIÇÃO DOS PINOS DO SENSOR DE REFLETANCIA

Ultrasonic ultrasonic(12, 13);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display


// Variável Global para armazenar os caracteres que o arduino receber do celular via módulo bluetooth
char DadosRecebidos;

int AuxServo01 = 90;
int Distancia = 200;
uint32_t temp, bip;  // Variáveis necessárias para contagem de tempo em milis segundos (substitui a função delay) - (variáveis uint32_t são inteiros de 32 bits sem sinal)
boolean BipSensor = 0;
int buzzer = A0;
//int Speed = 130;            //VELOCIDADE DOS MOTORES DE PASSO
//bool terraFirme;      // VÁRIÁVEL PARA CHECAR SE HÁ CHÃO A FRENTE DO ROBO
//bool perigo = false;  // VARIÁVEL PARA CONTROLE DE SEGURANÇA

// Variáveis do PID
double Setpoint, Input, Output;
double Kp = 2.0, Ki = 5.0, Kd = 1.0;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Variáveis de controle dos motores
unsigned long lastStepTimeRight = 0;
unsigned long lastStepTimeLeft = 0;
unsigned long stepIntervalRight = 1000;  // Intervalo entre passos (em microssegundos)
unsigned long stepIntervalLeft = 1000;   // Intervalo entre passos (em microssegundos)


void setup() {

  Serial.begin(9600);

  // Configurando portas do arduino
  pinMode(motoD1, OUTPUT);
  pinMode(motoD2, OUTPUT);
  pinMode(motoE1, OUTPUT);
  pinMode(motoE2, OUTPUT);

  pinMode(7, OUTPUT);  // LED
  pinMode(8, OUTPUT);  // Bomba

  pinMode(A0, OUTPUT);  // Buzzer
  Servo01.attach(9);    // cabeça

  // CONFIGURAÇÃO DO SENSOR DE REFLETÂNCIA
  pinMode(refletancia_pin, INPUT);
  Serial.begin(115200);
}

{
  lcd.init();  //inicializar LCD
  //Esquever Mensagem na Tela
  lcd.blacklight();
  lcd.setCursor(0, 0);
  lcd.print("Thiago Bomfim");
  delay(300);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Terror da");
  lcd.setCursor(0, 1);
  lcd.print("Automacao");



  // Inicialização do PID
  Setpoint = 20.0;  // Distância desejada do obstáculo em cm
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);
}





//Fim do Setup

void loop() {

  int refletancia = digitalRead(refletancia_pin);

  DadosRecebidos = Serial.read();  // Variável DadosRecebidos armazena os dados recebidos da comunicação serial entre o celular e o Módulo Bluetooth


  if (DadosRecebidos == 'I') {
    BipSensor = 1;
    //checarPerigo();
    Buzzer();

    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Modo");
    lcd.setCursor(3, 1);
    lcd.print("Automatico");

  }  //Modo Automatico
  else if (DadosRecebidos == 'i') {
    BipSensor = 0;
    parado();
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Modo");
    lcd.setCursor(3, 1);
    lcd.print("Manual");
  }  //Manual Android Application Control Command

  //analogWrite(enA, Speed); // Write The Duty Cycle 0 to 255 Enable Pin A for Motor1 Speed
  //analogWrite(enB, Speed); // Write The Duty Cycle 0 to 255 Enable Pin B for Motor2 Speed

  if (BipSensor = 0) {
    //===============================================================================
    //                        COMANDO MANUAL
    //===============================================================================
    if (DadosRecebidos == 'W') { Frente(); }  // if the bt_data is '1' the DC motor will go forward
    else if (DadosRecebidos == 'S') {
      Re();
    }                                                   // if the bt_data is '2' the motor will Reverse
    else if (DadosRecebidos == 'A') { paraEsqueda(); }  // if the bt_data is '3' the motor will turn left
    else if (DadosRecebidos == 'D') {
      paraDireita();
    }                                              // if the bt_data is '4' the motor will turn right
    else if (DadosRecebidos == 'Z') { Parado(); }  // if the bt_data '5' the motor will Stop

    //===============================================================================
    //                          COMANDO POR VOZ
    //===============================================================================
    else if (DadosRecebidos == 6) {
      paraEsqueda();
      delay(400);
      bt_data = 5;
    } else if (DadosRecebidos == 7) {
      paraDireita();
      delay(400);
      bt_data = 5;
    }
  } else {
    //===============================================================================
    //                          COMANDO AUTOMÁTICO
    //===============================================================================

    Distancia = ultrasonic.Ranging(CM);

    // Atualização do PID
    Input = Distancia;
    myPID.Compute();

    // Ajuste das velocidades dos motores com base no output do PID
    if (Output > 0) {
      stepIntervalRight = 1000 / Output;
      stepIntervalLeft = 1000 / (Output / 2);
      digitalWrite(motoD1, HIGH);  // Direção para frente
      digitalWrite(motoE1, HIGH);  // Direção para frente
    } else {
      stepIntervalRight = 1000 / (-Output);
      stepIntervalLeft = 1000 / (-Output / 2);
      digitalWrite(motoD1, LOW);  // Direção para trás
      digitalWrite(motoE1, LOW);  // Direção para trás
    }

    // Controle do motor direito
    if (micros() - lastStepTimeRight >= stepIntervalRight) {
      lastStepTimeRight = micros();
      digitalWrite(motoD1, HIGH);
      delayMicroseconds(10);
      digitalWrite(motoD1, LOW);
    }

    // Controle do motor esquerdo
    if (micros() - lastStepTimeLeft >= stepIntervalLeft) {
      lastStepTimeLeft = micros();
      digitalWrite(motoE1, HIGH);
      delayMicroseconds(10);
      digitalWrite(motoE1, LOW);
    }


    if (distancia < 20) {  // Objeto à frente a menos de 20 cm
      Parado();
      delay(500);
      Servo01.write(90);  // Centraliza o servo
      delay(500);

      // Verifica a distância para a direita
      Servo01.write(0);
      delay(1000);
      int distancia_direita = ultrasonic.Ranging(CM);

      // Verifica a distância para a esquerda
      Servo01.write(180);
      delay(1000);
      int distancia_esquerda = ultrasonic.Ranging(CM);

      // Decide para onde virar
      if (distancia_direita > distancia_esquerda) {
        paraDireita();
      } else {
        paraEsqueda();
      }

      Parado();
      delay(1000);
      Servo01.write(90);  // Centraliza o servo
      delay(500);
    } else if (refletancia == LOW) {  // Abismo detectado
      Parado();
      delay(500);
      naoCabeca();
      delay(500);
      Re();
      delay(1000);
      Parado();
      delay(500);
      paraDireita();
      Parado();
      delay(1000);
    } else {
      Frente();
    }
  }


  // BUZINA
  if (DadosRecebidos == 'E') {  // Verifica se o caractere enviado do celular e recebido pelo módulo Bluetooth é igual a letra E maiúsculo
    tone(A0, 400);              // Gera um tom com 400 Hz na porta A0 onde está ligado o Buzzer enquanto o botão estiver sendo precionado
    Serial.println("Botão Buzina Pressionado");
  } else if (DadosRecebidos == 'e') {  // Verifica se o caractere enviado do celular e recebido pelo módulo Bluetooth é igual a letra e minúsculo
    noTone(A0);                        // Faz parar todos os tons gerados na porta  4 onde está ligado o Buzzer
  }



  //LED
  if (DadosRecebidos == 'G') {
    digitalWrite(7, HIGH);
    Serial.print(DadosRecebidos);
    Serial.println(" = Acenda LED D");
  }

  //LED
  if (DadosRecebidos == 'g') {
    digitalWrite(7, LOW);
    Serial.print(DadosRecebidos);
    Serial.println(" = Apaga LED D");
  }

  //Bomba
  if (DadosRecebidos == 'H') {
    digitalWrite(8, HIGH);
    Serial.print(DadosRecebidos);
    Serial.println(" = Ativa a bomba");
  }

  if (DadosRecebidos == 'h') {
    digitalWrite(8, LOW);
    Serial.print(DadosRecebidos);
    Serial.println(" = Desativa a bomba");
  }

  //Servo 01
  if (DadosRecebidos == 'X') {
    AuxServo01 = Serial.parseInt();
    Serial.print("Servo 01: ");
    Serial.println(AuxServo01);
  }

  // Ativa e Desativa Bip do Sensor de Distância
  //if (DadosRecebidos == 'I') {  // Verifica se o caractere enviado do celular e recebido pelo módulo Bluetooth é igual a letra I mainúsculo
  //BipSensor = 1;
  // Serial.println("Ativa Bip do Sensor de Distancia");
}

//if (DadosRecebidos == 'i') {  // Verifica se o caractere enviado do celular e recebido pelo módulo Bluetooth é igual a letra i minúsculo
//BipSensor = 0;
// Serial.println("Desativa Bip do Sensor de Distancia");
}

if (BipSensor == 1) {
  Buzzer();  // Chamada a função Buzzer()
}


Servo01.write(AuxServo01);  // Configura a posição do Servo de acordo com a posição do joystick (valor da variável AuxServo01)


//Enviando dados do sensor ultrassônico de distância para o aplicativo
// Os dados serão enviados de 100 em 100 ms para não sobrecarregar a comunicação serial
if (millis() - temp > 100) {
  Distancia = ultrasonic.Ranging(CM);
  temp = millis();
}
}
// FIM DA FUNÇÃO LOOP()


void Buzzer() {

  Distancia = ultrasonic.Ranging(CM);

  if ((Distancia <= 40) && (Distancia > 30)) {
    if (millis() - bip > 1000) {
      tone(A0, 1000, 30);
      bip = millis();
    }
  }

  if ((Distancia <= 30) && (Distancia > 25)) {
    if (millis() - bip > 750) {
      tone(A0, 1000, 30);
      bip = millis();
    }
  }

  if ((Distancia <= 20) && (Distancia > 15)) {
    if (millis() - bip > 500) {
      tone(A0, 1000, 30);
      bip = millis();
    }
  }

  if ((Distancia <= 15) && (Distancia > 10)) {
    if (millis() - bip > 300) {
      tone(A0, 1000, 30);
      bip = millis();
    }
  }

  if ((Distancia <= 10) && (Distancia > 5)) {
    if (millis() - bip > 200) {
      tone(A0, 1000, 30);
      bip = millis();
    }
  }

  if (Distancia <= 5) {
    if (millis() - bip > 100) {
      tone(A0, 1000, 30);
      bip = millis();
    }
  }
}


void Frente() {                //Frente
  digitalWrite(motoD1, HIGH);  //Right Motor forword Pin
  digitalWrite(motoD2, LOW);   //Right Motor backword Pin
  digitalWrite(motoE1, HIGH);  //Left Motor backword Pin
  digitalWrite(motoE2, LOW);   //Left Motor forword Pin

  //if (perigo) {
  //Parado();
}

void Re() {                    //Ré
  digitalWrite(motoD1, LOW);   //Right Motor forword Pin
  digitalWrite(motoD2, HIGH);  //Right Motor backword Pin
  digitalWrite(motoE1, LOW);   //Left Motor backword Pin
  digitalWrite(motoE2, HIGH);  //Left Motor forword Pin

  //if (perigo) {
  //Parado();
}

void paraDireita() {           //paraDireita
  digitalWrite(motoD1, HIGH);  //Right Motor forword Pin
  digitalWrite(motoD2, LOW);   //Right Motor backword Pin
  digitalWrite(motoE1, LOW);   //Left Motor backword Pin
  digitalWrite(motoE2, HIGH);  //Left Motor forword Pin
  delay(500);

  //if (perigo) {
  //Parado();
}

void paraEsqueda() {           //paraEsqueda
  digitalWrite(motoD1, LOW);   //Right Motor forword Pin
  digitalWrite(motoD2, HIGH);  //Right Motor backword Pin
  digitalWrite(motoE1, HIGH);  //Left Motor backword Pin
  digitalWrite(motoE2, LOW);   //Left Motor forword Pin
  delay(500);

  //if (perigo) {
  //Parado();
}

void Parado() {               //parado
  digitalWrite(motoD1, LOW);  //Right Motor forword Pin
  digitalWrite(motoD2, LOW);  //Right Motor backword Pin
  digitalWrite(motoE1, LOW);  //Left Motor backword Pin
  digitalWrite(motoE2, LOW);  //Left Motor forword Pin
}


void naoCabeca() {

  Servo01.write(40);
  delay(250);
  Servo01.write(140);
  delay(250);
  Servo01.write(40);
  delay(250);
  //Servo01.write(140);
  //delay(250);
  Servo01.write(90);
  delay(250);
}



//FUNÇÃO PARA CHECAR PERIGO
//void checarPerigo() {
//if ( (millis() - temp > 100){
// Distancia = ultrasonic.Ranging(CM);
//temp = millis();
}

//terraFirme = !digitalRead(pinSensor);   // VERIFICA SE TEM CHÃO À FRENTE

//if ((Distancia <= 10) || !terraFirme) { // PARA O ROBO CASO NÃO TENHA CHÃO OU ENCONTRE UM OBSTÁCULO PERTO
//if (!perigo) {
//perigo = true;  // GUARDA VALOR PARADO PARA LEMBRAR QUE TEM OBSTÁCULO À FRENTE DURANTE A PRÓXIMA EXECUÇÃO DO LOOP
//Parado();
//naoCabeca();
}
}  //else {
//perigo = false;
}
}