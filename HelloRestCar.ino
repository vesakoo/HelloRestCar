/**
 * First version of using http rest api requests to control
 * two Motors with L293D encoder
 * 4 pwm pins in use, each connected to L293D in1-in4
 * en1 and en2 are enabled with circuit jumper.
 *  Lib: https://github.com/Seeed-Studio/WiFi_Shield.git
 *  HW: 
 *      Elecrow ACS17101S - Wifi Shield based on RN171 module.
 *       
 *      4tronix Initio 4WD ROBOALUSTA	
*/
 
#define DEBUG true
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFly.h>
#include "HTTPClient.h"
#include <string.h>
#include "SSID.h"



#define ACTION_TTL (unsigned long) 15000
// WIFLY_AUTH_OPEN / WIFLY_AUTH_WPA1 / WIFLY_AUTH_WPA1_2 / WIFLY_AUTH_WPA2_PSK
#define AUTH      WIFLY_AUTH_WPA2_PSK


// Pins' connection
// Arduino       WiFly
//  2    <---->    TX
//  3    <---->    RX
SoftwareSerial uart(5, 6);
WiFly wifly(uart);
HTTPClient http;
char get;
char twenty [20];
String action ="";
int actionNum =0;
unsigned long actionStarted = 0;
bool seqEndReported = false;
bool manualMode = false;

void setupWifi(){
  /*while(!Serial.available()){
    delay(1);
  }*/
  Serial.println("------- WIFLY HTTP --------");
  uart.begin(9600);         // WiFly UART Baud Rate: 9600
  // Wait WiFly to init
  //  delay(3000);
  // check if WiFly is associated with AP(SSID)
  if (!wifly.isAssociated(SSID)) {
    while (!wifly.join(SSID, KEY, AUTH)) {
      Serial.println("Failed to join " SSID);
      Serial.println("Wait 0.1 second and try again...");
      delay(100);
    }
    wifly.save();    // save configuration, 
  }

}

void postSeqEnd(){
    //const String url = "http://192.168.32.87:3002/seq/1/end";
    const String url = "http://robo.sukelluspaikka.fi/seq/1/end";
    int str_len = url.length() + 1;
    char char_array[str_len];
    url.toCharArray(char_array, str_len);
    http.post(char_array,"{\"seq\":\"end\"}",10000);
    

 }

int  getRestAction(int num, uint8_t *twenty){
  log("getRestAction(int" +(String) num +")");
   //seqEndReported = false;
  char headerStop [4];
  
  int counter =0;
  //String url = "http://192.168.32.87:3002/caction/" +(String)num;
  String url = "http://robo.sukelluspaikka.fi/caction/" +(String)num;
  int str_len = url.length() + 1;
  char char_array[str_len];
  url.toCharArray(char_array, str_len);
  Serial.println("\r\nTry to get url - " + url);
  //Serial.println("------------------------------");
  while (http.get(char_array, 30000) < 0) {
    delay(500);
  }
  //wifly.readString()
  while (wifly.receive((uint8_t *)&get, 1, 1000) == 1) {
    //Serial.print(get);
    if(
      headerStop[0]=='\r' &&
      headerStop[1]=='\n' &&
      headerStop[2]=='\r' &&
      headerStop[3]=='\n' 
    ){
      if(get!='\0'){ 
        for (int i = 1;i< 20+1;i++){
          twenty[i-1] =twenty[i];
        }
        twenty[19]=get;
      }
     
    }else{
      for (int i = 1;i< 4;i++){
        headerStop[i-1] =headerStop[i];
      }
      headerStop[3]=get;
    } 

  }
  
  for (int i=0; i< 20;i++){
    if(twenty[i]=='/'){
      return i;
    }
  }
  return 20;


  //while(!Wifly.connect("192.168.32.87","3002")); 
  //snprintf (s, sizeof s, " %6d cnt", num);
  //Wifly.writeToSocket(char_array);
  //Wifly.print((char)num);
  //Wifly.writeToSocket(" HTTP/1.0\r\n");
  //Wifly.writeToSocket("\r\n");
  //Wifly.sendCommand(char_array,"*CLOS*");
}

