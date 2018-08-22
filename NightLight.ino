/*
 * 
 *    Night light by Colin Hickey 
 *    
 *    You need to configure the number of LEDs your using and the datapin which is used for the din on the NeoPixels
 *  
 *    The code by Brian Lough has been used a base for the web interface
 *    https://github.com/witnessmenow/ESP8266-IR-Remote-Example 
 * 
*/


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Adafruit_NeoPixel.h>
#include <WS2812FX.h>

// LED constants  
const unsigned int nLEDs = 15;  // number of LED's in the strip
const unsigned int dataPin = D2; // pin connecting the LED data pin to the Arduino

const char* ssid = "chickey-house";
const char* password = "adamjoeollie";

ESP8266WebServer server(80);

unsigned int maxPower = 125;  // maximum brightness of LEDs
unsigned long next_submit;
bool  bedtime = false;

WS2812FX ws2812fx = WS2812FX(nLEDs, dataPin, NEO_GRB + NEO_KHZ800);

String rowDiv = "    <div class=\"row\" style=\"padding-bottom:1em\">\n";
String endDiv = "    </div>\n";

// Buttons are using the bootstrap grid for sizing - http://getbootstrap.com/css/#grid
String generateButton(String colSize, String id, String text, String url) {

  return  "<div class=\"" + colSize + "\" style=\"text-align: center\">\n" +
          "    <button id=\"" + id + "\" type=\"button\" class=\"btn btn-default\" style=\"width: 100%\" onclick='makeAjaxCall(\"" + url + "\")'>" + text + "</button>\n" +
          "</div>\n";
}

