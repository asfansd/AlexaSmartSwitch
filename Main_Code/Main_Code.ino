#include <SoftwareSerial.h>

String channel = "5"; //Initialize channel variale for communication
bool channel_status = false;

SoftwareSerial wifiSerial(2, 3);      // RX, TX for ESP8266

bool DEBUG = true;   //show more logs
int responseTime = 10; //communication timeout

void(* reset) (void) = 0;

void setup()
{
  
//  pinMode(6, OUTPUT);     //Power supply of arduino varying
//  digitalWrite(6, HIGH);
  
  pinMode(13,OUTPUT);  //set build in led as output
  digitalWrite(13, HIGH);  
  
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  
  // Open serial communications and wait for port to open esp8266:
  Serial.begin(115200);
  wifiSerial.begin(115200);
  if(wifiSerial){
    while(1){
      if(wifiSerial.isListening()){
      
        String response = "";
        
        response = sendToWifi("AT+CIPMUX=1",responseTime,DEBUG);
      //  Serial.println("CIPMUX Response : "+response+"\n\n");
        
        if(!(response.indexOf("OK")>=0)){
          delay(1000);
          reset();
        }
        delay(1000);
        
        response = sendToWifi("AT+CIPSERVER=1,80",responseTime,DEBUG);
      //  Serial.println("CIPSERVER Response : "+response+"\n\n");
        delay(1000);
        
        response = sendToWifi("AT+CIPSTA=\"192.168.0.200\"",responseTime,DEBUG);
      //  Serial.println("CIPStaticIP Response : "+response+"\n\n");
        delay(1000);
        
      //  Serial.write("Wifi Serial is now ready to listen!\n\n");
        digitalWrite(13, LOW);
        break;
      }
    }
  }
}

void loop(){
    String esp_response = "";
    if(wifiSerial.available())
    {
    // The esp has data so display its output to the serial window 
        String received_message = readWifiSerialMessage();
        checkChannelStatus(received_message);
        
        if(channel_status){
          channel = String(received_message[received_message.indexOf(",")+1]);
//          Serial.println("Channel : "+channel);
//          Serial.println("Received Message - Before : "+received_message);
          received_message = retrieveMessage(received_message);
//          Serial.println("Received Message - After : "+received_message);
          if((received_message.substring(0,2)).indexOf("AT")>=0){
            esp_response = sendToWifi(received_message,responseTime,DEBUG);
//            Serial.println("ESP Reply : "+ esp_response.substring(0,esp_response.length()-1));
            sendData(esp_response, channel);
          }
          else if((received_message.substring(0,2)).indexOf("C(")>=0){
            received_message = received_message.substring(2,received_message.indexOf(")"));
            String appliance_status = received_message.substring(0, received_message.indexOf(","));
            int number = (received_message.substring(received_message.indexOf(",")+1,received_message.length())).toInt();
//            Serial.println("Appliance Status : "+appliance_status);
//            Serial.println("Received Number : "+String(number));
            String appliance_response = control(appliance_status,number);
            sendData(appliance_response, channel);
          }
          else
            sendData("Error COMMAND Received\n\n", channel);
        }
    }
}    

/*
* Name: sendData
* Description: Function used to send string to tcp client using cipsend
* Params: 
* Returns: void
*/
void sendData(String str, String channel){
  String len="", sender="", closer="";
  len+=str.length();
  sender = "AT+CIPSEND="+channel+",";
  closer = "AT+CIPCLOSE="+channel;
//  Serial.println("Sender : "+sender+len);
//  Serial.println("Closer : "+closer);
  sendToWifi(sender+len,responseTime,DEBUG);
  delay(100);
  sendToWifi(str,responseTime,DEBUG);
  delay(100);
//  sendToWifi(closer,responseTime,DEBUG);
}

/*
* Name: find
* Description: Function used to match two string
* Params: 
* Returns: true if match else false
*/
boolean find(String string, String value){
  return string.indexOf(value)>=0;
}

void checkChannelStatus(String received_message){
  String str = received_message.substring(2, received_message.length());
  if(str.indexOf("CONNECT")>=0 || str.indexOf("CLOSED")>=0) 
    channel_status = false;
  else
    channel_status = true;
}

String retrieveMessage(String received_message){
  int index = received_message.indexOf(":");
  received_message = received_message.substring(index+1, received_message.length());
  return received_message;
}

String control(String appliance_status, int number){
  if(number+7<13){
//    Serial.println("Control : "+String(number));
    if(appliance_status.indexOf("ON")>=0)
      digitalWrite(number+7,HIGH);
    else
      digitalWrite(number+7,LOW);
    return("Appliance "+String(number)+" switched "+appliance_status+"\n\n");
  }
  else
    return("Appliance "+String(number)+" doesn't exist!\n\n");
}

/*
* Name: readWifiSerialMessage
* Description: Function used to read data from ESP8266 Serial.
* Params: 
* Returns: The response from the esp8266 (if there is a reponse)
*/
String  readWifiSerialMessage(){
//  char value[100]; 
//  int index_count =0;
//  while(wifiSerial.available()>0){
//    value[index_count]=wifiSerial.read();
//    index_count++;
//    value[index_count] = '\0'; // Null terminate the string
//  }
//  String str(value);
//  str.trim();
//  return str;

    String received_msg="";
    while(wifiSerial.available())
    {
    // The esp has data so display its output to the serial window 
      char c = wifiSerial.read(); // read the next character.
      received_msg+=c;
    }
    return received_msg;
}

/*
* Name: sendToWifi
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendToWifi(String command, const int timeout, boolean debug){
  String esp_response = "";
//  int i = 0;
//  char str[command.length()];
//  while(i<command.length()){
//    str[i] = command[i];
//    i++;
//  }
//  str[command.length()]='\0';
//  wifiSerial.write(str); // send the read character to the esp8266
  wifiSerial.println(command);
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(wifiSerial.available())
    {
    // The esp has data so display its output to the serial window 
      char c = wifiSerial.read(); // read the next character.
      esp_response+=c;
    }  
  }
//  Serial.println("ESP's Response : "+esp_response);
  return esp_response;
}
