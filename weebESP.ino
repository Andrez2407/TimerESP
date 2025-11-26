/*
  player_display.ino
  Lee /player.json desde LittleFS y muestra jugadores en un LCD I2C 16x2.
  Navegación con un encoder rotatorio (giro para cambiar jugador).
  Botón del encoder: recarga player.json desde LittleFS al pulsarlo.
*/

#include <Arduino.h>. //GPIO 22, 18, 25, 26
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "WebServer.h"
#include <TM1637Display.h>
#include "WebSocketsServer.h"

// --- CONFIGURACIÓN DE RED ---
const char* ssid = "ESP32_Webserver_Local";
const char* password = "password123";

// --- Sensor infrarrojo ---
#define SENSOR_IR 35  // LOW = detecta

// --- LEDs ---
#define ROJO1 13
#define ROJO2 12
#define ROJO3 14
#define ROJO4 27
#define VERDE1 26
#define VERDE2 25

// --------------------------- Ajustes ---------------------------
const char* playerFile = "/player.json";
const char* DATA_FILE = "/data.json";

// I2C LCD (ajusta la dirección si tu modulo usa otra, 0x27 o 0x3F son comunes)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- TM1637 Display ---
#define CLK_7SEG 18
#define DIO_7SEG 19
TM1637Display display(CLK_7SEG, DIO_7SEG);


// --- OBJETOS DE SERVIDOR ---
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);


// Pines del encoder
const int pinEncA = 32;
const int pinEncB = 33;
const int pinBtn = 4;  // botón del encoder (opcional)

// Encoder state
volatile long encoderPos = 0;  // posición absoluta
int currentIndex = 0;          // índice del jugador mostrado
int lastA = 0;

// Estructura jugador
struct Player {
  String Nombre;
  int numero;
  int record;
};
std::vector<Player> players;

int indexPlayerSelect = 0;


// ----------------------------------------------------------------------
//                        DECLARACIONES DE FUNCIONES
// ----------------------------------------------------------------------

String getContentType(String filename);
bool handleFileRead(String path);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

unsigned long previousMillisLed = 0;
const long intervaloLed = 500; // parpadeo cada 500ms

// Timing para debounce del botón
unsigned long lastBtnMillis = 0;
const unsigned long DEBOUNCE_MS = 50;

bool timerActivo = false;

// Tiempo del timer
unsigned long tiempoInicioTimer = 0;
const long duracionTimer = 5000; // 5 segundos

// ------------------------ Funciones LittleFS ------------------------
bool initLittleFS() {
  if (!LittleFS.begin(true)) {  // true -> format if mount fails
    Serial.println("Error montando LittleFS");
    return false;
  }
  Serial.println("LittleFS montado OK");
  return true;
}

void saveSampleIfMissing() {
  if (!LittleFS.exists(playerFile)) {
    Serial.println("player.json no existe: creando ejemplo inicial.");
    File f = LittleFS.open(playerFile, "w");
    if (!f) {
      Serial.println("ERROR al crear player.json");
      return;
    }
    // ejemplo
    const char* sample = R"(
      [
        {"Nombre":"Lionel Messi","numero":10,"record":404},
        {"Nombre":"Cristiano Ronaldo","numero":7,"record":404},
        {"Nombre":"Kylian Mbappe","numero":7,"record":404}
      ]
    )";
    f.print(sample);
    f.close();
  }
}

bool loadPlayers() {
  players.clear();

  if (!LittleFS.exists(playerFile)) {
    Serial.println("player.json no encontrado");
    return false;
  }

  File f = LittleFS.open(playerFile, "r");
  if (!f) {
    Serial.println("No se pudo abrir player.json");
    return false;
  }

  size_t size = f.size();
  if (size == 0) {
    Serial.println("player.json vacío");
    f.close();
    return false;
  }

  // Reservar doc con un margen razonable (ajusta si tu JSON es grande)
  DynamicJsonDocument doc(4096);
  DeserializationError err = deserializeJson(doc, f);
  f.close();

  if (err) {
    Serial.print("Error parseando JSON: ");
    Serial.println(err.c_str());
    return false;
  }

  if (!doc.is<JsonArray>()) {
    Serial.println("player.json no es un array");
    return false;
  }

  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject obj : arr) {
    Player p;
    p.Nombre = obj["Nombre"] | "";
    p.numero = obj["numero"] | 0;
    p.record = obj["record"] | 0;
    players.push_back(p);
  }

  Serial.printf("Cargados %d players\n", (int)players.size());
  return true;
}

