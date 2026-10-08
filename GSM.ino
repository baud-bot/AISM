/*
 * BUSCA SENAL -> ENVIA SMS AL INSTANTE
 * 
 * Espera a que CREG=1 Y CSQ>=5 y envia el SMS en ese mismo ciclo.
 * Reintenta hasta MAX_INTENTOS veces.
 * 
 * Pines: D10 (RX), D11 (TX)
 * Monitor Serie a 9600
 */

#include <SoftwareSerial.h>

SoftwareSerial sim900(10, 11);

// ============================================================
//  CONFIGURACION
// ============================================================
const char* TELEFONO = "+595986333773";
const char* MENSAJE  = "AUXILIO desde Arduino";

const unsigned long ESPERA_MAX   = 300000;   // 5 min buscando red
const uint8_t       MAX_INTENTOS = 15;
const uint8_t       CSQ_MINIMO   = 5;

char buf[80];

// ============================================================
//  HELPERS
// ============================================================
void limpiarGSM() {
  sim900.listen();
  while (sim900.available()) sim900.read();
}

bool cmd(const char* c, unsigned long ms = 1500) {
  sim900.listen();
  while (sim900.available()) sim900.read();
  sim900.println(c);

  unsigned long t = millis();
  uint8_t i = 0;
  buf[0] = 0;
  while (millis() - t < ms) {
    while (sim900.available() && i < sizeof(buf) - 1) {
      buf[i++] = (char)sim900.read();
      buf[i] = 0;
    }
  }
  return (strstr(buf, "OK") != NULL);
}

int leerCSQ() {
  if (!cmd("AT+CSQ")) return -1;
  char* p = strstr(buf, "+CSQ:");
  if (!p) return -1;
  p += 5;
  while (*p == ' ') p++;
  return atoi(p);
}

int leerCREG() {
  if (!cmd("AT+CREG?")) return -1;
  char* p = strstr(buf, "+CREG:");
  if (!p) return -1;
  p = strchr(p, ',');
  if (!p) return -1;
  return atoi(p + 1);
}

// ============================================================
//  ENVIO INMEDIATO DE SMS
// ============================================================
bool enviarSMS() {
  Serial.println();
  Serial.println(">>> ENVIANDO SMS");

  cmd("AT+CMGF=1", 1500);

  sim900.listen();
  while (sim900.available()) sim900.read();
  sim900.print("AT+CMGS=\"");
  sim900.print(TELEFONO);
  sim900.println("\"");
  delay(2000);
  while (sim900.available()) Serial.write(sim900.read());

  sim900.print(MENSAJE);
  delay(500);
  sim900.write(26);

  Serial.print("    Respuesta: ");
  buf[0] = 0;
  uint8_t i = 0;
  unsigned long t = millis();
  while (millis() - t < 15000) {
    while (sim900.available() && i < sizeof(buf) - 1) {
      char c = sim900.read();
      Serial.write(c);
      buf[i++] = c;
      buf[i] = 0;
    }
  }
  Serial.println();

  if (strstr(buf, "+CMGS:") != NULL) {
    Serial.println(">>> SMS ACEPTADO");
    return true;
  }
  if (strstr(buf, "+CMS ERROR:") != NULL) {
    Serial.println(">>> FALLO: +CMS ERROR");
  } else if (strlen(buf) == 0) {
    Serial.println(">>> FALLO: sin respuesta");
  } else {
    Serial.println(">>> FALLO: respuesta desconocida");
  }
  return false;
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.println("  BUSCA SENAL -> ENVIA SMS");
  Serial.println("========================================");
  Serial.print("Destino: ");
  Serial.println(TELEFONO);

  sim900.begin(9600);
  delay(3000);

  // Paso 1: verificar comunicacion
  Serial.println("\n[1] Verificando SIM900...");
  if (!cmd("AT", 2000)) {
    Serial.println("ERROR: SIM900 no responde");
    while (1) delay(1000);
  }
  Serial.println("SIM900 OK");
  cmd("ATE0", 1000);

  // Paso 2: buscar senal y enviar en cuanto aparezca
  Serial.println("\n[2] Buscando senal...");
  unsigned long t0 = millis();
  bool enviado = false;

  while (millis() - t0 < ESPERA_MAX) {
    int csq  = leerCSQ();
    int creg = leerCREG();

    Serial.print("[t=");
    Serial.print((millis() - t0) / 1000);
    Serial.print("s] CSQ=");
    Serial.print(csq);
    Serial.print(" CREG=");
    Serial.println(creg);

    // Si hay registro y senal -> enviar INMEDIATAMENTE
    if ((creg == 1 || creg == 5) && csq >= CSQ_MINIMO) {
      Serial.println(">>> SENAL DETECTADA - ENVIANDO AHORA");

      for (uint8_t intento = 1; intento <= MAX_INTENTOS; intento++) {
        Serial.print("--- INTENTO ");
        Serial.print(intento);
        Serial.print("/");
        Serial.print(MAX_INTENTOS);
        Serial.println(" ---");

        int c2 = leerCSQ();
        int r2 = leerCREG();
        Serial.print("    CSQ=");
        Serial.print(c2);
        Serial.print(" CREG=");
        Serial.println(r2);

        if ((r2 == 1 || r2 == 5) && c2 >= CSQ_MINIMO) {
          if (enviarSMS()) {
            enviado = true;
            break;
          }
        } else {
          Serial.println("    Senal bajo, esperando...");
          delay(2000);
        }
        delay(3000);
      }

      if (enviado) break;
      // Si no logro enviar, seguir buscando
      t0 = millis();
    }

    delay(2000);
  }

  // Resultado final
  Serial.println();
  Serial.println("========================================");
  if (enviado) {
    Serial.println("  SMS ENVIADO CON EXITO");
  } else {
    Serial.println("  NO SE PUDO ENVIAR EN TIEMPO");
    Serial.println("  Causa: sin senal suficiente");
  }
  Serial.println("========================================");
}

void loop() {}