#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>

// Configuration WiFi
const char* ssid = "ESP32_Test";
const char* password = "12345678";

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Serveur web
WebServer server(80);

// Variables globales partagées entre serial, web et LCD
String message = "";
int key = 0;
int mode = 0;  // 0=menu, 1=chiffrer, 2=déchiffrer
String result = "";
int step = 0;  // 0=choix mode, 1=saisie message, 2=saisie clé, 3=résultat

void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("=== METHODE CESAR ===");
  
  delay(2000);
  lcd.clear();
  lcd.print("Connexion WiFi...");
  
  WiFi.begin(ssid, password);
  Serial.print("Connexion au WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connecté!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
  
  lcd.clear();
  lcd.print("WiFi Connecte!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.on("/mode", HTTP_POST, handleMode);
  server.on("/message", HTTP_POST, handleMessage);
  server.on("/key", HTTP_POST, handleKey);
  server.on("/reset", HTTP_POST, handleReset);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Serveur web démarré");
  
  delay(3000);
  
  // Initialisation du menu
  resetToMenu();
}

void loop() {
  server.handleClient();
  
  // Lecture des données depuis le moniteur série
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    handleSerialInput(input);
  }
}

void handleSerialInput(String input) {
  if (step == 0) {  // Choix du mode
    if (input == "1") {
      setMode(1);
    } else if (input == "2") {
      setMode(2);
    } else {
      Serial.println("Option invalide. Tapez '1' pour Chiffrer ou '2' pour Déchiffrer.");
    }
  } else if (step == 1) {  // Saisie du message
    setMessage(input);
  } else if (step == 2) {  // Saisie de la clé
    setKey(input.toInt());
  }
}

void resetToMenu() {
  mode = 0;
  step = 0;
  message = "";
  key = 0;
  result = "";
  
  // Affichage LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TAPEZ '1' CHIFFRER");
  lcd.setCursor(0, 1);
  lcd.print("OU '2' DECHIFFRER");
  
  // Affichage série
  Serial.println("\nTapez '1' pour Chiffrer ou '2' pour Déchiffrer, suivi de 'Enter' :");
}

void setMode(int newMode) {
  mode = newMode;
  step = 1;
  
  // Affichage LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(mode == 1 ? "MODE:CHIFFREMENT" : "MODE:DECHIFFRMNT");
  lcd.setCursor(0, 1);
  lcd.print("ENTREZ MESSAGE:");
  
  // Affichage série
  Serial.println(mode == 1 ? "Mode : Chiffrement" : "Mode : Déchiffrement");
  Serial.println("Entrez un message (terminé par 'Enter') :");
}

void setMessage(String newMessage) {
  message = newMessage;
  step = 2;
  
  // Affichage LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MESSAGE RECU:");
  lcd.setCursor(0, 1);
  if (message.length() > 16) {
    lcd.print(message.substring(0, 16));
  } else {
    lcd.print(message);
  }
  
  // Affichage série
  Serial.println("Message reçu : " + message);
  Serial.println("Entrez une clé (nombre entier) :");
}

void setKey(int newKey) {
  key = newKey;
  step = 3;
  
  // Traitement
  int processKey = (mode == 2) ? -key : key;
  result = encryptCesar(message, processKey);
  
  // Affichage LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(mode == 2 ? "DECHIFFRE:" : "CHIFFRE:");
  lcd.setCursor(0, 1);
  if (result.length() > 16) {
    lcd.print(result.substring(0, 16));
  } else {
    lcd.print(result);
  }
  
  // Affichage série
  Serial.println("Clé : " + String(key));
  Serial.println(mode == 2 ? "Message déchiffré : " + result : "Message chiffré : " + result);
  Serial.println("Appuyez sur Enter pour continuer...");
}

