#include <UTFT.h>
#include <UTouch.h>

extern uint8_t SmallFont[];
/*
  Number 14 to 19 refers to A0 to A5
  Number 0 to 13 refers to digital pin 0 to 13
*/
UTFT        myGLCD(ITDB24D ,19,18,17,16);  //Relative tft pin: RS, WR, CS, RESET
UTouch      myTouch(15,10,14,9,8);  //Relative touch pin: tclk, tcs, din, dout, irq

void setup()
{
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  drawButtons();
  myGLCD.setColor(VGA_WHITE);
  pinMode(11, OUTPUT);
  analogWrite(11, BACKLIGHT_VALUE);
}

void loop()
{
  long x, y;
  
  while (myTouch.dataAvailable() == true)
  {
    myTouch.read();
    x = myTouch.getX();
    y = 240-myTouch.getY();
    if ((x!=-1)&&(y!=-1))
    {
      if(x<260)
      {
        myGLCD.fillCircle (x,y,5);
      }
      if(x>=270&&x!=319)
      {
        checkColor(y);
      }
    }
  }
}

void checkColor(long y)
{
   if(y<=50)
   {
     myGLCD.setColor(VGA_RED);
   }
   if(y<=100&&y>50)
   {
     myGLCD.setColor(VGA_YELLOW);
   }
   if(y<=150&&y>100)
   {
     myGLCD.setColor(VGA_GREEN);
   }
   if(y<=200&&y>150)
   {
     myGLCD.setColor(VGA_WHITE);
   }
   if(y<=250&&y>200)
   {
     myGLCD.clrScr();
     drawButtons();
     myGLCD.setColor(VGA_WHITE);
   }
}

void drawButtons()
{
  int x = 270;
  int y = 0;
  //Red Button
  myGLCD.setColor(VGA_RED);
  myGLCD.fillRect(x,y,x+49,y+50);
  //Yellow Button
  y += 50;
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.fillRect(x,y,x+49,y+50);
  //Green Button
  y += 50;
  myGLCD.setColor(VGA_GREEN);
  myGLCD.fillRect(x,y,x+49,y+50);
  //White Button
  y += 50;
  myGLCD.setColor(VGA_WHITE);
  myGLCD.fillRect(x,y,x+49,y+50);
  //Clear Button
  y += 50;
  myGLCD.print("Clear ",RIGHT,215);
}
