/*
  This program has been built by Simone Romano - www.sromano.altervista.org
  This software is part of WPSW(Windows Phone Smartwatch Project)
  and control one of 2 processor of board. This processor manage
  display, touch and serial communication with other processor.
*/
#include <UTFT.h>
#include <UTouch.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>  
/*
 Number 14 to 19 refers to A0 to A5
 Number 0 to 13 refers to digital pin 0 to 13
 */
UTFT        myGLCD(ITDB24D ,19,18,17,16);  //Relative tft pin: RS, WR, CS, RESET
UTouch      myTouch(15,10,14,9,8);  //Relative touch pin: tclk, tcs, din, dout, irq
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
int backlight = 11, BACKLIGHT_VALUE=250, MANUAL=1, AUTO=2, BACKLIGHT_STATUS=AUTO;
String notification_1="--- - ---", notification_2="--- - ---";
String country="V1.1", username="---", city="---", hour="--:--", temperature="--", date="--",weekDay="--";
char callingName[20]="--", callingNum[20]="--";
int HOME_PAGE = 0, SETTINGS_PAGE = 1, APP_PAGE = 2, ACTUAL_PAGE=-1;
int DISPLAY_OFF=0, DISPLAY_ON = 1, DISPLAY_STATUS=-1;
int DELAY_CALL_IN=10000;
int DEBUG_MODE = 0;
SoftwareSerial mySerial = SoftwareSerial(12, 13); // RX, TX
char inData[100];

void setup()
{
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);

  pinMode(backlight, OUTPUT);
  drawHome();  
  analogWrite(backlight, BACKLIGHT_VALUE);
  // define pin modes for tx, rx:
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);
  mySerial.begin(4800);  //4800 works fine; 9600 cause problems (maybe 3.3v is the problem)
  while (mySerial.available() > 0){ }
}

void loop()
{
  if(myTouch.dataAvailable() || mySerial.available()){
    if (myTouch.dataAvailable()==true){
      myTouch.read();
      long x = myTouch.getX();
      long y = 240-myTouch.getY();
      if(DISPLAY_STATUS==DISPLAY_OFF){
        ACTUAL_PAGE = -1;  //to redraw all;
        drawHome(); 
        powerOn();
        DISPLAY_STATUS=DISPLAY_ON;
        delay(1000);
        //continue;  
      }
      else if (clickInBox(x,y,278,199,318,236)){  //home button
        drawHome();
        ACTUAL_PAGE = HOME_PAGE;
        //continue;
      }
      else if (ACTUAL_PAGE == HOME_PAGE){
        if(clickInBox(x,y,216,20,316,100)==true){  //standby
          DISPLAY_STATUS=DISPLAY_OFF;
          standby();
          delay(1000);
          //continue; 
        }
        if (clickInBox(x,y,3,20,103,100)==true){  //settings
          drawSettings();
          ACTUAL_PAGE = SETTINGS_PAGE;
          //continue; 
        }
        if (clickInBox(x,y,109,20,210,100)==true){  //app
          drawApp();
          ACTUAL_PAGE = APP_PAGE;
          //continue; 
        }
        if (clickInBox(x,y,3,103,318,189)==true){  //Hour
          drawHome();  //if new information are in will be displayed
          //continue; 
        }
      }       
      else if (ACTUAL_PAGE == SETTINGS_PAGE){
        if (clickInBox(x,y,158,46,190,73)==true){  //+ backlight
          BACKLIGHT_STATUS = MANUAL;
          updateAutoInSettings(); 
          if (BACKLIGHT_VALUE <250){
            BACKLIGHT_VALUE += 25;
            analogWrite(backlight,BACKLIGHT_VALUE);
            delay(500);
          }
          //continue; 
        }
        if (clickInBox(x,y,201,45,222,64)==true){  //- backlight
          BACKLIGHT_STATUS = MANUAL;
          updateAutoInSettings(); 
          if (BACKLIGHT_VALUE >=50){
            BACKLIGHT_VALUE -= 25;
            analogWrite(backlight,BACKLIGHT_VALUE);
            delay(500);
          }
          //continue; 
        }
        if (clickInBox(x,y,239,46,283,73)==true){  //auto backlight
          BACKLIGHT_STATUS = AUTO;
          updateAutoInSettings(); 
          mySerial.write("auto_backlight");  //will go to windows phone device
          //continue; 
        }
        if (clickInBox(x,y,25,103,143,128)==true){  //about
          //continue; 
        }
      }
      else if (ACTUAL_PAGE == APP_PAGE){
        if (clickInBox(x,y,3,44,104,105)==true){  //light ON
          mySerial.write("on"); 
          delay(200);
        }
       if (clickInBox(x,y,3,113,104,174)==true){  //light OFF
          mySerial.write("off"); 
          delay(200);
        }
      }
    }
    if(mySerial.available()==true){
      debug("Serial in");
      serialSleeping();
      readStringFromSerial();   
      //first letter of inData can be 'P'(periodical info) or 'C'(call in info)
      updateVarFromInData(inData[0]);
      if (inData[0] == 'P'){
        if (BACKLIGHT_STATUS == AUTO)  analogWrite(backlight, BACKLIGHT_VALUE); 
        if (ACTUAL_PAGE == HOME_PAGE)  drawHome();
        else{
         drawHeader();
        }
     }
      if (inData[0] == 'C'){
        drawCallIn();
      }
      if (inData[0] == 'A'){
        drawFooter();
      }
      if (strcmp(inData,"HOME")==0){
        drawHome();
        ACTUAL_PAGE = HOME_PAGE;
      }
      inData[100];
      delay(200);
    }
  }
}

