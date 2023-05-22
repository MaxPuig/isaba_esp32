// LLIBRERIES
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "WIFIcredentials.h"

//DECLARACIÓ DE PINS
#define nivell_pin 34  //Pin connectat al sensor de nivell d'aigua
#define humitat_pin 35 //Pin connectat al sensor de nivell d'humitat
#define ldr_pin 32
#define bomba_pin 33
#define ventilador_pin 25
#define temp_pin 26
#define led_pin 21

//VARIABLES ANALÒGIQUES
unsigned int analog_temp = 0;
unsigned int analog_hum = 0;
unsigned int analog_lvl = 0;
unsigned int analog_ldr = 0;

//VARIABLES ANALÒGIQUES TRANSFORMADES A UNITATS O %
float temperatura = 0;
unsigned int humitat = 0;
unsigned int nivell = 0;
unsigned int lluminositat = 0;

bool estat_ventilador = false;
bool estat_bomba = false;
bool estat_leds = false;

// Create an instance of the web server
WebServer server(80);

void handleRoot() {
  //LDR
  analog_ldr = analogRead(ldr_pin);
  lluminositat = map(analog_ldr, 0, 4095, 0, 100);

  //SENSOR D'HUMITAT
  analog_hum = analogRead(humitat_pin);
  humitat = map(analog_hum, 0, 4095, 0, 100);

  //SENSOR DE NIVELL D'AIGUA
  analog_lvl = analogRead(nivell_pin);
  if (analog_lvl < 1400) {
    analog_lvl = 1400;
  };
  nivell = map(analog_lvl, 1400, 4095 / 2, 0, 100);

  //LM35
  analog_temp = 100 * (analogRead(temp_pin));
  temperatura = 5.0 * (analog_temp / 4095.0);

  // RECARREGAR PÀGINA CADA 5s
  server.sendHeader("Refresh", "5");
  String html = "<html><body>";
  // INFO DELS SENSORS
  html += "<p>Lluminositat: " + String(lluminositat) + "%</p>";
  html += "<p>Humitat: " + String(humitat) + "%</p>";
  html += "<p>Nivell d'aigua: " + String(nivell) + "%</p>";
  html += "<p>Temperatura: " + String(temperatura) + "ºC</p>";

  // INFO DELS ACTUADORS
  if (estat_ventilador) html += "<p>Ventilador: ON</p>";
  else html += "<p>Ventilador: OFF</p>";
  if (estat_bomba) html += "<p>Bomba: ON</p>";
  else html += "<p>Bomba: OFF</p>";
  if (estat_leds) html += "<p>LEDS: ON</p>";
  else html += "<p>LEDS: OFF</p>";

  // BOTONS
  html += "<form method='post' action='/ventilador'><button type='submit'>Ventilador ON/OFF</button>";
  html += "<form method='post' action='/bomba'><button type='submit'>Bomba ON/OFF</button>";
  html += "<form method='post' action='/leds'><button type='submit'>Leds ON/OFF</button>";
  
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleVentilador() {
  estat_ventilador = !estat_ventilador;
  digitalWrite(ventilador_pin, estat_ventilador);
  // redirect to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void handleBomba() {
  estat_bomba = !estat_bomba;
  digitalWrite(bomba_pin, estat_bomba);
  // redirect to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void handleLeds() {
  estat_leds = !estat_leds;
  digitalWrite(led_pin, estat_leds);
  // redirect to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200); // Només si Serial.print
  //PINOUT
  pinMode(led_pin, OUTPUT);
  pinMode(ventilador_pin, OUTPUT);
  pinMode(bomba_pin, OUTPUT);
  pinMode(temp_pin, INPUT);
  pinMode(humitat_pin, INPUT);
  pinMode(nivell_pin, INPUT);
  pinMode(ldr_pin, INPUT);

  // Connect to Wi-Fi network
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  // Start the web server
  server.on("/", handleRoot);
  server.on("/ventilador", handleVentilador);
  server.on("/bomba", handleBomba);
  server.on("/leds", handleLeds);
  server.begin();
}

void loop() {
  // Handle web server requests
  server.handleClient();
}