// Page d'accueil - Interface étape par étape
void handleRoot() {
  String html = R"(<!DOCTYPE html><html><head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>César ESP32 - Moniteur</title>
<style>
body{font-family:'Courier New',monospace;max-width:500px;margin:20px auto;padding:20px;background:#000;color:#0f0}
.terminal{background:#111;padding:20px;border-radius:10px;border:2px solid #0f0;margin:10px 0}
.lcd{background:#004400;color:#0f0;padding:15px;border-radius:5px;margin:10px 0;font-weight:bold;text-align:center}
.lcd-line{display:block;margin:2px 0}
input,button{background:#003300;color:#0f0;border:2px solid #0f0;padding:10px;margin:5px;border-radius:5px;font-family:'Courier New',monospace;font-size:14px}
button{cursor:pointer;background:#006600}
button:hover{background:#009900}
.result{background:#004400;padding:15px;border-radius:5px;margin:10px 0;border:1px solid #0f0}
.step{color:#ff0;font-weight:bold}
input{width:200px}
</style>
</head><body>
<div class='terminal'>
<h1 style='text-align:center;color:#0f0'>🔐 CÉSAR ESP32 - MONITEUR</h1>

<div class='lcd'>
<div class='lcd-line'>)";

  // Affichage de l'état actuel du LCD
  if (step == 0) {
    html += "TAPEZ '1' CHIFFRER</div><div class='lcd-line'>OU '2' DECHIFFRER";
  } else if (step == 1) {
    html += (mode == 1 ? "MODE:CHIFFREMENT" : "MODE:DECHIFFRMNT");
    html += "</div><div class='lcd-line'>ENTREZ MESSAGE:";
  } else if (step == 2) {
    html += "MESSAGE RECU:</div><div class='lcd-line'>";
    html += (message.length() > 16 ? message.substring(0, 16) : message);
  } else if (step == 3) {
    html += (mode == 2 ? "DECHIFFRE:" : "CHIFFRE:");
    html += "</div><div class='lcd-line'>";
    html += (result.length() > 16 ? result.substring(0, 16) : result);
  }
  
  html += "</div></div>";
  
  // Interface correspondant à l'étape actuelle
  if (step == 0) {
    html += R"(<p class='step'>ÉTAPE 1: CHOISISSEZ LE MODE</p>
<form action='/mode' method='POST' style='text-align:center'>
<button type='submit' name='mode' value='1'>1 - CHIFFRER</button>
<button type='submit' name='mode' value='2'>2 - DÉCHIFFRER</button>
</form>)";
  } else if (step == 1) {
    html += R"(<p class='step'>ÉTAPE 2: ENTREZ LE MESSAGE</p>
<form action='/message' method='POST' style='text-align:center'>
<input type='text' name='message' placeholder='Votre message...' required autofocus>
<button type='submit'>VALIDER</button>
</form>)";
  } else if (step == 2) {
    html += R"(<p class='step'>ÉTAPE 3: ENTREZ LA CLÉ</p>
<form action='/key' method='POST' style='text-align:center'>
<input type='number' name='key' placeholder='Clé (nombre)' required autofocus>
<button type='submit'>TRAITER</button>
</form>)";
  } else if (step == 3) {
    html += "<div class='result'><b>RÉSULTAT COMPLET:</b><br>" + result + "</div>";
    html += R"(<form action='/reset' method='POST' style='text-align:center'>
<button type='submit'>NOUVEAU TRAITEMENT</button>
</form>)";
  }
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Traitement du choix de mode
void handleMode() {
  if (server.hasArg("mode") && step == 0) {
    int selectedMode = server.arg("mode").toInt();
    setMode(selectedMode);
    
    Serial.println("=== Mode sélectionné via WEB ===");
    Serial.println("Mode: " + String(selectedMode == 1 ? "Chiffrement" : "Déchiffrement"));
  }
  
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// Traitement du message
void handleMessage() {
  if (server.hasArg("message") && step == 1) {
    String webMessage = server.arg("message");
    setMessage(webMessage);
    
    Serial.println("=== Message reçu via WEB ===");
    Serial.println("Message: " + webMessage);
  }
  
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// Traitement de la clé
void handleKey() {
  if (server.hasArg("key") && step == 2) {
    int webKey = server.arg("key").toInt();
    setKey(webKey);
    
    Serial.println("=== Clé reçue via WEB ===");
    Serial.println("Clé: " + String(webKey));
    Serial.println("Résultat: " + result);
  }
  
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// Reset vers le menu
void handleReset() {
  resetToMenu();
  
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// Page 404
void handleNotFound() {
  server.send(404, "text/plain", "404 - Page non trouvée");
}

// Fonction César
String encryptCesar(String msg, int shift) {
  String result = "";
  shift = shift % 26;

  for (int i = 0; i < msg.length(); i++) {
    char c = msg[i];
    if (isAlpha(c)) {
      char base = isLowerCase(c) ? 'a' : 'A';
      c = (c - base + shift + 26) % 26 + base;
    }
    result += c;
  }
  return result;
}

bool isAlpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool isLowerCase(char c) {
  return c >= 'a' && c <= 'z';
}
