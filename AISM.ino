// ------------------------------ Feria Tecnologica CCTECH 3ra Edicion 2026 -------------------------------
// ---------------------- "AISM" (Asistente Inteligente, Seguridad para Motociclistas) --------------------------
// ---------------- Integrantes: Cecilia D. / Raul B. / Giannina R. / Vannia D. / Maile Z. ----------------

// =======================================================

// ---------------- Librerias ----------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPSPlus.h>

// ---------------- Pines ----------------
const uint8_t TRIG_I = 2, ECHO_I = 3;
const uint8_t TRIG_D = 4, ECHO_D = 5;
const uint8_t TRIG_A = 6, ECHO_A = 7;
const uint8_t GSM_RX = 10, GSM_TX = 11;
const uint8_t BUZZER  = A0;
const uint8_t SWITCH  = A1;

// ---------------- Parametros ----------------
const float DIST_ALERTA  = 25.0;
const float DIST_CRITICA = 12.5;
const float ANG_ALERTA   = 30.0;
const float ANG_CRITICO  = 60.0;
const float ACC_ALERTA   = 1.5;
const float GIRO_ALERTA  = 180.0;

const uint16_t MS_AVISO  = 3000;
const uint16_t MS_CAMBIO = 200;
const uint16_t MS_LECT   = 150;

const float ALPHA_ACC  = 0.25;
const float ALPHA_GIRO = 0.35;
const float ALPHA_DIST = 0.15;

// ---------------- Configuraciones ----------------
const char TELEFONO[] = "+595986333773";
const float ACC_CHOQUE = 2.5;
const float ANG_CHOQUE = 65.0;
const float ANG_RECUP  = 25.0;
const uint16_t MS_CONFIRMA  = 5000;
const uint16_t MS_RESULTADO = 10000;

// ---------------- Instrucciones ----------------
void regMPU(uint8_t reg, uint8_t val);
void leerMPU();
void leerSensores();
float medir(uint8_t trig, uint8_t echo);
void evaluar(unsigned long ahora);
void buzzer();
void lcdUpdate();
void escribir(uint8_t fila, const char* txt, bool forzar);
void escribirF(uint8_t fila, const __FlashStringHelper* txt, bool forzar);
void centrar(char* dst, const char* src);
void centrarF(char* dst, const __FlashStringHelper* src);
void dist2(char* dst, float d);
bool gsmCmd(const char* c, unsigned long ms);
void enviarSMS();
void manejarChoque(unsigned long ahora);
void debug();

// ---------------- Objetos ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
AltSoftSerial gpsSerial;
SoftwareSerial gsmSerial(GSM_RX, GSM_TX);
TinyGPSPlus gps;

// ---------------- Variables ----------------
float dI = 400, dD = 400, dA = 400;
float dIs = 400, dDs = 400, dAs = 400;
float roll = 0, pitch = 0;
float accFilt = 1.0, giroFilt = 0.0;

bool incPrec = false, incPel = false;
bool distPrec = false, distPel = false;
bool avisoGiro = false, avisoMov = false;
unsigned long tGiro = 0, tMov = 0;
unsigned long tLect = 0;
unsigned long tCambio0 = 0, tCambio1 = 0;

char prevL0[17] = {0};
char prevL1[17] = {0};
char lcdBuf[17];

uint16_t freqActual = 0;

float latG = 0.0, lngG = 0.0;
int   satsG = 0;
bool  tieneFix = false;

bool choquePendiente = false;
bool smsEnviado = false;
bool smsFallo = false;
unsigned long tChoque = 0;
unsigned long tResult = 0;

bool encendido = true;

char atBuf[32];
char smsMsg[80];

