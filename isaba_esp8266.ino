// LLIBRERIES
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

#define SSID "WIFI_SSID"
#define PASSWORD "WIFI_PASSWORD"

// DECLARACIÓ DE PINS
#define nivell_pin 35  // Pin connectat al sensor de nivell d'aigua
#define humitat_pin 34 // Pin connectat al sensor de nivell d'humitat
#define ldr_pin 33
#define temp_pin 32
#define bomba_pin 25
#define ventilador_pin 26

// VARIABLES ANALÒGIQUES
unsigned int analog_temp = 0;
unsigned int analog_hum = 0;
unsigned int analog_lvl = 0;
unsigned int analog_ldr = 0;

// VARIABLES ANALÒGIQUES TRANSFORMADES A UNITATS O %
float temperatura = 0.0;
unsigned int humitat = 0;
unsigned int nivell = 0;
unsigned int lluminositat = 0;
float objectiu_temp = 20.0;
unsigned long last_time = 0;

bool estat_ventilador = false;
bool estat_bomba = false;

// Create an instance of the web server
WebServer server(80);

void handleRoot()
{
  // RECARREGAR PÀGINA CADA 2s
  server.sendHeader("Refresh", "2");
  String html = "<html><head><title>TERRARI ISABA</title><style>";
  html += ".alert { background-color: red; color: white; font-weight: bold; padding: 10px; margin-bottom: 10px;} button { padding: 10px 20px; font-size: 18px; } .sensors, .info-section { font-size: 20px; padding: 20px; margin-bottom: 20px; }";
  html += "#title-image { display: block; margin: 0 auto; text-align: center; } </style></head><body>";
  html += "<img id='title-image' src='https://i.ibb.co/tMnQftd/4-RSwihe-Imgur.png'>";
  html += "<div style='display: flex; flex-direction: column; align-items: center; text-align: center;'> <div class='sensors' style='border: 1px solid black; margin-bottom: 10px;'>";
  // INFO DELS SENSORS
  html += "<p>Lluminositat: " + String(lluminositat) + "%</p>";
  html += "<p>Humitat: " + String(humitat) + "%</p>";
  html += "<p>Nivell d'aigua: " + String(nivell) + "%</p>";
  html += "<p>Temperatura: " + String(temperatura) + "&deg;C (Objectiu: " + String(objectiu_temp) + "&deg;C)</p>";
  html += "</div><div style='display: flex; justify-content: space-between;'><div class='info-section' style='border: 1px solid black; padding: 10px; margin-bottom: 10px;'>";

  // INFO VENTILADOR
  if (estat_ventilador)
    html += "<p>Ventilador: ON</p>";
  else
    html += "<p>Ventilador: OFF</p>";
  html += "<form method='post' action='/ventilador'><button type='submit'>Ventilador ON/OFF</button></form>";
  html += "</div><div class='info-section' style='border: 1px solid black; padding: 10px; margin-bottom: 10px;'>";

  // INFO BOMBA
  if (estat_bomba)
    html += "<p>Bomba: ON</p>";
  else
    html += "<p>Bomba: OFF</p>";
  html += "<form method='post' action='/bomba'><button type='submit'>Bomba ON/OFF</button></form>";
  html += "</div></div>";

  // Alerta Humitat
  if (humitat <= 0.0)
    html += "<div class='alert'><p style='margin: 0;'>Alerta Humitat Baixa</p></div>";

  // Alerta Nivell d'aigua
  if (nivell <= 0.0)
    html += "<div class='alert'><p style='margin: 0;'>Alerta Nivell d'aigua Baix</p></div>";

  // BOTONS
  html += "<div style='display: flex; justify-content: center;'>";
  html += "<form method='post' action='/objtempup' style='margin-right: 10px;'><button type='submit'>Pujar Temperatura Objectiu</button></form>";
  html += "<form method='post' action='/objtempdown'><button type='submit'>Baixar Temperatura Objectiu</button></form>";

  html += "</div></div></body></html>";
  server.send(200, "text/html", html);
}

void handleVentilador()
{
  estat_ventilador = !estat_ventilador;
  digitalWrite(ventilador_pin, estat_ventilador);
  // redirect to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void handleBomba()
{
  estat_bomba = !estat_bomba;
  digitalWrite(bomba_pin, estat_bomba);
  // redirect to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void handleObjTempPujar()
{
  objectiu_temp++;
  // redirect to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void handleObjTempBaixar()
{
  objectiu_temp--;
  // redirect to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void setup()
{
  Serial.begin(115200); // Només si Serial.print

  // PINOUT
  pinMode(ventilador_pin, OUTPUT);
  pinMode(bomba_pin, OUTPUT);
  pinMode(temp_pin, INPUT);
  pinMode(humitat_pin, INPUT);
  pinMode(nivell_pin, INPUT);
  pinMode(ldr_pin, INPUT);

  // Connect to Wi-Fi network
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  // Start the web server
  server.on("/", handleRoot);
  server.on("/ventilador", handleVentilador);
  server.on("/bomba", handleBomba);
  server.on("/objtempup", handleObjTempPujar);
  server.on("/objtempdown", handleObjTempBaixar);
  server.begin();
}

unsigned long getUptime() { return millis() / 1000; };

void loop()
{
  // LDR
  analog_ldr = analogRead(ldr_pin);
  lluminositat = map(analog_ldr, 0, 4095 / 2, 0, 100);

  // SENSOR D'HUMITAT
  analog_hum = analogRead(humitat_pin);
  humitat = map(analog_hum, 0, 4095, 0, 100);

  // SENSOR DE NIVELL D'AIGUA
  analog_lvl = analogRead(nivell_pin);
  nivell = map(analog_lvl, 0, 4095 / 2, 0, 100);

  // LM35
  analog_temp = (analogRead(temp_pin));
  temperatura = analog_temp * 2 * (330.0 / 4096.0);

  // IMPRIMIR PER CONSOLA
  Serial.println(WiFi.localIP());
  Serial.print("Lluminositat: ");
  Serial.println(lluminositat);
  Serial.print("Humitat: ");
  Serial.println(humitat);
  Serial.print("Nivell d'aigua: ");
  Serial.println(nivell);
  Serial.print("Temperatura: ");
  Serial.println(temperatura);
  Serial.println();

  if (getUptime() - last_time > 2) // cada 2 segons
  {
    last_time = getUptime();
    if (temperatura > objectiu_temp)
    {
      if (!estat_ventilador)
      {
        estat_ventilador = !estat_ventilador;
        digitalWrite(ventilador_pin, estat_ventilador);
      }
    }
    else
    {
      if (estat_ventilador)
      {
        estat_ventilador = !estat_ventilador;
        digitalWrite(ventilador_pin, estat_ventilador);
      }
    }
  }
  // Handle web server requests
  server.handleClient();
  delay(250);
}