// ----------------------- Funciones display ------------------------
void showNoPlayers() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("No players");
  lcd.setCursor(0, 1);
  lcd.print("player.json vacio");
}

void showPlayerAt(int idx) {
  if (players.size() == 0) {
    showNoPlayers();
    return;
  }
  if (idx < 0) idx = 0;
  if (idx >= (int)players.size()) idx = players.size() - 1;

  Player& p = players[idx];

  // Primera línea: nombre (recortar a 16)
  String line1 = p.Nombre;
  if(indexPlayerSelect == idx){
    line1 = "-> " + p.Nombre;
  }
  if (line1.length() > 16) line1 = line1.substring(0, 16);

  // Segunda línea: número y record
  String line2 = "#" + String(p.numero) + " R:" + String(p.record);
  if (line2.length() > 16) line2 = line2.substring(0, 16);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ----------------------- Encoder (polling sencillo) -----------------------
// Usamos lectura en loop() en vez de interrupciones para simplicidad con ESP32

void setupEncoderPins() {
  pinMode(pinEncA, INPUT_PULLUP);
  pinMode(pinEncB, INPUT_PULLUP);
  pinMode(pinBtn, INPUT_PULLUP);

  lastA = digitalRead(pinEncA);
}

// Lee cambios del encoder; actualiza encoderPos en consecuencia
void pollEncoder() {
  int a = digitalRead(pinEncA);
  int b = digitalRead(pinEncB);

  if (a != lastA) {  // cambio en A -> movimiento
    if (b != a) {
      encoderPos++;  // sentido horario
    } else {
      encoderPos--;  // sentido antihorario
    }
    lastA = a;
  }
}

// --------------------------- Setup / Loop ---------------------------
void setup() {

  pinMode(2, INPUT);
  pinMode(33, INPUT); 
  pinMode(SENSOR_IR, INPUT);

  //Semaforos
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(26, OUTPUT);

  Serial.begin(115200);
  delay(100);

  // Display 7 segmentos
  display.setBrightness(7);
  display.showNumberDec(0);

  // Inicia LittleFS
  if (!initLittleFS()) {
    Serial.println("Fallo LittleFS, reiniciar necesario");
  }

  saveSampleIfMissing();  // opcional: crea ejemplo si falta

  // Inicializa display
  Wire.begin();  // SDA, SCL por defecto (21,22)
  lcd.init();
  lcd.backlight();

  // Encoder
  setupEncoderPins();

  // Carga players la primera vez
  if (!loadPlayers()) {
    showNoPlayers();
  } else {
    currentIndex = 0;
    showPlayerAt(currentIndex);
  }

  // 1. Inicializar LittleFS

  // 2. Configurar el ESP32 como AP
  WiFi.softAP(ssid, password);
  Serial.print(" AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // 3. Inicializar Servidor WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.print("WebSocket server iniciado en puerto: ");
  Serial.println(81);  // Imprime el puerto 81 de forma completa

  // 4. Inicializar Servidor HTTP (Manejo de Rutas)

  // Manejador para la ruta raíz ("/") -> Sirve el index.html
  server.on("/", HTTP_GET, []() {
    if (!handleFileRead("/index.html")) {
      server.send(404, "text/plain", "404: Archivo index.html no encontrado en LittleFS");
    }
  });


  // ================================
  // 📁 RUTAS API PARA TIEMPOS
  // ================================
  const char* dataFile = "/data.json";

  // ---- LEER (GET) ----
  server.on("/api/get", HTTP_GET, [dataFile]() {
    if (!LittleFS.exists(dataFile)) {
      server.send(200, "application/json", "[]");
      return;
    }

    File file = LittleFS.open(dataFile, "r");
    if (!file || file.size() == 0) {  // Evita leer vacíos
      server.send(200, "application/json", "[]");
      return;
    }

    String content = file.readString();
    file.close();

    // Validar que sea JSON correcto
    StaticJsonDocument<1024> test;
    DeserializationError err = deserializeJson(test, content);
    if (err) {
      server.send(200, "application/json", "[]");
      return;
    }

    server.send(200, "application/json", content);
  });



  // ---- AGREGAR (POST) ----
  server.on("/api/add", HTTP_POST, [dataFile]() {
    if (!server.hasArg("plain")) {
      server.send(400, "text/plain", "No hay cuerpo JSON");
      return;
    }

    String body = server.arg("plain");
    StaticJsonDocument<512> newObj;
    DeserializationError err = deserializeJson(newObj, body);
    if (err) {
      server.send(400, "text/plain", "JSON invalido");
      return;
    }

    // Cargar el contenido actual o crear un nuevo array
    DynamicJsonDocument data(2048);
    if (LittleFS.exists(dataFile)) {
      File f = LittleFS.open(dataFile, "r");
      DeserializationError err2 = deserializeJson(data, f);
      f.close();
      if (err2) data.to<JsonArray>();
    } else {
      data.to<JsonArray>();
    }

    JsonArray arr = data.as<JsonArray>();
    arr.add(newObj);

    File f = LittleFS.open(dataFile, "w");
    if (!f) {
      server.send(500, "text/plain", "Error al abrir data.json para escribir");
      return;
    }
    serializeJsonPretty(arr, f);
    f.close();

    // Confirmación en formato JSON válido
    server.send(200, "application/json", "{\"msg\":\"Agregado correctamente\"}");
  });


  // ---- EDITAR (POST) ----
  server.on("/api/edit", HTTP_POST, [dataFile]() {
    if (!server.hasArg("plain")) {
      server.send(400, "text/plain", "No hay cuerpo JSON");
      return;
    }
    String body = server.arg("plain");

    StaticJsonDocument<512> editObj;
    DeserializationError err = deserializeJson(editObj, body);
    if (err) {
      server.send(400, "text/plain", "JSON inválido");
      return;
    }

    if (!LittleFS.exists(dataFile)) {
      server.send(404, "application/json", "{\"error\":\"Archivo no encontrado\"}");
      return;
    }

    File file = LittleFS.open(dataFile, "r");
    DynamicJsonDocument data(2048);
    deserializeJson(data, file);
    file.close();

    JsonArray arr = data.as<JsonArray>();
    bool found = false;

    for (JsonObject obj : arr) {
      if (obj["nombre"] == editObj["nombre"]) {
        obj["s1"] = editObj["s1"];
        obj["s2"] = editObj["s2"];
        obj["s3"] = editObj["s3"];
        found = true;
        break;
      }
    }

    if (found) {
      file = LittleFS.open(dataFile, "w");
      serializeJsonPretty(arr, file);
      file.close();
      server.send(200, "application/json", "{\"msg\":\"Editado correctamente\"}");
    } else {
      server.send(404, "application/json", "{\"error\":\"Objeto no encontrado\"}");
    }
  });

  // ---- ELIMINAR (GET) ----
  server.on("/api/delete", HTTP_GET, [dataFile]() {
    if (!server.hasArg("nombre")) {
      server.send(400, "text/plain", "Falta parametro nombre");
      return;
    }

    String nombre = server.arg("nombre");

    if (!LittleFS.exists(dataFile)) {
      server.send(404, "application/json", "{\"error\":\"Archivo no encontrado\"}");
      return;
    }

    File file = LittleFS.open(dataFile, "r");
    DynamicJsonDocument data(2048);
    deserializeJson(data, file);
    file.close();

    JsonArray arr = data.as<JsonArray>();
    bool found = false;

    for (size_t i = 0; i < arr.size(); i++) {
      if (arr[i]["nombre"] == nombre) {
        arr.remove(i);
        found = true;
        break;
      }
    }

    if (found) {
      file = LittleFS.open(dataFile, "w");
      serializeJsonPretty(arr, file);
      file.close();
      server.send(200, "application/json", "{\"msg\":\"Eliminado correctamente\"}");
    } else {
      server.send(404, "application/json", "{\"error\":\"Objeto no encontrado\"}");
    }
  });


  // Manejador para el resto de archivos (imagen, fuentes, etc.)
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404: Not Found");
    }
  });

  server.begin();
  Serial.println("HTTP server iniciado en puerto 80.");

  Serial.println("Setup listo.");

  if (LittleFS.exists("/index.html")) {
  Serial.println("✅ index.html encontrado en LittleFS");
} else {
  Serial.println("❌ index.html NO encontrado en LittleFS");
}

}

