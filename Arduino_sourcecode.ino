
#include <DS3231.h>
#include <Keypad.h>
#include <TM1637.h>
#include "Wire.h"  
#include "LiquidCrystal_I2C.h" 

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7); 

int CLK = 22;
int DIO = 23;

TM1637 tm(CLK, DIO);
const byte ROWS = 4;
const byte COLS = 4; 

const int stepPin = 12;
const int dirPin = 13;

char keys[ROWS][COLS] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};



byte rowPins[ROWS] = {6, 8, 7, 9}; 
byte colPins[COLS] = {2, 3, 4, 5}; 

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

DS3231  rtc(A4, A5); 
int t1, t2, t3, t4, t5, t6, portion, IRSensor = 28, lowlvlsensor = 30;
boolean feed1 = false;
boolean feed2 = false; 
boolean feed3 = false;
char key;
int r[6], f[6], v[6];
int start;

void setup()
{
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);

  tm.init();
  tm.set(2);
  Serial.begin(115200); 
  rtc.begin();
//         rtc.setDOW(WEDNESDAY);     // Set Day-of-Week
//         rtc.setTime(21,31,00);     // Set the time (24hr format)
//         rtc.setDate(01,02,2023);   // Set the date 

  lcd.begin(20, 4);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);

  pinMode(IRSensor, INPUT);
  pinMode(lowlvlsensor, INPUT);
  image();
}

