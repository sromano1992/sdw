
/*
  This program has been built by Simone Romano - www.sromano.altervista.org
  This software is part of WPSW(Windows Phone Smartwatch Project)
  and did use to control bluetooth data flow.
*/
#include <SoftwareSerial.h>

SoftwareSerial mySerial(12,13); // RX, TX of bluetooth 
String inString;
int FLASH = 4;
unsigned long start, finished, elapsed;

void setup(){  
  pinMode(FLASH, OUTPUT);
  digitalWrite(FLASH, LOW);
    
  delay(2000);
  mySerial.begin(38400);
  Serial.begin(9600);
  while (mySerial.available() > 0){ }
  Serial.println("Starting test...");
}

void loop(){
  if (mySerial.available()>0){  
       Serial.println("Data in...");
       readStringFromSerial();
       Serial.println(inString);
       inString = "";
     }

}

void testFlash(){ 
    if (inString.equals("on")){      
      digitalWrite(FLASH, HIGH);
    }
    else if(inString.equals("off")){
      digitalWrite(FLASH, LOW);      
    }   
    inString = "";
    delay(200);
}

void testCall(){
  mySerial.write("C;luigia;3*631**6*5_");
}

void testFooter(){
  mySerial.write("A;1 mail: ScontoDigitale;Whatsapp: amo_");
}

void testPeriodicalInfo(){
  mySerial.write("P;18:50;28/11/2014;Venerdi';Siano;Simone;100;21 C_");
}

void readStringFromSerial(){ 
  Serial.println("Reading...");
  int index = 0;
  char tmp;   
  while (mySerial.available() > 0){    
    tmp = mySerial.read(); 
    inString += tmp;
  }
}