// ---------------- Vista Previa ----------------
void setup() {
  Serial.begin(9600);
  pinMode(TRIG_I, OUTPUT); pinMode(ECHO_I, INPUT);
  pinMode(TRIG_D, OUTPUT); pinMode(ECHO_D, INPUT);
  pinMode(TRIG_A, OUTPUT); pinMode(ECHO_A, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);

  Wire.begin();
  Wire.setClock(400000);

  lcd.init();
  lcd.backlight();

  regMPU(0x6B, 0x00);
  regMPU(0x1C, 0x10);
  regMPU(0x1B, 0x00);
  regMPU(0x1A, 0x03);

  gpsSerial.begin(9600);
  gsmSerial.begin(9600);
  delay(2500);

  escribirF(0, F("ASISTENTE MOTO"), true);
  escribirF(1, F("INICIANDO..."),   true);
  delay(1500);

  Serial.println(F("=== ASISTENTE MOTO ==="));

  gsmCmd("AT", 1500);
  gsmCmd("ATE0", 1000);
  gsmCmd("AT+CMGF=1", 1000);
}

// ---------------- Ciclos ----------------
void loop() {
  unsigned long ahora = millis();

  bool enc = !digitalRead(SWITCH);

  if (enc != encendido) {
    encendido = enc;
    prevL0[0] = 0;
    prevL1[0] = 0;
    if (!encendido) {
      choquePendiente = false;
    }
  }

  if (!encendido) {
    noTone(BUZZER);
    freqActual = 0;
    escribirF(0, F("ASISTENTE MOTO"), true);
    escribirF(1, F("APAGADO"),     true);
    return;
  }

  if (ahora - tLect >= MS_LECT) {
    tLect = ahora;
    leerSensores();
    evaluar(ahora);
    buzzer();
    lcdUpdate();
  }

  while (gpsSerial.available()) gps.encode(gpsSerial.read());
  if (gps.location.isValid() && gps.satellites.value() >= 4) {
    latG = gps.location.lat();
    lngG = gps.location.lng();
    satsG = (int)gps.satellites.value();
    tieneFix = true;
  }

  manejarChoque(ahora);
  debug();
}

// ---------------- Sensores Ultrasonicos ----------------
void leerSensores() {
  dI = medir(TRIG_I, ECHO_I);
  dD = medir(TRIG_D, ECHO_D);
  dA = medir(TRIG_A, ECHO_A);
  leerMPU();
}

float medir(uint8_t trig, uint8_t echo) {
  digitalWrite(trig, LOW);  delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long d = pulseIn(echo, HIGH, 18000);
  if (d == 0) return 400.0;
  float cm = d / 58.0;
  return (cm < 2.0 || cm > 300.0) ? 400.0 : cm;
}

void regMPU(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(0x68);
  Wire.write(reg); Wire.write(val);
  Wire.endTransmission(true);
}

void leerMPU() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 14, true);

  float ax = (int16_t)(Wire.read() << 8 | Wire.read()) / 4096.0;
  float ay = (int16_t)(Wire.read() << 8 | Wire.read()) / 4096.0;
  float az = (int16_t)(Wire.read() << 8 | Wire.read()) / 4096.0;
  Wire.read(); Wire.read();
  float gx = (int16_t)(Wire.read() << 8 | Wire.read()) / 131.0;
  float gy = (int16_t)(Wire.read() << 8 | Wire.read()) / 131.0;
  float gz = (int16_t)(Wire.read() << 8 | Wire.read()) / 131.0;

  roll  = atan2(ay, az) * 180.0 / PI;
  pitch = atan2(-ax, sqrt(ay*ay + az*az)) * 180.0 / PI;

  float a = sqrt(ax*ax + ay*ay + az*az);
  float g = sqrt(gx*gx + gy*gy + gz*gz);
  accFilt  = ALPHA_ACC  * a + (1 - ALPHA_ACC)  * accFilt;
  giroFilt = ALPHA_GIRO * g + (1 - ALPHA_GIRO) * giroFilt;
}

void evaluar(unsigned long ahora) {
  float dMin = min(dI, min(dD, dA));
  distPel  = dMin < DIST_CRITICA;
  distPrec = (dMin < DIST_ALERTA) && !distPel;

  float ang = max(abs(roll), abs(pitch));
  incPel  = ang > ANG_CRITICO;
  incPrec = (ang > ANG_ALERTA) && !incPel;

  if (giroFilt > GIRO_ALERTA) { avisoGiro = true; tGiro = ahora; }
  if (accFilt  > ACC_ALERTA)  { avisoMov  = true; tMov  = ahora; }
  if (avisoGiro && ahora - tGiro > MS_AVISO) avisoGiro = false;
  if (avisoMov  && ahora - tMov  > MS_AVISO) avisoMov  = false;
}

