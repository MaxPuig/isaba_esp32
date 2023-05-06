#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "WIFIcredentials.h"

int state;


// Create an instance of the ESP8266 web server
ESP8266WebServer server(80);

void handleRoot() {
  // Display the state of BUILTIN_LED
  state = digitalRead(BUILTIN_LED);
  // define string which will contain ON/OFF button's state
  String estado_ventilador;
  if (state == 1)
    estado_ventilador = "OFF";
  else
    estado_ventilador = "ON";
  // reload the page every 5 seconds
  server.sendHeader("Refresh", "5");
  server.send(200, "text/html", "<html><body><p>Ventilador: " + String(estado_ventilador) + "</p><form method='post' action='/ventilador'><button type='submit'>Ventilador ON/OFF</button></form></body></html>");
}

void handleVentilador() {
  // Set the state of BUILTIN_LED to the same value
  state = !state;
  digitalWrite(BUILTIN_LED, state);
  // redirect to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void setup() {
  // Set the BUILTIN_LED modes
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  // Connect to Wi-Fi network
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  // Start the web server
  server.on("/", handleRoot);
  server.on("/ventilador", handleVentilador);
  server.begin();
}

void loop() {
  // Handle web server requests
  server.handleClient();
}
