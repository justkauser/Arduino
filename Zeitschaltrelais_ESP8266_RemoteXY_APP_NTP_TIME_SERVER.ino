#define REMOTEXY_MODE__ESP8266WIFI_LIB
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <RemoteXY.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID ""       //Bitte ausfüllen
#define REMOTEXY_WIFI_PASSWORD ""   //Bitte ausfüllen
#define REMOTEXY_SERVER_PORT xxxx   //Bitte ausfüllen
#define REMOTEXY_ACCESS_PASSWORD "" //Bitte ausfüllen



// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 55 bytes
  { 255,22,0,123,0,48,0,16,24,1,3,131,9,7,45,17,36,26,67,4,
  0,54,63,8,2,24,31,7,36,0,32,63,8,133,24,2,21,67,4,0,
  62,63,8,2,24,31,67,4,0,70,63,8,2,24,61 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t select_1; // =0 if select position A, =1 if position B, =2 if position C, ... 
  char inputstring[21];  // string UTF8 end zero  

    // output variables
  char output1[31];  // string UTF8 end zero 
  char output2[31];  // string UTF8 end zero 
  char output3[61];  // string UTF8 end zero 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
///////////////////////////////////////////// 

bool starting = true;
float timerprev=0;
float timer_C1=0;
float timer_C2=0;
int schaltermode = 0;
int sm_old = 0;
int schalter = 0;
int modus = 0;
int select_alt = 0;
int zeitschaltuhr[7][8];
String Wochentage[7] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
String zsp[7][3];
int z_wo = 0;

#define seconds() (millis()/1000)

const long utcOffsetInSeconds = 3600;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


void setup() 
{   
  for(int i = 0; i < 5; i++){
  zeitschaltuhr[i][0]=8; // Anschalten Stunde
  zeitschaltuhr[i][1]=5; // Anschalten Min
  zeitschaltuhr[i][2]=22;// Abschalten Stunde
  zeitschaltuhr[i][3]=30;// Abschalten Min
  zeitschaltuhr[i][4]=12;// Pause Anf Stunde
  zeitschaltuhr[i][5]=0; // Pause Anf Min
  zeitschaltuhr[i][6]=15;// Pause End Stunde
  zeitschaltuhr[i][7]=0; // Pause End Min
}

 for(int i = 5; i < 7; i++){
  zeitschaltuhr[i][0]=9; // Anschalten Stunde
  zeitschaltuhr[i][1]=30; // Anschalten Min
  zeitschaltuhr[i][2]=22;// Abschalten Stunde
  zeitschaltuhr[i][3]=30;// Abschalten Min
  zeitschaltuhr[i][4]=12;// Pause Anf Stunde
  zeitschaltuhr[i][5]=0; // Pause Anf Min
  zeitschaltuhr[i][6]=15;// Pause End Stunde
  zeitschaltuhr[i][7]=0; // Pause End Min
}


  

  Serial.begin(9600);
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  RemoteXY_Init (); 
  timeClient.begin();
  pinMode(4, OUTPUT);
  //Serial.println(WiFi.localIP()); 
  // TODO you setup code
  
}

void loop() 
{ RemoteXY_Handler ();

time_t epochTime = timeClient.getEpochTime();
struct tm *ptm = gmtime ((time_t *)&epochTime);
int DD = ptm->tm_mday;
int MM= ptm->tm_mon+1;

if ((MM>3 && MM < 10)  ||  (MM == 3 && DD >= 29) || (MM == 10 && DD <= 31)) { timeClient.setTimeOffset (7200);} //falls wir in diesem Zeitraum sind

else {timeClient.setTimeOffset(3600);}


  
  digitalWrite(4, schalter);
  
  if (timeClient.getSeconds()%10==0 && seconds()-timerprev>5){

  for(int i = 0; i < 4; i++){
  Serial.println("");}    
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");    
  Serial.println("OUT PIN 4: D2");  
  Serial.println("OUTPUT: "+String(schalter));  
  Serial.println("Install RemoteXY");  
  Serial.println("");
  Serial.println("akuelle Uhrzeit:");
  String Helfer_h = "";
  String Helfer_m = "";
  String Helfer_s = "";

  if (timeClient.getHours()<10){Helfer_h = "0";}
  if (timeClient.getMinutes()<10){Helfer_m = "0";}
  if (timeClient.getSeconds()<10){Helfer_s = "0";}  
  String Uhrzeit = String(Wochentage[(timeClient.getDay()+6)%7])+", "+Helfer_h+String(timeClient.getHours())+":"+Helfer_m+String(timeClient.getMinutes())+":"+Helfer_s+String(timeClient.getSeconds());
  Serial.println(Uhrzeit);
  Serial.println("");

  Serial.println("Zeitschaltplan:");
  for(int i = 0; i < 7; i++){
  
  zsp[i][0]= Wochentage[i]+":          ";
  
  String H_h0 = "";
  String H_h4 = "";
  String H_h6 = "";
  String H_h2 = "";

  String H_m1 = "";
  String H_m5 = "";
  String H_m7 = "";
  String H_m3 = "";

  if (zeitschaltuhr[i][0]<10){H_h0="0";}
  if (zeitschaltuhr[i][4]<10){H_h4="0";}
  if (zeitschaltuhr[i][6]<10){H_h6="0";}
  if (zeitschaltuhr[i][2]<10){H_h2="0";}

  if (zeitschaltuhr[i][1]<10){H_m1="0";}
  if (zeitschaltuhr[i][5]<10){H_m5="0";}
  if (zeitschaltuhr[i][7]<10){H_m7="0";}
  if (zeitschaltuhr[i][3]<10){H_m3="0";}


  zsp[i][1] = H_h0+String(zeitschaltuhr[i][0])+":"+H_m1+String(zeitschaltuhr[i][1])+" - "+H_h4+String(zeitschaltuhr[i][4])+":"+H_m5+String(zeitschaltuhr[i][5]); //Von Anschalten bis Anfang Pause
  zsp[i][2] =H_h6+String(zeitschaltuhr[i][6])+":"+H_m7+String(zeitschaltuhr[i][7])+" - "+H_h2+String(zeitschaltuhr[i][2])+":"+H_m3+String(zeitschaltuhr[i][3]); //Von Ende  Pause bis Abschalten

  }  
  for(int i = 0; i < 3; i++){
  Serial.println(zsp[0][i]+"   "+zsp[1][i]+"   "+zsp[2][i]+"   "+zsp[3][i]+"   "+zsp[4][i]+"   "+zsp[5][i]+"   "+zsp[6][i]+"   ");}

   z_wo++;
   timerprev = seconds();}

  timeClient.update();
  
  schaltermode = 0;
  int Zeit_in_min = timeClient.getHours()*60+timeClient.getMinutes();
  
  int tag = (timeClient.getDay()+6)%7;

  int start_zeit = zeitschaltuhr[tag][0] * 60 + zeitschaltuhr[tag][1];
  int end_zeit = zeitschaltuhr[tag][2] * 60 + zeitschaltuhr[tag][3];
  int pause_start_zeit = zeitschaltuhr[tag][4] * 60 + zeitschaltuhr[tag][5];
  int pause_end_zeit = zeitschaltuhr[tag][6] * 60 + zeitschaltuhr[tag][7];
  if (start_zeit < Zeit_in_min && Zeit_in_min < pause_start_zeit || pause_end_zeit < Zeit_in_min && Zeit_in_min < end_zeit){schaltermode = 1;}

  if(schaltermode != sm_old){modus = 0; sm_old=schaltermode;}  
  if(RemoteXY.select_1 != select_alt){modus = select_alt = RemoteXY.select_1; }



String SM = "OFF";
if (schalter == 1){SM = "ON";}

strcpy  (RemoteXY.output1, String(zsp[z_wo%7][0] +"              "+SM).c_str());  
strcpy  (RemoteXY.output2, String(zsp[z_wo%7][1]).c_str());  
strcpy  (RemoteXY.output3, String(zsp[z_wo%7][2]).c_str()); 
  
  String Helfer_h = "";
  String Helfer_m = "";
  String Helfer_s = "";

  if (timeClient.getHours()<10){Helfer_h = "0";}
  if (timeClient.getMinutes()<10){Helfer_m = "0";}
  if (timeClient.getSeconds()<10){Helfer_s = "0";}  
  String Uhrzeit = String(Wochentage[(timeClient.getDay()+6)%7])+", "+Helfer_h+String(timeClient.getHours())+":"+Helfer_m+String(timeClient.getMinutes())+":"+Helfer_s+String(timeClient.getSeconds());
  strcpy  (RemoteXY.inputstring, Uhrzeit.c_str());
  
 if (modus == 0){schalter = schaltermode;}
 else if (modus == 1){schalter = 1;}
 else if (modus == 2){schalter = 0;}

}