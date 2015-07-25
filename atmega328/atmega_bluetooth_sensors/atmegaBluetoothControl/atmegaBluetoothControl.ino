
/*
  This program has been built by Simone Romano - www.sromano.altervista.org
  This software is part of WPSW(Windows Phone Smartwatch Project)
  and control one of 2 processor of board. This processor manage
  bluetooth communication with Nokia Lumia Smartphone and serial
  communication with other processor.
*/
#include <SoftwareSerial.h>

SoftwareSerial mySerial(12,13); // RX, TX
String inString = "";
int FLASH = 4;
unsigned long start, finished, elapsed;

void setup(){  
  pinMode(FLASH, OUTPUT);
  digitalWrite(FLASH, LOW);
    
  delay(2000);
  mySerial.begin(4800);  
  Serial.begin(9600);
  while (mySerial.available() > 0){ }
}

void loop(){
  if (Serial.available()>0 || mySerial.available()>0){
     if (Serial.available()>0){//bluetooth request from device
       readStringFromDefaultSerial();
       if(!testFlash())
         mySerial.print(inString);
       Serial.print("From Device: " + inString);
       inString = "";
     }
     else if (mySerial.available()>0){//req from smartwatch     
       readStringFromSerial();
       testFlash();
       Serial.print("From SmartWatch: " + inString);  //when I read from smartwatch I will write message on bluetooth(now for debug)
       inString = "";
     }
  } 
}

boolean testFlash(){ 
    boolean flashCommand = false;
    if (inString.equals("on")){      
      digitalWrite(FLASH, HIGH);
      flashCommand = true;
    }
    else if(inString.equals("off")){
      digitalWrite(FLASH, LOW);
     flashCommand = true;      
    }   
    delay(200);
    return flashCommand;
}

void testCall(){
  mySerial.write("C;luigia;3*631**6*5_");
}

void testFooter(){
  delay(5000);
  mySerial.print("A;1 mail: ScontoDigitale;Whatsapp: amo_");  
  Serial.print("string sended...");
  //mySerial.write("A;1 mail: ScontoDigitale;Whatsapp: amo_");
}

void testPeriodicalInfo(){
  mySerial.write("P;18:50;28/11/2014;Venerdi';Siano;Simone;100;21 C_");
}

void readStringFromSerial(){ 
  int index = 0;
  char tmp;   
  while (mySerial.available() > 0){    
    tmp = mySerial.read(); 
    inString += tmp;
    delay(50);
  }
}

void readStringFromDefaultSerial(){ 
  int index = 0;
  char tmp;   
  while (Serial.available() > 0){    
    tmp = Serial.read(); 
    inString += tmp;
    delay(50);
  }
}