void loop() {

  server.handleClient(); // Maneja las peticiones HTTP (¡fundamental!)
  webSocket.loop();

  // Poll encoder y actualizar indice si cambió
  pollEncoder();

  static long lastSeenPos = 0;
  if (encoderPos != lastSeenPos) {
    long diff = encoderPos - lastSeenPos;
    lastSeenPos = encoderPos;

    // Cambiar índice segun movimientos (puede moverse varios pasos)
    if (players.size() > 0) {
      // sensiblidad: cada '1' de encoderPos avanza 1 índice
      currentIndex = (currentIndex + (int)diff) % (int)players.size();
      if (currentIndex < 0) currentIndex += players.size();
      showPlayerAt(currentIndex);
      Serial.printf("Index: %d  (pos=%ld)\n", currentIndex, encoderPos);
    } else {
      showNoPlayers();
    }
  }

  if (digitalRead(pinBtn) == LOW) {

    indexPlayerSelect = currentIndex;

    unsigned long now = millis();
    if (now - lastBtnMillis > DEBOUNCE_MS) {
      lastBtnMillis = now;
      Serial.println("Botón pulsado: recargando player.json ...");
      if (loadPlayers()) {
        showPlayerAt(currentIndex);
      } else {
        showNoPlayers();
      }
    }
    // esperar a que suelte para evitar múltiples recargas rápidas
    while (digitalRead(pinBtn) == LOW) delay(10);
  }

  if(digitalRead(2) == HIGH){
    Serial.print(".");
    delay(500);
    startRace();
    delay(500);
  }

  delay(5);  // pequeño retardo para alivianar CPU
}


