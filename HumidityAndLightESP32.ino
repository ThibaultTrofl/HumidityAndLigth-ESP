#define APP_NAME "HumidityAndLight-v1"
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#define DHTPIN 4
#define DHTTYPE DHT11
// Declare un objet de type DHT
// Il faut passer en parametre du constructeur
// de l'objet la broche et le type de capteur
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 ecranOLED(128, 64, &Wire, -1);
//Pins où on va connecter l'écran (RS, E, D4, D5, D6, D7)
// LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
const int pinSpray = 7;
const int pinFan = 13;
const int pinLigth = 5;
const bool fanOn = HIGH;
const bool fanOff = !fanOn;
const bool sprayOn = HIGH;
const bool sprayOff = !fanOn;
const bool ligthOn = HIGH;
const bool ligthOff = !ligthOn;
const int SecondInOneminute = 59;
const int maxMinutes = 59;
const int maxHours = 23;
const int hourLightOn = 7;
const int hourLightOff = 19;
int hours;
int minutes;
int seconds;
const int limitHumidity = 60;
int16_t couleur = SSD1306_WHITE;

const char* ssid = "************";
const char* password = "************************";
const char* serverUrl = "http://localhost:3001";
void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println(APP_NAME);
    WiFi.mode(WIFI_STA);  //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  dht.begin();
  // initWifi();
  initScreen();
  hours = 12;
  minutes = 00;
  seconds = 00;
}

float temperature = -1000;
float humidity = -100;

// ----- voir serial read pour expliciter un ordre
void loop() {
  ecranOLED.clearDisplay();
  updateTime();

  float aValue = dht.readHumidity();

  if (aValue > (limitHumidity * 0.85)) {
    while (aValue > (limitHumidity * 0.90)) {
      digitalWrite(pinFan, fanOn);
    }
    digitalWrite(pinFan, fanOff);
  } else if (aValue < (limitHumidity * 1.15)) {
    while (aValue > (limitHumidity * 1.20)) {
      digitalWrite(pinSpray, sprayOn);
    }
    digitalWrite(pinSpray, sprayOff);
  }

if(seconds % 5 == 0){
    Serial.print("Data send");
  sendDataToServer(temperature, humidity);
}
  updateHumidity(aValue);
  updateTemperature(dht.readTemperature());

  ecranOLED.display();
  // if (hours == hourLightOn & minutes == 0) {
  //   digitalWrite(pinLigth, ligthOn);
  // } else if (hours == hourLightOff & minutes == 0) {
  //   digitalWrite(pinLigth, ligthOff);
  // }
  delay(1000);
}
void updateTemperature(float temp) {
  ecranOLED.setCursor(0, 40);        //Place le pointeur sur la colonne 0, ligne 0
  ecranOLED.write("Temperature: ");  //écrit le message
  ecranOLED.print(temp);             //écrit le message
  ecranOLED.print((char)247);
  ecranOLED.print("C");
  Serial.print("Nouvelle temperature: ");
  Serial.println(temp);
}

void updateHumidity(float hum) {
  ecranOLED.setCursor(0, 30);
  ecranOLED.write("Humidite: ");  //écrit le message  //Place le pointeur sur la colonne 0, ligne 1
  ecranOLED.print(hum);           //écrit le message
  ecranOLED.write("%");
  Serial.print("Nouveau taux: ");
  Serial.println(hum);
}

void updateTime() {
  seconds++;
  Serial.print(hours);
  Serial.print(":");
  Serial.println(minutes);
  if (seconds == SecondInOneminute) {
    if (minutes < maxMinutes) {
      minutes = minutes + 1;
    } else {
      minutes = 00;
      if (hours < maxHours) {
        hours = hours + 1;
      } else {
        hours = 00;
      }
    }
    seconds = 0;
  }
  ecranOLED.setCursor(52, 1);
  timeFormat(hours);
  ecranOLED.setCursor(63, 1);
  if (seconds % 2 == 0) {
    ecranOLED.write(":");
  } else {
    ecranOLED.write(" ");
  }
  timeFormat(minutes);
  ecranOLED.setCursor(67, 1);
  ecranOLED.write("    ");
}

void timeFormat(float time) {
  if (time < 10) {
    ecranOLED.write("0");
    ecranOLED.print(floor(time), 0);
  } else {
    ecranOLED.print(floor(time), 0);
  }
}

void sendDataToServer(float temperature, float humidity) {
  HTTPClient http;

  String url = String(serverUrl) + "/data";
  String data = "temperature=" + String(temperature) + "&humidity=" + String(humidity);

  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = http.POST(data);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Erreur lors de l'envoi des données au serveur. Code d'erreur : ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void initScreen() {
  ecranOLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  ecranOLED.setTextSize(1);
  ecranOLED.setTextColor(WHITE);
  ecranOLED.clearDisplay();
  ecranOLED.setCursor(6, 0);
  ecranOLED.print(APP_NAME);
  ecranOLED.display();  // pinMode(pinFan, OUTPUT);
  digitalWrite(pinFan, fanOff);
  pinMode(pinLigth, OUTPUT);
  digitalWrite(pinLigth, ligthOff);
  delay(3000);
  ecranOLED.clearDisplay();
}