// ---------------- Buzzer ----------------
void buzzer() {
  uint16_t freqDeseada = 0;
  if (choquePendiente) freqDeseada = 1500;
  else if (smsEnviado || smsFallo) freqDeseada = 0;
  else {
    bool peligro    = incPel  || distPel;
    bool precaucion = incPrec || distPrec;
    if (peligro) freqDeseada = 1000;
    else if (precaucion) freqDeseada = 800;
  }
  if (freqDeseada == freqActual) return;
  freqActual = freqDeseada;
  if (freqDeseada == 0) noTone(BUZZER);
  else                  tone(BUZZER, freqDeseada);
}

// ---------------- Pantalla LCD ----------------
void _writeBuf(uint8_t fila, bool forzar) {
  char* prev = (fila == 0) ? prevL0 : prevL1;
  unsigned long& tC = (fila == 0) ? tCambio0 : tCambio1;
  if (strcmp(lcdBuf, prev) == 0) return;
  if (!forzar && (millis() - tC < MS_CAMBIO)) return;
  strcpy(prev, lcdBuf);
  tC = millis();
  lcd.setCursor(0, fila);
  lcd.print(lcdBuf);
}

void escribir(uint8_t fila, const char* txt, bool forzar) {
  uint8_t i = 0;
  while (i < 16 && txt[i]) lcdBuf[i] = txt[i], i++;
  while (i < 16) lcdBuf[i++] = ' ';
  lcdBuf[16] = 0;
  _writeBuf(fila, forzar);
}

void escribirF(uint8_t fila, const __FlashStringHelper* txt, bool forzar) {
  PGM_P p = reinterpret_cast<PGM_P>(txt);
  uint8_t i = 0;
  while (i < 16) {
    char c = pgm_read_byte(p + i);
    if (!c) break;
    lcdBuf[i++] = c;
  }
  while (i < 16) lcdBuf[i++] = ' ';
  lcdBuf[16] = 0;
  _writeBuf(fila, forzar);
}

void centrar(char* dst, const char* src) {
  uint8_t n = strlen(src);
  if (n > 16) n = 16;
  uint8_t izq = (16 - n) / 2;
  uint8_t p = 0;
  for (uint8_t i = 0; i < izq; i++) dst[p++] = ' ';
  for (uint8_t i = 0; i < n; i++) dst[p++] = src[i];
  while (p < 16) dst[p++] = ' ';
  dst[16] = 0;
}

void centrarF(char* dst, const __FlashStringHelper* src) {
  PGM_P p = reinterpret_cast<PGM_P>(src);
  uint8_t n = 0;
  while (n < 16 && pgm_read_byte(p + n)) n++;
  uint8_t izq = (16 - n) / 2;
  uint8_t k = 0;
  for (uint8_t i = 0; i < izq; i++) dst[k++] = ' ';
  for (uint8_t i = 0; i < n; i++) dst[k++] = pgm_read_byte(p + i);
  while (k < 16) dst[k++] = ' ';
  dst[16] = 0;
}

void dist2(char* dst, float d) {
  int n = (int)d;
  if (n < 2 || n > 99) { dst[0]='-'; dst[1]='-'; dst[2]=0; return; }
  dst[0] = '0' + n / 10;
  dst[1] = '0' + n % 10;
  dst[2] = 0;
}

