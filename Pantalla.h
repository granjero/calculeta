#ifndef PANTALLA_H
#define PANTALLA_H

#include <map>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Logo.h"
#include "DataSeries.h"

class Pantalla {
public:
   Pantalla(); // Default constructor
   void contadorPiletas(unsigned int contador); 
   void cronos(int numero, int x, int y, int tamFuente);
   void cronoDescanso(int numero);
   void metrosSerie(unsigned int largos);
   void metrosTot(unsigned int total);
   void anotaSeries(DataSeries* data);
   void resumen(int tamanioFuente, DataSeries* data);
   void inicio();
   void borraContador();
   void logoCalculeta();
   void macAddress(String mac);
   void esperandoWiFi();
   void conectadoWiFi();
   void finalizadoWiFi();
   void errorWiFi(int error);

private:
   #define TFT_CS    D2
   #define TFT_RST   D3
   #define TFT_DC    D4
   #define TFT_CLK   D5
   #define TFT_MOSI  D7

   #define LARGO_PILETA  50
   #define CANT_MAX_SERIES 128
   #define CANT_MAX_PILETAS  128

   #define AN_PANTALLA  240 
   #define AL_PANTALLA  320

   #define TIEMPO_INCREMENTO_SERIE 10000 // 10 segundos

   #define TAM_FUENTE_CONTADOR   15
   #define TAM_FUENTE_CRON       3
   #define TAM_FUENTE_CRON_CHICO 2
   #define TAM_FUENTE_MTS        3
   #define TAM_FUENTE_MTS_TOT    4

   #define AN_FUENTE_UNITARIA    6 // pixeles 
   #define AL_FUENTE_UNITARIA    8 // pixeles
   #define AN_CHAR_CONTADOR      90 // 15*6
   #define AL_CHAR_CONTADOR      120 // 15*8
   #define AN_CHAR_MTS           18 // 3*6
   #define AL_CHAR_MTS           24 // 3*8
   #define AN_CHAR_MTS_TOT       24 // 4*6
   #define AL_CHAR_MTS_TOT       32 // 4*8
   #define AN_CHAR_CRON          18 // 3*6
   #define AL_CHAR_CRON          24 // 3*8
   #define AN_CHAR_CRON_CHICO    12 // 2*6
   #define AL_CHAR_CRON_CHICO    16 // 2*8

   Adafruit_ILI9341 tft;

   GFXcanvas1 canvas_contador_piletas;
   GFXcanvas1 canvas_crono;
   GFXcanvas1 canvas_metros_serie;
   GFXcanvas1 canvas_metros_tot;

   char* reloj(int segundos);
};

#endif 

