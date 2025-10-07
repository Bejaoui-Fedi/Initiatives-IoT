#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Configuration des pins
#define DHT_PIN 4           // Pin de données du DHT22
#define DHT_TYPE DHT22      // Type de capteur DHT
#define LED_VERTE 2         // LED verte (état normal)
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
unsigned long derniereLecture = 0;
const unsigned long intervalLecture = 2000;  // Lecture toutes les 2 secondes

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);
  Serial.println("Démarrage du système de monitoring...");
  
  // Configuration des pins LED
  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);
  
  // Test initial des LEDs
  digitalWrite(LED_VERTE, HIGH);
  digitalWrite(LED_ROUGE, HIGH);
  delay(1000);
  digitalWrite(LED_VERTE, LOW);
  digitalWrite(LED_ROUGE, LOW);
  
  // Initialisation du capteur DHT22
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
  lcd.print("Initialisation..");
  delay(2000);
  lcd.clear();
  
  Serial.println("Système prêt !");
}

void loop() {
  // Lecture des données toutes les 2 secondes
  if (millis() - derniereLecture >= intervalLecture) {
    lireCapteurs();
    verifierAlertes();
    afficherDonnees();
    gererLEDs();
    
    derniereLecture = millis();
  }
  
  delay(100);  // Petite pause pour éviter la surcharge du processeur
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
  Serial.println("%");
}

void verifierAlertes() {
  // Vérification des seuils
  alerte = false;
  
  if (temperature < TEMP_MIN || temperature > TEMP_MAX) {
    alerte = true;
    Serial.println("⚠️ ALERTE TEMPÉRATURE !");
  }
  
  if (humidite < HUMID_MIN || humidite > HUMID_MAX) {
    alerte = true;
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
  
  // Première ligne : Température
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
  
  // Deuxième ligne : Humidité
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(humidite, 1);
  lcd.print("%");
  
  // Affichage des seuils dépassés
  if (temperature < TEMP_MIN || temperature > TEMP_MAX) {
    lcd.setCursor(8, 1);
    lcd.print(" T!");
  }
  if (humidite < HUMID_MIN || humidite > HUMID_MAX) {
    lcd.setCursor(11, 1);
    lcd.print(" H!");
  }
}

void gererLEDs() {
  if (alerte) {
    // Mode alerte : LED rouge clignotante, LED verte éteinte
    digitalWrite(LED_VERTE, LOW);
    
    // Clignotement de la LED rouge
    static unsigned long dernierClignotement = 0;
    static bool etatLEDRouge = false;
    
    if (millis() - dernierClignotement >= 500) {  // Clignotement toutes les 500ms
      etatLEDRouge = !etatLEDRouge;
      digitalWrite(LED_ROUGE, etatLEDRouge ? HIGH : LOW);
      dernierClignotement = millis();
    }
  } else {
    // Mode normal : LED verte allumée, LED rouge éteinte
    digitalWrite(LED_VERTE, HIGH);
    digitalWrite(LED_ROUGE, LOW);
  }
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
  Serial.println("================================\n");
}

// Fonction de diagnostic (optionnelle)
void diagnostic() {
  Serial.println("\n=== DIAGNOSTIC DU SYSTÈME ===");
  
  // Test des LEDs
  Serial.println("Test des LEDs...");
  digitalWrite(LED_VERTE, HIGH);
  digitalWrite(LED_ROUGE, HIGH);
  delay(1000);
  digitalWrite(LED_VERTE, LOW);
  digitalWrite(LED_ROUGE, LOW);
  
  // Test de l'écran LCD
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
  
  Serial.println("=============================\n");
}