void lcdUpdate() {
  if (smsEnviado) {
    escribirF(0, F("  SMS ENVIADO  "), false);
    escribirF(1, F("  Correcto!    "), false);
    return;
  }
  if (smsFallo) {
    escribirF(0, F("   SMS FALLO   "), false);
    escribirF(1, F(" Revisar senal "), false);
    return;
  }
  if (choquePendiente) {
    int seg = (MS_CONFIRMA - (millis() - tChoque) + 999) / 1000;
    if (seg < 0) seg = 0;
    char tmp[17];
    snprintf(tmp, 17, "!! CHOQUE !! %ds", seg);
    centrar(lcdBuf, tmp);
    _writeBuf(0, true);
    escribirF(1, F("Enderezar moto"), false);
    return;
  }

  dIs = (dI > 300.0) ? 400.0 : ALPHA_DIST * dI + (1 - ALPHA_DIST) * dIs;
  dDs = (dD > 300.0) ? 400.0 : ALPHA_DIST * dD + (1 - ALPHA_DIST) * dDs;
  dAs = (dA > 300.0) ? 400.0 : ALPHA_DIST * dA + (1 - ALPHA_DIST) * dAs;

  bool alI = (dI < DIST_ALERTA);
  bool alD = (dD < DIST_ALERTA);
  bool alA = (dA < DIST_ALERTA);
  float ang = max(abs(roll), abs(pitch));
  int angInt = (int)ang; if (angInt > 99) angInt = 99;
  bool incDanger = incPel || incPrec;

  if (incDanger) {
    if (incPel) escribirF(0, F(" !! PELIGRO !!  "), false);
    else        escribirF(0, F(" ! PRECAUCION ! "), false);
  } else if (alI || alD || alA) {
    char a[3], b[3], c[3];
    dist2(a, dIs); dist2(b, dDs); dist2(c, dAs);
    char tmp[17]; uint8_t p = 0;
    if (alI) { tmp[p++]='I'; tmp[p++]=':'; tmp[p++]=a[0]; tmp[p++]=a[1]; }
    if (alD) { if (p) tmp[p++]=' '; tmp[p++]='D'; tmp[p++]=':'; tmp[p++]=b[0]; tmp[p++]=b[1]; }
    if (alA) { if (p) tmp[p++]=' '; tmp[p++]='A'; tmp[p++]=':'; tmp[p++]=c[0]; tmp[p++]=c[1]; }
    tmp[p] = 0;
    escribir(0, tmp, false);
  } else {
    char a[3], b[3], c[3];
    dist2(a, dIs); dist2(b, dDs); dist2(c, dAs);
    char tmp[17];
    snprintf(tmp, 17, "I:%s D:%s A:%s", a, b, c);
    escribir(0, tmp, false);
  }

  char tmp[17];
  if (incDanger) {
    if (avisoGiro && avisoMov) snprintf(tmp, 17, "INC:%02d%c G M", angInt, (char)223);
    else if (avisoGiro)        snprintf(tmp, 17, "INC:%02d%c G", angInt, (char)223);
    else if (avisoMov)         snprintf(tmp, 17, "INC:%02d%c M", angInt, (char)223);
    else                       snprintf(tmp, 17, "INC:%02d%c", angInt, (char)223);
    escribir(1, tmp, false);
    return;
  }

  bool distDanger = distPel || distPrec;
  if (distDanger) {
    if (avisoGiro && avisoMov) snprintf(tmp, 17, "INC:%02d%c G M", angInt, (char)223);
    else if (avisoGiro)        snprintf(tmp, 17, "INC:%02d%c G", angInt, (char)223);
    else if (avisoMov)         snprintf(tmp, 17, "INC:%02d%c M", angInt, (char)223);
    else if (distPel)          { centrarF(lcdBuf, F(" !! PELIGRO !!  ")); _writeBuf(1, false); return; }
    else                       { centrarF(lcdBuf, F(" ! PRECAUCION ! ")); _writeBuf(1, false); return; }
    escribir(1, tmp, false);
  } else if (avisoGiro || avisoMov) {
    if (avisoGiro && avisoMov) snprintf(tmp, 17, "INC:%02d%c G M", angInt, (char)223);
    else if (avisoGiro)        snprintf(tmp, 17, "INC:%02d%c G", angInt, (char)223);
    else                       snprintf(tmp, 17, "INC:%02d%c M", angInt, (char)223);
    escribir(1, tmp, false);
  } else {
    snprintf(tmp, 17, "INC:%02d%c SEGURO", angInt, (char)223);
    escribir(1, tmp, false);
  }
}

