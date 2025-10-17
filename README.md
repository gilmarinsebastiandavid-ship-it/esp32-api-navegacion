# 🚗 Sistema IoT de Cálculo de Rutas con ESP32

## 📋 Descripción del Proyecto

Sistema IoT completo que integra un dispositivo ESP32 con display OLED y servicios en la nube para calcular y mostrar rutas entre dos ubicaciones. El sistema permite ingresar direcciones desde una interfaz web, procesa las ubicaciones mediante geocodificación, calcula la ruta óptima según el medio de transporte seleccionado, y muestra el tiempo estimado y distancia en un display OLED.

### 🎯 Objetivos

**Objetivo General:**
Desarrollar un sistema IoT completo que integre un dispositivo ESP32 con display OLED y servicios en la nube, demostrando competencias en Internet de las Cosas, APIs REST y computación distribuida.

**Objetivos Específicos:**
1. Implementar un dispositivo IoT con ESP32 y display SSD1306 OLED
2. Integrar servicios web externos mediante APIs gratuitas
3. Desplegar servicios en la nube para recepción y envío de datos
4. Configurar comunicación bidireccional entre dispositivo y nube
5. Documentar el proceso completo de desarrollo e implementación

---

## 🛠️ Componentes y Requisitos

### Hardware Necesario

| Componente | Especificaciones | Cantidad |
|------------|------------------|----------|
| ESP32 DevKit | Cualquier modelo (WROOM-32, DevKitC, etc.) | 1 |
| Display OLED | SSD1306 128x64 píxeles, I2C | 1 |
| Cables Jumper | Macho-Hembra o Macho-Macho según conexión | 4 |
| Cable USB | Micro-USB o USB-C según ESP32 | 1 |
| Protoboard | Opcional para organización | 1 |

### Software Necesario

