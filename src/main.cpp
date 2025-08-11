#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_Sensor.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <LittleFS.h>
#include <string>
#include <iostream>
#include <iomanip>

#define TEMP 5
#define HUMIDITY 4
#define LDRout 32
#define Atomizer 33
#define Fan 35
#define LED 34

#define WIFI_SSID "Redmi Note 10S"
#define WIFI_PASSWORD "1234567899"

//Hi..., MAMAOSHA
//Redmi Note 10S, 1234567899
//NEIN NEIN NEIN!, 1357902468
//The Destructive, Amjad#159357
// Siemens, neilm412

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587

#define AUTHOR_EMAIL "adamlotfalla233@gmail.com"
#define AUTHOR_PASSWORD "pacl esxc ryrb mgqf"
#define RECEPIENT_EMAIL1 "adamlotfalla233@gmail.com"
#define RECEPIENT_EMAIL2 "amjad.m.mounir@gmail.com"
#define RECEPIENT_EMAIL3 "eyad.1922014@stemgharbiya.moe.edu.eg"
#define RECEPIENT_EMAIL4 "ahmad.1922003@stemgharbiya.moe.edu.eg"


OneWire oneWire1(TEMP);
DallasTemperature tempSensors(&oneWire1);
DHTesp dht22;

SMTPSession smtp;

float humReadingIn = dht22.getHumidity();


void smtpCallback(SMTP_Status status);

int stepUpR = 4700;
bool Day = true;
int hum = 0;

const int duration = 900;
const int interval = 300;
int TInData[duration/interval];
unsigned long startTime = 0;

void printAddress(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void setup(){



  Serial.begin(9600);
//for temperature
  tempSensors.begin(); 

//for humidity
  dht22.setup(HUMIDITY, DHTesp::DHT22);
  pinMode(Fan,OUTPUT);
  pinMode(Atomizer,OUTPUT);

//for LDRs
  // pinMode(LDRin,INPUT);
  pinMode(LDRout,INPUT);
  pinMode(LED,OUTPUT);

// for WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.print("Waiting for NTP server time reading");
  Serial.println();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < ESP_MAIL_CLIENT_VALID_TS)
  {
      delay(100);
  }
  startTime=millis();
}


void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
  if (status.success()) {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");
    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      SMTP_Result result = smtp.sendingResult.getItem(i);
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
    smtp.sendingResult.clear();
  }
}

void sendEmail (String messageContent){

  Session_Config config;

  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = F("127.0.0.1");

  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  SMTP_Message message;
  message.html.content = messageContent.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.sender.name = F("ESP smart feedback system");
  message.sender.email = AUTHOR_EMAIL;
  String recipientEmails[] = {RECEPIENT_EMAIL1, RECEPIENT_EMAIL2, RECEPIENT_EMAIL3, RECEPIENT_EMAIL4};
  for(String i:recipientEmails){
    message.addRecipient("19211",i);
  }
  message.subject = "House action taken";
  message.text.content = messageContent.c_str();
  smtp.callback(smtpCallback);
  smtp.connect(&config);

  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}

