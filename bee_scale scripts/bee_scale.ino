
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <String.h>
#include "HX711_LoadCell.h"

#define PIN_TX    7
#define PIN_RX    8
#define BAUDRATE  9600
#define PHONE_NUMBER "xxxxxxxxxx"

#define MESSAGE_LENGTH 160

GPRS gprsTest(PIN_TX,PIN_RX,BAUDRATE);                      //RX,TX,BaudRate
HX711 cell(3, 2);

char _time[50];                                             // array used for network time

//arrays for date and time + 1
char timeStamp [20];
char dt[11];
char tm[6];

//array for scale measurment
char currentMeasurment [10];

//complete message for each day can contain 160 characters
char completeMessage [160];

//SMS message on demand can also contains 160 characters
char messageForSender [160];

char charMessage[166];
bool flag1=false, flag2=false, flag3=false, flag4=false;    // flags for time measurements

char senderMessage[MESSAGE_LENGTH];                         //message received from GPRS shield
int messageIndex = 0;                                       //variable used for unread messages from SMS

char senderPhone[16];                                       //senders phone number
char datetime[24];                                          //time when message from sender is received


void setup() {
  Serial.begin(9600);
  while(!gprsTest.init()) {
      delay(1000);
      Serial.print("init error\r\n");
  }  
  Serial.println("gprs init success");
  delay (2000);

}

void loop() {
  getTimeStamp();

  timeSplitter();
     
  if(tm[0] == '0' && tm[1] == '6' && tm[3] == '0' && tm[4] == '0')
    {
      if(flag1 == false)
      {
        strcpy( completeMessage, dt);  
        strcat( completeMessage, "\n");

        strcat( completeMessage, tm);  
        strcat( completeMessage, ":");
        strcat( completeMessage, " ");
        
        calibrate();
        
        strcat( completeMessage, currentMeasurment);
        strcat( completeMessage, "k");
        strcat( completeMessage, "g");
        strcat( completeMessage, "\n");
      }
      flag1 = true;
      flag4 = false;

    }

  else if(tm[0] == '1' && tm[1] == '2' && tm[3] == '0' && tm[4] == '0')
    {
      if(flag2 == false)
      {
        strcat( completeMessage, tm);  
        strcat( completeMessage, ":");
        strcat( completeMessage, " ");
        calibrate();
        strcat( completeMessage, currentMeasurment);
        strcat( completeMessage, "k");
        strcat( completeMessage, "g");
        strcat( completeMessage, "\n");
      }
     flag2 = true;
    }
  else if(tm[0] == '1' && tm[1] == '6' && tm[3] == '0' && tm[4] == '0')
    {
      if(flag3 == false)
      {
        strcat( completeMessage, tm);  
        strcat( completeMessage, ":");
        strcat( completeMessage, " ");
        
        calibrate();
        
        strcat( completeMessage, currentMeasurment);
        strcat( completeMessage, "k");
        strcat( completeMessage, "g");
        strcat( completeMessage, "\n");
      }
     flag3 = true;
    }
  else if(tm[0] == '2' && tm[1] == '0' && tm[3] == '0' && tm[4] == '0')
    {
      if(flag4 == false)
      {
        strcat( completeMessage, tm);  
        strcat( completeMessage, ":");
        strcat( completeMessage, " ");
        
        calibrate();
        
        strcat( completeMessage, currentMeasurment);
        strcat( completeMessage, "k");
        strcat( completeMessage, "g");
        strcat( completeMessage, "\n");

        Serial.println(completeMessage);
        gprsTest.sendSMS(PHONE_NUMBER, completeMessage);                 //define phone number and text
        memset(completeMessage, 0, sizeof completeMessage);
        Serial.println(completeMessage);
      }
     flag4 = true;
     flag1 = false;
     flag2 = false;
     flag3 = false;
    }
  delay(5000);

  checkUnreadSMS();
}

const char* getTimeStamp()
{
  gprsTest.getLocalDateTime(_time);
}

void calibrate()
  {
      float  val = (cell.read() - 8491946)/ 38715.0f * 2;
      //conversion from float values to char
      dtostrf(val, 6, 2, currentMeasurment);
  }

//Division arrays into a day, month and a year
void  timeSplitter()                                          
{

  dt[0] = _time[6];  //day
  dt[1] = _time[7];  //day
  dt[2] = '/';
  dt[3] = _time[3];  //month
  dt[4] = _time[4];  //month
  dt[5] = '/';
  dt[6] = '2';       //year
  dt[7] = '0';       //year
  dt[6] = _time[0];  //year
  dt[7] = _time[1];  //year

  tm[0] = _time[9];  //hour
  tm[1] = _time[10]; //hour
  tm[2] = ':';
  tm[3] = _time[12]; //minute
  tm[4] = _time[13]; //minute 

  strcpy( timeStamp,dt);  
  strcat( timeStamp, " ");
  strcat( timeStamp,tm);

  //memcpy(timeStamp, tS, sizeof(tS));
}

void checkUnreadSMS()
{
     messageIndex = gprsTest.isSMSunread();

     //At least, there is one UNREAD SMS
     if (messageIndex > 0) { 
      gprsTest.readSMS(messageIndex, senderMessage, MESSAGE_LENGTH, senderPhone, datetime);           
      
      //In order not to full SIM Memory, is better to delete it
      gprsTest.deleteSMS(messageIndex);
      
      if(String(senderMessage) == "kg?")
      {
        gprsTest.getLocalDateTime(_time);

        timeSplitter();

        strcpy( messageForSender, dt);  
        strcat( messageForSender, "\n");

        strcat( messageForSender, tm);  
        strcat( messageForSender, ":");
        strcat( messageForSender, " ");
        
        calibrate();
        
        strcat( messageForSender, currentMeasurment);
        strcat( messageForSender, "k");
        strcat( messageForSender, "g");
        strcat( messageForSender, "\n");

        gprsTest.sendSMS(senderPhone, messageForSender);    //send message with scale values to anyone who requested it

        memset(messageForSender, 0, sizeof messageForSender);
        Serial.println(messageForSender);

      }
      else
      {
	//nothing	
      }
   }
}


