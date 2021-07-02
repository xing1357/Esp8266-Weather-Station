// Simple esp8266 Weather Station

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include <WiFiUdp.h>                          // Library to send and receive UDP messages
#include <WiFiClient.h>
#include <NTPClient.h>


#define TFT_DC 0
#define TFT_CS 15

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


#define TFT_GREY        0x5AEB
#define TFT_BLACK       0x0000      
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0      
#define TFT_DARKCYAN    0x03EF      
#define TFT_MAROON      0x7800      
#define TFT_PURPLE      0x780F      
#define TFT_OLIVE       0x7BE0      
#define TFT_LIGHTGREY   0xC618      
#define TFT_DARKGREY    0x7BEF      
#define TFT_BLUE        0x001F  
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F      
#define TFT_YELLOW      0xFFE0      
#define TFT_WHITE       0xFFFF    
#define TFT_ORANGE      0xFDA0      
#define TFT_GREENYELLOW 0xB7E0  
#define TFT_PINK        0xFC9F    

// Don't change!!!
#define TFT_DC  D3
#define TFT_CS  D8
#define TFT_MOSI D7
#define TFT_SCK D5
//#define TFT_RST D4





const char* ssid     = "BELL535";                 // SSID of local network
const char* password = "D75C997F6127";                    // Password on network
String APIKEY = "f193428ed7e2cb619fa808aa70569caa";                                 
String CityID = "6183235";                                 //Your City ID

int TimeZone = -5;// GMT +1
const long utcOffsetInSeconds = -18000;


WiFiClient client;
char servername[]="api.openweathermap.org";              // remote server we will connect to
String result;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

String getTime(){
  timeClient.update();
  String timeS = timeClient.getFormattedTime();
  int length = timeS.length();
  return timeS.substring(length-8,length-3);
}

String getDate(){
  String date_time = timeClient.getFormattedDate();
  int index_date = date_time.indexOf("T");
  String date = date_time.substring(0, index_date);
  return date;
}

int  counter = 10;                                      

String weatherDescription ="";
String weatherLocation = "";
String Country;
float Temperature;
float Humidity;
float Pressure;
int Sunrise;
int Sunset;
float Speed;
float Temp_min;
float Temp_max;
float Visibility;
float Wind_angle;



void setup()
{
    Serial.begin(115200);
    int cursorPosition=0;
    tft.begin();
    tft.fillScreen(TFT_BLACK );
    tft.setRotation(0); //2
    tft.setTextWrap(0);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    
    tft.drawRect(0,0,240,320,TFT_GREEN);
    tft.drawRect(1,1,238,318,TFT_GREEN);
    tft.drawLine(2,30,240,30, TFT_GREEN);
    tft.drawLine(2,70,240,70, TFT_GREEN);
    tft.drawLine(2,110,240,110, TFT_GREEN);
    tft.drawLine(2,153,240,153, TFT_GREEN);
    tft.drawLine(110,110,110,152, TFT_GREEN);//
    tft.drawLine(2,180,240,180, TFT_GREEN);
    tft.drawLine(2,207,240,207, TFT_GREEN);
    tft.drawLine(2,270,240,270, TFT_GREEN);
    
    
    
    
    tft.setCursor(10,10); 
    Serial.println("Connecting");
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
          delay(1000);
    }
    delay(1000);
}    

void loop(){
    if(counter == 10){
        counter = 0;
        displayGettingData();
        delay(1000);
        getWeatherData();
        delay(1000);
      } 
      else {
        counter++;
        displayWeather(weatherLocation,weatherDescription);
         delay(1000);
        displayConditions(Temperature,Humidity,Pressure);
        delay(1000);
        displayWindSpeed(Speed , Temp_min , Temp_max, Visibility, Wind_angle);
        delay(1000);
   }
}

