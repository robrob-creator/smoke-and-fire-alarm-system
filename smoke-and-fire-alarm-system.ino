#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <FirebaseArduino.h>
#include <time.h>
#define MQ2pin (0)
SoftwareSerial SIM900(D7, D8);

String textForSMS,phoneNumber,id,emergencyType,residents,link,finalMessage;
String fbaseresidents="\n";
const unsigned char Passive_buzzer=2;
const int flame = D0; 
float sensorValue;
String UserID;
const int buzpin=11;
String systemCode="EjhbZsxrxy";
int timezone = 7 * 3600;
int dst = 0;


// connection to firebase database.
#define FIREBASE_HOST "fire-alarm-system-ab525-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "wD5rHi2LoNIY1dHynNW0GFH8CnrYn2JzUuTMhOBj"
#define WIFI_SSID "FAMILIA VERCEDE"
#define WIFI_PASSWORD "theSUPERNATURAL101"

void setup() {
  pinMode(flame,INPUT);
  pinMode (Passive_buzzer,OUTPUT) ;
  Serial.begin(9600);
  
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

 configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  Serial.println("\nWaiting for Internet time");

  while(!time(nullptr)){
     Serial.print("*");
     delay(1000);
  }
  Serial.println("\nTime response....OK");   
}

String a,b;
void loop(){
String date;  
String thetime;

//get the time
time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  //date
  int theday =  p_tm->tm_mday;
  int themonth= p_tm->tm_mon + 1;
  int theyear = p_tm->tm_year + 1900;
  //time
  int thehour = p_tm->tm_hour;
  int theminute = p_tm->tm_min;
  int theseconds = p_tm->tm_sec;
  //converting the date's int values to String
  String sday = String(theday);
  String smonth = String(themonth);
  String syear = String(theyear);
  //converting the time's int values to String
  String shour= String( thehour);
  String smin = String(theminute);
  String ssec = String(theseconds);

  //concatenating the value to complete the date format
  date = sday+"/"+smonth+"/"+syear;
  //concatenating the value to complete the time format
  thetime = shour+":"+smin+":"+ssec;
  Serial.println(date+" "+thetime);
  delay(1000);  
  
//condtion to trigger the alarms

int t = digitalRead(flame);
sensorValue = analogRead(MQ2pin);
int order=1;

 Serial.println(t);
  delay (1000);
   if (t==0) {  
   Firebase.setInt("FireSystem/EjhbZsxrxy/currentUser/OnFire",1); 
    tone(Passive_buzzer, 523) ; //DO note 523 Hz  
    emergencyType ="\nFire Detected!";
    getAddress(emergencyType);

    int timer= Firebase.getInt("FireSystem/EjhbZsxrxy/currentUser/timer");
    int finalTime = timer/5;
    
    
    getOperation(timer,date,thetime,emergencyType,order);
    delay(finalTime);
   
   getOperation(timer,date,thetime,emergencyType,order+1);
   delay(finalTime);
   
   getOperation(timer,date,thetime,emergencyType,order+2);
    delay(finalTime);

   getOperation(timer,date,thetime,emergencyType,order+3);
    delay(finalTime);
    
    getOperation(timer,date,thetime,emergencyType,order+5);
    delay(finalTime);
   int cancel= Firebase.getInt("FireSystem/EjhbZsxrxy/currentUser/cancel");
   getOperation(timer,date,thetime,emergencyType,cancel);
    Firebase.setInt("FireSystem/EjhbZsxrxy/currentUser/cancel",7); 
   
  }
   if(sensorValue >800)
  {
   tone(Passive_buzzer, 523) ; //DO note 523 Hz
    emergencyType ="\nSmokeDetected!";
    getAddress(emergencyType);
    int timer= Firebase.getInt("FireSystem/EjhbZsxrxy/currentUser/timer");
    int finalTime = timer/5;
    
    
    getOperation(timer,date,thetime,emergencyType,order);
    delay(finalTime);
   
   getOperation(timer,date,thetime,emergencyType,order+1);
   delay(finalTime);
   
   getOperation(timer,date,thetime,emergencyType,order+2);
    delay(finalTime);

   getOperation(timer,date,thetime,emergencyType,order+3);
    delay(finalTime);
    
    getOperation(timer,date,thetime,emergencyType,order+5);
    delay(finalTime);
   int cancel= Firebase.getInt("FireSystem/EjhbZsxrxy/currentUser/cancel");
   getOperation(timer,date,thetime,emergencyType,cancel);
    Firebase.setInt("FireSystem/EjhbZsxrxy/currentUser/cancel",7); 
   
  }
  noTone(Passive_buzzer) ;
  
  int convert=(int)sensorValue;
  String fsend=String(convert);

  
  Firebase.setString("FireSystem/EjhbZsxrxy/SmokeSensor",fsend);
  Firebase.setString("FireSystem/EjhbZsxrxy/UnitCode",systemCode);
   Serial.println(convert);
}
//operation #1 getting condition
void getOperation(int timer,String date,String thetime, String emergencyType,int order){
   int sendMessage = Firebase.getInt("FireSystem/EjhbZsxrxy/currentUser/send");
    Serial.println("\nsend stats");
     Serial.println(sendMessage);
     Serial.println("\n order");
      Serial.println(sendMessage);
     Serial.println(order);
    delay (5000);
  if(sendMessage == 1 || order == 7){
    getresidents(emergencyType);
     delay (5000);
    getlocation(emergencyType);
    delay (5000); 
    getContact(emergencyType);
    delay (5000); 
         Serial.println("\n initiate time");
     notification(date,thetime);
    Firebase.setInt("FireSystem/EjhbZsxrxy/currentUser/send",0); 
    Firebase.setInt("FireSystem/EjhbZsxrxy/currentUser/OnFire",0); 
    }else{
      Serial.println("\nnot sending");
      }
}


