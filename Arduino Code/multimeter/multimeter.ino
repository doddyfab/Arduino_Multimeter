/*
  An open-source multimeter
  Source :   https://www.sla99.fr
  Inspired by : https://jpralves.net/post/2015/08/18/ohmmeter-with-auto-ranging.html
  Inspired by : http://www.electroschematics.com/9351/arduino-digital-voltmeter/
  Date : 2018-03-05
*/
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED


//The second sketch tests using a set of resistors. If you do not have the values of the resistance of the scheme you can set the value of the resistors in the code, always placing them in ascending order.
//Read more at: https://jpralves.net/post/2015/08/18/ohmmeter-with-auto-ranging.html

//gestion du bouton poussoir
const int button = A2;
const int pinRelais = 3;
const int pinRelais2 = 4;

//ohmmetre
const byte resistorPin = A1;  
const byte resistorPins[] = {12,11,10,9,8,7,6}; 
# define NUMBERPINS sizeof(resistorPins) 
const int resistorValues[NUMBERPINS] = {220, 560, 1000, 2200, 4700, 10000, 22000}; 
// Valor das resistências 
int resistorReads[NUMBERPINS] = {}; 
double vx; 
float rx; 
double i;
boolean novalue; 
//continuité
int buttonNb = 0; //en fct des menus
const int pinContinuity=2;
//voltmetre
const int pinVoltmeter = 6;
float vout = 0.0;
float vin = 0.0;
float R1 = 100000.0; // resistance of R1 (100K) -see text!
float R2 = 10000.0; // resistance of R2 (10K) - see text!
int value = 0;


void setup(void) { 

  Serial.begin(9600);  
  u8g2.begin();
  pinMode(button, INPUT);
  digitalWrite(button, HIGH);
  pinMode(pinVoltmeter, INPUT);
  pinMode(pinContinuity,OUTPUT);
  digitalWrite(pinContinuity,LOW);
  pinMode(resistorPin, INPUT); 
  pinMode(pinRelais,OUTPUT);
  digitalWrite(pinRelais,HIGH);
  pinMode(pinRelais2,OUTPUT);
  digitalWrite(pinRelais2,HIGH);
  
} 

void loop(void){ 
    int val = digitalRead(button);    
    if (val == LOW) {
      buttonNb += 1;
      delay(100);
    }
    if (buttonNb > 3) {
      buttonNb = 0;
    }
    if(buttonNb == 0){
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_logisoso32_tr);
      u8g2.drawStr(0,32,"M");
      u8g2.sendBuffer(); 
      delay(100);
      u8g2.drawStr(20,32,"U"); 
      u8g2.sendBuffer();  
      delay(100);
      u8g2.drawStr(40,32,"L");
      u8g2.sendBuffer(); 
      delay(100);
      u8g2.drawStr(55,32,"T");
      u8g2.sendBuffer(); 
      delay(100);
      u8g2.drawStr(75,32,"I");
      u8g2.sendBuffer(); 
      delay(100);
      u8g2.drawStr(85,32,"M");
      u8g2.sendBuffer(); 
      delay(100);
      u8g2.clearBuffer();
      digitalWrite(pinContinuity,LOW);      
    }
    if(buttonNb == 1){
      ohmmeter();
    }
    if(buttonNb == 2){
      voltmeter();
    }
    if(buttonNb == 3){
      continuity();
    }
}
  
//fonctions
int readvalues(byte mask) { 
  for(byte p = 0; p < NUMBERPINS; p++) { 
    pinMode(resistorPins[p], INPUT); 
  } 
  for(byte p = 0; p < NUMBERPINS; p++) { 
    if ((mask & (1 << p)) != 0) { 
      pinMode(resistorPins[p], OUTPUT); 
      digitalWrite(resistorPins[p], HIGH); 
    } 
  }
  return analogRead(resistorPin); 
}

void ohmmeter(void){
  digitalWrite(pinContinuity,LOW);
  digitalWrite(pinRelais,HIGH);  
  digitalWrite(pinRelais2,LOW);
  u8g2.clearBuffer(); 
  u8g2.setFont(u8g2_font_logisoso32_tr);
  for(byte p = 0; p < NUMBERPINS; p++) { 
    resistorReads[p] = readvalues(1 << p); 
  }
  novalue = true; 
  for(byte p = NUMBERPINS; p > 0; p--) { 
    if (resistorReads[p-1] >= 450) { 
      vx = (resistorReads[p-1]) * (5.0 / 1024.0); 
      i = (5.0/vx) - 1; 
      rx = (resistorValues[p-1] / i); 
      novalue = false; 
      break;
    }
  }
  if (novalue) { 
    vx = (resistorReads[0]) * (5.0 / 1024.0); 
    i = (5.0/vx) - 1; rx = (resistorValues[0] / i); 
  } 
  if(vx > 4.8) { 
    Serial.println("----INFINITY----"); 
    u8g2.drawStr(0,32,"INFINI");
    u8g2.sendBuffer();
  }
  else { 
    if(rx < 1000) { 
      Serial.println(rx); 
      char tempBuffer1[6];
      dtostrf(rx, 4, 1, tempBuffer1);
      u8g2.drawStr(0,32,tempBuffer1);
      u8g2.sendBuffer();
    } 
    else { 
      rx = rx / 1000; 
      char tempBuffer2[6];
      char Sortie[7];
      dtostrf(rx, 4, 1, tempBuffer2);
      snprintf(Sortie,20, "%sK", tempBuffer2);
      u8g2.drawStr(0,32,Sortie);
      u8g2.sendBuffer();
    } 
  }
  delay(500);

}

void voltmeter(){
  digitalWrite(pinContinuity,LOW);
  for(byte p = 0; p < NUMBERPINS; p++) { 
    pinMode(resistorPins[p], INPUT); 
  } 

  //on active le relais pour le voltmetre
  digitalWrite(pinRelais,LOW);   
  digitalWrite(pinRelais2,HIGH); 
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso32_tr); 
  value = analogRead(pinVoltmeter);
  vout = (value * 5.0) / 1024.0;
  vin = vout / (R2/(R1+R2)); 
  if (vin<0.09) {
    vin=0.0;
  } 
  char tempBuffer2[6];
  char Sortie[7];
  dtostrf(vin, 4, 2, tempBuffer2);
  snprintf(Sortie,20, "%sV", tempBuffer2);
  u8g2.drawStr(0,32,Sortie);
  u8g2.sendBuffer();
  delay(500);
 }


void continuity() {
  digitalWrite(pinContinuity,HIGH); 
  for(byte p = 0; p < NUMBERPINS; p++) { 
    pinMode(resistorPins[p], INPUT); 
  } 
  digitalWrite(pinRelais,HIGH);   
  digitalWrite(pinRelais2,HIGH);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso32_tr);
  u8g2.drawStr(0,32,"CONTI");
  u8g2.sendBuffer();   
}

