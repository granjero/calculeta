#ifndef CONEXION_H
#define CONEXION_H

#include "ESP8266WiFi.h"
#include "WiFiClientSecure.h"
#include "ESP8266HTTPClient.h"

class Conexion {
   public:
      Conexion();
      void conectar();
      int pileta(String pileta);
      // void testRequest();

   private:
      WiFiClientSecure client;
      X509List cert; // Create a list of certificates with the server certificate
      HTTPClient https;

      const char* ssid = "calculeta";
      const char* password = "calculeta00";

      const char* apiUrlRegistro = "https://dukarevich.com.ar/api/c/calculeta";
      const char* apiUrlPileta = "https://dukarevich.com.ar/api/c/pileta";
};

#endif 