void readStringFromSerial(){ 
  serialSleeping(); 
  debug("reading");
  int index = 0;
  char tmp;
  while (mySerial.available() > 0){ 
    tmp = mySerial.read(); 
    inData[index] = tmp; 
    index++; 
    delay(50);  //necessary! without it it will read more rapidly and it doesn't work!
    //debug
    inData[index] = '\0';
    debug(inData);
    //
  }
  inData[index] = '\0';
  //debug(inData);
  serialSleeping();
  //while (mySerial.available() > 0){}
  mySerial.write("Read: ");
  mySerial.write(inData);
}


void updateVarFromInData(char inDataType){  //P or C data
  if (inDataType=='P'){
    int number = 0;
    for (int i=2; inData[i]!='_'; i++){
      char tmp[30] = "";
      int count=0;
       for (int j=i; inData[j]!=';'||inData[j]!='_'; j++){
         tmp[count] = inData[j];
         count++;
         i++;
         tmp[count] = '\0';
         if (inData[j] == ';') break;
         if (inData[j] == '_') break;
       }
       i--;  //will be increment in the for istruction
       tmp[count-1] = '\0';  //-1 to remove ';' or '_'  
       number++;
       if(number == 1)  hour = tmp;
       if(number == 2)  date = tmp;
       if(number == 3)  weekDay = tmp;
       if(number == 4)  city = tmp;
       if(number == 5)  username = tmp;
       if(number == 6)  BACKLIGHT_VALUE = atoi(tmp);
       if(number == 7)  temperature = tmp;
       if(number == 7)  break;
    }
  }
  if(inDataType=='C'){
    int number = 0;
    for (int i=2; inData[i]!='_'; i++){
      char tmp[20] = "";
      int count=0;
       for (int j=i; inData[j]!=';'||inData[j]!='_'; j++){
         tmp[count] = inData[j];
         count++;
         i++;
         tmp[count] = '\0';
         if (inData[j] == ';') break;
         if (inData[j] == '_') break;
       }
       i--;  //will be increment in the for istruction
       tmp[count-1] = '\0';  //-1 to remove ';' or '_'
       number = number + 1;
       if(number == 1)  strcpy(callingName,tmp);
       if(number == 2)  strcpy(callingNum,tmp);
       if(number == 2)  break;
    }
  }
  if(inDataType=='A'){
    int number = 0;
    for (int i=2; inData[i]!='_'; i++){
      char tmp[20];
      int count=0;
       for (int j=i; inData[j]!=';'||inData[j]!='_'; j++){
         tmp[count] = inData[j];
         count++;
         i++;
         tmp[count] = '\0';
         if (inData[j] == ';') break;
         if (inData[j] == '_') break;
       }
       i--;  //will be increment in the for istruction
       tmp[count-1] = '\0';  //-1 to remove ';' or '_'
       number++;
       if(number == 1)  notification_1 = tmp;
       if(number == 2)  notification_2 = tmp;
       if(number == 2)  break;
    }
  }
}