//operation #1 getting user address
void getAddress(String emergencyType){
  String userID= Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/uid");
  String address= Firebase.getString("users/"+userID+"/address");
   String link = "https://creative-faloodeh-c306a8.netlify.app/?system_code="+systemCode;
  String messagetoOwner = "System Alert\n" +emergencyType +"please confirm:\nlink: " +link;
   sendSMStoOwner(messagetoOwner,emergencyType);
}
void getContact(String emergencyType){
  String userID= Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/uid");
  String number= Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/residentNumber");
   String username = Firebase.getString("users/"+userID+"/userfname");
  String messagetoBFP = "Contact No. for " +username +" is :" +number;
   sendContacts(messagetoBFP,emergencyType);
}
//operation #2 get the phone number and send SMS
void sendSMStoOwner(String messagetoOwner, String emergencyType){
  String userID= Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/user");
  String ownerNumber =Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/residentNumber");
       Serial.println(ownerNumber);
        Serial.print(messagetoOwner); 
        SIM900.begin(9600);
        sendSMStoOwner(messagetoOwner,ownerNumber,emergencyType);
}

//operation #3 the actuall sending commands
void sendSMStoOwner(String messagetoOwner, String ownerNumber, String emergencyType)
{

  SIM900.print("AT+CMGF=1\r");                     // AT command to send SMS message
  delay(1000);
 SIM900.println("AT + CMGS = \"+63"+ownerNumber+"\"");  // recipient's mobile number, in international format
 
  delay(5000);
  SIM900.println(messagetoOwner);                         // message to send
  delay(1000);
  SIM900.println((char)26);                        // End AT command with a ^Z, ASCII code 26
  delay(1000); 
  SIM900.println();
  delay(200);                                     // give module time to send SMS
  
}

//operation #4 getting the list of residents
void getresidents(String emergencyType){
  String fresidents= Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/residentList");
     
    String residents = emergencyType+"\nResidents:\n"+fresidents;
    sendListOfResidents(residents,emergencyType);
   
}

//operation #5 getting  the  contact number of residents
void sendListOfResidents(String residents,String emergencyType){
  String phoneNumber = Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/hotline");
       Serial.println(phoneNumber);
        finalMessage = residents + "\nNote: link shall be sent shortly";  
        Serial.print(finalMessage); 
        SIM900.begin(9600);
        sendSMS(finalMessage,phoneNumber,emergencyType);
 }
 void sendContacts(String messagetoBFP,String emergencyType){
  String phoneNumber = Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/hotline");
       Serial.println(phoneNumber);
        finalMessage = messagetoBFP;  
        Serial.print(finalMessage); 
        SIM900.begin(9600);
        sendSMS(finalMessage,phoneNumber,emergencyType);
 }
//operation #6 sending commands
void sendSMS(String finalMessage, String phoneNumber, String emergencyType)
{

  SIM900.print("AT+CMGF=1\r");                     // AT command to send SMS message
  delay(1000);
 SIM900.println("AT + CMGS = \"+63"+phoneNumber+"\"");  // recipient's mobile number, in international format
 
  delay(5000);
  SIM900.println(finalMessage);                         // message to send
  delay(1000);
  SIM900.println((char)26);                        // End AT command with a ^Z, ASCII code 26
  delay(1000); 
  SIM900.println();
  delay(200);                                     // give module time to send SMS
}
//operation #7 getting location
void getlocation(String emergencyType){
   String userID = Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/uid");
   String ownername = Firebase.getString("users/"+userID+"/userfname");
   String link = Firebase.getString("users/"+userID+"/resident/0/residentLink");
   String address = Firebase.getString("users/"+userID+"/resident/0/residentinputddress");
   String location ="Location for establishment owned by "+ownername+"\n"+ address +"\n"+link;
   Serial.println(location);

   sendLocationToBFP(location);
}
// operation #8 getting  the  contact number of BFP
void  sendLocationToBFP(String location){
  String numberBFP = Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/hotline");
        Serial.println(numberBFP);
        String locationMessage = location;  
        Serial.print(locationMessage); 
        SIM900.begin(9600);
        sendSMSLocation(locationMessage,numberBFP);
 }
