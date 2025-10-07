 #include <LiquidCrystal_I2C.h>

// Initialisation de l'écran LCD I2C
// Adresse I2C par défaut : 0x27 (peut être 0x3F sur certains modules)
// Format: LiquidCrystal_I2C(adresse, colonnes, lignes)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables pour stocker le message, la clé et le mode
String message = "";
int key = 0;
int mode = 0; // 1 pour chiffrement, 2 pour déchiffrement

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);
  
  // Initialisation de l'écran LCD I2C
  lcd.init();
  lcd.backlight(); // Allumer le rétroéclairage
  
  lcd.clear();
  lcd.print("=== METHODE CESAR ===");
  
  delay(2000); // Attendre un moment avant de demander l'option
  lcd.clear();
  
  Serial.println("Tapez '1' pour Chiffrer ou '2' pour Déchiffrer, suivi de 'Enter' :");
  
  lcd.setCursor(0, 0);
  lcd.print("TAPEZ '1' CHIFFRER");
  lcd.setCursor(0, 1);
  lcd.print("OU '2' DECHIFFRER");
 
  delay(2000); // Attendre un moment avant de demander l'option
}

void loop() {
  // Lecture des données depuis le moniteur série
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Lire jusqu'à la fin de la ligne
    input.trim(); // Supprimer les espaces inutiles

    if (mode == 0) { // Si le mode n'est pas défini
      if (input == "1") {
        mode = 1;
        Serial.println("Mode : Chiffrement");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MODE:CHIFFREMENT");
        Serial.println("Entrez un message (terminé par 'Enter') :");
        lcd.setCursor(0, 1);
        lcd.print("ENTREZ MESSAGE:");
      } else if (input == "2") {
        mode = 2;
        Serial.println("Mode : Déchiffrement");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MODE:DECHIFFRMNT");
        Serial.println("Entrez un message (terminé par 'Enter') :");
        lcd.setCursor(0, 1);
        lcd.print("ENTREZ MESSAGE:");
      } else {
        Serial.println("Option invalide. Tapez '1' pour Chiffrer ou '2' pour Déchiffrer.");
      }
    } else if (message == "") { // Si le message n'est pas encore entré
      message = input;
      Serial.println("Entrez une clé (nombre entier) :");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MESSAGE RECU:");
      lcd.setCursor(0, 1);
      // Afficher seulement les 16 premiers caractères du message sur LCD
      if (message.length() > 16) {
        lcd.print(message.substring(0, 16));
      } else {
        lcd.print(message);
      }
    } else if (key == 0) { // Si la clé n'est pas encore entrée
      key = input.toInt();
      if (mode == 2) key = -key; // Si déchiffrement, inverser la clé
      String result = encryptCesar(message, key);
      
      // Affichage sur le LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(mode == 2 ? "DECHIFFRE:" : "CHIFFRE:");
      lcd.setCursor(0, 1);
      // Afficher seulement les 16 premiers caractères du résultat sur LCD
      if (result.length() > 16) {
        lcd.print(result.substring(0, 16));
      } else {
        lcd.print(result);
      }

      // Affichage complet dans le terminal
      Serial.println(mode == 2 ? "Message déchiffré : " + result : "Message chiffré : " + result);

      // Réinitialiser pour un autre essai
      message = "";
      key = 0;
      mode = 0;
      
      delay(3000); // Laisser du temps pour lire le résultat
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TAPEZ '1' CHIFFRER");
      lcd.setCursor(0, 1);
      lcd.print("OU '2' DECHIFFRER");
      
      Serial.println("\nTapez '1' pour Chiffrer ou '2' pour Déchiffrer, suivi de 'Enter' :");
    }
  }
}

// Fonction de chiffrement/déchiffrement de César
String encryptCesar(String msg, int shift) {
  String result = "";
  shift = shift % 26; // Réduction de la clé pour éviter des dépassements inutiles

  for (int i = 0; i < msg.length(); i++) {
    char c = msg[i];
    if (isAlpha(c)) {
      char base = isLowerCase(c) ? 'a' : 'A'; // Détermine si c'est une lettre minuscule ou majuscule
      c = (c - base + shift + 26) % 26 + base; // Calcul du décalage avec prise en charge des valeurs négatives
    }
    result += c; // Ajouter le caractère (chiffré ou non) au résultat
  }
  return result;
}

// Vérifie si un caractère est une lettre alphabétique
bool isAlpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Vérifie si un caractère est en minuscule
bool isLowerCase(char c) {
  return c >= 'a' && c <= 'z';
}
