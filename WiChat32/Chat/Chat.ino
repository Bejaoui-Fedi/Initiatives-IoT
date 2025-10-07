#include <WiFi.h>
#include <WebServer.h>

// 🌐 CONFIGURATION WIFI (remplacez par vos identifiants)
const char* ssid = "ESP32_Test";        // Nom de votre WiFi
const char* password = "12345678"; // Mot de passe de votre WiFi

// Serveur Web sur le port 80
WebServer server(80);

// Stockage des messages (max 50)
String messages[50];
int msgIndex = 0;

// Page HTML avec style hacker dark (identique)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>🔥 ESP32 Hacker Chat</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Fira+Code:wght@300;400;500;700&display=swap');
    
    * { 
      box-sizing: border-box; 
      margin: 0; 
      padding: 0; 
    }
    
    body { 
      font-family: 'Fira Code', 'Courier New', monospace;
      background: #0d1117;
      color: #00ff00;
      height: 100vh;
      overflow: hidden;
      position: relative;
    }
    
    /* Matrix effect background */
    body::before {
      content: "";
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: 
        linear-gradient(90deg, transparent 98%, #00ff0020 100%),
        linear-gradient(180deg, transparent 98%, #00ff0020 100%);
      background-size: 20px 20px;
      pointer-events: none;
      z-index: 1;
    }
    
    .container {
      height: 100vh;
      display: flex;
      flex-direction: column;
      position: relative;
      z-index: 2;
      background: rgba(13, 17, 23, 0.95);
      backdrop-filter: blur(5px);
    }
    
    .header {
      background: linear-gradient(45deg, #0d1117, #161b22);
      border-bottom: 2px solid #00ff00;
      padding: 15px;
      text-align: center;
      box-shadow: 0 0 20px #00ff0050;
    }
    
    .header h1 {
      color: #00ff00;
      font-size: 24px;
      text-shadow: 0 0 10px #00ff00;
      animation: glow 2s ease-in-out infinite alternate;
    }
    
    @keyframes glow {
      from { text-shadow: 0 0 10px #00ff00; }
      to { text-shadow: 0 0 20px #00ff00, 0 0 30px #00ff00; }
    }
    
    /* Login Screen */
    .login-screen {
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      height: 100vh;
      background: radial-gradient(ellipse at center, #0d1117 0%, #000000 100%);
    }
    
    .login-box {
      background: rgba(22, 27, 34, 0.9);
      border: 2px solid #00ff00;
      border-radius: 10px;
      padding: 40px;
      text-align: center;
      box-shadow: 0 0 30px #00ff0030;
      animation: pulse 3s infinite;
    }
    
    @keyframes pulse {
      0%, 100% { box-shadow: 0 0 30px #00ff0030; }
      50% { box-shadow: 0 0 40px #00ff0060; }
    }
    
    .login-box h2 {
      color: #00ff00;
      margin-bottom: 20px;
      font-size: 28px;
    }
    
    .login-box p {
      color: #8b949e;
      margin-bottom: 30px;
      font-size: 14px;
    }
    
    /* Chat Interface */
    .chat-interface {
      display: none;
      flex: 1;
      flex-direction: column;
    }
    
    .user-info {
      background: #161b22;
      padding: 10px 20px;
      border-bottom: 1px solid #21262d;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    
    .user-info .username {
      color: #00ff00;
      font-weight: bold;
    }
    
    .user-info .status {
      color: #8b949e;
      font-size: 12px;
    }
    
    #chatbox {
      flex: 1;
      overflow-y: auto;
      padding: 20px;
      background: #0d1117;
    }
    
    .message {
      background: rgba(22, 27, 34, 0.8);
      border-left: 3px solid #00ff00;
      margin: 10px 0;
      padding: 15px;
      border-radius: 0 8px 8px 0;
      animation: slideIn 0.3s ease-out;
    }
    
    @keyframes slideIn {
      from { transform: translateX(-100%); opacity: 0; }
      to { transform: translateX(0); opacity: 1; }
    }
    
    .message-header {
      color: #00ff00;
      font-weight: bold;
      font-size: 14px;
      margin-bottom: 8px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    
    .message-time {
      color: #8b949e;
      font-size: 11px;
      font-weight: normal;
    }
    
    .message-content {
      color: #f0f6fc;
      line-height: 1.4;
      word-wrap: break-word;
    }
    
    .input-section {
      background: #161b22;
      border-top: 2px solid #00ff00;
      padding: 20px;
    }
    
    .input-container {
      display: flex;
      gap: 10px;
      align-items: center;
    }
    
    input[type="text"] {
      flex: 1;
      background: #0d1117;
      border: 2px solid #21262d;
      color: #00ff00;
      padding: 12px 15px;
      border-radius: 6px;
      font-family: 'Fira Code', monospace;
      font-size: 14px;
      transition: all 0.3s;
    }
    
    input[type="text"]:focus {
      outline: none;
      border-color: #00ff00;
      box-shadow: 0 0 10px #00ff0030;
    }
    
    input[type="text"]::placeholder {
      color: #8b949e;
    }
    
    button {
      background: linear-gradient(45deg, #238636, #2ea043);
      color: white;
      border: none;
      padding: 12px 20px;
      border-radius: 6px;
      font-family: 'Fira Code', monospace;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.3s;
      text-transform: uppercase;
    }
    
    button:hover {
      background: linear-gradient(45deg, #2ea043, #238636);
      box-shadow: 0 0 15px #00ff0040;
    }
    
    button:active {
      transform: scale(0.98);
    }
    
    .login-btn {
      background: linear-gradient(45deg, #1f6feb, #388bfd);
      padding: 15px 30px;
      font-size: 16px;
    }
    
    .login-btn:hover {
      background: linear-gradient(45deg, #388bfd, #1f6feb);
    }
    
    /* Network info */
    .network-info {
      background: #161b22;
      color: #8b949e;
      padding: 5px 20px;
      font-size: 11px;
      text-align: center;
      border-bottom: 1px solid #21262d;
    }
    
    /* Scrollbar */
    #chatbox::-webkit-scrollbar {
      width: 8px;
    }
    
    #chatbox::-webkit-scrollbar-track {
      background: #161b22;
    }
    
    #chatbox::-webkit-scrollbar-thumb {
      background: #00ff00;
      border-radius: 4px;
    }
    
    .welcome-msg {
      text-align: center;
      color: #8b949e;
      font-style: italic;
      margin: 50px 0;
    }
  </style>
</head>
<body>
  <!-- Login Screen -->
  <div id="loginScreen" class="login-screen">
    <div class="login-box">
      <h2>🔒 ACCÈS SÉCURISÉ</h2>
      <p>Entrez votre nom d'utilisateur pour accéder au chat</p>
      <input type="text" id="usernameInput" placeholder="Votre nom (ex: Neo, Trinity...)" maxlength="15" />
      <br><br>
      <button class="login-btn" onclick="login()">🚀 SE CONNECTER</button>
    </div>
  </div>

  <!-- Chat Interface -->
  <div id="chatInterface" class="chat-interface">
    <div class="header">
      <h1>⚡ HACKER CHAT ESP32 ⚡</h1>
    </div>
    
    <div class="network-info" id="networkInfo">
      🌐 Connecté au réseau WiFi • ESP32 en ligne
    </div>
    
    <div class="user-info">
      <div>
        <span class="username" id="currentUser"></span>
        <span style="color: #8b949e;"> • En ligne</span>
      </div>
      <div class="status" id="connectionStatus">Connecté</div>
    </div>
    
    <div id="chatbox">
      <div class="welcome-msg">
        🔥 Bienvenue dans le chat hacker ! Messages chargement... 🔥
      </div>
    </div>
    
    <div class="input-section">
      <div class="input-container">
        <input type="text" id="messageInput" placeholder="Tapez votre message..." maxlength="200" />
        <button onclick="sendMessage()">📤 SEND</button>
      </div>
    </div>
  </div>

  <script>
    let currentUsername = '';
    let isLoggedIn = false;
    
    function login() {
      const username = document.getElementById('usernameInput').value.trim();
      
      if (username === '') {
        alert('⚠️ Nom d\'utilisateur requis !');
        return;
      }
      
      if (username.length < 2) {
        alert('⚠️ Le nom doit contenir au moins 2 caractères !');
        return;
      }
      
      currentUsername = username;
      isLoggedIn = true;
      
      document.getElementById('loginScreen').style.display = 'none';
      document.getElementById('chatInterface').style.display = 'flex';
      document.getElementById('currentUser').textContent = '👤 ' + currentUsername;
      
      document.getElementById('messageInput').focus();
      refreshChat();
    }
    
    function sendMessage() {
      const messageInput = document.getElementById('messageInput');
      const message = messageInput.value.trim();
      
      if (!isLoggedIn || message === '') return;
      
      fetch('/send?name=' + encodeURIComponent(currentUsername) + '&msg=' + encodeURIComponent(message))
        .then(response => {
          if (response.ok) {
            messageInput.value = '';
            document.getElementById('connectionStatus').textContent = 'Envoyé ✓';
            setTimeout(() => {
              document.getElementById('connectionStatus').textContent = 'En ligne';
            }, 2000);
            setTimeout(refreshChat, 100);
          }
        })
        .catch(error => {
          document.getElementById('connectionStatus').textContent = 'Erreur';
        });
    }
    
    function refreshChat() {
      if (!isLoggedIn) return;
      
      fetch('/messages')
        .then(response => response.text())
        .then(data => {
          const chatbox = document.getElementById('chatbox');
          const shouldScroll = chatbox.scrollTop + chatbox.clientHeight >= chatbox.scrollHeight - 50;
          
          if (data.trim() === '') {
            chatbox.innerHTML = '<div class="welcome-msg">🔥 Aucun message... Soyez le premier ! 🔥</div>';
          } else {
            chatbox.innerHTML = data;
          }
          
          if (shouldScroll) {
            chatbox.scrollTop = chatbox.scrollHeight;
          }
        })
        .catch(error => {
          document.getElementById('connectionStatus').textContent = 'Erreur réseau';
        });
    }
    
    document.getElementById('usernameInput').addEventListener('keypress', function(e) {
      if (e.key === 'Enter') login();
    });
    
    document.getElementById('messageInput').addEventListener('keypress', function(e) {
      if (e.key === 'Enter') sendMessage();
    });
    
    setInterval(refreshChat, 2000);
    
    window.onload = function() {
      document.getElementById('usernameInput').focus();
    };
  </script>
</body>
</html>
)rawliteral";

// Reste du code identique...
void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleSend() {
  if (server.hasArg("name") && server.hasArg("msg")) {
    String name = server.arg("name");
    String msg = server.arg("msg");
    
    name.trim();
    msg.trim();
    
    if (name.length() == 0 || msg.length() == 0) {
      server.send(400, "text/plain", "Données invalides");
      return;
    }
    
    if (name.length() > 15) name = name.substring(0, 15);
    if (msg.length() > 200) msg = msg.substring(0, 200);
    
    // Timestamp
    unsigned long now = millis() / 1000;
    String timeStr = String(now % 86400 / 3600) + ":" + 
                    String((now % 3600) / 60 < 10 ? "0" : "") + 
                    String((now % 3600) / 60) + ":" +
                    String(now % 60 < 10 ? "0" : "") + 
                    String(now % 60);
    
    String formattedMsg = "<div class='message'>"
                         "<div class='message-header'>"
                         "🔥 " + name + 
                         "<span class='message-time'>[" + timeStr + "]</span>"
                         "</div>"
                         "<div class='message-content'>" + msg + "</div>"
                         "</div>";
    
    messages[msgIndex] = formattedMsg;
    msgIndex = (msgIndex + 1) % 50;
    
    Serial.println("💬 [" + timeStr + "] " + name + ": " + msg);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Paramètres manquants");
  }
}

void handleMessages() {
  String allMessages = "";
  
  for (int i = 0; i < 50; i++) {
    int index = (msgIndex + i) % 50;
    if (messages[index] != "") {
      allMessages += messages[index];
    }
  }
  
  server.send(200, "text/html", allMessages);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n🔥 ESP32 HACKER CHAT - MODE STATION 🔥");
  Serial.println("🌐 Connexion au WiFi...");
  
  // 🌐 MODE STATION : Connexion au WiFi existant
  WiFi.begin(ssid, password);
  
  // Attendre la connexion (timeout 20 secondes)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ CONNECTÉ AU WIFI !");
    Serial.println("📡 SSID: " + String(ssid));
    Serial.println("🌐 Adresse IP: " + WiFi.localIP().toString());
    Serial.println("🔗 Ouvrez: http://" + WiFi.localIP().toString());
    Serial.println("📶 Signal: " + String(WiFi.RSSI()) + " dBm");
  } else {
    Serial.println("\n❌ ÉCHEC DE CONNEXION WIFI !");
    Serial.println("🔴 Vérifiez vos identifiants WiFi");
    Serial.println("🔄 Redémarrage dans 5 secondes...");
    delay(5000);
    ESP.restart();
  }
  
  // Démarrer le serveur web
  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.on("/messages", handleMessages);
  
  server.begin();
  Serial.println("🚀 Serveur web démarré !");
  Serial.println("🔒 Chat sécurisé prêt !");
}

void loop() {
  server.handleClient();
  
  // Vérifier la connexion WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi déconnecté ! Reconnexion...");
    WiFi.begin(ssid, password);
  }
}