void handleRoot() {
  //digitalWrite(led, 0);
  String website = "<!DOCTYPE html>\n";
  website = website + "<html>\n";
  website = website + "  <head>\n";
  website = website + "    <meta charset=\"utf-8\">\n";
  website = website + "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n";
  website = website + "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  website = website + "    <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\">\n";
  website = website + "  </head>\n";
  website = website + "  <body>\n";
  website = website + "    <div class=\"container-fluid\">\n";
  // ------------------------- Power Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-12", "Off","Lights Off", "Off");
  website = website + endDiv;
  website = website + rowDiv;
  website = website + generateButton("col-xs-4", "StrobeWhite","Strobe White", "StrobeWhite");
  website = website + generateButton("col-xs-4", "MultiStrobe","Multi Strobe", "MultiStrobe");
  website = website + generateButton("col-xs-4", "RainbowCycle","Rainbow Cycle", "RainbowCycle");
  website = website + endDiv;
  // ------------------------- Channel Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-4", "ColourWipe","Colour Wipe", "ColourWipe");
  website = website + generateButton("col-xs-4", "TheaterChaseRainbow","Theater Chase Rainbow", "TheaterChaseRainbow");
  website = website + generateButton("col-xs-4", "BlackoutRainbow","Blackout Rainbow", "BlackoutRainbow");
  website = website + endDiv;
  // ------------------------- Volume Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-4", "RandomColour","Random Colour", "RandomColour");
  website = website + generateButton("col-xs-4", "FireFlicker","Fire Flicker", "FireFlicker");
  website = website + generateButton("col-xs-4", "RunningColour","Running Colour", "RunningColour");
  website = website + endDiv;
  // ------------------------- Volume Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-4", "SolidRed","Red", "SolidRed");
  website = website + generateButton("col-xs-4", "SolidBlue","Blue", "SolidBlue");
  website = website + generateButton("col-xs-4", "SolidGreen","Green", "SolidGreen");
  website = website + endDiv;
  // ------------------------- Satelite Controls --------------------------
  website = website + rowDiv;
  website = website + generateButton("col-xs-12", "ChaseWhite","Chase White", "ChaseWhite");
  website = website + endDiv;
  website = website + rowDiv;
  website = website + generateButton("col-xs-12", "BedTime","Bed Time", "BedTime");
  website = website + endDiv;
  website = website + endDiv;
  website = website + "    <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>\n";
  website = website + "    <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\"></script>\n";
  website = website + "    <script> function makeAjaxCall(url){$.ajax({\"url\": url})}</script>\n";
  website = website + "  </body>\n";
  website = website + "</html>\n";

  server.send(200, "text/html", website);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void defaultlight(void) {
  maxPower=125;
  ws2812fx.setBrightness(maxPower);
  ws2812fx.start();
  bedtime=false;
}

void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS Responder Started");
  }

  server.on("/", handleRoot);

  server.on("/Off", [](){
    Serial.println("Turn Lights Off");
    ws2812fx.stop();
    server.send(200, "text/plain", "Lights Off");
  });
  
  server.on("/StrobeWhite", [](){
    Serial.println("Strobe");
    ws2812fx.setColor(WHITE);
    ws2812fx.setMode(FX_MODE_BREATH);
    defaultlight();
    server.send(200, "text/plain", "Strobe");
  });

  server.on("/RainbowCycle", [](){
    Serial.println("Rainbow Cycle");
    ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
    defaultlight();
    server.send(200, "text/plain", "Rainbow Cycle");
  });

  server.on("/ColourWipe", [](){
    Serial.println("Colour Wipe");
    ws2812fx.setMode(FX_MODE_COLOR_WIPE);
    defaultlight();
    server.send(200, "text/plain", "Colour Wipe");
  });

  server.on("/RandomColour", [](){
    Serial.println("Random Colour");
    ws2812fx.setMode(FX_MODE_RANDOM_COLOR);
    defaultlight();
    server.send(200, "text/plain", "Random Colour");
  });

  server.on("/TheaterChaseRainbow", [](){
    Serial.println("THEATER_CHASE");
    ws2812fx.setMode(FX_MODE_THEATER_CHASE_RAINBOW);
    defaultlight();
    server.send(200, "text/plain", "TheaterChaseRainbow");
  });

  server.on("/BlackoutRainbow", [](){
    Serial.println("Blackout Rainbow");
    ws2812fx.setMode(FX_MODE_CHASE_BLACKOUT_RAINBOW);
    defaultlight();
    server.send(200, "text/plain", "Surround Sound BlackoutRainbow");
  });

  server.on("/FireFlicker", [](){
    Serial.println("Fire Flicker");
    ws2812fx.setColor(RED);
    ws2812fx.setMode(FX_MODE_FIRE_FLICKER);
    defaultlight();
    server.send(200, "text/plain", "Fire Flicker");
  });

  server.on("/MultiStrobe", [](){
    Serial.println("Multi Strobe");
    ws2812fx.setMode(FX_MODE_MULTI_STROBE);
    defaultlight();
    server.send(200, "text/plain", "Multi Strobe");
  });

  server.on("/SolidRed", [](){
    Serial.println("Red");
    ws2812fx.setColor(RED);
    defaultlight();
    server.send(200, "text/plain", "Red");
  });

  server.on("/SolidBlue", [](){
    Serial.println("Blue");
    ws2812fx.setColor(BLUE);
    defaultlight();
    server.send(200, "text/plain", "Blue");
  });

  server.on("/SolidGreen", [](){
    Serial.println("Green");
    ws2812fx.setColor(GREEN);
    defaultlight();
    server.send(200, "text/plain", "Green");
  });

  server.on("/RunningColour", [](){
    Serial.println("Running Colour");
    ws2812fx.setMode(FX_MODE_RUNNING_COLOR);
    defaultlight();
    server.send(200, "text/plain", "Running Colour");
  });

  server.on("/ChaseWhite", [](){
    Serial.println("Chase White ");
    ws2812fx.setMode(FX_MODE_CHASE_WHITE);
    defaultlight();
    server.send(200, "text/plain", "Chase White");
  });

  server.on("/BedTime", [](){
    Serial.println("BedTimer");
    bedtime=true;
    ws2812fx.setMode(FX_MODE_STATIC);
    server.send(200, "text/plain", "BedTime");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP Server Started");

  ws2812fx.init();
  ws2812fx.setBrightness(maxPower);
  ws2812fx.setSpeed(200);
  ws2812fx.setColor(WHITE);
}

void loop(void){

  if ((bedtime == true) && (millis() > next_submit)){
       //Count down for 30mins
        next_submit = millis() + 14400;
        maxPower = maxPower-1;
        Serial.println("Power = " + String(maxPower));
        ws2812fx.setBrightness(maxPower);
        ws2812fx.setColor(WHITE);
        ws2812fx.start();
        if (maxPower == 10) bedtime=false;
    }

  server.handleClient();
  ws2812fx.service();
}
