#define TS_ENABLE_SSL // For HTTPS SSL connection
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "displayOLED.h"  //Header file of setting up the OLED display 128 x 64 0.96 inch
#include "secrets.h"  //Header file of your channel security information
#include "dhtfile.h"  //Header file for the temperature sensor
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include <neotimer.h> //Library for the watchdog Timer
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 1;            // your network key Index number (needed only for WEP)
WiFiClientSecure  client;
Neotimer mytimer;
Neotimer recTimer;
unsigned long myChannelNumber = SECRET_CH_ID; //variable for channel id
const char * WriteAPIKey = SECRET_WRITE_APIKEY; //variable for write api key
const char * ReadAPIKey = SECRET_READ_APIKEY_COUNTER; //variable for read api key
// Initialize our values
int number1 = 0;
int number2 = 0;
String myStatus = "";

// Fingerprint check, make sure that the certificate has not expired.
const char * fingerprint = NULL; // use SECRET_SHA1_FINGERPRINT for fingerprint check

int val1;
float val2, val3;
int buzzPin = 14;
void setup() {
  start_connection();
  recTimer.set(5000); //Timer for requesting value of output data from the website
  mytimer.set(5000);  //Timer for sending value of output data into the display website
  recTimer.start(), mytimer.start();
}

void loop() {
  // set the fields with the values
  ThingSpeak.setField(1, number1);  //order of the output
  outputDHT();
  if (recTimer.done())
  {
    Serial.println("Retrieve Timer Finish \tGetting Data...\n");
    recTimer.stop();
    number2++;
    ThingspeakGetData();
    runTimer("Get", 2); //set timer run 2 minute each to get data from the Thingspeak Channel

  }
  if (mytimer.done())
  {
    Serial.println("Send Timer Finish \nUploading Data...");
    mytimer.stop();
    number1++;
    ThingspeakStatData();
    runTimer("Send", 10); //set timer run 10 minute each to send data from the Thingspeak Channel
    sendTrigger();
  }
}
void start_connection()
{
  Serial.begin(115200);// Initialize serial
  delay(3 * 1000);
  Serial.println("\nPerforming Starting Mode");
  pinMode(buzzPin, OUTPUT);
  HT.begin();
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
  }
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
    Serial.print(".");
    delay(5000);
  }
  Serial.println("\nWiFi Connected");
  if (fingerprint != NULL) {
    client.setFingerprint(fingerprint);
  }
  else {
    client.setInsecure(); // To perform a simple SSL Encryption
  }
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  int yA;
  for (int x; x < 30; x++)
  {
    yA += 8;
    display.drawLine(0, 0, 150, yA , SSD1306_WHITE);
    display.display();
    delay(10);
  }
  tone(buzzPin, 600);
  delay(500);
  noTone(buzzPin);
  delay(1);

}

void ThingspeakStatData()   //verivfying the data output before sending into the Thingspeak Channel function
{
  int x = ThingSpeak.writeFields(myChannelNumber, WriteAPIKey);
  if (x == 200) {
    Serial.println("Channel Update order" + String(number1) + " successfull."); //Updating channel status to inform order of overall channel update
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x)); //Issue with updating channel
  }
}
void ThingspeakGetData()  //Getting data from the Thingspeak Channel function
{
  int statCode = ThingSpeak.readMultipleFields(myChannelNumber, ReadAPIKey);
  if (statCode == 200)
  {
    val1 = ThingSpeak.getFieldAsInt(1);
    val2 = ThingSpeak.getFieldAsFloat(2);
    val3 = ThingSpeak.getFieldAsFloat(3);
    Serial.print(String(number2) + ". Output Num:" + String(val1));
    Serial.print("\tTemp Celcius:" + String(val2));
    Serial.println("\tHumidity:" + String(val3));
  }
  else {
    Serial.println("Problem reading channel. HTTP error code " + String(statCode));
  }
}
void outputDHT()  //sending output of the temperature into the Thingspeak Channel function
{
  tempC = HT.readTemperature();
  humid = HT.readHumidity();
  ThingSpeak.setField(2, tempC);
  ThingSpeak.setField(3, humid);
  String myStatus = "Successfully Updating Value";
  ThingSpeak.setStatus(myStatus);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(x0, 0);
  display.print(F("Hygrometer"));
  display.setTextSize(1); // Draw 2X-scale text
  display.drawLine(4, 18, 118, 18, SSD1306_WHITE);
  display.setCursor(x0, 24);
  display.print("Celcius: " + String(tempC) + "C");
  display.setCursor(x0, 36);
  display.print("Humidity: " + String(humid) + "%");
  display.setCursor(x0, 48);
  display.print("Made By M.Irsyad.Y");
  display.display();
  delay(50);
}
void runTimer(String Mode, int i) // function for watchdog timer without interrupting the next program
{
  if (Mode == "Send")
  {
    i = i * 1000 * 60; //Minutes
    mytimer.set(i);
    mytimer.start();
  }
  if (Mode == "Get")
  {
    i = i * 1000 * 20;
    recTimer.set(i);
    recTimer.start();
  }
}
void sendTrigger()  //Trigger Passive Buzzer Function
{
  for (int i = 0; i < 2; i++)
  {
    tone(buzzPin, 800); delay(150);
    noTone(buzzPin); delay(15);
  }
}