// ----------------------------------------------------------------------
//                        FUNCIONES DE WEBSOCKETS
// ----------------------------------------------------------------------

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Desconectado!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Conectado desde %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        webSocket.sendTXT(num, "Conexion OK");
      }
      break;
    case WStype_TEXT:
      // Manejo de comandos del cliente
      break;
    case WStype_BIN:
    case WStype_ERROR:
      break;
  }
}

// ----------------------------------------------------------------------
//                       FUNCIONES AUXILIARES HTTP/FS
// ----------------------------------------------------------------------

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".woff")) return "font/woff";
  else if (filename.endsWith(".woff2")) return "font/woff2";
  else if (filename.endsWith(".ttf")) return "font/ttf";
  else if (filename.endsWith(".eot")) return "application/vnd.ms-fontobject";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}

bool handleFileRead(String path) {
  String pathWithGz = path + ".gz";

  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) {
    String finalPath = LittleFS.exists(pathWithGz) ? pathWithGz : path;

    File file = LittleFS.open(finalPath, "r");
    String contentType = getContentType(path);

    if (finalPath.endsWith(".gz")) {
      server.sendHeader("Content-Encoding", "gzip");
    }

    Serial.println("Enviando archivo: " + path + " (" + contentType + ")");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}


void startRace(){
  lcd.clear();
  lcd.print("Preparado...");
  display.showNumberDec(0);

  // Secuencia tipo F1
  digitalWrite(ROJO4, HIGH); delay(1000);
  digitalWrite(ROJO3, HIGH); delay(1000);
  digitalWrite(ROJO1, HIGH); delay(1000);
  digitalWrite(ROJO2, HIGH); delay(1000);

  delay(500);

  // Verdes → iniciar cronómetro
  apagarRojos();
  digitalWrite(VERDE1, HIGH);
  digitalWrite(VERDE2, HIGH);
  lcd.clear();
  lcd.print("Cronometrando...");
  display.clear();

  unsigned long startTime = millis();
  bool detectado = false;
  float tiempoFinal = 0;

  while (!detectado) {
    tiempoFinal = (millis() - startTime) / 1000.0;
    int tiempoInt = (int)(tiempoFinal * 100);
    display.showNumberDecEx(tiempoInt, 0b01000000, true);

    lcd.setCursor(0, 1);
    lcd.print(tiempoFinal, 2);
    lcd.print(" s   ");


    if (digitalRead(SENSOR_IR) == LOW){
      detectado = true;
    }else{
      Serial.print(".");
    }
    
    delay(20);
  }

  // Resultado
  apagarTodos();
  lcd.clear();
  Serial.println(tiempoFinal);
  lcd.print("Tiempo final:");
  lcd.setCursor(0, 1);
  lcd.print(tiempoFinal, 2);
  lcd.print(" s");
  display.showNumberDecEx((int)(tiempoFinal * 100), 0b01000000, true);

  subirTiempo(indexPlayerSelect, tiempoFinal);

  delay(5000);
  display.clear();
}

