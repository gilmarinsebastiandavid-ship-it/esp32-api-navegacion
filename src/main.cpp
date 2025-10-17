/*
 * Sistema IoT de Cálculo de Rutas
 * ESP32 + OLED SSD1306 + OpenRouteService + ThingSpeak
 * 
 * Funcionalidades:
 * - Recibe origen/destino desde ThingSpeak
 * - Consulta rutas en OpenRouteService
 * - Muestra tiempo y distancia en OLED
 * - Soporta: Auto, Bicicleta, A pie
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== CONFIGURACIÓN ====================
// WiFi
const char* WIFI_SSID = "MOVISTAR WIFI7026";
const char* WIFI_PASSWORD = "sebas0827";

// ThingSpeak
const char* THINGSPEAK_CHANNEL_ID = "3122609";
const char* THINGSPEAK_READ_API_KEY = "1JCFL26B66UD5HFY";

// OpenRouteService
const char* ORS_API_KEY = "eyJvcmciOiI1YjNjZTM1OTc4NTExMTAwMDFjZjYyNDgiLCJpZCI6ImRlYmU0N2U1YjUxMTQ4ZTg4NTNiZDhiMzhmOTRmNjdkIiwiaCI6Im11cm11cjY0In0=";

// OLED Display (128x64)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==================== VARIABLES GLOBALES ====================
struct RouteData {
  String origen;
  String destino;
  float origenLat;
  float origenLon;
  float destinoLat;
  float destinoLon;
  String vehiculo; // "auto", "bici", "pie"
  float distanciaKm;
  int tiempoMinutos;
  bool dataValid;
  unsigned long lastUpdate;
};

RouteData currentRoute;
unsigned long lastThingSpeakCheck = 0;
const unsigned long THINGSPEAK_INTERVAL = 15000; // Verificar cada 15 segundos
bool wifiConnected = false;

// ==================== DECLARACIÓN DE FUNCIONES ====================
void connectWiFi();
void checkThingSpeakData();
bool geocodeAddress(String address, float &lat, float &lon);
void calculateRoute();
void displayMessage(String line1, String line2, String line3);
void displayRoute();

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== Sistema de Rutas IoT ===");
  
  // Inicializar OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Error: No se detectó el display OLED"));
    while(1);
  }
  
  display.clearDisplay();
  displayMessage("Iniciando...", "Sistema de Rutas", "");
  
  // Conectar WiFi
  connectWiFi();
  
  // Inicializar estructura
  currentRoute.dataValid = false;
  currentRoute.lastUpdate = 0;
  
  displayMessage("Sistema Listo", "Esperando", "direcciones...");
  Serial.println("Sistema listo. Esperando datos de ThingSpeak...");
}

// ==================== LOOP PRINCIPAL ====================
void loop() {
  // Verificar conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    displayMessage("Error WiFi", "Reconectando...", "");
    connectWiFi();
    return;
  } else {
    wifiConnected = true;
  }
  
  // Verificar ThingSpeak periódicamente
  if (millis() - lastThingSpeakCheck >= THINGSPEAK_INTERVAL) {
    lastThingSpeakCheck = millis();
    checkThingSpeakData();
  }
  
  delay(100);
}

// ==================== CONEXIÓN WiFi ====================
void connectWiFi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID);
  
  displayMessage("Conectando", "WiFi...", "");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
    
    displayMessage("WiFi OK", WiFi.localIP().toString(), "");
    delay(2000);
    wifiConnected = true;
  } else {
    Serial.println("\nError: No se pudo conectar a WiFi");
    displayMessage("Error WiFi", "Verifique config", "");
    delay(5000);
    wifiConnected = false;
  }
}

// ==================== THINGSPEAK ====================
void checkThingSpeakData() {
  if (!wifiConnected) return;
  
  Serial.println("\n--- Consultando ThingSpeak ---");
  
  HTTPClient http;
  String url = "https://api.thingspeak.com/channels/" + String(THINGSPEAK_CHANNEL_ID) + 
               "/feeds/last.json?api_key=" + String(THINGSPEAK_READ_API_KEY);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Respuesta recibida:");
    Serial.println(payload);
    
    // Parsear JSON
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      // ThingSpeak fields:
      // field1: origen (texto o lat)
      // field2: destino (texto o lon origen)
      // field3: vehiculo (auto/bici/pie)
      // field4: latitud origen (opcional)
      // field5: longitud origen (opcional)
      // field6: latitud destino (opcional)
      // field7: longitud destino (opcional)
      
      String field1 = doc["field1"].as<String>();
      String field2 = doc["field2"].as<String>();
      String field3 = doc["field3"].as<String>();
      String field4 = doc["field4"].as<String>();
      String field5 = doc["field5"].as<String>();
      String field6 = doc["field6"].as<String>();
      String field7 = doc["field7"].as<String>();
      
      // Verificar si hay datos nuevos
      if (field1.length() > 0 && field2.length() > 0) {
        bool needsUpdate = false;
        
        // Verificar si cambió algo
        if (currentRoute.origen != field1 || 
            currentRoute.destino != field2 || 
            currentRoute.vehiculo != field3) {
          needsUpdate = true;
        }
        
        if (needsUpdate || !currentRoute.dataValid) {
          // Actualizar datos
          currentRoute.origen = field1;
          currentRoute.destino = field2;
          currentRoute.vehiculo = field3.length() > 0 ? field3 : "auto";
          
          // Verificar si vienen coordenadas o direcciones de texto
bool hasCoordinates = false;

// Verificar si los campos NO son null en el JSON
if (!doc["field4"].isNull() && !doc["field5"].isNull() && 
    !doc["field6"].isNull() && !doc["field7"].isNull()) {
  
  // Verificar que realmente son números válidos
  float testLat = doc["field4"].as<float>();
  float testLon = doc["field5"].as<float>();
  
  // Si al menos uno no es cero, asumimos que son coordenadas válidas
  if (testLat != 0.0 || testLon != 0.0) {
    hasCoordinates = true;
  }
}

Serial.print("DEBUG - hasCoordinates: ");
Serial.println(hasCoordinates ? "true" : "false");

if (hasCoordinates) {
            // Vienen coordenadas directas
            currentRoute.origenLat = field4.toFloat();
            currentRoute.origenLon = field5.toFloat();
            currentRoute.destinoLat = field6.toFloat();
            currentRoute.destinoLon = field7.toFloat();
            
            Serial.println("Coordenadas recibidas directamente");
            calculateRoute();
          } else {
            // Vienen direcciones de texto, necesitamos geocodificar
            Serial.println("Direcciones de texto recibidas, geocodificando...");
            displayMessage("Geocodificando", "origen...", "");
            
            if (geocodeAddress(currentRoute.origen, currentRoute.origenLat, currentRoute.origenLon)) {
              delay(1100);
              displayMessage("Geocodificando", "destino...", "");
              if (geocodeAddress(currentRoute.destino, currentRoute.destinoLat, currentRoute.destinoLon)) {
                calculateRoute();
              } else {
                displayMessage("Error", "Destino no", "encontrado");
                Serial.println("Error: No se pudo geocodificar el destino");
              }
            } else {
              displayMessage("Error", "Origen no", "encontrado");
              Serial.println("Error: No se pudo geocodificar el origen");
            }
          }
        } else {
          Serial.println("Sin cambios en los datos");
        }
      }
    } else {
      Serial.print("Error parseando JSON: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("Error HTTP: ");
    Serial.println(httpCode);
    displayMessage("Error", "ThingSpeak", String(httpCode));
  }
  
  http.end();
}

// ==================== GEOCODIFICACIÓN ====================
bool geocodeAddress(String address, float &lat, float &lon) {
  HTTPClient http;
  
  // Encodear para URL
 // Encodear para URL - usar solo %20 para espacios
address.replace(" ", "%20");
address.replace(",", "%2C");
address.replace("#", "%23");
address.replace("+", "%2B");
  
  // Usar Nominatim (OpenStreetMap) - gratis y sin API key
  String url = "https://nominatim.openstreetmap.org/search?q=" + address + 
               "&countrycodes=co" +  // Solo Colombia
               "&format=json&limit=1";
  
  Serial.print("Geocodificando con Nominatim: ");
  Serial.println(address);
  
  http.begin(url);
  http.addHeader("User-Agent", "ESP32-IoT-Route-System/1.0");
  http.setTimeout(10000);
  
  int httpCode = http.GET();
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);
  
  if (httpCode == 200) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, http.getStream());
    
    if (!error) {
      if (doc.size() > 0) {
        // Nominatim devuelve array, tomar el primer resultado
        lat = doc[0]["lat"].as<String>().toFloat();
        lon = doc[0]["lon"].as<String>().toFloat();
        
        Serial.print("Coordenadas encontradas: ");
        Serial.print(lat, 6);
        Serial.print(", ");
        Serial.println(lon, 6);
        
        http.end();
        
        // Validar que las coordenadas estén en Colombia (aprox)
        if (lat >= -4.0 && lat <= 13.0 && lon >= -79.0 && lon <= -66.0) {
          Serial.println("Coordenadas validadas en Colombia");
          return true;
        } else {
          Serial.println("Coordenadas fuera de Colombia, rechazadas");
          return false;
        }
      } else {
        Serial.println("No se encontraron resultados");
      }
    } else {
      Serial.print("Error JSON: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("Error HTTP: ");
    Serial.println(httpCode);
  }
  
  http.end();
  return false;
}
// ==================== CÁLCULO DE RUTA ====================
void calculateRoute() {
  Serial.println("\n--- Calculando Ruta ---");
  displayMessage("Calculando", "ruta...", "");
  
  // Alimentar watchdog
  delay(10);
  yield();
  
  HTTPClient http;
  
  // Determinar perfil según vehículo
  String profile = "driving-car";
  if (currentRoute.vehiculo == "bici") {
    profile = "cycling-regular";
  } else if (currentRoute.vehiculo == "pie") {
    profile = "foot-walking";
  }
  
  Serial.print("Perfil: ");
  Serial.println(profile);
  
  // Construir URL (sin parámetros de coordenadas)
  String url = "https://api.openrouteservice.org/v2/directions/" + profile;
  
  // Construir JSON body
  StaticJsonDocument<512> jsonDoc;
  JsonArray coordinates = jsonDoc.createNestedArray("coordinates");
  
  JsonArray start = coordinates.createNestedArray();
  start.add(currentRoute.origenLon);
  start.add(currentRoute.origenLat);
  
  JsonArray end = coordinates.createNestedArray();
  end.add(currentRoute.destinoLon);
  end.add(currentRoute.destinoLat);
  
  String jsonBody;
  serializeJson(jsonDoc, jsonBody);
  
  Serial.print("JSON Body: ");
  Serial.println(jsonBody);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String(ORS_API_KEY));
  http.setTimeout(15000);
  
  int httpCode = http.POST(jsonBody);
  
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);
  
 if (httpCode == 200) {
    String payload = http.getString();
    
    // Usar filtro optimizado
    StaticJsonDocument<200> filter;
    filter["routes"][0]["summary"]["distance"] = true;
    filter["routes"][0]["summary"]["duration"] = true;
    
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    
    if (!error) {
      // La estructura correcta es routes[0].summary
      float distance = doc["routes"][0]["summary"]["distance"];
      float duration = doc["routes"][0]["summary"]["duration"];
      
      currentRoute.distanciaKm = distance / 1000.0;
      currentRoute.tiempoMinutos = duration / 60.0;
      currentRoute.dataValid = true;
      currentRoute.lastUpdate = millis();
      
      Serial.println("Ruta calculada exitosamente:");
      Serial.print("  Distancia: ");
      Serial.print(currentRoute.distanciaKm, 2);
      Serial.println(" km");
      Serial.print("  Tiempo: ");
      Serial.print(currentRoute.tiempoMinutos);
      Serial.println(" minutos");
      
      displayRoute();
    } else {
      Serial.print("Error parseando respuesta: ");
      Serial.println(error.c_str());
      displayMessage("Error", "Parseo JSON", "");
      currentRoute.dataValid = false;
    }
  } else {
    Serial.print("Error HTTP al calcular ruta: ");
    Serial.println(httpCode);
    
    String errorMsg = "HTTP: " + String(httpCode);
    displayMessage("Error Ruta", errorMsg, "Revisar datos");
    currentRoute.dataValid = false;
  }
  
  http.end();
}

// ==================== FUNCIONES DE DISPLAY ====================
void displayMessage(String line1, String line2, String line3) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 10);
  display.println(line1);
  
  display.setCursor(0, 30);
  display.println(line2);
  
  if (line3.length() > 0) {
    display.setCursor(0, 50);
    display.println(line3);
  }
  
  display.display();
}

void displayRoute() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Título
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("RUTA CALCULADA");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Vehículo
  display.setCursor(0, 15);
  display.print("Modo: ");
  if (currentRoute.vehiculo == "auto") {
    display.println("Auto");
  } else if (currentRoute.vehiculo == "bici") {
    display.println("Bicicleta");
  } else {
    display.println("A pie");
  }
  
  // Distancia
  display.setCursor(0, 28);
  display.print("Dist: ");
  display.print(currentRoute.distanciaKm, 1);
  display.println(" km");
  
  // Tiempo (destacado)
  display.setTextSize(2);
  display.setCursor(0, 43);
  
  if (currentRoute.tiempoMinutos < 60) {
    display.print(currentRoute.tiempoMinutos);
    display.println(" min");
  } else {
    int horas = currentRoute.tiempoMinutos / 60;
    int minutos = currentRoute.tiempoMinutos % 60;
    display.print(horas);
    display.print("h ");
    display.print(minutos);
    display.print("m");
  }
  
  display.display();
  
  Serial.println("Display actualizado con información de ruta");
}

// ==================== FIN DEL CÓDIGO ====================