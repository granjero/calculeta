#ifndef CONEXION_H
#define CONEXION_H

#include "ESP8266WiFi.h"
#include "WiFiClientSecure.h"
#include "ESP8266HTTPClient.h"
#include "Cert.h"

class Conexion {

public:
   Conexion();

   void conectaAlWiFi();
   void testRequest();


private:

   X509List cert;
   WiFiClientSecure client;
   HTTPClient https;

   #define CONSTANTES
   const char* ssid = "calculeta";
   const char* password = "calculeta00";
};

#endif 

