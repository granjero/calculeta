#include <map>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Button.h"
#include "logo.h"

#define BOTON     D1
#define TFT_CS    D2
#define TFT_RST   D3
#define TFT_DC    D4
#define TFT_CLK   D5
#define TFT_MOSI  D7
// #define TFT_MISO  D8

#define LARGO_PILETA    50
#define ANCHO_PANTALLA  240 
#define ALTO_PANTALLA   320

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

Button boton(BOTON);

boolean contando = false;
boolean incrementarSerie = false;

unsigned long cronometroPiletas= 0;
unsigned long cronometroTotal = 0;

int contadorPiletas = 0;
int contadorTotal = 0;
int contadorSeries = 0;

unsigned long timestampUltimoCronometroImpreso = 0;
int unSegundo = 1000; // un segundo = mil milisegundos
int tiempoParaIncrementarSerie = 3000; // el tiempo que tiene el usuario para incrementar series
int tiempoParaResetar = 3500;
unsigned long timestampBotonPresionado = 0;

typedef struct
{
  int piletas;
  int tiempo;
} datos_serie;

datos_serie series[20];

// coordenadas y tamaños de las cosas impresas.
int16_t xCronometro, yCronometro;
uint16_t wContador, hContador, wMensajeMtsSerie, hMensajeMtsSerie, wCronometro, hCronometro;

void setup() {
  boton.begin();
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_WHITE);
  tft.fillScreen(0x7DFA);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  
  tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, ILI9341_NAVY);
}

void loop() {

  if (!contando) // no estamos contando
  {
    if (boton.pressed())
    {
      contando = true;
      timestampBotonPresionado = millis();
      tft.fillScreen(ILI9341_BLACK);
      imprimeContador(0, ILI9341_WHITE);
      imprimeMetros(ILI9341_GREEN, ILI9341_YELLOW);
      imprimeSeries();
    }
  }
  else // comenzó a funcionar el aparato
  {
    if (boton.pressed())
    {
      timestampBotonPresionado = millis();
      if(!incrementarSerie) // aumenta el numero de piletas
      {
        incrementarSerie = true;
        contadorPiletas++;
        contadorTotal++;
        // tft.fillScreen(ILI9341_RED);
        // tft.fillScreen(ILI9341_BLACK);
      }
      else // aumenta el numero de series
      {
        if (contadorPiletas != 0) 
        {
          series[contadorSeries].piletas = contadorPiletas;
          series[contadorSeries].tiempo = cronometroPiletas;
          contadorSeries++;

          contadorPiletas = 0;
          cronometroPiletas = 0;
        }
      }
      imprimeSeries();
      imprimeMetros(ILI9341_GREEN, ILI9341_GREENYELLOW);
      imprimeContador(contadorPiletas, ILI9341_WHITE);
    }
    imprimeCronometros(ILI9341_WHITE);
    sePuedeIncrementarSerie();

    // RESET
    if(boton.released()) {
      if (millis() - timestampBotonPresionado >= tiempoParaResetar) {
        timestampBotonPresionado = millis();
        contando = false;
        cronometroPiletas= 0;
        cronometroTotal = 0;
        contadorPiletas = 0;
        contadorTotal = 0;

        for (int i = 0; i < 10; )
        {
          series[i].tiempo = 0;
          series[i].piletas = 0;
        }
        
        tft.fillScreen(ILI9341_WHITE);
        tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, ILI9341_NAVY);
      }
    }
  }
}

void sePuedeIncrementarSerie()
{
  if (millis() - timestampBotonPresionado >= tiempoParaIncrementarSerie)
  {
    incrementarSerie = false;
  }
}