void image() 
{
  byte image09[8] = {B00000, B00000, B00000, B00010, B00101, B10101, B01101, B00001};
  byte image08[8] = {B01000, B10100, B00100, B10010, B00001, B01000, B10000, B00000};
  byte image07[8] = {B00100, B01011, B01000, B01010, B10001, B10101, B10011, B10000};
  byte image23[8] = {B10000, B10000, B10000, B10000, B01001, B01010, B00100, B00000};
  byte image24[8] = {B00000, B00000, B00000, B01110, B01010, B01010, B10001, B00000};
  byte image25[8] = {B00001, B00001, B00001, B00010, B11010, B11010, B00100, B00000};
  
  lcd.createChar(0, image09);
  lcd.createChar(1, image08);
  lcd.createChar(2, image07);
  lcd.createChar(3, image23);
  lcd.createChar(4, image24);
  lcd.createChar(5, image25);

  byte heart[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
  lcd.createChar(6, heart);
}

void displayTime(int minutes)
{
  int hour = minutes / 60;
  int mins = minutes % 60;

  tm.point(1);
  tm.display(3, mins % 10);
  tm.display(2, mins / 10 % 10);
  tm.display(1, hour % 10);
  tm.display(0, hour / 10 % 10);
}

void loop()
{
  if (start == 0)
  {
    lcd.setCursor(5, 1);
    Serial.println("Starting");
    lcd.print("Starting");

    lcd.setCursor(3, 2);
    lcd.print("Please Wait...");

    delay(2000);
    lcd.clear();
    start++;
  }

  else
  {

    int IRstatus, lowlvlstatus;
    char settimebutton;
    settimebutton = kpd.getKey();
    IRstatus = digitalRead(IRSensor);
    lowlvlstatus = digitalRead(lowlvlsensor);

    if (lowlvlstatus == 1)
    {
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("Food Supply LOW");
      lcd.setCursor(1, 2);
      lcd.print("Refill the Food...");
      delay(2000);
      lcd.clear();
    } 
    else

      if (settimebutton == 'A')
      {
        if (portion == 0)
        {
          portionnotset();
        }
        else
        {
          setFeedingTime1();
        }
      }
    if (settimebutton == 'B')
    {
      if (portion == 0)
      {
        portionnotset();
      }
      else
      {
        setFeedingTime2();
      }
    }
    if (settimebutton == 'C')
    {
      if (portion == 0)
      {
        portionnotset();
      }
      else
      {
        setFeedingTime3();
      }
    }
    if (settimebutton == 'D')
    {
      if (IRstatus == 0)
      {
        if (portion == 0)
        {
          portionnotset();
        } 
        else
        {
          lcd.clear();
          lcd.setCursor(3, 1);
          lcd.print("Food Tray FULL");
          lcd.setCursor(2, 2);
          lcd.print("CANNOT Dispense");
          delay(3000);
          lcd.clear();
        }
      }
      else
      {
        if (portion == 0)
        {
          portionnotset();
        }
        if (portion == 1)
        {
          portion1();
        }
        if (portion == 2)
        {
          portion2();
        }
        if (portion == 3)
        {
          portion3();
        }
      }
    }


    if (settimebutton == '#')
      setFeedingPortion();


    lcd.setCursor(2, 0);
    lcd.print("Time: ");

    String t = "";
    t = rtc.getTimeStr();
    t1 = t.charAt(0) - 48;
    t2 = t.charAt(1) - 48;
    t3 = t.charAt(3) - 48;
    t4 = t.charAt(4) - 48;
    t5 = t.charAt(6) - 48;
    t6 = t.charAt(7) - 48;

    Time now = rtc.getTime();
    int num = (now.hour * 60) + (now.min);
    displayTime(num);


    lcd.print(rtc.getTimeStr());
    lcd.setCursor(2, 1);
    lcd.print("Date: ");
    lcd.print(rtc.getDateStr());
    
    lcd.setCursor(1, 2);
    lcd.print("'#' to SET Portion");

    if (feed1 == true)
    {
      lcd.setCursor(0, 3);
      lcd.print(r[0]);
      lcd.print(r[1]);
      lcd.print(":");
      lcd.print(r[2]);
      lcd.print(r[3]);
    }

    if (feed2 == true)
    {
      lcd.setCursor(7, 3);
      lcd.print(f[0]);
      lcd.print(f[1]);
      lcd.print(":");
      lcd.print(f[2]);
      lcd.print(f[3]);
    }

    if (feed3 == true)
    {
      lcd.setCursor(14, 3);
      lcd.print(v[0]);
      lcd.print(v[1]);
      lcd.print(":");
      lcd.print(v[2]);
      lcd.print(v[3]);
    }


    if (t1 == r[0] && t2 == r[1] && t3 == r[2] && t4 == r[3] && t5 < 1 && t6 < 3 && feed1 == true) //Feeding time1
    {
      if (IRstatus == 0)
      {
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("Food Tray FULL");
        lcd.setCursor(2, 2);
        lcd.print("CANNOT Dispense");
        delay(3000);
        lcd.clear();
      }
      else
      {
        if (portion == 1)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 1");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6)); 
          
          portion1();
          delay(3000);
          lcd.clear();
        }
        if (portion == 2)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 1");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6)); 
          
          portion2();
          delay(3000);
          lcd.clear();
        }
        if (portion == 3)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 1");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6)); 
          
          portion3();
          delay(3000);
          lcd.clear();
        }
      }
    }
    if (t1 == f[0] && t2 == f[1] && t3 == f[2] && t4 == f[3] && t5 < 1 && t6 < 3 && feed2 == true) //Feeding time2
    {
      if (IRstatus == 0)
      {
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("Food Tray FULL");
        lcd.setCursor(2, 2);
        lcd.print("CANNOT Dispense");
        delay(3000);
        lcd.clear();

      }
      else
      {
        if (portion == 1)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 2");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6)); 
          
          portion1();
          delay(3000);
          lcd.clear();
        }
        if (portion == 2)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 2");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6));
           
          portion2();
          delay(3000);
          lcd.clear();
        }
        if (portion == 3)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 2");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6)); 
          
          portion3();
          delay(3000);
          lcd.clear();
        }
      }
    }
    if (t1 == v[0] && t2 == v[1] && t3 == v[2] && t4 == v[3] && t5 < 1 && t6 < 3 && feed3 == true) //Feeding time 3
    {
      if (IRstatus == 0)
      {
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("Food Tray FULL");
        lcd.setCursor(2, 2);
        lcd.print("CANNOT Dispense");
        delay(3000);
        lcd.clear();
      }
      else
      {
        if (portion == 1)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 3");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6)); 
          
          portion1();
          delay(3000);
          lcd.clear();
        }
        if (portion == 2)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 3");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6)); 
          
          portion2();
          delay(3000);
          lcd.clear();
        }
        if (portion == 3)
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Feeding Time 3");
          lcd.setCursor(3, 1);
          lcd.print("Food is coming!");
          
          lcd.setCursor(10, 2);
          lcd.write(byte(0));
          lcd.setCursor(9, 2);
          lcd.write(byte(1));
          lcd.setCursor(8, 2);
          lcd.write(byte(2));
          lcd.setCursor(8, 3);
          lcd.write(byte(3));
          lcd.setCursor(9, 3);
          lcd.write(byte(4));
          lcd.setCursor(10, 3);
          lcd.write(byte(5)); 
          lcd.setCursor(12, 2);
          lcd.write(byte(6)); 
          
          portion3();
          delay(3000);
          lcd.clear();
        }
      }
    }
  }
}


