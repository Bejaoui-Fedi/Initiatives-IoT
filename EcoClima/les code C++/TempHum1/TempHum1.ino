 #include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Configuration WiFi
const char* ssid = "ESP32_Test";
const char* password = "12345678";

// Configuration Supabase
const char* supabaseUrl = "******************";
const char* supabaseKey = "*********************************";
const char* tableName = "sensor_data";

// Configuration des pins
#define DHT_PIN 4           // Pin de données du DHT22
#define DHT_TYPE DHT22      // Type de capteur DHT
#define LED_ROUGE 5         // LED rouge (alerte)

// Configuration I2C pour LCD
#define SDA_PIN 21          // Pin SDA par défaut de l'ESP32
#define SCL_PIN 22          // Pin SCL par défaut de l'ESP32

// Seuils d'alerte
#define TEMP_MIN 15.0       // Température minimale (°C)
#define TEMP_MAX 30.0       // Température maximale (°C)
#define HUMID_MIN 30.0      // Humidité minimale (%)
#define HUMID_MAX 70.0      // Humidité maximale (%)

// Initialisation des objets
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adresse I2C 0x27, écran 16x2

// Variables globales
float temperature = 0;
float humidite = 0;
bool alerte = false;
bool temperatureAlert = false;
bool humidityAlert = false;
unsigned long derniereLecture = 0;
unsigned long derniereEnvoi = 0;
const unsigned long intervalLecture = 2000;     // Lecture toutes les 2 secondes
const unsigned long intervalEnvoi = 30000;      // Envoi vers Supabase toutes les 30 secondes

// Variables de connexion
bool wifiConnected = false;

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);
  Serial.println("Démarrage du système de monitoring...");
  
  // Configuration des pins LED
  pinMode(LED_ROUGE, OUTPUT);
  
  // Test initial des LEDs
  digitalWrite(LED_ROUGE, HIGH);
  delay(1000);
  digitalWrite(LED_ROUGE, LOW);
  dht.begin();
  Serial.println("Capteur DHT22 initialisé");
  
  // Initialisation de l'écran LCD I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  
  // Message de démarrage sur LCD
  lcd.setCursor(0, 0);
  lcd.print("Monitoring T/H");
  lcd.setCursor(0, 1);
  lcd.print("Connexion WiFi..");
  
  // Connexion WiFi
  connectWiFi();
  
  Serial.println("Système prêt !");
}

void loop() {
  // Vérifier la connexion WiFi
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("WiFi déconnecté, tentative de reconnexion...");
    connectWiFi();
  } else {
    wifiConnected = true;
  }
  
  // Lecture des données toutes les 2 secondes
  if (millis() - derniereLecture >= intervalLecture) {
    lireCapteurs();
    verifierAlertes();
    afficherDonnees();
    gererLEDs();
    
    derniereLecture = millis();
  }
  
  // Envoi vers Supabase toutes les 30 secondes
  if (wifiConnected && (millis() - derniereEnvoi >= intervalEnvoi)) {
    envoyerVersSupabase();
    derniereEnvoi = millis();
  }
  
  delay(100);
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connexion au WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connecté !");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
    
    // Affichage sur LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi connecte");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println();
    Serial.println("Échec de connexion WiFi");
    wifiConnected = false;
    
    // Affichage sur LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi ERREUR");
    lcd.setCursor(0, 1);
    lcd.print("Mode local");
    delay(2000);
  }
}

void lireCapteurs() {
  // Lecture de l'humidité et de la température
  humidite = dht.readHumidity();
  temperature = dht.readTemperature();
  
  // Vérification des erreurs de lecture
  if (isnan(humidite) || isnan(temperature)) {
    Serial.println("Erreur de lecture du capteur DHT22 !");
    lcd.setCursor(0, 0);
    lcd.print("Erreur capteur  ");
    lcd.setCursor(0, 1);
    lcd.print("DHT22           ");
    return;
  }
  
  // Affichage dans le moniteur série
  Serial.print("Température: ");
  Serial.print(temperature);
  Serial.print("°C, Humidité: ");
  Serial.print(humidite);
  Serial.print("% - WiFi: ");
  Serial.println(wifiConnected ? "OK" : "NOK");
}

void verifierAlertes() {
  // Vérification des seuils
  alerte = false;
  temperatureAlert = false;
  humidityAlert = false;
  
  if (temperature < TEMP_MIN || temperature > TEMP_MAX) {
    alerte = true;
    temperatureAlert = true;
    Serial.println("⚠️ ALERTE TEMPÉRATURE !");
  }
  
  if (humidite < HUMID_MIN || humidite > HUMID_MAX) {
    alerte = true;
    humidityAlert = true;
    Serial.println("⚠️ ALERTE HUMIDITÉ !");
  }
  
  if (alerte) {
    Serial.println("État: ALERTE");
  } else {
    Serial.println("État: NORMAL");
  }
}

