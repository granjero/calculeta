#include <map>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Button.h"
#include "Logo.h"

// reed switch
#define BOTON     D1
// display pins
#define TFT_CS    D2
#define TFT_RST   D3
#define TFT_DC    D4
#define TFT_CLK   D5
#define TFT_MOSI  D7
// display dimensiones
#define ANCHO_PANTALLA  240 
#define ALTO_PANTALLA   320
// algunas cosntantes
#define LARGO_PILETA    50
#define UN_SEGUNDO      1000
#define TIEMPO_INCREMENTO_SERIE 10000 // 10 segundos
#define TIEMPO_PULSO_RESET      3000
#define CANT_MAX_SERIES         100
#define ALTO_FUENTE_UNITARIA    8
#define ANCHO_FUENTE_UNITARIA   6
#define TAM_FUENTE_CONTADOR     15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Button boton(BOTON);

boolean contando = false;
boolean descansando = false;
boolean puedeIncrementarPileta = true;

unsigned long timestampUltimoCronometroImpreso = 0;
unsigned long timestampBotonPresionado = 0;

typedef struct
{
  int piletas;
  int tiempo;
} datos_serie;
datos_serie series[CANT_MAX_SERIES];

typedef struct
{
  unsigned int pileta;
  unsigned int serie;
  unsigned int descanso;
  unsigned int total;
} cronometros;
cronometros cronometro;

typedef struct
{
  unsigned int piletas;
  unsigned int series;
  unsigned int total;
} contadores;
contadores contador;

unsigned int hContador = TAM_FUENTE_CONTADOR * ALTO_FUENTE_UNITARIA; // altura del contador para posicionar otras cosas

void setup() 
{
  boton.begin();  // inicializa el boton
  tft.begin();    // inicializa la pantalla
  tft.fillScreen(0x9E1E);
  tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, 0x002D);
}

void loop() 
{
  if (!contando) // no estamos contando
  {
    if (boton.pressed())
    {
      contando = true;
      timestampBotonPresionado = millis();
      tft.fillScreen(ILI9341_BLACK);
      imprimeContador(contador.piletas, ILI9341_WHITE, TAM_FUENTE_CONTADOR);
      imprimeMetrosSerie(ILI9341_GREEN);
      imprimeMetrosTotales(ILI9341_GREEN);
    }
  }

  else // comenzó a funcionar el aparato
  {
    if (millis() - timestampUltimoCronometroImpreso >= UN_SEGUNDO) // cada vez que pasa un segundo
    {
      cronometro.pileta++;
      cronometro.total++;
      if (!descansando) cronometro.serie++;
      if (descansando) cronometro.descanso++;

      if (descansando) imprimeCronometroDescanso(cronometro.descanso, ILI9341_ORANGE);
      if (!descansando) imprimeCronometroPileta(cronometro.pileta, ILI9341_WHITE, puedeIncrementarPileta);
      imprimeCronometroSerie(cronometro.serie, ILI9341_WHITE);
      imprimeCronometroTotal(cronometro.total, ILI9341_WHITE);
      timestampUltimoCronometroImpreso = millis();
    }

    // Se presiona el boton
    if (boton.pressed())
    {
      timestampBotonPresionado = millis(); 

      if (!descansando)
      {
        if(puedeIncrementarPileta) // aumenta el numero de piletas
        {
          puedeIncrementarPileta = false; // pone en falso para que se pueda incrementar la serie
          contador.piletas++;
          contador.total++;
          cronometro.pileta = 0;
        }
        else // aumenta el numero de series
        {
          if (contador.piletas != 0) // no tiene sentido una serie de 0 piletas 
          {
            // almacena los valores en la estructura
            series[contador.series].piletas = contador.piletas;
            series[contador.series].tiempo = cronometro.serie;
            // incrementa el valor de series
            contador.series++;
            // resetea las variables para la proxima cuenta.
            contador.piletas = 0;
            cronometro.pileta = 0;
            cronometro.serie = 0;
            cronometro.descanso = 0;
            descansando = true;
            tft.fillRect(0, 0, 240, 145, ILI9341_BLACK);
          }
        }

        imprimeSeries();
        imprimeMetrosSerie(ILI9341_GREEN);
        imprimeMetrosTotales(ILI9341_GREEN);
        if (!descansando) imprimeContador(contador.piletas, ILI9341_WHITE, TAM_FUENTE_CONTADOR);
        if (!descansando) imprimeCronometroPileta(cronometro.pileta, ILI9341_WHITE, true);
      }

      else // descansando = true
      {
        descansando = false;
        cronometro.pileta = 0;
        tft.fillRect(0, 0, 240, 120, ILI9341_BLACK); // borra el cronometro del descanso
        imprimeCronometroPileta(cronometro.pileta, ILI9341_WHITE, puedeIncrementarPileta);
        imprimeContador(contador.piletas, ILI9341_WHITE, TAM_FUENTE_CONTADOR);
      }
    }

    if(boton.released() && millis() - timestampBotonPresionado >= TIEMPO_PULSO_RESET) {
      //RESET
      reset();
    }

    puedeIncrementarPileta = sePuedeIncrementarPileta();

  }
}

void reset()
{
  // timestampBotonPresionado = millis();
  contando = false;

  cronometro.pileta= 0;
  cronometro.serie= 0;
  cronometro.total = 0;

  contador.piletas = 0;
  contador.total = 0;

  for (int i = 0; i < CANT_MAX_SERIES; i++)
  {
    series[i].tiempo = 0;
    series[i].piletas = 0;
  }
  tft.fillScreen(0x9E1E);
  tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, 0x002D);
}