void setFeedingPortion()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Set Portion Size");
  lcd.setCursor(6, 1);
  lcd.print("From 1-3");
  lcd.setCursor(2, 3);
  lcd.print("Portion Size : ");
  delay(1000);
  while (1)
  {
    key = kpd.getKey();

    if (key == '1')
    {
      portion = 1;
      lcd.print(key);
      delay(1000);
      lcd.clear();
      break;
    }
    if (key == '2')
    {
      portion = 2;
      lcd.print(key);
      delay(1000);
      lcd.clear();
      break;
    }
    if (key == '3')
    {
      portion = 3;
      lcd.print(key);
      delay(1000);
      lcd.clear();
      break;
    }
    if (key != '1' && key != '2' && key != '3' && key != NO_KEY )
    {
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("INVALID Size");
      lcd.setCursor(1, 2);
      lcd.print("Set POSSIBLE Size");
      delay(2000);
      lcd.clear();
      setFeedingPortion();
    }
  }

}

void portionnotset()
{
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Please SET Portion");
  lcd.setCursor(2, 2);
  lcd.print("Press '#' to SET");
  
  delay(2200);
  lcd.clear();
}

void portion1()
{
    digitalWrite(dirPin, LOW); 
    for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
      digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  
}
void portion2()
{

    digitalWrite(dirPin, LOW); 
    for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
    digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }

}
void portion3()
{
    digitalWrite(dirPin, LOW); 
    for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
      digitalWrite(dirPin, HIGH); 
  for (int x = 0; x < 400; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(dirPin, LOW); 
  for (int x = 0; x < 50; x++)
  {
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
  }
  
}
void setFeedingTime1()
{
  feed1 = true;
  int i = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Feeding Time 1");
  
  lcd.setCursor(5, 1);
  lcd.print("Set HH:MM");
  lcd.setCursor(1, 4);
  lcd.print("Press * to CANCEL");
  lcd.setCursor(4, 1);
  char j = 0;

  while (1)
  {
    key = kpd.getKey();


    if (j == 0 && key >= '3' || j == 3 && key >= '6')
    {
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("INVALID Time");
      lcd.setCursor(1, 2);
      lcd.print("Set POSSIBLE Time");
      delay(2000);
      lcd.clear();
      key = 0;
      setFeedingTime1();
      break;
    }

    if (key == '*') 
    { 
      lcd.clear();
      key = 0;
      feed1 = false;
      break;
    }

    if (key != NO_KEY)
    {
      lcd.setCursor(j+9, 2);
      lcd.print(key);
      r[i] = key - 48;

      i++;
      j++;

      if (j == 2)
      {
        lcd.print(":"); 
        j++;
      }

      delay(500);
    }

    if (j == 5)
    { lcd.clear();
      key = 0; 
      break;
    }
  }
}
void setFeedingTime2()
{
  feed2 = true;
  int i = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Feeding Time 2");

  lcd.setCursor(5, 1);
  lcd.print("Set HH:MM");
  lcd.setCursor(1, 4);
  lcd.print("Press * to CANCEL");
  lcd.setCursor(4, 1);
  char j = 0;

  while (1)
  {
    key = kpd.getKey();

    if (j == 0 && key >= '3' || j == 3 && key >= '6')
    {
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("INVALID time");
      lcd.setCursor(1, 2);
      lcd.print("Set POSSIBLE time");
      delay(2000);
      lcd.clear();
      key = 0;
      setFeedingTime2();
      break;
    }
    if (key == '*') { 
      lcd.clear();
      key = 0;
      feed2 = false;
      break;
    }

    if (key != NO_KEY)
    {
      lcd.setCursor(j+9, 2);
      lcd.print(key);
      f[i] = key - 48;

      i++;
      j++;

      if (j == 2)
      {
        lcd.print(":"); j++;
      }

      delay(500);
    }

    if (j == 5)
    { lcd.clear();
      key = 0; 
      break;
    }
  }
}
void setFeedingTime3()
{
  feed3 = true;
  int i = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Feeding Time 3");

  lcd.setCursor(5, 1);
  lcd.print("Set HH:MM");
  lcd.setCursor(1, 4);
  lcd.print("Press * to CANCEL");
  lcd.setCursor(4, 1);
  char j = 0;

  while (1)
  {
    key = kpd.getKey();

    if (j == 0 && key >= '3' || j == 3 && key >= '6')
    {
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("INVALID time");
      lcd.setCursor(1, 2);
      lcd.print("Set POSSIBLE time");
      delay(2000);
      lcd.clear();
      key = 0;
      setFeedingTime3();
      break;
    }

    if (key == '*') 
    {
      lcd.clear();
      key = 0;
      feed3 = false;
      break;
    }

    if (key != NO_KEY)
    {
      lcd.setCursor(j+9, 2);
      lcd.print(key);
      v[i] = key - 48;

      i++;
      j++;

      if (j == 2)
      {
        lcd.print(":"); j++;
      }

      delay(500);
    }

    if (j == 5)
    { lcd.clear();
      key = 0; 
      break;
    }
  }
}