void loop(){

  // Temperature work
    tempSensors.requestTemperatures();
    float tempReadingIn = tempSensors.getTempCByIndex(0);
    float tempReadingOut = tempSensors.getTempCByIndex(1);

    Serial.print("Temp sensor inside: ");
    Serial.print(tempReadingIn+1); // there is a systematic error of 1 degree 
    Serial.print(" C, Temp sensor outside: ");
    Serial.print(tempReadingOut);
    Serial.print(" C");
    Serial.println();

  // Humidity work
    delay(dht22.getMinimumSamplingPeriod());
    float humReadingIn = dht22.getHumidity()-21.6; //21.6 is the value of the systematic error in the sensor

    Serial.print("Humidity inside: ");
    Serial.print(humReadingIn);
    Serial.print(" %");
    Serial.println();

  // LDRs work
    float LintensityOUT = analogRead(LDRout);
    Serial.print("LDR voltage outside: ");
    Serial.print(LintensityOUT);
    Serial.println();



    if (Day) {Serial.printf("It is day time! \n");}
    if (!Day){Serial.printf("It is night time! \n");}

    if (LintensityOUT<1500 && Day){
      Day = false;
      sendEmail("<!DOCTYPE html><html><body><h2 style=\"color: #dc3545; font-family: arial, helvetica, sans-serif; margin:auto\">Sunset Time!</h2><p style=\"margin:10px; font-family: Calibri\">As the sun goes down and night comes, urgent action of turning on lights and closing window blinds done ✅</p></body></html>");
      delay(5000);
    }
    if (LintensityOUT>3500 && Day == false){
      Day=true;
      sendEmail("<!DOCTYPE html><html><body><h2 style=\"color: #dc3545; font-family: arial, helvetica, sans-serif; margin:auto\">Daytime!</h2><p style=\"margin:10px; font-family: Calibri\">As the sun shines and night goes down, urgent action of turning off  lights and opening window blinds done ✅</p></body></html>");
      delay(5000);
    }


    if(humReadingIn>50 && hum!=1){
      String text = "<!DOCTYPE html><html><body><h2 style=\"color: #dc3545; font-family: arial, helvetica, sans-serif; margin:auto\">Humidity level HIGH!</h2><p style=\"margin:10px; font-family: Calibri\">The house's weather is too humid as its level approached &nbsp;";
      text+=String(humReadingIn);
      text+="%.</p><p style=\"margin:10px; font-family: Calibri\">Urgent action of turning on fan done ✅</p></body></html>";
      sendEmail(text);
      digitalWrite(Fan,HIGH);
      hum=1;
      delay(5000);
    }
    else if (humReadingIn<30 && hum!=-1)
    {
      String text = "<!DOCTYPE html><html><body><h2 style=\"color: #dc3545; font-family: arial, helvetica, sans-serif; margin:auto\">Humidity level is LOW!</h2><p style=\"margin:10px; font-family: Calibri\">The house's weather is not humid enough as its level approached &nbsp;";
      text+=String(humReadingIn);
      text+="%.</p><p style=\"margin:10px; font-family: Calibri\">Urgent action of turning on atomizer done ✅</p></body></html>";
      sendEmail(text);
      digitalWrite(Atomizer,HIGH);
      hum=-1;
      delay(5000);
    }
    else if(humReadingIn>30 && humReadingIn<50 && hum!=0){
      digitalWrite(Atomizer,LOW);
      digitalWrite(Fan,LOW);
      hum=0;
      delay(5000);
    }
    Serial.println();
    Serial.print("------------------------------------------------");
    Serial.println();
    Serial.println();

    unsigned long current_time = millis();

    if((current_time-startTime)%duration==0){
      String text= "<!DOCTYPE html><html><body style=\" width: fit-content; margin: 0;padding: 0;-webkit-text-size-adjust: 100%;background-color: #F7F8F9;color: #000000\"><div style=\"position: absolute; left: 50%; transform: translateX(-50%);\"><h1 style=\"color: #dc3545; margin-left: auto; position: relative; font-family:arial,helvetica,sans-serif;\">Daily Report</h1><table border=\"1\" cellpadding=\"10\" cellspacing=\"0\" align=\"center\" style=\"font-family:arial,helvetica,sans-serif;\"><caption style=\"padding-bottom: 10px; text-align: left;\">Data are collected every &nbsp;";
      text+= String(interval/60);
      text+="&nbsp; minutes...</caption><thead><tr><td>Time</td><td>Temperature Inside</td><td>Temperature Outside</td><td>Humidity</td><td>Day/night</td></tr><tr><td>";
      
      for(int k=0; k<(duration/interval); k++){
        std::stringstream time;
        time<<std::setw(2)<<std::setfill('0')<< 5<<':';
        time<<std::setw(2)<<std::setfill('0')<< 79;
        text+= String(time);
        text+="</td><td>";  
        text+= String(tempReadingIn);
        text+="&nbsp; C </td><td>";
        text+= String(tempReadingOut);
        text+="&nbsp; C </td><td>";
        text+= String(humReadingIn);
        "&nbsp; % </td><td>";
        if(Day){text+="Daytime";}
        else{text+="Night";}
        "</td></tr></tbody></table></div></body></html>";
      }
      

      
    }
}