bool sePuedeIncrementarPileta() // vuelve a poner en true cuando pasó el tiempo que tenia el usuario para incrementar una serie
{
  return millis() - timestampBotonPresionado >= TIEMPO_INCREMENTO_SERIE;
}

void imprimeCronometroTotal(unsigned int cronometro, uint16_t color)
{
  int16_t x, y;
  uint16_t w, h;
  char reloj[6];
  creaRelojGPT(cronometro, reloj, sizeof(reloj));

  tft.setTextColor(color, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.getTextBounds(reloj, 0, 0, &x, &y, &w, &h);
  tft.setCursor(0, ALTO_PANTALLA - h);
  tft.print(" ");
  tft.println(reloj);
}

void imprimeCronometroSerie(unsigned int cronometro, uint16_t color)
{
  char reloj[6];
  creaRelojGPT(cronometro, reloj, sizeof(reloj));

  tft.setTextColor(color, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, hContador);
  tft.write(0x0F); // sibolo del reloj
  tft.print(" ");
  tft.println(reloj);
}

void imprimeCronometroDescanso(unsigned int cronometro, uint16_t color)
{
  char reloj[6];
  creaRelojGPT(cronometro, reloj, sizeof(reloj));

  tft.setTextColor(color, ILI9341_BLACK);
  tft.setTextSize(8);
  tft.setCursor(0, 25);
  tft.println(reloj);
}

void imprimeCronometroPileta(unsigned long cronometro, uint16_t color, bool ventanaSerie)
{
  char reloj[6];
  creaRelojGPT(cronometro, reloj, sizeof(reloj));

  if (contador.piletas == 10) tft.fillRect(0, 0, 60, 60, ILI9341_BLACK);
  if (!descansando)
    tft.setTextColor((contador.piletas != 0) ? ((ventanaSerie) ? color : 0xFB54) : color, ILI9341_BLACK);
  else
    tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  tft.setTextSize(contador.piletas >= 10 ? 2 : 3);
  tft.setCursor(0, 0);
  tft.println(reloj);
}

void creaRelojGPT(unsigned int tiempo, char* buffer, size_t bufferSize)
{
  unsigned int segundos = tiempo % 60;
  unsigned int minutos = tiempo / 60;

  snprintf(buffer, bufferSize, "%02lu:%02lu", minutos, segundos);
}

void imprimeContador(unsigned int numero, uint16_t color, int tamanioFuente)
{
  int16_t x, y;
  uint16_t w, h;

  char contador[3];
  sprintf(contador, "%u", numero);

  tft.setTextWrap(false);
  tft.setTextSize(tamanioFuente);
  tft.getTextBounds(contador, 0, 0, &x, &y, &w, &h);
  tft.setCursor(ANCHO_PANTALLA - w, 0);
  tft.setTextColor(color, ILI9341_BLACK);
  tft.print(contador);
}

void imprimeMetrosSerie(uint16_t color)
{
  int16_t x, y;
  uint16_t w, h;

  char mts[5];
  snprintf(mts, sizeof(mts), "%lum", contador.piletas * LARGO_PILETA);

  tft.setTextSize(3);
  tft.getTextBounds(mts, 0, 0, &x, &y, &w, &h);
  tft.setTextColor(color, ILI9341_BLACK);
  tft.setCursor(ANCHO_PANTALLA - w, hContador);
  tft.print(mts);
}

void imprimeMetrosTotales(uint16_t color)
{
  int16_t x, y;
  uint16_t w, h;
  char mts[6];
  snprintf(mts, sizeof(mts), "%lum", contador.total * LARGO_PILETA);

  tft.setTextSize(4);
  tft.getTextBounds(mts, 0, 0, &x, &y, &w, &h);
  tft.setTextColor(color, ILI9341_BLACK);
  tft.setCursor(ANCHO_PANTALLA - w, ALTO_PANTALLA - h);
  tft.print(mts);
}

void imprimeSeries()
{
  tft.setCursor(0, 150);
  tft.setTextSize(2);

  std::map<int, std::pair<int, int>> counts;  // Declare a map named 'counts'

  // Count occurrences of each unique 'piletas' value and store associated 'tiempo'
  for (int i = 0; i < 7; ++i) {
    if (series[i].piletas == 0) continue;
    int piletas = series[i].piletas;
    int tiempo = series[i].tiempo;

    // If the 'piletas' value is not yet in the map, add it
    if (counts.find(piletas) == counts.end()) {
        counts[piletas] = std::make_pair(1, tiempo);
    } else {
        // Otherwise, increment the count and update 'tiempo'
        counts[piletas].first++;
        counts[piletas].second += tiempo;
    }
  }

  for (const auto &entry : counts) {
    int piletas = entry.first;
    int count = entry.second.first;
    int totalTiempo = entry.second.second;

    char reloj[6];
    creaRelojGPT(totalTiempo, reloj, sizeof(reloj));

    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print(count);
    tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    tft.print(F("x "));
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.print(piletas);
    tft.print(F(" "));
    tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
    tft.print(count * piletas * LARGO_PILETA);
    tft.print(F("m "));
    tft.setTextColor(0x90D5, ILI9341_BLACK);
    tft.print(reloj);
    tft.print(F(" "));
    tft.println();
    tft.setCursor(0, tft.getCursorY()+5);
  }

}
