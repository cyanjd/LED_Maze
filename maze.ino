// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
//MQTT SETUP

// Arduino library dependencies
// modify only if your application requires additional libraries
#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>

char ssid[] = "DukeOpen";
char pass[] = "";
int status = WL_IDLE_STATUS;     // variable to hold WiFi radio's status

////// if using TLS/SSL for MQTT, use this line
//WiFiSSLClient WiFiclient; // instantiate a secure WiFi client
WiFiClient WiFiclient;

// MQTT CONFIGURATION SETTINGS
#define MQTT_BROKER "mqtt.colab.duke.edu"  // specify MQTT broker address (IP or URL)
#define MQTT_PORT 1883  // use 8883 for TLS/SSL MQTT; use 1883 for non-secure

PubSubClient client(WiFiclient); // use WiFi for MQTT communication

// each MQTT client should have a unique ID
String device_id = "FeatherM0-";  // this will be used with Mac address for uniqueness

// variables for non-blocking MQTT reconnection
long lastReconnectAttempt = 0;
long now;

//--------------------------------------------------
//LED SETUP
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      196

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second
int walls[] = {1,4,7,20,23,26,29,32,35,38,39,44,48,51,60,63,67,72,73,74,75,76,79,82,83,88,107,112,113,116,119,120,121,122,123,124,125,128,138,141,151,156,157,158,161,164,165,166,174,189};

int userLocation;
int xLoc;
int yLoc;
String command = "right";
int myArray[14][14];
//-----------------------------------------------------------

void doSubscriptions() {
  // Specify topic subscriptions line by line below, such as:  client.subscribe("/some/topic");
  client.subscribe("/move");
}

//------------------------------------------------------------
// Parse incoming message from subscribed topics
void parseMQTT(char* topic, byte* payload, unsigned int length) {
  
  payload[length] = '\0';  // important - do not delete
  
  // print to serial every message, regardless of topic
  // (optional)
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print((char*)payload);
  Serial.println();


  if (strcmp(topic, "/move") == 0) { // if topic published is "/move" 
    if(strcmp((char*)payload, "left") == 0){
      // do left
      command = "left";
      Serial.println("left");
    }
    if(strcmp((char*)payload, "right") == 0){
      // do right
      command = "right";
      Serial.println("right");
    }
    if(strcmp((char*)payload, "up") == 0){
      // do up
      command = "up";
      Serial.println("up");
    }
    if(strcmp((char*)payload, "down") == 0){
      // do down
      command = "down";
      Serial.println("down");
    }
    if(strcmp((char*)payload, "neutral") == 0){
      // do neutral
      command = "neutral";
      Serial.println("neutral");
    }
  }

}

// END SUBSCRIPTIONS SETUP
//----------------------------------------------------------------------------------
void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.

  for(int i=0;i<50;i++){
    int wall = walls[i];
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(wall, pixels.Color(0,0,150)); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(delayval); // Delay for a period of time (in milliseconds).

  }

  //Initialize Array of Arrays 
  int num = 0;
  
   for (int x = 0; x < 14; x++){
     if (x%2 != 0){
       for (int y = 0; y < 14; y++){
         myArray[x][y] = num;
//         pixels.setPixelColor(myArray[x][y], pixels.Color(150,0,150)); 
//         pixels.show();
//         delay(delayval);

         num++;
       }
     }
     if (x%2 == 0){
       for (int y = 13; y >= 0; y--){
         myArray[x][y] = num;
//         pixels.setPixelColor(myArray[x][y], pixels.Color(150,0,150)); 
//         pixels.show();
//         delay(delayval);
         num++;
       }
     }
   }

  xLoc = 7;
  yLoc = 0;
  userLocation = myArray[xLoc][yLoc];
  Serial.print(userLocation);

  //Start  
  pixels.setPixelColor(userLocation, pixels.Color(150,0,150)); 
  pixels.show();
  delay(delayval);

  //End Point
  pixels.setPixelColor(84, pixels.Color(0,150,0)); 
  pixels.show();
  delay(delayval);
  pixels.setPixelColor(111, pixels.Color(0,150,0)); 
  pixels.show();
  delay(delayval);


  //---------------------------------------------------------------------------
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // Feather M0-specific WiFi pins
  WiFi.setPins(8, 7, 4, 2);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi chip not present or accessible");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WiFi:
    if (sizeof(pass) <= 1) {
      status = WiFi.begin(ssid);
    }
    else {
      status = WiFi.begin(ssid, pass);
    }

    // wait 5 seconds for connection:
    delay(5000);
  }

  // you're connected now, so print out a success message:
  Serial.println("You're connected to the network");

  // print your Feather's IP address and MQTT info:
  IPAddress ip = WiFi.localIP();
  Serial.print("Device IP Address: ");
  Serial.println(ip);

  // get your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  String mac_address;
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) mac_address += "0";
    mac_address += String(mac[i], HEX);
    if (i > 0) mac_address += ":";
  }
  // append to device_id
  device_id += mac_address;
  Serial.print("Attempting to connect to MQTT Broker: ");
  Serial.print(MQTT_BROKER);
  Serial.print(":");
  Serial.println(MQTT_PORT);
  lastReconnectAttempt = 0;

    // initiate first connection to MQTT broker            
    client.setServer(MQTT_BROKER, MQTT_PORT);
  
    // specify a function to call upon receipt of a msg
    // on a subscribed channel; in this case parseMQTT()
    client.setCallback(parseMQTT);
  
}