void standby(){
  analogWrite(backlight,0);
  myGLCD.clrScr();
}

void powerOn(){
  analogWrite(backlight,BACKLIGHT_VALUE);  
}

void drawHome(){
  //xmax = 320
  //ymax = 240
  //Grey Button
  drawHeader();
  if(ACTUAL_PAGE != HOME_PAGE){
    clrScreen();
    //settings rectangle
    myGLCD.setColor(255,127,39);
    myGLCD.fillRect(3,20,103,100);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(255,127,39);
    myGLCD.setFont(SmallFont);
    myGLCD.print("Impostazioni",6,52);
  
    //App rectangle
    myGLCD.setColor(63,72,204);
    myGLCD.fillRect(109,20,210,100);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(63,72,204);
    myGLCD.setFont(BigFont);
    myGLCD.print("APP",135,55);
  
    //Standby rectangle
    myGLCD.setColor(237,28,36);
    myGLCD.fillRect(216,20,316,100);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(237,28,36);
    myGLCD.setFont(SmallFont);
    myGLCD.print("Standby",238,52);
  }
  
  if(ACTUAL_PAGE == HOME_PAGE)
    clrHourScreen();
    
  //hour-temp
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setFont(BigFont);
  myGLCD.print(hour,180,125);
  myGLCD.print(temperature,180,147);
  myGLCD.setFont(SmallFont);
  myGLCD.print(weekDay,50,125);
  myGLCD.print(date,50,147);
  myGLCD.print("www.sromano.altervista.org/sdw/",45,168);

  drawFooter();
  ACTUAL_PAGE = HOME_PAGE;
}

void drawSettings(){  
  clrScreen();
  
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);  
  myGLCD.print("Luminosita'",29,59);
  myGLCD.print("About",29,106);
  myGLCD.print("Project built by Simone Romano",29,130);
  myGLCD.print("www.sromano.altervista.org/sdw/",29,155);
  myGLCD.print("V1.1",29,175);

  myGLCD.setBackColor(VGA_WHITE); 
  //backlight
  myGLCD.fillRect(158,46,190,73);
  myGLCD.fillRect(198,46,230,73);
  myGLCD.fillRect(239,46,283,73);


  //+ and -
  myGLCD.setBackColor(VGA_WHITE); 
  myGLCD.setColor(VGA_BLACK); 
  myGLCD.print("+",171,54);
  myGLCD.print("-",211,56);
  myGLCD.print("auto",246,55);
  
  updateAutoInSettings();
}

/* This method will update settings page by
 * showing if auto backlight mode is enabled (green)
 * or disabled (white)
 */
void updateAutoInSettings(){
  if(BACKLIGHT_STATUS == 2){  //auto enabled
    myGLCD.setColor(VGA_GREEN); 
    //backlight
    myGLCD.fillRect(239,46,283,73);
    myGLCD.setBackColor(VGA_GREEN); 
    myGLCD.setColor(VGA_BLACK); 
    myGLCD.print("auto",246,55);
  }
  else{
    myGLCD.setColor(VGA_WHITE); 
    //backlight
    myGLCD.fillRect(239,46,283,73);
    myGLCD.setBackColor(VGA_WHITE); 
    myGLCD.setColor(VGA_BLACK); 
    myGLCD.print("auto",246,55);
  }
}

