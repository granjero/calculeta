#include "Conexion.h"

Conexion::Conexion() : https(), client(), cert() { 
  client.setInsecure(); // no me interesa corroborar nada
}


void Conexion::conectar() {
  WiFi.mode(WIFI_STA); // conecta al wifi
  WiFi.begin(ssid, password);

  // Serial.print("esperando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    // Serial.print(".");
    delay(500);
  }
  // Serial.println("\nconectado IP: ");
  // Serial.println(WiFi.localIP());
  // Serial.println(WiFi.macAddress());

  return;
}

/*
 * @ pileta String con los datos
 * @ return codigo http de respuesta
 * 400 Bad Request
 * 401 Unauthorized
 * 201 Created
*/
int Conexion::pileta(String pileta) {
  conectar();
  int httpCode;
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  if (https.begin(client, apiUrlPileta)) {  // HTTPS
    // Serial.println(apiUrlPileta);
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "email=";
    httpRequestData += mac;
    httpRequestData += "@calculeta.com.ar";
    httpRequestData += "&password=";
    httpRequestData += mac;
    httpRequestData += "&pileta=";
    httpRequestData += pileta;
    httpCode = https.POST(httpRequestData);
    // Serial.printf("[HTTPS] TEST REQUEST CODE: %d\n", httpCode);
    String payload = https.getString();
    // Serial.println(httpCode);
    // return payload;
  } //else Serial.println(httpCode); 
  // Serial.println(mac);
  
  if (httpCode == 201) {
    WiFi.mode(WIFI_OFF); // apaga el wifi 
  }
  
  return httpCode;
}