//operation #9 sending commands
void sendSMSLocation(String locationMessage, String numberBFP)
{

  SIM900.print("AT+CMGF=1\r");                     // AT command to send SMS message
  delay(1000);
 SIM900.println("AT + CMGS = \"+63"+numberBFP+"\"");  // recipient's mobile number, in international format
 
  delay(5000);
  SIM900.println(locationMessage);                         // message to send
  delay(1000);
  SIM900.println((char)26);                        // End AT command with a ^Z, ASCII code 26
  delay(1000); 
  SIM900.println();
  delay(200);                                     // give module time to send SMS
}
//operation #10 Send data to the web application
void  notification(String date, String thetime){ 
  
int local=1;
int caseslocal=1;
int adminLocal=1;
int n = 0;
String admin ="Admin";
String userID = Firebase.getString("FireSystem/EjhbZsxrxy/currentUser/uid");
String slink = Firebase.getString("users/"+userID+"/resident/0/residentDesktopLink");  
String station = Firebase.getString("users/"+userID+"/station");  
String username= Firebase.getString("users/"+userID+"/userfname");

  Serial.println("\n complete");
  
String countDirectory = "NotificationCount/"+station;
int count = Firebase.getInt("NotificationCount/"+station);
//cases variables
String caseCountDirectory = "CaseCount/"+station;
int casesCount = Firebase.getInt("CaseCount/"+station);
//admin variables
String adminCountDirectory = "NotificationCount/Admin";
int adminCount = Firebase.getInt("NotificationCount/"+admin);
  Serial.println("\n userID");
    Serial.println(userID);
      Serial.println("\n station");
    Serial.println(station);
      Serial.println("\n date");
    Serial.println(date);
//send data to the selected Station
  Firebase.setString( "Notifications/" + station +"/"+count+"/warning_message", emergencyType);
  Firebase.setString( "Notifications/"  + station +"/"+count+"/name",username);
  Firebase.setString( "Notifications/" +  station +"/"+count+"/link",slink);
  Firebase.setString( "Notifications/" +  station +"/"+count+"/date",date);
  Firebase.setInt( "Notifications/" +  station +"/"+count+"/id",count);
  Firebase.setString( "Notifications/" +  station +"/"+count+"/time",thetime);
  Firebase.setInt( "Notifications/" +  station +"/"+count+"/status",0);
  Firebase.setString( "Notifications/" +  station +"/"+count+"/station",station);
  Firebase.setString( "Notifications/" +  station +"/"+count+"/userId",userID);
  local=local+count;
  Firebase.setInt(countDirectory,local); 
   delay(1000);
//send a copy of data to the cases directory  
  Firebase.setString( "Cases/" + station + "/" +casesCount+ "/warning_message",emergencyType);
  Firebase.setString( "Cases/" + station + "/" +casesCount+ "/name",username);
  Firebase.setString( "Cases/" + station + "/" +casesCount+ "/link",slink);
  Firebase.setString( "Cases/" + station + "/" +casesCount+ "/date",date);
  Firebase.setString( "Cases/" + station + "/" +casesCount+ "/time", thetime);
    Firebase.setInt( "Notifications/" +  station +"/"+count+"/status",0);
  caseslocal= caseslocal + casesCount;
  Firebase.setInt( caseCountDirectory, caseslocal ); 
  delay(1000);
//send data to admin
  Firebase.setString( "Notifications/"+admin +"/"+adminCount+"/warning_message", emergencyType);
  Firebase.setString( "Notifications/"+admin +"/"+adminCount+"/name",username);
  Firebase.setString( "Notifications/"+admin +"/"+adminCount+"/link",slink);
  Firebase.setString( "Notifications/"+admin +"/"+adminCount+"/date",date);
  Firebase.setString( "Notifications/"+admin +"/"+adminCount+"/time",thetime);
  Firebase.setInt( "Notifications/" +  admin +"/"+adminCount+"/status",0);
  Firebase.setString( "Notifications/" +  admin +"/"+adminCount+"/userId",userID);
  Firebase.setInt( "Notifications/" +  admin +"/"+adminCount+"/id",count);
  Firebase.setString( "Notifications/" +  admin +"/"+count+"/station",station);
  adminLocal=adminLocal+adminCount;
  Firebase.setInt(adminCountDirectory,adminLocal); 
   delay(1000);

  
}