void loop() {
  // get the current time
  now = millis();

  // if MQTT connection lost
  if (!client.connected()) {
    // only attempt to reconnect every 5 secs
    if (now - lastReconnectAttempt > 5000) { // 5 secs since last reconnect attempt?
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // MQTT client connected
    client.loop();
  }
  //------------------------------------------------------------------------------------------
//  pixels.setPixelColor(userLocation, pixels.Color(150,0,0)); 
//  pixels.show();
//  delay(delayval);
   if (userLocation != 84 && userLocation != 111){ //Game isn't over yet
      int val = 0;
      int newXLoc;
      int newYLoc;
      //val will be the next spot to move to based on the command
      if (command == "up"){
        newXLoc = xLoc - 1;
        newYLoc = yLoc;
        if (newXLoc >= 0){ //Check if out of bounds
          val = myArray[newXLoc][yLoc];
        }
        if(newXLoc < 0){
          newXLoc = xLoc;
          newYLoc = yLoc;
          val = myArray[newXLoc][yLoc];
        }
      }
      if (command == "down"){
        newXLoc = xLoc + 1;
        newYLoc = yLoc;
        if (newXLoc <= 13){ //Check if out of bounds
          val = myArray[newXLoc][yLoc];
        }
        else{
          newXLoc = 13; 
          newYLoc = yLoc;
          val = myArray[newXLoc][yLoc];
        }
      }
      if (command == "left"){
        newYLoc = yLoc - 1;
        newXLoc = xLoc;
        if (newYLoc >= 0){ //Check if out of bounds
          val = myArray[xLoc][newYLoc];
        }
        else{
          newYLoc = 0;
          newXLoc = xLoc;
          val = myArray[xLoc][newYLoc];
        }
      }
      if (command == "right"){
        newYLoc = yLoc + 1;
        newXLoc = xLoc;
        if (newYLoc <= 13){ //Check if out of bounds
          val = myArray[xLoc][newYLoc];
        }
        else{
          newYLoc = 13;
          newXLoc = xLoc;
          val = myArray[xLoc][newYLoc];
        }
      }
  
      if (command == "neutral"){
        val = userLocation;
        newXLoc = xLoc;
        newYLoc = yLoc;
      }

      bool touchedWall = false;
      int wallTouched;
      for (int i = 0; i < 50; i++){
        int wall2 = walls[i];
        if (wall2 == val){
          touchedWall = true;
          wallTouched == val;
        }
      }
      if (touchedWall == false){ //if the user didn't touch the wall and they moved, move the userLocation accordingly
        if (val != userLocation){ //Moving userLocation to val
           //turn off LED at old userLocation
           pixels.setPixelColor(userLocation, pixels.Color(0,0,0)); 
           pixels.show();
           delay(delayval);

           //turn on LED at new userLocation(val);
           userLocation = val;
           pixels.setPixelColor(userLocation, pixels.Color(150,0,150)); 
           pixels.show();
           delay(delayval);
        }
        xLoc = newXLoc;
        yLoc = newYLoc;
      }
      if (touchedWall == true){ //If the user touched the wall, blink the wall they touched red
        pixels.setPixelColor(val, pixels.Color(150,0,0)); 
        pixels.show();
        delay(delayval);

        pixels.setPixelColor(val, pixels.Color(0,0,150)); 
        pixels.show();
        delay(delayval);

        //TO DO, fix xLoc and yLoc
      }

   }
   if (userLocation == 84 || userLocation == 111){//They win!
     //what happens when they win
     pixels.setPixelColor(84, pixels.Color(150,0,150)); 
     pixels.show();
     delay(delayval);
    
     pixels.setPixelColor(111, pixels.Color(150,0,150)); 
     pixels.show();
     delay(delayval);
   }
 }
 //------------------------------------------------------------------------------------
 boolean reconnect() {
  if (client.connect(device_id.c_str())) {
    Serial.print(device_id);
    Serial.println(" connected to MQTT broker");
    doSubscriptions();  // (re)subscribe to desired topics
    return client.connected();
  }
  Serial.print("MQTT connection failed, rc=");
  Serial.println(client.state());
  Serial.println("Trying again ...");
  return 0;
}
 