| Software | Versión Mínima | Enlace |
|----------|----------------|--------|
| Visual Studio Code | Última versión | [descargar](https://code.visualstudio.com/) |
| PlatformIO Extension | Última versión | [extensión](https://platformio.org/) |
| Navegador Web | Moderno (Chrome, Firefox, Edge) | - |
| Driver USB-Serial | CP210x o CH340 según tu ESP32 | [drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |

### Servicios en la Nube (Gratuitos)

| Servicio | Propósito | Límites Gratuitos |
|----------|-----------|-------------------|
| ThingSpeak | Middleware de comunicación | Ilimitado para uso básico |
| Nominatim (OpenStreetMap) | Geocodificación de direcciones | 1 request/segundo |
| OpenRouteService | Cálculo de rutas | 2,000 requests/día |

---

## 📐 Diagrama de Arquitectura

```
┌─────────────────┐
│  Página Web     │
│  (HTML/JS)      │
└────────┬────────┘
         │
         │ HTTPS POST
         ▼
┌─────────────────┐
│   ThingSpeak    │
│   (Nube)        │
└────────┬────────┘
         │
         │ HTTPS GET
         ▼
┌─────────────────┐      ┌──────────────────┐
│     ESP32       │─────▶│  Nominatim API   │
│   + WiFi        │      │  (Geocodificar)  │
└────────┬────────┘      └──────────────────┘
         │
         │ HTTPS POST
         ▼
┌─────────────────┐
│ OpenRouteService│
│  (Calcular Ruta)│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Display OLED   │
│  (Mostrar Info) │
└─────────────────┘
```

---

## 🔌 Conexión del Hardware

### Diagrama de Conexión

```
ESP32 DevKit          Display OLED SSD1306
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO 21 (SDA)    ───▶    SDA
GPIO 22 (SCL)    ───▶    SCL
3.3V             ───▶    VCC
GND              ───▶    GND
```

### Instrucciones de Conexión

1. **Apaga el ESP32** (desconéctalo del USB)
2. Conecta los cables siguiendo el diagrama:
   - **SDA del OLED** → **GPIO 21** del ESP32
   - **SCL del OLED** → **GPIO 22** del ESP32
   - **VCC del OLED** → **3.3V** del ESP32 (⚠️ NO uses 5V)
   - **GND del OLED** → **GND** del ESP32
3. Verifica que no haya cables sueltos
4. Conecta el ESP32 al puerto USB

### ⚠️ Advertencias

- **No conectes VCC a 5V**: El SSD1306 funciona a 3.3V
- Verifica la polaridad antes de conectar
- Si el OLED tiene más de 4 pines, usa solo: VCC, GND, SDA, SCL

---

## 🚀 Configuración Paso a Paso

### Paso 1: Instalar PlatformIO en VS Code

1. Abre **Visual Studio Code**
2. Ve a la pestaña de **Extensiones** (Ctrl+Shift+X)
3. Busca **"PlatformIO IDE"**
4. Haz clic en **Install**
5. Reinicia VS Code cuando termine la instalación
6. Verifica que aparezca el ícono de PlatformIO (🏠) en la barra lateral

---

### Paso 2: Crear el Proyecto en PlatformIO

1. Haz clic en el ícono de **PlatformIO Home** (🏠)
2. Selecciona **"New Project"**
3. Configura el proyecto:
   - **Name:** `sistema-rutas-iot`
   - **Board:** Busca y selecciona **"Espressif ESP32 Dev Module"**
   - **Framework:** Selecciona **"Arduino"**
   - **Location:** Deja la ubicación por defecto o elige una carpeta
4. Haz clic en **"Finish"**
5. Espera a que PlatformIO descargue las herramientas necesarias (puede tardar 5-10 min la primera vez)

---

### Paso 3: Configurar el Archivo `platformio.ini`

1. Abre el archivo **`platformio.ini`** (está en la raíz del proyecto)
2. Reemplaza todo el contenido con esto:

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
board_build.arduino.memory_type = qio_qspi
build_flags = 
    -DARDUINO_LOOP_STACK_SIZE=16384

lib_deps = 
    adafruit/Adafruit GFX Library@^1.11.9
    adafruit/Adafruit SSD1306@^2.5.9
    bblanchon/ArduinoJson@^6.21.4
```

3. Guarda el archivo (Ctrl+S)
4. PlatformIO instalará automáticamente las librerías necesarias

---

### Paso 4: Obtener Credenciales de ThingSpeak

#### 4.1 Crear Cuenta

1. Ve a [https://thingspeak.com](https://thingspeak.com)
2. Haz clic en **"Get Started For Free"**
3. Crea una cuenta con tu email
4. Confirma tu email

#### 4.2 Crear Canal

1. Inicia sesión en ThingSpeak
2. Haz clic en **"Channels"** → **"My Channels"**
3. Haz clic en **"New Channel"**
4. Configura el canal:

```
Name: Sistema de Rutas IoT
Description: Control de rutas ESP32

☑ Field 1: origen
☑ Field 2: destino
☑ Field 3: vehiculo
☑ Field 4: origenLat
☑ Field 5: origenLon
☑ Field 6: destinoLat
☑ Field 7: destinoLon
```

5. Desmarca los campos 8 en adelante
6. Haz clic en **"Save Channel"**

#### 4.3 Obtener API Keys

1. En tu canal, ve a la pestaña **"API Keys"**
2. Anota estos valores (los necesitarás después):
   - **Channel ID**: Número como `2345678`
   - **Write API Key**: Código como `ABCD1234EFGH5678`
   - **Read API Key**: Código como `XYZ9876STUV5432`

📝 **Guarda estos datos en un archivo de texto temporal**

---

### Paso 5: Obtener API Key de OpenRouteService

1. Ve a [https://openrouteservice.org/dev/#/signup](https://openrouteservice.org/dev/#/signup)
2. Crea una cuenta gratuita
3. Confirma tu email
4. Inicia sesión en [https://openrouteservice.org/dev/#/home](https://openrouteservice.org/dev/#/home)
5. Ve a la sección **"Tokens"**
6. Copia tu **API Key** (se ve así: `5b3ce3597851110001cf6248...`)

📝 **Guarda esta API Key junto con las credenciales de ThingSpeak**

**Límite gratuito:** 2,000 requests por día (más que suficiente para el proyecto)

---

### Paso 6: Configurar el Código del ESP32

#### 6.1 Agregar el Código

1. En VS Code, abre el archivo **`src/main.cpp`**
2. Borra todo el contenido
3. Copia **TODO** el código del artefacto "ESP32 Sistema de Rutas IoT - Corregido"
4. Pégalo en `main.cpp`

#### 6.2 Configurar Credenciales

Busca estas líneas al inicio del código (alrededor de la línea 20-30):

```cpp
// WiFi
const char* WIFI_SSID = "TU_WIFI_SSID";
const char* WIFI_PASSWORD = "TU_WIFI_PASSWORD";

// ThingSpeak
const char* THINGSPEAK_CHANNEL_ID = "TU_CHANNEL_ID";
const char* THINGSPEAK_READ_API_KEY = "TU_READ_API_KEY";

// OpenRouteService
const char* ORS_API_KEY = "TU_OPENROUTESERVICE_API_KEY";
```

**Reemplaza con tus datos reales:**

```cpp
// WiFi
const char* WIFI_SSID = "MiWiFi2024";              // ← Tu nombre de WiFi
const char* WIFI_PASSWORD = "mipassword123";       // ← Tu contraseña WiFi

// ThingSpeak
const char* THINGSPEAK_CHANNEL_ID = "2345678";     // ← Tu Channel ID
const char* THINGSPEAK_READ_API_KEY = "XYZ9876STUV5432"; // ← Tu Read API Key

// OpenRouteService
const char* ORS_API_KEY = "5b3ce3597851110001cf6248abc123..."; // ← Tu API Key de ORS
```

⚠️ **Importante:**
- El WiFi debe ser de **2.4 GHz** (ESP32 no soporta 5 GHz)
- Verifica que no haya espacios extra en las credenciales
- Las comillas deben ser **dobles** `"` no simples `'`

#### 6.3 Guardar

Presiona **Ctrl+S** para guardar los cambios

---

### Paso 7: Compilar y Subir el Código

#### 7.1 Conectar el ESP32

1. Conecta el ESP32 a tu computadora mediante USB
2. Espera a que el sistema operativo lo reconozca
3. Si no lo reconoce, instala los drivers USB-Serial

#### 7.2 Compilar el Proyecto

1. En la barra inferior azul de VS Code, haz clic en el ícono **✓** (Build)
2. Espera a que compile (primera vez puede tardar 2-3 minutos)
3. Verifica que diga **"SUCCESS"** en la terminal

#### 7.3 Subir al ESP32

1. En la barra inferior, haz clic en el ícono **→** (Upload)
2. PlatformIO detectará automáticamente el puerto COM
3. Espera a que suba el código (30-60 segundos)
4. Verás mensajes como:
   ```
   Writing at 0x00010000... (100%)
   Hard resetting via RTS pin...
   ```

#### 7.4 Abrir Monitor Serial

1. Haz clic en el ícono **🔌** (Serial Monitor) en la barra inferior
2. O presiona **Ctrl+Alt+S**
3. Deberías ver:

```
=== Sistema de Rutas IoT ===
Conectando a WiFi: TuWiFi
.....
WiFi conectado!
IP: 192.168.1.100
Sistema listo. Esperando datos de ThingSpeak...
```

📝 **Si ves esto, ¡el ESP32 está funcionando correctamente!**

---

### Paso 8: Crear la Interfaz Web

#### 8.1 Crear el Archivo HTML

1. Crea una carpeta en tu computadora llamada `interfaz-web`
2. Dentro, crea un archivo llamado `control-rutas.html`
3. Abre el archivo con un editor de texto
4. Copia **TODO** el contenido del artefacto "Interfaz Web - Control de Rutas" (si lo generé)
5. Si no tienes la interfaz web, usa este código básico:

```html
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Control de Rutas IoT</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        h1 {
            color: #667eea;
            text-align: center;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
            color: #333;
        }
        input, select {
            width: 100%;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 16px;
        }
        button {
            width: 100%;
            padding: 15px;
            background: #667eea;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 18px;
            cursor: pointer;
            transition: 0.3s;
        }
        button:hover {
            background: #764ba2;
        }
        .status {
            margin-top: 20px;
            padding: 15px;
            border-radius: 5px;
            text-align: center;
            display: none;
        }
        .success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚗 Sistema de Rutas IoT</h1>
        
        <div class="form-group">
            <label>Channel ID de ThingSpeak:</label>
            <input type="text" id="channelId" placeholder="Ej: 2345678">
        </div>
        
        <div class="form-group">
            <label>Write API Key:</label>
            <input type="text" id="writeApiKey" placeholder="Ej: ABCD1234EFGH5678">
        </div>
        
        <hr style="margin: 30px 0;">
        
        <div class="form-group">
            <label>📍 Dirección de Origen:</label>
            <input type="text" id="origen" placeholder="Ej: Hospital Santa Fe, Bogota">
        </div>
        
        <div class="form-group">
            <label>📍 Dirección de Destino:</label>
            <input type="text" id="destino" placeholder="Ej: Universidad Nacional, Bogota">
        </div>
        
        <div class="form-group">
            <label>🚙 Modo de Transporte:</label>
            <select id="vehiculo">
                <option value="auto">🚗 Auto</option>
                <option value="bici">🚴 Bicicleta</option>
                <option value="pie">🚶 A pie</option>
            </select>
        </div>
        
        <button onclick="enviarRuta()">Enviar Ruta al ESP32</button>
        
        <div id="status" class="status"></div>
    </div>

    <script>
        // Cargar credenciales guardadas
        window.onload = function() {
            document.getElementById('channelId').value = localStorage.getItem('channelId') || '';
            document.getElementById('writeApiKey').value = localStorage.getItem('writeApiKey') || '';
        };

        function enviarRuta() {
            const channelId = document.getElementById('channelId').value;
            const writeApiKey = document.getElementById('writeApiKey').value;
            const origen = document.getElementById('origen').value;
            const destino = document.getElementById('destino').value;
            const vehiculo = document.getElementById('vehiculo').value;
            
            // Validar campos
            if (!channelId || !writeApiKey) {
                mostrarEstado('Por favor ingresa las credenciales de ThingSpeak', false);
                return;
            }
            
            if (!origen || !destino) {
                mostrarEstado('Por favor ingresa origen y destino', false);
                return;
            }
            
            // Guardar credenciales
            localStorage.setItem('channelId', channelId);
            localStorage.setItem('writeApiKey', writeApiKey);
            
            // Construir URL de ThingSpeak
            const url = `https://api.thingspeak.com/update?api_key=${writeApiKey}&field1=${encodeURIComponent(origen)}&field2=${encodeURIComponent(destino)}&field3=${vehiculo}`;
            
            // Enviar datos
            fetch(url)
                .then(response => response.text())
                .then(data => {
                    if (data !== '0') {
                        mostrarEstado('✅ Ruta enviada exitosamente al ESP32', true);
                    } else {
                        mostrarEstado('❌ Error al enviar. Verifica tus credenciales', false);
                    }
                })
                .catch(error => {
                    mostrarEstado('❌ Error de conexión: ' + error, false);
                });
        }
        
        function mostrarEstado(mensaje, exito) {
            const statusDiv = document.getElementById('status');
            statusDiv.textContent = mensaje;
            statusDiv.className = 'status ' + (exito ? 'success' : 'error');
            statusDiv.style.display = 'block';
            
            setTimeout(() => {
                statusDiv.style.display = 'none';
            }, 5000);
        }
    </script>
</body>
</html>
```

6. Guarda el archivo

#### 8.2 Configurar la Interfaz Web

1. **Abre el archivo `control-rutas.html`** con tu navegador (doble clic)
2. Ingresa tus credenciales de ThingSpeak:
   - **Channel ID**: El número de tu canal
   - **Write API Key**: Tu Write API Key (no el Read API Key)
3. Las credenciales se guardarán automáticamente en tu navegador

---

## ✅ Prueba del Sistema Completo

### Prueba 1: Usando Lugares Conocidos (Recomendado)

1. En la interfaz web, ingresa:
   - **Origen:** `Hospital Santa Fe, Bogota`
   - **Destino:** `Universidad Nacional, Bogota`
   - **Modo:** Auto 🚗
2. Haz clic en **"Enviar Ruta al ESP32"**
3. Deberías ver: "✅ Ruta enviada exitosamente"

**En el Monitor Serial del ESP32 verás:**
```
--- Consultando ThingSpeak ---
DEBUG - hasCoordinates: false
Direcciones de texto recibidas, geocodificando...
Geocodificando con Nominatim: Hospital%20Santa%20Fe%2C%20Bogota
HTTP Code: 200
Coordenadas encontradas: 4.XXXX, -74.XXXX
--- Calculando Ruta ---
Ruta calculada exitosamente:
  Distancia: X.XX km
  Tiempo: XX minutos
```

**En el Display OLED verás:**
```
RUTA CALCULADA
─────────────────
Modo: Auto
Dist: 8.5 km

   15 min
```

### Prueba 2: Cambiar Modo de Transporte

Prueba la misma ruta con diferentes modos:

| Modo | Tiempo Aprox | Comentarios |
|------|--------------|-------------|
| 🚗 Auto | 15-20 min | Ruta más rápida por carreteras |
| 🚴 Bicicleta | 30-40 min | Puede usar ciclovías |
| 🚶 A pie | 60-90 min | Rutas peatonales más directas |

### Prueba 3: Ejemplos de Direcciones que Funcionan Bien

**Hospitales:**
- `Hospital San Ignacio, Bogota`
- `Clinica Shaio, Bogota`
- `Hospital Kennedy, Bogota`

**Universidades:**
- `Universidad de los Andes, Bogota`
- `Universidad Javeriana, Bogota`
- `Universidad Libertadores, Bogota`

**Centros Comerciales:**
- `Centro Comercial Andino, Bogota`
- `Unicentro Bogota`
- `Centro Mayor, Bogota`

**Lugares Emblemáticos:**
- `Plaza de Bolivar, Bogota`
- `Aeropuerto El Dorado, Bogota`
- `Terminal de Transporte, Bogota`

---

## 🔧 Solución de Problemas

### ❌ "Error WiFi - No se pudo conectar"

**Causa:** Credenciales incorrectas o WiFi de 5GHz

**Solución:**
1. Verifica que el SSID y contraseña sean correctos
2. Asegúrate de usar WiFi de **2.4 GHz** (ESP32 no soporta 5 GHz)
3. Acerca el ESP32 al router
4. Verifica que no haya caracteres especiales en la contraseña

---

### ❌ "Error: No se detectó el display OLED"

**Causa:** Cables mal conectados o dirección I2C incorrecta

**Solución:**
1. Verifica las conexiones:
   - SDA → GPIO 21
   - SCL → GPIO 22
   - VCC → 3.3V (NO 5V)
   - GND → GND
2. Prueba este escáner I2C para detectar la dirección:
   ```cpp
   // Sube este código temporal para detectar el display
   #include <Wire.h>
   void setup() {
     Serial.begin(115200);
     Wire.begin(21, 22);
     Serial.println("Escaneando I2C...");
     for(byte i = 0; i < 127; i++) {
       Wire.beginTransmission(i);
       if (Wire.endTransmission() == 0) {
         Serial.print("Dispositivo encontrado en: 0x");
         Serial.println(i, HEX);
       }
     }
   }
   void loop() {}
   ```
3. Si encuentra `0x3D` en lugar de `0x3C`, cambia esta línea en el código:
   ```cpp
   #define SCREEN_ADDRESS 0x3D
   ```

---

### ❌ "Error HTTP 400" al geocodificar

**Causa:** Nominatim rechazando la petición

**Solución:**
1. Usa direcciones más simples: `Hospital Santa Fe, Bogota` (sin tildes)
2. Asegúrate de esperar 1 segundo entre geocodificaciones
3. Verifica tu conexión a internet

---

### ❌ "Error HTTP 404" o "400" al calcular ruta

**Causa:** Coordenadas fuera de rango o muy lejanas

**Solución:**
1. Verifica que las coordenadas geocodificadas estén en Colombia
2. Mira el Monitor Serial para ver las coordenadas encontradas
3. Si están incorrectas, usa lugares más conocidos

---

### ❌ "Coordenadas fuera de Colombia"

**Causa:** Geocodificación encontró lugar con nombre similar en otro país

**Solución:**
1. Agrega "Colombia" al final: `Universidad Nacional, Bogota, Colombia`
2. Usa lugares más específicos
3. Verifica que no haya errores tipográficos

---

### ❌ "Guru Meditation Error" (Reset del ESP32)

**Causa:** Stack overflow por falta de memoria

**Solución:**
1. Verifica que `platformio.ini` tenga:
   ```ini
   build_flags = 
       -DARDUINO_LOOP_STACK_SIZE=16384
   ```
2. Reduce el tamaño de `DynamicJsonDocument` si es muy grande
3. Asegúrate de usar filtros JSON donde sea posible

---

### ❌ La página web dice "Error al enviar"

**Causa:** Credenciales de ThingSpeak incorrectas

**Solución:**
1. Verifica el **Channel ID** (número, sin espacios)
2. Usa el **Write API Key**, NO el Read API Key
3. Verifica que no hayas copiado espacios extra
4. Prueba directamente en tu navegador:
   ```
   https://api.thingspeak.com/update?api_key=TU_WRITE_KEY&field1=test
   ```
   Debe devolver un número (entry ID), no "0"

---

## 📊 Monitoreo y Debugging

### Ver Datos en ThingSpeak

1. Entra a tu canal en [ThingSpeak](https://thingspeak.com)
2. Ve a la pestaña **"Private View"**
3. Verás gráficas con los últimos valores de:
   - Field 1: Origen
   - Field 2: Destino
   - Field 3: Vehículo

### Monitor Serial - Comandos Útiles

**Ver logs en tiempo real:**
- PlatformIO: Haz clic en 🔌 (Serial Monitor)
- Velocidad: 115200 baud

**Filtrar mensajes:**
En la terminal de PlatformIO puedes buscar con `Ctrl+F`

**Limpiar terminal:**
Cierra y vuelve a abrir el Serial Monitor

---

## 📈 Mejoras y Extensiones Futuras

### Ideas para Expandir el Proyecto

1. **Agregar Botones Físicos**
   - Conecta 3 botones al ESP32
   - Botón 1: Cambiar a modo Auto
   - Botón 2: Cambiar a modo Bici
   - Botón 3: Cambiar a modo A pie

2. **Guardar Rutas Favoritas**
   - Usa SPIFFS para almacenar 5 rutas frecuentes
   - Selección rápida sin geocodificar

3. **Integrar GPS Real**
   - Módulo GPS NEO-6M
   - Ubicación actual automática como origen

4. **Alertas de Tráfico**
   - Integrar API de tráfico en tiempo real
   - Mostrar si hay congestión en la ruta

5. **Display más Grande**
   - OLED 1.3" (128x64) para más info
   - Mostrar mini-mapa ASCII de la ruta

6. **Historial de Rutas**
   - Guardar las últimas 10 rutas consultadas
   - Mostrar estadísticas (ruta más usada, etc.)

7. **App Móvil**
   - Crear app con Blynk o React Native
   - Control desde smartphone

8. **Múltiples Paradas**
   - Calcular ruta con paradas intermedias
   - Optimización de ruta multi-punto

---

## 📚 Recursos Adicionales

### Documentación Oficial

- **ESP32:** [https://docs.espressif.com/projects/esp-idf/](https://docs.espressif.com/projects/esp-idf/)
- **PlatformIO:** [https://docs.platformio.org/](https://docs.platformio.org/)
- **ThingSpeak:** [https://www.mathworks.com/help/thingspeak/](https://www.mathworks.com/help/thingspeak/)
- **OpenRouteService:** [https://openrouteservice.org/dev/#/api-docs](https://openrouteservice.org/dev/#/api-docs)
- **Nominatim:** [https://nominatim.