void log(String msg){
  //if(Serial.available()){
    Serial.println(msg);
  //}
}




/*******************CAR *****************/

int M_ENA_A=9, M_ENA_B=10;
int M_A1=14, M_A2=15,M_B1=16,M_B2=17;//A=left, B=rigth

int DA0 = 2, DA1 = 4, DB0 = 3, DB1 = 8; // Pins for Right and Left encoders. DL0 and DR0 must be Interrupts
int aStat0, aStat1, bStat0, bStat1;
int apos = 0, bpos = 0;  // variables keeping count of the pulses on each side
String dir = "None";

void initCar(){
  pinMode(M_ENA_A, OUTPUT);     
  pinMode(M_ENA_B, OUTPUT);     
  pinMode(M_A1, OUTPUT);     
  pinMode(M_A2, OUTPUT);
  pinMode(M_B1, OUTPUT);
  pinMode(M_B2, OUTPUT);
}

void rampUp(int from, int to, unsigned int pin1,int pin2){
  for (int i = from; i < to; i++)
  {
    analogWrite(pin1,i);
    analogWrite(pin2,i);
   delay(10);
  }
}

void rampDown(int from, int to, unsigned int pin1, int pin2){
  for (int i = from; i > to; i--)
  {
    if(i%2){
      analogWrite(pin1,i);
      analogWrite(pin2,i);
    }else{
      analogWrite(pin2,i);
      analogWrite(pin1,i);
    }
    delay(10);
  }
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
}

void breakMotor(){
  log("breakMotor()");
  digitalWrite(M_ENA_A,LOW);
  digitalWrite(M_ENA_B,LOW);
  digitalWrite(M_A1,LOW);
  digitalWrite(M_A2,LOW);
  digitalWrite(M_B1,LOW);
  digitalWrite(M_B2,LOW);
  
}

void fwd(unsigned int speed){
  log("left()");
  breakMotor();
  analogWrite(M_ENA_A,speed);
  analogWrite(M_ENA_B,speed);
  digitalWrite(M_A1,HIGH);
  digitalWrite(M_A2,LOW);
  digitalWrite(M_B1,LOW);
  digitalWrite(M_B2,HIGH);

}

void rew(unsigned int speed){
  log("right()");
  breakMotor();
  analogWrite(M_ENA_A,speed);
  analogWrite(M_ENA_B,speed);
  digitalWrite(M_A1,LOW);
  digitalWrite(M_A2,HIGH);
  digitalWrite(M_B1,HIGH);
  digitalWrite(M_B2,LOW);
}

void left(unsigned int speed){
  log("fwd()");
 breakMotor();
 analogWrite(M_ENA_A,speed);
 analogWrite(M_ENA_B,speed);
 digitalWrite(M_A1,HIGH);
  digitalWrite(M_A2,LOW);
  digitalWrite(M_B1,HIGH);
  digitalWrite(M_B2,LOW);

}

void right(unsigned int speed){
  log("rew()");
 breakMotor();
 analogWrite(M_ENA_A,speed);
 analogWrite(M_ENA_B,speed);
 digitalWrite(M_A1,LOW);
 digitalWrite(M_A2,HIGH);
 digitalWrite(M_B1,LOW);
 digitalWrite(M_B2,HIGH);

 
}

void ttlEvent(){
  log("ttlEvent()");
  breakMotor();
}


void testMotor(){
  log("TEST fwd ");
  fwd(100);
  delay(15000);
  log("TEST rew");
  rew(100);
  delay(15000);
  log("TEST left");
  left(150);
  delay(15000);
  log("TEST right");
  right(150);
  delay(15000);

}

//String action ="";
//int actionNum =0;