void clrScreen(){
  //clear center of screen
  myGLCD.setColor(VGA_BLACK);
  myGLCD.fillRect(3,20,316,186);
  // 
}

void clrHourScreen(){
  //clear hour's on screen
  myGLCD.setColor(VGA_BLACK);
  myGLCD.fillRect(3,102,316,186);
  // 
}

void drawApp(){
  clrScreen();
  myGLCD.setColor(225,127,39);  //orange
  myGLCD.fillRect(3,20,104,38);  //light target

  myGLCD.setColor(63,72,204);  //blue
  myGLCD.fillRect(108,20,316,38);  //command target

  myGLCD.setColor(237,28,36);  //red
  myGLCD.fillRect(3,44,104,105);  //on light rect
  myGLCD.drawCircle(159,63,15);  //tv off circle
  myGLCD.setColor(195,195,195);  //grey color
  myGLCD.fillRect(3,113,104,174);  //off light rect

  myGLCD.drawCircle(222,94,15);  //tv ch+ circle
  myGLCD.drawCircle(222,134,15);  //tv ch- circle
  myGLCD.drawCircle(266,94,15);  //tv vol+ circle
  myGLCD.drawCircle(266,134,15);  //tv vol- circle

  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(225,127,39);
  myGLCD.setFont(SmallFont);
  myGLCD.print("Torcia",26,23);
  myGLCD.setBackColor(237,28,36);
  myGLCD.print("ON",45,69);
  myGLCD.setBackColor(195,195,195);
  myGLCD.print("OFF",38,139);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("Off",146,57);
  myGLCD.setBackColor(VGA_BLACK);  
  myGLCD.print("+",218,89);
  myGLCD.print("-",220,131);
  myGLCD.print("+",263,89);
  myGLCD.print("-",265,131);

  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("Ch",211,62);
  myGLCD.print("Vol",254,62);
  myGLCD.setBackColor(63,72,204);
  myGLCD.print("Telecomando",160,22);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("Samsung tv",164,155);

}

//This method requires in input 2 point of a box(high-left point and low-right point)
//to check if touch press is in box
boolean clickInBox(long x, long y, int high_x, int high_y, int low_x, int low_y){
  if (x>=high_x && x<=low_x && y>=high_y && y<=low_y)
    return true;
  return false;
}  

void drawHeader(){
  //clear area
  myGLCD.setColor(VGA_BLACK);
  myGLCD.fillRect(0,0,318,17);
  //serial control for connection, username and charge status
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setFont(SmallFont);

  myGLCD.print(country,3,3);
  myGLCD.print(username,135,3);
  myGLCD.print(city,250,3);
}

void drawFooter(){
  //clear area
  myGLCD.setColor(VGA_BLACK);
  myGLCD.fillRect(0,197,273,239);
  
  myGLCD.setFont(SmallFont);
  //print notification_s
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print(notification_1,3,200);
  myGLCD.print(notification_2,3,220);

  //home button
  myGLCD.fillRect(278,199,318,236);
  myGLCD.setColor(VGA_BLACK);
  myGLCD.fillRect(288,208,297,217);
  myGLCD.fillRect(300,208,309,217);
  myGLCD.fillRect(288,220,297,229);
  myGLCD.fillRect(300,220,309,229);
  
}

void drawCallIn(){
  clrScreen();
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setFont(BigFont);
  
  myGLCD.print(callingNum,47,70);
  myGLCD.print(callingName,134,97);
  delay(DELAY_CALL_IN);
  ACTUAL_PAGE = -1;  //to redraw all home page
  drawHome();
}

/**
  *This function print debug message
  *on display if DEBUG_MODE var is 1.
  */
void debug(char* toPrint){  
  if(DEBUG_MODE == 1){
    clrScreen();
    myGLCD.setBackColor(VGA_BLACK);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.print(toPrint,47,70);
    delay(400);
  }
}

void serialSleeping(){
   delay(1000); 
}

