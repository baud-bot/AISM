# 🦈 AISM — Asistente Inteligente, Seguridad para Motociclistas

**🌐 Idiomas:** [English](README.md) · **Español**

Un sistema de seguridad embebido para motociclistas construido sobre el **Arduino Nano**. Detecta obstáculos cercanos, ángulos de inclinación peligrosos, movimientos bruscos y posibles choques, y luego envía automáticamente una **alerta por SMS con un enlace de Google Maps** a un contacto de emergencia predefinido.

Desarrollado para la **Feria Tecnológica CCTECH 3ra Edición 2026**.

---

## ✨ Características

- **Detección de obstáculos** en tres direcciones (izquierda, derecha y trasera) mediante sensores ultrasónicos.
  
- **Monitoreo del ángulo de inclinación** (roll y pitch) con una IMU de 6 ejes.
  
- **Detección de movimientos y giros bruscos** con umbrales configurables.
  
- **Detección de choques** basada en umbrales simultáneos de aceleración y ángulo.
  
- **Cuenta regresiva de 5 segundos** para evitar falsas alarmas.
  
- **Cancelación de la alarma** al enderezar la moto o apagar el switch.
  
- **Alerta automática por SMS** con coordenadas GPS en un enlace de Google Maps.
  
- **Visualización en LCD** con caché anti-parpadeo.
  
- **Alerta sonora** mediante buzzer con tonos diferenciados según el estado.
  
- **Optimizado para RAM** del ATmega328P (2 KB SRAM).

---

## 🔧 Requisitos de Hardware

| Componente | Cantidad | Notas |
|---|---|---|
| Arduino Nano (ATmega328P) | 1 | 16 MHz, 5 V |
| Sensor ultrasónico HC-SR04 | 3 | Izquierda, derecha, atrás |
| MPU6050 (acelerómetro + giroscopio) | 1 | I2C, configurado a ±8 g |
| LCD 16x2 con módulo I2C | 1 | Dirección `0x27` o `0x3F` |
| Buzzer pasivo | 1 | 5 V |
| Switch SPST | 1 | Encendido / apagado |
| GPS NEO-6M | 1 | TTL, 9600 baudios |
| Módulo GSM SIM900 | 1 | Quad-band, requiere 5 V / 2 A externos |
| Antena GSM | 1 | Conector SMA |
| Fuente 5 V / 2 A | 1 | Dedicada al SIM900 |
| Batería 9 V o USB | 1 | Para el Arduino |
| Cables Dupont y protoboard | — | Para el prototipado |

### ⚡ Notas de Alimentación

- El SIM900 consume picos de hasta **2 A** durante la transmisión de SMS.
- **Nunca alimentes el SIM900 desde el pin de 5 V del Arduino.**
- Usa una **fuente dedicada de 5 V / 2 A** para el módulo GSM.
- **Comparte solo el GND** entre el Arduino, el SIM900 y la fuente externa.

---

## 📍 Asignación de Pines

| Pin del Arduino Nano | Componente |
|---|---|
| **D2** | HC-SR04 IZQUIERDA — TRIG |
| **D3** | HC-SR04 IZQUIERDA — ECHO |
| **D4** | HC-SR04 DERECHA — TRIG |
| **D5** | HC-SR04 DERECHA — ECHO |
| **D6** | HC-SR04 TRASERA — TRIG |
| **D7** | HC-SR04 TRASERA — ECHO |
| **D8** | GPS TX (AltSoftSerial RX) |
| **D9** | GPS RX (AltSoftSerial TX) |
| **D10** | SIM900 TXD (SoftwareSerial RX) |
| **D11** | SIM900 RXD (SoftwareSerial TX) |
| **A0** | Buzzer (+) |
| **A1** | Switch (INPUT_PULLUP, otra pata → GND) |
| **A4 (SDA)** | LCD SDA + MPU6050 SDA |
| **A5 (SCL)** | LCD SCL + MPU6050 SCL |
| **5V** | LCD, MPU6050, GPS, sensores ultrasónicos |
| **GND** | Tierra común para todos los módulos |
| **VIN** | Batería 9 V (opcional) o USB |