// ---------------- Modulo GSM SIM ----------------
bool gsmCmd(const char* c, unsigned long ms) {
  gsmSerial.listen();
  while (gsmSerial.available()) gsmSerial.read();
  gsmSerial.println(c);

  unsigned long t = millis();
  uint8_t i = 0;
  atBuf[0] = 0;
  while (millis() - t < ms) {
    while (gsmSerial.available() && i < sizeof(atBuf) - 1) {
      atBuf[i++] = (char)gsmSerial.read();
      atBuf[i] = 0;
    }
  }
  return (strstr(atBuf, "OK") != NULL);
}

void enviarSMS() {
  escribirF(0, F("Enviando SMS de"), true);
  escribirF(1, F("alerta..."),       true);

  if (tieneFix) {
    char latS[11], lngS[11];
    dtostrf(latG, 1, 6, latS);
    dtostrf(lngG, 1, 6, lngS);
    snprintf(smsMsg, sizeof(smsMsg),
      "AUXILIO! https://maps.google.com/?q=%s,%s", latS, lngS);
  } else {
    snprintf(smsMsg, sizeof(smsMsg), "AUXILIO! GPS sin fix");
  }

  gsmCmd("AT+CMGF=1", 1500);

  gsmSerial.listen();
  while (gsmSerial.available()) gsmSerial.read();
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(TELEFONO);
  gsmSerial.println("\"");
  delay(2000);
  while (gsmSerial.available()) gsmSerial.read();

  gsmSerial.print(smsMsg);
  delay(500);
  gsmSerial.write(26);

  atBuf[0] = 0;
  uint8_t i = 0;
  unsigned long t = millis();
  while (millis() - t < 12000) {
    while (gsmSerial.available() && i < sizeof(atBuf) - 1) {
      atBuf[i++] = (char)gsmSerial.read();
      atBuf[i] = 0;
    }
  }

  if (strstr(atBuf, "+CMGS:") != NULL) {
    smsEnviado = true;  smsFallo = false;
  } else {
    smsEnviado = false; smsFallo = true;
  }
  tResult = millis();
}

// ---------------- Parametro de Choque ----------------
void manejarChoque(unsigned long ahora) {
  float ang = max(abs(roll), abs(pitch));
  bool umbrales      = (accFilt > ACC_CHOQUE) && (ang > ANG_CHOQUE);
  bool reincorporado = (ang < ANG_RECUP);

  if (umbrales && !choquePendiente && !smsEnviado && !smsFallo) {
    choquePendiente = true;
    tChoque = ahora;
  }
  if (choquePendiente && reincorporado) choquePendiente = false;
  if (choquePendiente && (ahora - tChoque >= MS_CONFIRMA)) {
    choquePendiente = false;
    enviarSMS();
  }
  if ((smsEnviado || smsFallo) && (ahora - tResult > MS_RESULTADO) && reincorporado) {
    smsEnviado = false;
    smsFallo = false;
  }
}

// ---------------- Monitor Serial ----------------
void debug() {
  static unsigned long tDbg = 0;
  if (millis() - tDbg < 1000) return;
  tDbg = millis();

  Serial.print(F("I:")); Serial.print(dI);
  Serial.print(F(" D:")); Serial.print(dD);
  Serial.print(F(" A:")); Serial.print(dA);
  Serial.print(F(" | INC:")); Serial.print((int)max(abs(roll), abs(pitch)));
  Serial.print(F(" | g:")); Serial.print(accFilt, 2);
  Serial.print(F(" | GPS:")); Serial.print(tieneFix ? F("FIX") : F("--"));
  Serial.print(F(" S:")); Serial.print(satsG);
  Serial.print(F(" | SMS:"));
  Serial.print(smsEnviado ? F("OK") : (smsFallo ? F("ERR") : F("-")));
  Serial.print(F(" | CH:"));
  Serial.println(choquePendiente ? F("SI") : F("NO"));
}