void afficherDonnees() {
  // Affichage sur l'écran LCD
  lcd.clear();
  
  // Première ligne : Température et état WiFi
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C ");
  
  // État sur la première ligne
  if (alerte) {
    lcd.print("ALERTE");
  } else {
    lcd.print("OK");
  }
  
  // Indicateur WiFi
  lcd.setCursor(15, 0);
  lcd.print(wifiConnected ? "*" : "X");
  
  // Deuxième ligne : Humidité
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(humidite, 1);
  lcd.print("%");
  
  // Affichage des seuils dépassés
  if (temperatureAlert) {
    lcd.setCursor(8, 1);
    lcd.print(" T!");
  }
  if (humidityAlert) {
    lcd.setCursor(11, 1);
    lcd.print(" H!");
  }
}

void gererLEDs() {
  if (alerte) {
    // Mode alerte : LED rouge clignotante
    
    // Clignotement de la LED rouge
    static unsigned long dernierClignotement = 0;
    static bool etatLEDRouge = false;
    
    if (millis() - dernierClignotement >= 500) {  // Clignotement toutes les 500ms
      etatLEDRouge = !etatLEDRouge;
      digitalWrite(LED_ROUGE, etatLEDRouge ? HIGH : LOW);
      dernierClignotement = millis();
    }
  } else {
    // Mode normal : LED rouge éteinte
    digitalWrite(LED_ROUGE, LOW);
  }
}

void envoyerVersSupabase() {
  if (!wifiConnected) {
    Serial.println("Pas de connexion WiFi, envoi impossible");
    return;
  }
  
  // Vérification des données
  if (isnan(temperature) || isnan(humidite)) {
    Serial.println("Données invalides, envoi annulé");
    return;
  }
  
  HTTPClient http;
  
  // Construction de l'URL
  String url = String(supabaseUrl) + "/rest/v1/" + tableName;
  
  // Configuration de la requête
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));
  http.addHeader("Prefer", "return=minimal");
  
  // Création du JSON manuellement
  String jsonData = "{";
  jsonData += "\"temperature\":" + String(temperature, 2) + ",";
  jsonData += "\"humidity\":" + String(humidite, 2) + ",";
  jsonData += "\"alert_status\":" + String(alerte ? "true" : "false") + ",";
  jsonData += "\"temperature_alert\":" + String(temperatureAlert ? "true" : "false") + ",";
  jsonData += "\"humidity_alert\":" + String(humidityAlert ? "true" : "false") + ",";
  jsonData += "\"device_id\":\"ESP32_DHT22\"";
  jsonData += "}";
  
  Serial.println("Envoi vers Supabase: " + jsonData);
  
  // Envoi de la requête POST
  int httpResponseCode = http.POST(jsonData);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Réponse Supabase: " + String(httpResponseCode));
    
    if (httpResponseCode == 201) {
      Serial.println("✅ Données envoyées avec succès !");
    } else {
      Serial.println("⚠️ Erreur lors de l'envoi: " + response);
    }
  } else {
    Serial.println("❌ Erreur de connexion: " + String(httpResponseCode));
  }
  
  http.end();
}

// Fonction utilitaire pour afficher les seuils configurés
void afficherConfiguration() {
  Serial.println("\n=== CONFIGURATION DU SYSTÈME ===");
  Serial.print("Température : ");
  Serial.print(TEMP_MIN);
  Serial.print("°C à ");
  Serial.print(TEMP_MAX);
  Serial.println("°C");
  Serial.print("Humidité : ");
  Serial.print(HUMID_MIN);
  Serial.print("% à ");
  Serial.print(HUMID_MAX);
  Serial.println("%");
  Serial.println("Supabase URL: " + String(supabaseUrl));
  Serial.println("================================\n");
}

// Fonction de diagnostic (optionnelle)
void diagnostic() {
  Serial.println("\n=== DIAGNOSTIC DU SYSTÈME ===");
  
  // Test des LEDs
  Serial.println("Test des LEDs...");
  digitalWrite(LED_ROUGE, HIGH);
  delay(1000);
  digitalWrite(LED_ROUGE, LOW);
  Serial.println("Test de l'écran LCD...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test LCD OK");
  delay(2000);
  
  // Test du capteur DHT22
  Serial.println("Test du capteur DHT22...");
  float testTemp = dht.readTemperature();
  float testHumid = dht.readHumidity();
  
  if (!isnan(testTemp) && !isnan(testHumid)) {
    Serial.println("Capteur DHT22 : OK");
  } else {
    Serial.println("Capteur DHT22 : ERREUR");
  }
  
  // Test de connexion Supabase
  Serial.println("Test de connexion Supabase...");
  if (wifiConnected) {
    // Test simple de ping vers Supabase
    HTTPClient http;
    http.begin(String(supabaseUrl) + "/rest/v1/");
    http.addHeader("apikey", supabaseKey);
    int code = http.GET();
    if (code > 0) {
      Serial.println("Connexion Supabase : OK");
    } else {
      Serial.println("Connexion Supabase : ERREUR");
    }
    http.end();
  } else {
    Serial.println("Pas de WiFi, impossible de tester Supabase");
  }
  
  Serial.println("=============================\n");
}