void apagarTodos() {
  digitalWrite(ROJO1, LOW);
  digitalWrite(ROJO2, LOW);
  digitalWrite(ROJO3, LOW);
  digitalWrite(ROJO4, LOW);
  digitalWrite(VERDE1, LOW);
  digitalWrite(VERDE2, LOW);
}

void apagarRojos() {
  digitalWrite(ROJO1, LOW);
  digitalWrite(ROJO2, LOW);
  digitalWrite(ROJO3, LOW);
  digitalWrite(ROJO4, LOW);
}

void subirTiempo(int idx, float tiempoF) {
  const char* dataFile = "/data.json";

  // Validar índice del jugador
  if (idx < 0 || idx >= players.size()) {
    Serial.println("❌ Índice de jugador inválido");
    return;
  }

  String nombre = players[idx].Nombre;


  Serial.printf("⬆️ Guardando tiempo para %s: %.2f s\n", nombre.c_str(), tiempoF);

  // Documento JSON para almacenar el array existente o crear uno nuevo
  DynamicJsonDocument doc(4096);

  // Leer data.json si existe
  if (LittleFS.exists(dataFile)) {
    File f = LittleFS.open(dataFile, "r");
    if (f) {
      DeserializationError err = deserializeJson(doc, f);
      f.close();
      if (err) {
        Serial.println("⚠️ Error leyendo data.json, se creará nuevo archivo.");
        doc.to<JsonArray>();
      }
    }
  } else {
    doc.to<JsonArray>();
  }

  JsonArray arr = doc.as<JsonArray>();

  // Crear nueva entrada de tiempo
  JsonObject nuevo = arr.createNestedObject();
  nuevo["nombre"] = nombre;
  nuevo["s1"] = 0;          // por ahora sin sector 1
  nuevo["s2"] = 0;          // por ahora sin sector 2
  nuevo["s3"] = tiempoF;    // tiempo total

  // Guardar el archivo actualizado
  File f = LittleFS.open(dataFile, "w");
  if (!f) {
    Serial.println("❌ Error al abrir data.json para escribir");
    return;
  }

  serializeJsonPretty(arr, f);
  f.close();

  Serial.println("✅ Nuevo tiempo guardado correctamente en data.json");
}
