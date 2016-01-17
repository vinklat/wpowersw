/*
 * Wireless IP power switch. HTTP web interface and REST API. Includes temperature + humidity metering.
 * Powered by ESP8266 (wifi module), relay, DHTxx temperature/humidity sensor.
 * There is no arduino board required - operations will be served by mcu inside wifi module. 
 * Read more at https://github.com/vinklat/wpowersw
 * by Vaclav Vinklat, 2016
 */
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h> //not adafruit's library, but version extracted from mysensors project

/* 
 * Setup your application here:
 */
 
//#define DEBUG
#define PIN_RELAY 0
#define PIN_DHT 2

const char* ssid     = "enter_ssid";
const char* password = "enter_password";
const int port = 80;

/*
 *
 */

ESP8266WebServer server(port);
int relay_state;

#ifdef PIN_DHT
DHT dht;
#endif 

template<class T> inline Print &operator <<(Print &obj, T arg) { 
  obj.print(arg); return obj;
}

/*
 *
 */

String str="";

#define HTML_HEAD F("<!DOCTYPE HTML><html><body>")
#define HTML_FOOT F("</body></html>")
#define HTML_FORM_ON F("<form action=\"/flap\" method=\"POST\"><input type=\"submit\" value=\"Turn ON\"></form>")
#define HTML_FORM_OFF F("<form action=\"/flap\" method=\"POST\"><input type=\"submit\" value=\"Turn OFF\"></form>")

void handle_root() {
  #ifdef PIN_DHT
  float h = dht.getHumidity();
  float t = dht.getTemperature();
  #endif 

  str =  String(HTML_HEAD) + "relay: " + String(relay_state) + "<br />";
  #ifdef PIN_DHT
  str += "temperature: " + String(t) + "&deg;C<br />humidity: " + String(h) + "%<br/>";
  #endif
   
  if (relay_state)
    str += String(HTML_FORM_OFF);
  else
    str += String(HTML_FORM_ON);

  str +=  String(HTML_FOOT);

  server.send(200, "text/html", str);
  delay(dht.getMinimumSamplingPeriod());
}



void handle_state() {
  if (relay_state) 
  {
    digitalWrite(PIN_RELAY, HIGH);
    relay_state=0;
  }
  else 
  {
    digitalWrite(PIN_RELAY, LOW);
    relay_state=1;
  }
  
//  str=String(NO_CACHE) + String(REDIRECT);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Refresh", "1; url=/");
  
  server.send(200, "text/html", "");
}



void handle_api() {
  float h = dht.getHumidity();
  float t = dht.getTemperature();

  str="{\"state\": " + String(relay_state) + ", \"humidity\": " + String(h) + ", \"temperature\": " + String(t) + "}";
  server.send(200, "application/json", str);
  delay(100);
}



void handle_api_off() {
  if (relay_state) 
  {
    digitalWrite(PIN_RELAY, HIGH);
    relay_state=0;
  }
    
  server.send(200, "application/json", "{\"relay\": 0}");
  delay(100);
}

void handle_api_on() {
  if (!relay_state) 
  {
    digitalWrite(PIN_RELAY, LOW);
    relay_state=1;
  }

  server.send(200, "application/json", "{\"relay\": 1}");
  delay(100);
}

/*
 *
 */

void setup() {
  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  /*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  */
  #endif

  // Connect to WiFi network
  WiFi.begin(ssid, password);

  #ifdef DEBUG
  Serial << "\n\rWorking to connect";
  #endif

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    #ifdef DEBUG
    Serial << ".";
    #endif
  }
  #ifdef DEBUG
  Serial << "\n\rConnected to " << ssid;
  Serial << "\n\rServer is at " << WiFi.localIP() << '\n';
  #endif
  
  server.on("/", handle_root);
  server.on("/flap", handle_state);
  server.on("/api", handle_api);
  server.on("/api/0", handle_api_off);
  server.on("/api/1", handle_api_on);
  
  server.begin();

  // default relay pin state HIGH (=relay output LOW)
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, HIGH);
  relay_state=0;

  #ifdef PIN_DHT
  dht.setup(PIN_DHT);
  #endif
}


void loop(void)
{
  server.handleClient();
} 