> ⚠️ El GPS usa **AltSoftSerial**, que está fijo a los pines **D8 (RX)** y **D9 (TX)** en el Arduino Nano. Esto evita conflictos de timer con la función `tone()` del buzzer.

---

## 📚 Librerías Requeridas

Instala las siguientes librerías desde el **Gestor de Librerías del Arduino**:

| Librería | Autor | Propósito |
|---|---|---|
| **AltSoftSerial** | Paul Stoffregen | Comunicación serie con el GPS (fijo a D8/D9) |
| **TinyGPSPlus** | Mikal Hart | Interpretación de datos NMEA del NEO-6M |
| **LiquidCrystal_I2C** | Frank de Brabander | Control del LCD por I2C |
| **Wire** | Arduino | Comunicación I2C (incluida) |
| **SoftwareSerial** | Arduino | Comunicación serie con el SIM900 (incluida) |
| **math.h** | — | Funciones matemáticas (incluida) |

**Nota:** `Wire`, `SoftwareSerial` y `math.h` vienen incluidas con el IDE de Arduino y no requieren instalación adicional.

---

## ⚙️ Configuración del Entorno de Desarrollo

### Configuración de la placa

| Ajuste | Valor |
|---|---|
| Placa | Arduino Nano |
| Procesador | **ATmega328P (Old Bootloader)** |
| Monitor Serie | 9600 baudios |
| Baud rate del GPS | 9600 |
| Baud rate del SIM900 | 9600 |

> ⚠️ **La mayoría de los clones del Arduino Nano usan la configuración "Old Bootloader".** Si la carga falla con un error *"programmer is not responding"*, asegúrate de seleccionar esta opción antes de reintentar.

### Flujo de trabajo recomendado

1. Desconecta la alimentación del SIM900 antes de cargar el código al Arduino.
2. Carga el sketch desde el IDE de Arduino.
3. Reconecta la alimentación del SIM900.
4. Presiona el botón **RESET** del Arduino Nano.
5. Abre el Monitor Serie para verificar el funcionamiento.

Esto evita que las fluctuaciones de energía del módulo GSM interfieran con el proceso de carga.

---

## 🚀 Instalación