void setup()
{
  Serial.begin(115200);//use the hardware serial to communicate with the PC
  setupWifi();
  log("Connected");
  //pinMode(13, OUTPUT);
  initCar();
  //testMotor();
  //digitalWrite(13,HIGH);
}
bool send =true;

void loop(){
  //testMotor();
  //digitalWrite(13,HIGH);
  
  if(action.equals("") && send && actionNum < 1024){
     int pos =getRestAction(actionNum,(uint8_t *)&twenty);
     //twenty = twenty + pos;
     action = twenty +pos +'\0';
     action.remove(action.indexOf("*"),action.length()-action.indexOf("*") );
     
     //action = action + '\0';
     Serial.println("<Loop Action is:" +action +'>');
     Serial.println("twenty is: " + (char)twenty[13] );
     //Serial.println(action);
     if(
      twenty[13]=='n'||
      twenty[13]=='s' ||
      twenty[13]=='e' ||
      twenty[13]=='w' ||
      twenty[13]=='b' ||
      twenty[13]=='d' ||
      twenty[13]=='x' 
      ){
        actionNum++;
        send=false;
        
        //if(action.equals("")){
        //  action = "foo";
        //}
        actionStarted = millis();
      }
  }  
  /*if(Wifly.canReadFromSocket())
  { 
    action += (String)Wifly.readFromSocket();

  }*/

  if(millis() > actionStarted +ACTION_TTL  ){
    ttlEvent();
    action = "";
    send = true;

  }
  
  if(action.endsWith("/seq/end")){
    log("sequence end catched!");
    if(!seqEndReported){
      postSeqEnd();
      seqEndReported = true;
    }
    action ="";
    send = true;
    actionNum =0;
  } 
    

  //strcmp(&twenty +,"/car/fwd")
  else if(!send && twenty[13]=='n' /*action.endsWith("/fwd*CLOS*")*/){
    seqEndReported =false;
    //rampUp(50,100,L1,L3);
    fwd(170);
    log("------<action:>-------");
    log("Action on /car/fwd");
    log("-------------------");
    action="";
    send=true;
     
  }
  else if(!send && twenty[13]=='d'){
    seqEndReported =false;
    log("------<action:>-------");
    log("Action on /car/wait" );
    log("-------------------");
    //Wifly.closeAndExit();
    delay(5000);
    action="";
    send=true;   
  }
  else if( !send && twenty[13]=='b'){
    seqEndReported =false;
    //rampDown(100,0,)
    breakMotor();
    log("------<action:>-------");
    log("Action on /car/stop");
    log("-------------------");
    //Wifly.closeAndExit();
    action="";
    send=true;
  }
  else if( !send && twenty[13]=='s'){
    seqEndReported =false;
    //rampUp(0,100,L2,L4);
    rew(170);
    log("------<action:>-------");
    log("Action on /car/rew");
    log("-------------------");
    //Wifly.closeAndExit();
    action="";
    send=true;
  }
  else if( !send && twenty[13]=='w'){
    seqEndReported =false;
    left(170);
    log("------<action:>-------");
    log("Action on /car/left");
    log("-------------------");
    //Wifly.closeAndExit();
    action="";
    send=true;
  }
  else if( !send && twenty[13]=='e'){
    seqEndReported =false;
    right(170);
    //log("------<action:>-------");
    log("Action on /light/right");
    //log("-------------------");
    //Wifly.closeAndExit();
    action="";
    send=true;
  } else if( !send && twenty[13]=='x'){
    seqEndReported =false;
    breakMotor();
    //digitalWrite(13,LOW);
    //log("------<action:>-------");
    log("Action on /seq/end");
    //log("-------------------");
    //Wifly.closeAndExit();
    actionNum =0;
    action="";
    send=true;
  } else {
    seqEndReported =false;
    log('Error! no action found');
  }
  /*if(Serial.available())
  {
    Wifly.print((char)Serial.read());
  }*/
  //delay(20000);
  //delay(1);
}