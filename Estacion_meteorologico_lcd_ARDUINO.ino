
// DFPLAYER MINI MP3 - PARLANTE
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"
SoftwareSerial mySoftwareSerial(10, 11);  //Rx, Tx
DFRobotDFPlayerMini myDFPlayer;

int tempAnterior = 0;
int tempAudio = 0;
int tiempo[6] = {0, 1, 05, 15, 30, 60};
int posicion = 3;
unsigned long tiempoAnterior;

//PANTALLA display
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// DHT11 - TEMPERATURA Y HUMEDAD
#include "DHT.h"
#define sensor_dht 2
#define Tipo DHT11
DHT dht(sensor_dht, Tipo);

float temperatura = 0.0;
int humedad = 0;

// MQ135 - CALIDAD DE AIRE
#define digital_mq135 3
int analogo_mq135 = 0;

#define alerta 6
#define buzzer 5
#define boton 9

// CARACTERES ESPECIAL - DIBUJO VOLUMEN
byte vol1 [8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00001};
byte vol2 [8] = {B00000, B00000, B00000, B00000, B00001, B00001, B00001, B00001};
byte vol3 [8] = {B00000, B00000, B00001, B00001, B00001, B00001, B00001, B00001};
byte vol4 [8] = {B00001, B00001, B00001, B00001, B00001, B00001, B00001, B00001};

// POTENCIOMETRO - VOLUMEN
int volumen = 0;

// ============================================================

void setup() {
  Serial.begin(9600);

  dht.begin();
  pinMode(digital_mq135, INPUT);

  pinMode(buzzer, OUTPUT);
  pinMode(alerta, OUTPUT);
  pinMode(boton, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, vol1);
  lcd.createChar(1, vol2);
  lcd.createChar(2, vol3);
  lcd.createChar(3, vol4);

  mySoftwareSerial.begin(9600);
  myDFPlayer.begin(mySoftwareSerial);
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Incapaz de empezar"));
    Serial.println(F("1. ¡Vuelva a comprobar la conexión!"));
    Serial.println(F("2. ¡Inserte la tarjeta SD!"));
  }
  myDFPlayer.volume(map(analogRead(A1), 0, 1023, 0, 30));   //De 0 a 30
  temperatura = dht.readTemperature();
  tempAudio = temperatura;
  tempAnterior = temperatura;
  myDFPlayer.playFolder(2, tempAudio);
}

// ============================================================

void loop() {
  unsigned long tiempoActual = millis();

  if (myDFPlayer.available()) {
    if (myDFPlayer.readType() > 6) {
      digitalWrite(alerta, HIGH);
      delay(1000);
      digitalWrite(alerta, LOW);
    }
  }
  // Serial.println(myDFPlayer.available());
  // Serial.println(myDFPlayer.readType());  
  
  // PULSADOR
  if (!digitalRead(boton)) {
    posicion = (posicion + 1) % 6;
    while (!digitalRead(boton));
  }

  lcd.clear();

  // DHT11 - TEMPERATURA Y HUMEDAD
  humedad = dht.readHumidity();
  temperatura = dht.readTemperature();
  if (isnan(humedad) || isnan(temperatura)) {

    lcd.setCursor (0, 0);
    lcd.print("Error DHT11...");

    digitalWrite(alerta, HIGH);
    delay(1000);
    digitalWrite(alerta, LOW);

  } else {
    tempAudio = temperatura;

    lcd.setCursor (0, 0);
    lcd.print("T=");
    lcd.print(temperatura);
    lcd.print("C");

    lcd.setCursor (10, 0);
    lcd.print("H=");
    lcd.print(humedad);
    lcd.print("%");
  }

  // CALIDAD DEL AIRE MQ135- ANALOGO
  analogo_mq135 = analogRead(A0); //Lemos la salida analógica del MQ

  //lcd.setCursor (10, 1);
  //lcd.print(analogo_mq135);

  // CALIDAD DEL AIRE MQ135- DIGITAL
  boolean mq_estado = digitalRead(digital_mq135); //Leemos el sensor

  lcd.setCursor (0, 1);

  if (mq_estado) //si la salida del sensor es 1
  {
    lcd.print("EXCELENTE");
  }
  else //si la salida del sensor es 0
  {
    lcd.print("PELIGRO");

    // BUZZER
    for (int i = 0; i < 4; i++) {
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
      delay(300);
    }
  }

  // POTENCIOMETRO - VOLUMEN
  volumen = map(analogRead(A1), 0, 1023, 0, 30);
  myDFPlayer.volume(volumen);
  lcd.setCursor (15, 1);

  if (volumen < 2) {
    //nada
  } else if (volumen < 7) {
    lcd.write(byte(0));
  } else if (volumen < 14) {
    lcd.write(byte(1));
  } else if (volumen < 21) {
    lcd.write(byte(2));
  } else {
    lcd.write(byte(3));
  }

  // DFPLAYER MINI MP3 - PARLANTE (cada cierto tiempo)
  if (posicion == 0) {
    lcd.setCursor (11, 1);
    lcd.print("off");
  }
  else if (posicion == 1) { // MODO AUTOMATICO - DFPLAYER MINI MP3 - PARLANTE (cada vez que cambia un grado de diferencia)
    if (tempAudio != tempAnterior)
    {
      myDFPlayer.play(tempAudio);
      tempAnterior = temperatura;
    }
    lcd.setCursor (11, 1);
    lcd.print("aut");
  }
  else {
    lcd.setCursor (11, 1);
    lcd.print(tiempo[posicion]);
    lcd.setCursor (13, 1);
    lcd.print("m");

    double minutos = (tiempoActual - tiempoAnterior) / 60000;

    if (minutos > tiempo[posicion]) {
      myDFPlayer.playFolder(2, tempAudio);
      tiempoAnterior = millis();
    }
  }
  Serial.println(posicion);


  lcd.display();
  delay(2000); // 2 seg.
}
