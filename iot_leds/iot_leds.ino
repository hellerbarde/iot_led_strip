#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>

#define PIN 0
Adafruit_NeoPixel strip = Adafruit_NeoPixel(9, PIN, NEO_GRB + NEO_KHZ800);

const char *ssid = "33C3-open-legacy";
const char *password = "";

char * const global_color = "#4A256F";

ESP8266WebServer server ( 80 );

int eeprom_addr = 0;
byte eeprom_val = 0;

const int led = 2;

void handleRoot() {
	digitalWrite ( led, 1 );
	char temp[400];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

	snprintf ( temp, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

		hr, min % 60, sec % 60
	);
	server.send ( 200, "text/html", temp );
	digitalWrite ( led, 0 );
}

void setHtmlColor(const char * color){

  long color_nr = strtol(color+1, 0, 16);
  Serial.println(color_nr);
  uint8_t r = (color_nr >> 16) & 0xff;
  uint8_t g = (color_nr >> 8) & 0xff;
  uint8_t b = color_nr & 0xff;
  Serial.println(r);
  Serial.println(g);
  Serial.println(b);
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r,g,b));
  }
  strip.show();

}

void handleColor() {

  if (server.method() == HTTP_POST){
    if (server.hasArg("color")){   
      Serial.print("Found color: ");
      String color = server.arg("color");
      Serial.println(color);
      setHtmlColor(color.c_str());
      strcpy(global_color, color.c_str());
      Serial.print ( "start write eeprom: " );
      Serial.println ( millis() );
      for (int i=0; i < 7; i++){
        EEPROM.write(i, global_color[i]);
      }
      EEPROM.commit();
      Serial.print ( "end write eeprom: " );
      Serial.println ( millis() );
      Serial.print ( "saved color: " );
      Serial.println ( global_color );
    }
  }

  //if (server.method() == HTTP_GET){
    char temp[600];
    snprintf ( temp, 600,
"<!DOCTYPE html>\
<head>\
  <title>Set Light Color</title>\
  <style type=\"text/css\">\
    body{\
      margin:40px auto;\
      max-width:960px;\
      line-height:1.6;\
      font-size:18px;\
      color:#444;\
      padding:0 10px}\
    h1,h2,h3{line-height:1.2}\
  </style>\
</head>\
<h1>The Amazing Color Picker!</h1>\
<form action=\"color\" method=\"POST\">\
  <label for=\"ledcolor\">Choose a color: </label>\
  <input name=\"color\" id=\"ledcolor\" value=\"%s\" type=\"color\">\
  <button type=\"submit\" name=\"submit\">Go!</button>\
</form>", global_color);
    server.send ( 200, "text/html", temp );
  //}
  
}

void handleNotFound() {
	digitalWrite ( led, 1 );
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
	digitalWrite ( led, 0 );
}

void setup ( void ) {

  Serial.begin ( 115200 );
  Serial.print ( "start read eeprom: " );
  Serial.println ( millis() );
  EEPROM.begin(512);
  int value = EEPROM.read(eeprom_addr++);
  if (value == '#'){
    global_color[1] = EEPROM.read(eeprom_addr++);
    global_color[2] = EEPROM.read(eeprom_addr++);
    global_color[3] = EEPROM.read(eeprom_addr++);
    global_color[4] = EEPROM.read(eeprom_addr++);
    global_color[5] = EEPROM.read(eeprom_addr++);
    global_color[6] = EEPROM.read(eeprom_addr++);
    eeprom_addr = 0;
    Serial.print ( "loaded color: " );
    Serial.println ( global_color );
  }
  Serial.print ( "end read eeprom: " );
  Serial.println ( millis() );
  strip.begin();
  setHtmlColor(global_color);
//  for(uint16_t i=0; i<strip.numPixels(); i++) {
//    strip.setPixelColor(i, strip.Color(20,20,20));
//  }
  strip.show();
  
	pinMode ( led, OUTPUT );
	digitalWrite ( led, 1 );
	
//  WiFi.begin ( ssid, password );
  WiFi.begin ( ssid );
	Serial.println ( "" );
  rainbow(5);

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
		//delay ( 500 );
		Serial.print ( "." );
   rainbow(40);
	}

	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.println ( ssid );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );

	if ( MDNS.begin ( "esp8266" ) ) {
		Serial.println ( "MDNS responder started" );
	}

	server.on ( "/", handleRoot );
	//server.on ( "/test.svg", drawGraph );
//	server.on ( "/inline", []() {
//		server.send ( 200, "text/plain", "this works as well" );
//	} );
  server.on("/color", HTTP_ANY, handleColor);
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );
}

void loop ( void ) {
	server.handleClient();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*8; j+=strip.numPixels()*3) {
    for(i=0; i < strip.numPixels(); i+=1) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
        strip.show();
        delay(wait);
    }
    
    //delay(wait);
  }
}