1. **Clona el repositorio:**

   ```bash

git clone https://github.com/tu-usuario/AISM.git

cd AISM

1. Instala las librerías requeridas con el Gestor de Librerías del Arduino.

2. Configura tu número de emergencia en el código fuente:

    cpp
   
    const char TELEFONO[] = "+595XXXXXXXXX";

3. Ajusta los umbrales de detección si es necesario:

   cpp

    const float ACC_CHOQUE = 2.5;   // Umbral de choque (g)
    const float ANG_CHOQUE = 60.0;  // Umbral de inclinación (grados)
    const float ACC_ALERTA = 1.5;   // Umbral de movimiento brusco (g)

4. Conecta todos los componentes siguiendo la tabla de pines.

5. Carga el sketch en el Arduino Nano.

6. Abre el Monitor Serie a 9600 baudios para observar la actividad.

---

## 🧠 Cómo Funciona

# Bucle principal

Cada 150 ms el Arduino:

-    Lee los tres sensores ultrasónicos y el MPU6050.

-    Evalúa los umbrales de distancia, inclinación, giro y movimiento.

-    Actualiza el tono del buzzer y el LCD.

-    Lee datos del GPS de forma continua mediante AltSoftSerial.

-    Verifica condiciones de choque.

---

## Detección de choque

Un choque se detecta cuando ambas condiciones ocurren simultáneamente:

-    Aceleración filtrada mayor a 2.5 g.

-    Inclinación máxima (roll o pitch) mayor a 60°.

# Al detectarse:

-    El LCD muestra !! CHOQUE !! Xs con cuenta regresiva de 5 segundos.

-    El buzzer emite un tono de 1500 Hz.

# El usuario puede cancelar:

-    Enderezando la moto (inclinación menor a 25°).

-    Apagando el switch.

  Si no se cancela en 5 segundos, se envía un SMS con la última posición GPS válida.

  ---

## Niveles de alerta

Condición	Buzzer= LCD
Objeto a menos de 25 cm	800 Hz =	! PRECAUCION !
Objeto a menos de 12.5 cm	1000 Hz =	!! PELIGRO !!
Inclinación mayor a 30°	800 Hz =	! PRECAUCION !
Inclinación mayor a 60°	1000 Hz =	!! PELIGRO !!
Choque detectado	1500 Hz =	!! CHOQUE !! 5s
SMS enviado	Silencio =	SMS ENVIADO / Correcto!
SMS falló	Silencio =	SMS FALLO / Revisar senal

---

## 📱 Formato del SMS

Cuando se confirma un choque, el sistema envía un SMS con el siguiente formato:
text

AUXILIO! https://maps.google.com/?q=-25.268281,-57.509490

Al tocar el enlace se abre Google Maps en la ubicación del accidente. Si el GPS no tiene fix al momento de enviar, el SMS dirá:
text

AUXILIO! GPS sin fix

---

## 🛠 Solución de Problemas

Síntoma	Causa Probable	Solución Sugerida
Falla la carga: "programmer is not responding"	Fluctuación de alimentación del SIM900	Desconecta la fuente del SIM900 durante la carga
El LCD no muestra nada	Dirección I2C incorrecta o contraste	Prueba 0x3F; ajusta el potenciómetro
El buzzer suena distorsionado	Conflicto de timer con SoftwareSerial	Usa AltSoftSerial para el GPS (D8/D9)
El GSM no se registra	Señal 2G débil o modo de banda inválido	Reinicia el módulo; prueba en zona con buena cobertura
CSQ = 0	Sin señal	Muévete a un área abierta; revisa la antena
AT+CBAND? → INVALID_BAND_MODE	Problema de firmware en algunos SIM900	Reinicia el módulo; puede requerir actualización
GPS Chars = 0	Conexión o baud rate incorrectos	Verifica TX → D8 y RX → D9; prueba 38400
GPS recibe datos pero sin fix	Sin visibilidad al cielo	Ve al exterior; espera 1–5 minutos
SMS no enviado (+CMS ERROR: 500)	Señal débil	Muévete a una ubicación con CSQ ≥ 5

## ⚠️ Limitaciones Conocidas

  -  Dependencia de 2G: El SIM900 solo opera en redes 2G, que están siendo descontinuadas en muchas regiones. Para confiabilidad a largo plazo, considera migrar a un módulo 4G LTE como el SIM7600.

  -  Tiempo de fix del GPS: El NEO-6M requiere vista clara al cielo. El primer fix puede tardar entre 1 y 5 minutos.

  -  Uso de RAM: El sistema usa aproximadamente el 70% de la SRAM del ATmega328P. Funciones adicionales pueden requerir reducir buffers o cambiar a una placa con más memoria.

  -  Un solo destinatario: El sistema envía SMS a un único número, definido en tiempo de compilación.

  -  Sin registro de eventos: Cada choque genera un SMS único; los eventos no se almacenan localmente.

---

## 🔮 Mejoras Futuras

   - Migrar a 4G LTE para mayor cobertura.

   - Agregar un botón de pánico manual.

   - Soportar múltiples destinatarios.

   - Registrar eventos en una tarjeta SD.

   - Enviar datos a un panel en la nube (MQTT, HTTP).

   - Integración con servicios de emergencia locales.

   - Diseñar una PCB personalizada para un encapsulado compacto.

   - Agregar una batería recargable de Li-ion integrada a la moto.

---

## 👥 Autores

# Equipo AISM — CCTECH 3ra Edición 2026

  -  Cecilia D.

  -  Raul B.

  -  Giannina R.

  -  Vannia D.

  -  Maile Z.

 ---

## 📄 Licencia

Este proyecto se distribuye bajo la Licencia MIT. Consulta el archivo LICENSE para más detalles.

##🙏 Agradecimientos

  - Sr. Richar Gonzalez, por su asesoría y acompañamiento durante el proyecto.

  -  CCTECH 2026, por la oportunidad de presentar este proyecto.

  -  Paul Stoffregen, por AltSoftSerial.

  -  Mikal Hart, por TinyGPSPlus.

  -  Frank de Brabander, por LiquidCrystal_I2C.

  -  La comunidad de Arduino, por su extensa documentación y ejemplos.

<p align="center"> <b>🦈 AISM — Conduce seguro. Recibe ayuda rápido.</b> </p> ```