void imprimeCronometros(uint16_t color)
{
  if (millis() - timestampUltimoCronometroImpreso >= unSegundo)
  {
    timestampUltimoCronometroImpreso = millis();
    cronometroPiletas++;
    cronometroTotal++;

    String relojSerie = reloj(cronometroPiletas);
    String relojTotal = reloj(cronometroTotal);

    tft.setTextColor(color, ILI9341_BLACK);
    tft.setTextSize(3);
    tft.getTextBounds((String)relojSerie, 0, 0, &xCronometro, &yCronometro, &wCronometro, &hCronometro);
    tft.setCursor(0, hContador);
    tft.write(0x0F); // sibolo del reloj
    tft.println(" " + relojSerie);

    tft.setCursor(0, ALTO_PANTALLA - hCronometro);
    tft.println(" " + relojTotal);
  }
}

String reloj(unsigned int tiempo)
{
    String segundos = String(tiempo % 60);
    segundos = segundos.length() == 1 ? "0" + segundos : segundos;

    String minutos = String((int)floor(tiempo / 60));
    minutos = minutos.length() == 1 ? "0" + minutos : minutos;

    String reloj = minutos + ":" + segundos;

    return reloj;
}

void imprimeContador(int numero, uint16_t colorTexto)
{
  int16_t x, y;
  uint16_t w, h;

  String mensaje = " " + (String)numero;
  tft.setTextWrap(false);
  tft.setTextSize(15);
  tft.setCursor(0,0);
  tft.setTextColor(colorTexto, ILI9341_BLACK);
  tft.getTextBounds((String)mensaje, 0, 0, &x, &y, &w, &h);
  tft.setCursor(ANCHO_PANTALLA - w, 0);
  tft.print(mensaje);
  
  hContador = h;
}

void imprimeMetros(uint16_t colorSerie, uint16_t colorTotal)
{
  int16_t x, y;
  uint16_t w, h;
  String mtsSerie = "  " + String(contadorPiletas * LARGO_PILETA) + "m";
  String mtsTotal = String(contadorTotal * LARGO_PILETA) + "m";

  tft.setTextSize(3);
  tft.getTextBounds((String)mtsSerie, 0, 0, &x, &y, &w, &h);
  tft.setTextColor(colorSerie, ILI9341_BLACK);
  tft.setCursor(ANCHO_PANTALLA - w, hContador);
  tft.print(mtsSerie);

  tft.setTextSize(4);
  tft.getTextBounds((String)mtsTotal, 0, 0, &x, &y, &w, &h);
  tft.setTextColor(colorTotal, ILI9341_BLACK);
  tft.setCursor(ANCHO_PANTALLA - w, ALTO_PANTALLA - h);
  tft.print(mtsTotal);
}

void imprimeSeries()
{
  tft.setCursor(0, 150);
  tft.setTextSize(2);

  std::map<int, std::pair<int, int>> counts;  // Declare a map named 'counts'

  // Count occurrences of each unique 'piletas' value and store associated 'tiempo'
  for (int i = 0; i < 10; ++i) {
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

    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print(count);
    tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    tft.print("x ");
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.print(piletas);
    tft.print(" ");
    tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
    tft.print(count * piletas * LARGO_PILETA);
    tft.print("m ");
    tft.setTextColor(0x90D5, ILI9341_BLACK);
    tft.print(reloj(totalTiempo));
    tft.print(" ");
    tft.println();
    tft.setCursor(0, tft.getCursorY()+5);
  }

  // std::map<int, int> counts;  // Using a map to store counts
  //
  // // Count occurrences of each unique 'piletas' value
  // for (int i = 0; i < 10; ++i) {
  //   if (series[i].piletas == 0) continue;
  //   int piletas = series[i].piletas;
  //   counts[piletas]++;
  // }
  //
  // // Print the counts in the desired format
  // for (const auto &entry : counts) {
  //   tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  //   tft.print(entry.second);
  //   tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  //   tft.print("x");
  //   tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  //   tft.print(entry.first);
  //   tft.print(" ");
  //   tft.setTextColor(0x90D5, ILI9341_BLACK);
  //   tft.print(entry.second * entry.first * LARGO_PILETA);
  //   tft.println("m  ");
  //   tft.setCursor(0, tft.getCursorY()+2);
  // }


}
