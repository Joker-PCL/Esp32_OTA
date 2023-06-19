#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

//OLED Credential
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SH1106 display(21, 22);

#include <HTTPUpdate.h>
#include <SD.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>


// Replace with your network credentials
const char* ssid = "Ahmad";
const char* password = "ahmadwifi";
String text;
String chat_id;

//Pin Declarations
int Vibration1 = 13;
int Vibration2 = 3;
int blueled = 27;

// Initialize Telegram BOT
#define BOTtoken "5649845757:AAFq4Mxd5Ccs69HR2w-VU6VYUNY4irsz6DE"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "1205293055"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
bool ledState = LOW;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    chat_id = String(bot.messages[i].chat_id);
    text = bot.messages[i].text;
    
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    if (text.length()> 200){
       bot.sendMessage(chat_id, "Very Long Message ,Please Write Message Smaller Than 300 Charecters", "");
      continue;
    }
    if (text == "/available"){
       bot.sendMessage(chat_id, "Yes , I Am Here", "");
      continue;
    }
    
    // Print the received message
    Serial.println(text);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(text);
    display.display();
    display.clearDisplay();
    digitalWrite(Vibration1, HIGH);
    delay(100);
    digitalWrite(Vibration1, LOW);
    delay(100);
    digitalWrite(Vibration1, HIGH);
    delay(100);
    digitalWrite(Vibration1, LOW);
    bot.sendMessage(chat_id, "Hmmm Recieved", "");

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/Turn_On to turn GPIO ON \n";
      welcome += "/Turn_Off to turn GPIO OFF \n";
      welcome += "/heartbeat to turn GPIO OFF \n";
      welcome += "/Wish_On to turn GPIO OFF \n";
      welcome += "/state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/Turn_On") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(blueled, ledState);
      digitalWrite(14, LOW);
    }
    if (text == "/HummingBird") {
      bot.sendMessage(chat_id, "Hmm Tell", "");
    }

    if (text == "/Hello HummingBird") {
      bot.sendMessage(chat_id, "Hi", "");
    }

    if (text == "/Are You Feeling Sleepy") {
      bot.sendMessage(chat_id, "Kind Of Bye Goodnight", "");
    }

    if (text == "/I Am Missing You") {
      bot.sendMessage(chat_id, "Hmm I am here only", "");
    }

     if (text == "/heartbeat") {
      bot.sendMessage(chat_id, "Heartbeat set to ON", "");
      ledState = HIGH;
      digitalWrite(Vibration1, HIGH);
      delay(2000);
      digitalWrite(Vibration1, LOW);
    }

    if (text == "/clear") {
      bot.sendMessage(chat_id, "Clearing the Screen", "");
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(10,30);
      display.println("Waiting For New Message");
      display.display();
      display.clearDisplay();
    }
    
    if (text == "/Turn_Off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(blueled, ledState);
      digitalWrite(14, LOW);
    }
    
    if (text == "/state") {
      if (digitalRead(blueled)){
        bot.sendMessage(chat_id, "It's Turned ON", "");
      }
      else{
        bot.sendMessage(chat_id, "it's Turned OFF", "");
      }
    }

    //OTA Update
    if (bot.messages[i].type == "message")
    {
      if (bot.messages[i].hasDocument == true)
      {
        httpUpdate.rebootOnUpdate(false);
        t_httpUpdate_return ret = (t_httpUpdate_return)3;
        if (bot.messages[i].file_caption == "write spiffs")
        {
          size_t spiffsFreeSize = SPIFFS.totalBytes() - SPIFFS.usedBytes();
          if (bot.messages[i].file_size < spiffsFreeSize)
          {
            bot.sendMessage(bot.messages[i].chat_id, "File downloading.", "");
            HTTPClient http;
            if (http.begin(client, bot.messages[i].file_path))
            {
              int code = http.GET();
              if (code == HTTP_CODE_OK)
              {
                int total = http.getSize();
                int len = total;
                uint8_t buff[128] = {0};
                WiFiClient *tcp = http.getStreamPtr();
                if (SPIFFS.exists("/" + bot.messages[i].file_name))
                  SPIFFS.remove(("/" + bot.messages[i].file_name));
                File fl = SPIFFS.open("/" + bot.messages[i].file_name, FILE_WRITE);
                if (!fl)
                {
                  bot.sendMessage(bot.messages[i].chat_id, "File open error.", "");
                }
                else
                {
                  while (http.connected() && (len > 0 || len == -1))
                  {
                    size_t size_available = tcp->available();
                    Serial.print("%");
                    Serial.println(100 - ((len * 100) / total));
                    if (size_available)
                    {
                      int c = tcp->readBytes(buff, ((size_available > sizeof(buff)) ? sizeof(buff) : size_available));
                      fl.write(buff, c);
                      if (len > 0)
                      {
                        len -= c;
                      }
                    }
                    delay(1);
                  }
                  fl.close();
                  if (len == 0)
                    bot.sendMessage(bot.messages[i].chat_id, "Success.", "");
                  else
                    bot.sendMessage(bot.messages[i].chat_id, "Error.", "");
                }
              }
              http.end();
            }
          }
          else
          {
            bot.sendMessage(bot.messages[i].chat_id, "SPIFFS size to low (" + String(spiffsFreeSize) + ") needed: " + String(bot.messages[i].file_size), "");
          }
        }
        else
        {
          if (bot.messages[i].file_caption == "update firmware")
          {
            bot.sendMessage(bot.messages[i].chat_id, "Firmware writing...", "");
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(10,20);
            display.println("....Please Wait...");
            display.display();
            display.setCursor(10,40);
            display.println("Updating Firmware..");
            display.display();
            
            
            ret = httpUpdate.update(client, bot.messages[i].file_path);
          }
          if (bot.messages[i].file_caption == "update spiffs")
          {
            bot.sendMessage(bot.messages[i].chat_id, "SPIFFS writing...", "");
            ret = httpUpdate.updateSpiffs(client, bot.messages[i].file_path);
          }
          switch (ret)
          {
          case HTTP_UPDATE_FAILED:
            bot.sendMessage(bot.messages[i].chat_id, "HTTP_UPDATE_FAILED Error (" + String(httpUpdate.getLastError()) + "): " + httpUpdate.getLastErrorString(), "");
            break;

          case HTTP_UPDATE_NO_UPDATES:
            bot.sendMessage(bot.messages[i].chat_id, "HTTP_UPDATE_NO_UPDATES", "");
            break;

          case HTTP_UPDATE_OK:
            bot.sendMessage(bot.messages[i].chat_id, "UPDATE OK.\nRestarting...", "");
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(10,20);
            display.println("UPDATE OK");
            display.display();
            display.setCursor(10,40);
            display.println("RESTARTING NOW");
            display.display();
            display.clearDisplay();
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            ESP.restart();
            break;
          default:
            break;
          }
        }
      }
      if (bot.messages[i].text == "/dir")
      {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        String files = "";
        while (file)
        {
          files += String(file.name()) + " " + String(file.size()) + "B\n";
          file = root.openNextFile();
        }
        bot.sendMessage(bot.messages[i].chat_id, files, "");
      }
      else if (bot.messages[i].text == "/format")
      {
        
        bool res = SPIFFS.format();
         bot.sendMessage(bot.messages[i].chat_id, "Formatting SPIFFS", "");
        if (!res)
          bot.sendMessage(bot.messages[i].chat_id, "Format unsuccessful", "");
        else
          bot.sendMessage(bot.messages[i].chat_id, "SPIFFS formatted.", "");
          
      }
    }
    
  }
}

void setup() {
  Serial.begin(115200);
  String text = " ";

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

  pinMode(blueled, OUTPUT);
   pinMode(14, OUTPUT);
  pinMode(Vibration1, OUTPUT);
  digitalWrite(blueled, ledState);
  

  /* initialize OLED with I2C address 0x3C */
  display.begin(SH1106_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(27,20);
    display.println("Hello");
     display.setCursor(27,40);
    display.println("HummingBird");
    display.display();
    display.clearDisplay();
    delay(1000);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10,5);
    display.println("Connecting To Wifi");
    display.setCursor(20,22);
    display.println(password);
    display.setCursor(20,32);
    display.println(ssid);
    display.display();
    display.clearDisplay();
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10,30);
    display.println("Connected To ");
    display.setCursor(10,43);
    display.println(ssid);
    display.clearDisplay();
    display.println("Waiting For ");
    display.setCursor(10,43);
    display.println("Message");
    display.display();
    bot.sendMessage(chat_id, "Hi", "");
   
}

void loop() {
  
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}



 
