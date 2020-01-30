//LIBRERIAS UTILIZADAS
#include "U8g2lib.h"
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <SHT15.h>
#include <Arduino.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//Sensor SHT15
SHT15 sensor;

//Sensor de oxígeno
const float VRefer = 5; //Voltaje a emplear
const int pinAdc   = A15; //Puerto a utilizar
int oxigenoAdecuado = 40;
int humedadAdecuadda = 70;

//Relay Pin Variable
int relayPin4 = 28;
int relayPin3 = 27;
int relayPin2 = 26;
int relayPin1 = 25;

//Botones control temperatura (2 botones)
const int  SetTempUp = 29; //pin digital
const int  SetTempDown = 30; //pin digital
int controlTemp          = 35;   //temp deseada
bool estadoBotonUp        = 0;
bool estadoBotonDown     = 0;
bool ultimoEstadoBotonUp    = 0;
bool ultimoEstadoBotonDown  = 0;

//Bontones control humedad (2 botones)
const int pinBotonUp = 31;
const int pinBotonDown = 32;
int controlHum = 70;
bool estadoBotonUpHum = 0;
bool estadoBotonDownHum = 0;
bool ultimoEstadoBotonUpHum = 0;
bool ultimoEstadoBotonDownHum = 0;



//Sensor pH
const int analogInPin = A14;
int sensorValue = 0;
unsigned long int avgValue;
float b;
int buf[10], temp;

int phValue = 0;

float readO2Vout()
{
  long sum = 0;
  for (int i = 0; i < 32; i++)
  {
    sum += analogRead(pinAdc);
  }

  sum >>= 5;

  float MeasuredVout = sum * (VRefer / 1023.0);
  return MeasuredVout;
}

int readConcentration()
{

  float MeasuredVout = readO2Vout();

  //when its output voltage is 2.0V,
  float Concentration = MeasuredVout * 0.21 / 2.0;
  float Concentration_Percentage = Concentration * 100;
  return Concentration_Percentage;
}



void setup(void)
{
  mlx.begin();
  Serial.begin(9600);
  u8g2.begin();
  sensor.init(40, 41);
  //Pines relevadores
  pinMode(relayPin4, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin1, OUTPUT);

  //pines botones control temperatura
  pinMode(SetTempUp, INPUT_PULLUP);
  pinMode(SetTempDown, INPUT_PULLUP);

  //pines botones control humedad
  pinMode(pinBotonUp, INPUT_PULLUP);
  pinMode(pinBotonDown, INPUT_PULLUP);
}

void loop()
{



  //CONTROL DE TEMPERATURA//

  // lectura del boton de incremento de temperatura
  estadoBotonUp = (digitalRead(SetTempUp) == 0);     // lectura del boton de incremento de temperatura
  if (estadoBotonUp != ultimoEstadoBotonUp) { // comparar TempButtonState con su estado previo
    if (estadoBotonUp == 1)    {              // si el ultimo estado cambio, incrementar el contador
      controlTemp ++;

    }
  }
  // guardar el estado actual como ultimo estado,
  ultimoEstadoBotonUp = estadoBotonUp;

  // lectura del boton de decremento de temperatura

  estadoBotonDown = (digitalRead(SetTempDown) == 0);
  if (estadoBotonDown != ultimoEstadoBotonDown) {    // compara el estado del boton (incremento o decremento) con el ultimo estado
    if (estadoBotonDown == 1) { // si el ultimo estado cambio, decrementar el contador
      controlTemp--;

    }
  }
  // guardar el estado actual como ultimo estado,
  ultimoEstadoBotonDown = estadoBotonDown;

  //CONTROL DE HUMEDAD//

  // lectura del boton de incremento de humedad
  estadoBotonUpHum = (digitalRead(pinBotonUp) == 0);     // lectura del boton de incremento de temperatura
  if (estadoBotonUpHum != ultimoEstadoBotonUpHum) { // comparar TempButtonState con su estado previo
    if (estadoBotonUpHum == 1)    {              // si el ultimo estado cambio, incrementar el contador
      controlHum ++;

    }
  }
  // guardar el estado actual como ultimo estado,
  ultimoEstadoBotonUpHum = estadoBotonUpHum;

  // lectura del boton de decremento de humedad

  estadoBotonDownHum = (digitalRead(pinBotonDown) == 0);
  if (estadoBotonDownHum != ultimoEstadoBotonDownHum) {    // compara el estado del boton (incremento o decremento) con el ultimo estado
    if (estadoBotonDownHum == 1) { // si el ultimo estado cambio, decrementar el contador
      controlHum--;

    }
  }
  // guardar el estado actual como ultimo estado,
  ultimoEstadoBotonDownHum = estadoBotonDownHum;




  //Condiciones para encendido de actuador en base al sensor de temperatura
  float temperature = mlx.readAmbientTempC();
  if (temperature >= controlTemp) {
    digitalWrite(relayPin1, LOW);
  } else  {
    digitalWrite(relayPin1, HIGH);
  }
  float humedad = sensor.measure(HUMI);
  if (humedad > controlHum) {
    digitalWrite(relayPin2, LOW);
  } else  {
    digitalWrite(relayPin2, HIGH);
  }

  if (humedad < controlHum) {
    digitalWrite(relayPin4, LOW);
  } else  {
    digitalWrite(relayPin4, HIGH);
  }



  //Condiciones para encendido de actuador de oxñigeno (ventilador)
  float oxigeno = readConcentration();

  if (oxigeno >= oxigenoAdecuado) {
    digitalWrite(relayPin3, LOW);
  } else  {
    digitalWrite(relayPin3, HIGH);
  }

  //Codigo sensor pH
  for (int i = 0; i < 10; i++)
  {
    buf[i] = analogRead(analogInPin);

  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)
    avgValue += buf[i];
  float pHVol = (float)avgValue * 5.0 / 1024 / 6;
  int phValue = -5.70 * pHVol + 21.34;

  //CODIGO LOOP DEL DISPLAY OLED
  Wire.end();
  u8g2.clearBuffer();

  //CODIGO DISPLAY QUE FUNCIONA EN BASE AL LOOP

  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.drawStr(35, 10, " C");
  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.setCursor(0, 10);
  u8g2.print(mlx.readAmbientTempC());
  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.drawStr(35, 25, " C");
  u8g2.setCursor(20, 25);
  u8g2.print(controlTemp);
  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.setCursor(0, 25);



  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.drawStr(31, 61, "pH");
  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.setCursor(10, 64);
  u8g2.print(phValue);



  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.setCursor(70, 60);
  u8g2.print(readConcentration());
  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.drawStr(90, 60, "% Oxi");



  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.setCursor(65, 10);
  u8g2.print(sensor.measure(HUMI));
  u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.drawStr(105, 18, "HR%");
   u8g2.setFont(u8g2_font_pxplusibmvga8_tf);
  u8g2.setCursor(80, 25);
  u8g2.print(controlHum);
  




  u8g2.sendBuffer();
  Wire.begin();

  delay(10);
}
