#include "Conexion.h"

Conexion::Conexion() : cert() { 
  // ssid = "calculeta";
  // password = "calculeta00";
}

void Conexion::conectaAlWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("esperando AP llamado calculeta");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("\nconectado.\nseteando la hora");
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // parece que se necesita la hora para el handshake de https
  time_t now = time(nullptr);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
}

void Conexion::testRequest() {
  if ((WiFi.status() == WL_CONNECTED)) {
    X509List cert(IRG_Root_X1);
    client.setTrustAnchors(&cert); // linkea el cliente al certificado
    HTTPClient https;
    if (https.begin(client, "https://calculeta.estonoesunaweb.com.ar/api/test")) {  // HTTPS
      int httpCode = https.GET();
      if (httpCode > 0) {
        String payload = https.getString();
        Serial.println(payload);
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  Serial.println();
  Serial.println("Waiting 2min before the next round...");
  delay(120000);
}