void getWeatherData(){
    if (client.connect(servername, 80)){                                         //starts client connection, checks for connection
          client.println("GET /data/2.5/weather?id="+CityID+"&units=metric&APPID="+APIKEY);
          client.println("Host: api.openweathermap.org");
          client.println("User-Agent: ArduinoWiFi/1.1");
          client.println("Connection: close");
          client.println();
        } 
     else {
          Serial.println("connection failed");        //error message if no client connect
          Serial.println();
        }

    while(client.connected() && !client.available()) 
      delay(1);                                          //waits for data
    while (client.connected() || client.available()){                                             //connected or data available
        char c = client.read();                     //gets byte from ethernet buffer
        result = result+c;
        }

    client.stop();                                      //stop client
    result.replace('[', ' ');
    result.replace(']', ' ');
    Serial.println(result);
    char jsonArray [result.length()+1];
    result.toCharArray(jsonArray,sizeof(jsonArray));
    jsonArray[result.length() + 1] = '\0';
    StaticJsonBuffer<1024> json_buf;
    JsonObject &root = json_buf.parseObject(jsonArray);
    
    if (!root.success())
    {
      Serial.println("parseObject() failed");
    }
    
    String location = root["name"];
    String country = root["sys"]["country"];
    float temperature = root["main"]["temp"];
    float humidity = root["main"]["humidity"];
    String weather = root["weather"]["main"];
    String description = root["weather"]["description"];
    float pressure = root["main"]["pressure"];
    float sunrise = root["sys"]["sunrise"];
    float sunset = root["sys"]["sunset"];
    float temp_min = root["main"]["temp_min"];
    float temp_max = root["main"]["temp_max"];
    float speed = root["wind"]["speed"];
    float visibility = root["visibility"];
    float wind_angle = root["wind"]["deg"];
    weatherDescription = description;
    weatherLocation = location;
    Country = country;
    Temperature = temperature;
    Humidity = humidity;
    Pressure = pressure;
    Sunrise = sunrise;
    Sunset = sunset;
    Speed = speed;
    Temp_min = temp_min;
    Temp_max = temp_max;
    Visibility = visibility;
    Wind_angle = wind_angle;
    
    
    
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10,276);
    tft.print("IP:");
    tft.print(WiFi.localIP());
    
    
    delay(1000);
}



void displayWeather(String location,String description)
{
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10,10);
    tft.print(location);
    tft.print(", ");
    tft.print(Country);
    tft.setCursor(15,35);
    tft.print(getTime());
    tft.setCursor(15,52);
    tft.print(getDate());
    tft.setCursor(10,93);
    tft.print(description);
    
    tft.setTextSize(2);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setCursor(45,75);
    tft.print("Description");
    delay(1000);
}
    
    void displayConditions(float Temperature,float Humidity, float Pressure)
    {   
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(30,115);//Printing Temperature
    tft.print("Temp"); 
    
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(20,133);//Printing Temperature
    tft.print(Temperature,1);
    tft.print(char(247));
    tft.print("C "); 
    
    tft.setCursor(120,115);  
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);                                      
    tft.print("Humidity"); //Printing Humidity
    
    tft.setCursor(145,133);  
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.print(Humidity,1);
    tft.print(" %"); 
    
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(10,160);                     //Printing Pressure
    tft.print("P: ");
    tft.print(Pressure,1);
    tft.print(" hPa");
    delay(1000);
    }
    
    void displayGettingData()
    {
    
    tft.setTextSize(1);
    tft.setCursor(10,295);
    tft.setTextColor(TFT_GREEN, TFT_BLACK); 
    tft.print(ssid);
    
    
    
    tft.setTextSize(1);
    tft.setCursor(10,305);
    tft.setTextColor(TFT_GREEN, TFT_BLACK); 
    tft.print(APIKEY);
    delay(1000);
}

void displayWindSpeed(float Speed , float Temp_min, float Temp_max , float Visibility , float Wind_angle)
{
    tft.setTextSize(2);
    tft.setCursor(10,187);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
    tft.print("Wind:");
    
    tft.setTextSize(2);
    tft.setCursor(95,187);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
    tft.print(Speed,1);
    
    tft.setTextSize(2);
    tft.setCursor(160,187);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
    tft.print("m/s");
    
    tft.setTextSize(2);
    tft.setCursor(10,210);
    tft.setTextColor(TFT_BLUE, TFT_BLACK); 
    tft.print("Tmin:");
    tft.print(Temp_min);
    tft.print(char(247));
    tft.print("C");
    
    tft.setTextSize(2);
    tft.setCursor(10,231);
    tft.setTextColor(TFT_RED, TFT_BLACK); 
    tft.print("Tmax:");
    tft.print(Temp_max);
    tft.print(char(247));
    tft.print("C");
    
    tft.setTextSize(2);
    tft.setCursor(10,252);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK); 
    tft.print("Vis:");
    tft.print(Visibility,0);
    tft.print(" meters");
    
    tft.setTextSize(2);
    tft.setCursor(165,210);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
    tft.print("Angle");
    
    tft.setTextSize(2);
    tft.setCursor(170,231);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
    tft.print(Wind_angle,0);
    tft.print(char(247));
    
    delay(1000);
}
