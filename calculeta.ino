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
#define RENGLONES_SERIE         7

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
datos_serie series[RENGLONES_SERIE];

typedef struct
{
  unsigned long pileta;
  unsigned long serie;
  unsigned long descanso;
  unsigned long total;
} cronometros;
cronometros cronometro;

typedef struct
{
  unsigned long piletas;
  unsigned long series;
  unsigned long total;
} contadores;
contadores contador;

uint16_t hContador; // altura del contador para posicionar otras cosas

void setup() {
  boton.begin();  // inicializa el boton
  tft.begin();    // inicializa la pantalla
  tft.fillScreen(0x9E1E);
  tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, 0x002D);
}

void loop() {

  if (!contando) // no estamos contando
  {
    if (boton.pressed())
    {
      contando = true;
      timestampBotonPresionado = millis();
      tft.fillScreen(ILI9341_BLACK);
      imprimeContador((String)contador.piletas, ILI9341_WHITE);
      imprimeMetrosSerie(ILI9341_GREEN);
      imprimeMetrosTotales(ILI9341_GREEN);
      imprimeSeries();
    }
  }

  else // comenzó a funcionar el aparato
  {
    if (millis() - timestampUltimoCronometroImpreso >= UN_SEGUNDO) //si pasó un segundo...
    {
      // incrementa los cronometros
      cronometro.pileta++;
      if (!descansando) cronometro.serie++;
      if (descansando) cronometro.descanso++;
      cronometro.total++;
      // actualiza el contador para que titile mientras espera
      if (descansando) imprimeCronometroDescanso(cronometro.descanso, ILI9341_ORANGE);
      // actualiza los cronometros en la pantalla
      if (!descansando) imprimeCronometroPileta(cronometro.pileta, ILI9341_WHITE, puedeIncrementarPileta);
      imprimeCronometroSerie(cronometro.serie, ILI9341_WHITE);
      imprimeCronometroTotal(cronometro.total, ILI9341_WHITE);
      // toma el tiempo en el que se imprimieron los cronometros
      timestampUltimoCronometroImpreso = millis();
    }

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
            tft.fillRect(0, 0, 240, 120, ILI9341_BLACK);
          }
        }
        imprimeSeries();
        imprimeMetrosSerie(ILI9341_GREEN);
        imprimeMetrosTotales(ILI9341_GREEN);
        if (!descansando) imprimeContador((String)contador.piletas, ILI9341_WHITE);
        if (!descansando) imprimeCronometroPileta(cronometro.pileta, ILI9341_WHITE, true);
      }

      else // descansando = true
      {
        descansando = false;
        cronometro.pileta = 0;
        tft.fillRect(0, 0, 240, 120, ILI9341_BLACK);
        imprimeContador((String)contador.piletas, ILI9341_WHITE);
      }
    }

    if(boton.released()) {
      //RESET
      reset();
    }

    puedeIncrementarPileta = sePuedeIncrementarPileta();

  }
}

void reset()
{
  if (millis() - timestampBotonPresionado >= TIEMPO_PULSO_RESET ) {
    timestampBotonPresionado = millis();
    contando = false;

    cronometro.pileta= 0;
    cronometro.serie= 0;
    cronometro.total = 0;

    contador.piletas = 0;
    contador.total = 0;

    for (int i = 0; i < RENGLONES_SERIE; i++)
    {
      series[i].tiempo = 0;
      series[i].piletas = 0;
    }
    tft.fillScreen(0x9E1E);
    tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, 0x002D);
  }
}

bool sePuedeIncrementarPileta() // vuelve a poner en true cuando pasó el tiempo que tenia el usuario para incrementar una serie
{
  if (millis() - timestampBotonPresionado >= TIEMPO_INCREMENTO_SERIE)
  {
    return true;
  }
  else 
  {
    return false;
  }
}

void imprimeMensajeIncrementarSerie(bool imprime)
{
  tft.setTextSize(2);
  tft.setCursor(0,40);
  if (imprime)
  {
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  }
  else
  {
    tft.setTextColor(ILI9341_BLACK, ILI9341_BLACK);
  }
  tft.write(0x15); 
  tft.print(" ");
  tft.write(0x18); 
}

void imprimeCronometroTotal(unsigned long cronometro, uint16_t color)
{
  int16_t x, y;
  uint16_t w, h;
  String reloj = creaReloj(cronometro);

  tft.setTextColor(color, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.getTextBounds(reloj, 0, 0, &x, &y, &w, &h);
  tft.setCursor(0, ALTO_PANTALLA - h);
  tft.println(" " + reloj);
}

void imprimeCronometroSerie(unsigned long cronometro, uint16_t color)
{
  String reloj = creaReloj(cronometro);

  tft.setTextColor(color, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, hContador);
  tft.write(0x0F); // sibolo del reloj
  tft.println(" " + reloj);
}

void imprimeCronometroDescanso(unsigned long cronometro, uint16_t color)
{
  String reloj = creaReloj(cronometro);

  tft.setTextColor(color, ILI9341_BLACK);
  tft.setTextSize(8);
  tft.setCursor(0, 25);
  tft.println(reloj);
}

void imprimeCronometroPileta(unsigned long cronometro, uint16_t color, bool ventanaSerie)
{
  String reloj = creaReloj(cronometro);
  if (contador.piletas == 10) tft.fillRect(0, 0, 60, 60, ILI9341_BLACK);
  if (!descansando)
    tft.setTextColor((contador.piletas != 0) ? ((ventanaSerie) ? color : 0xFB54) : color, ILI9341_BLACK);
  else
    tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  tft.setTextSize(contador.piletas >= 10 ? 2 : 3);
  tft.setCursor(0, 0);
  tft.println(reloj);
}

String creaReloj(unsigned long tiempo)
{
    String segundos = String(tiempo % 60);
    segundos = segundos.length() == 1 ? "0" + segundos : segundos;

    String minutos = String((int)floor(tiempo / 60));
    minutos = minutos.length() == 1 ? "0" + minutos : minutos;

    return minutos + ":" + segundos;
}

void imprimeContador(String numero, uint16_t colorTexto)
{
  int16_t x, y;
  uint16_t w, h;

  tft.setTextWrap(false);
  tft.setTextSize(15);
  tft.getTextBounds(numero, 0, 0, &x, &y, &w, &h);
  tft.setCursor(ANCHO_PANTALLA - w, 0);
  tft.setTextColor(colorTexto, ILI9341_BLACK);
  tft.print(numero);
  
  hContador = h;
}

void imprimeMetrosSerie(uint16_t color)
{
  int16_t x, y;
  uint16_t w, h;
  String mtsSerie = "  " + String(contador.piletas * LARGO_PILETA) + "m";

  tft.setTextSize(3);
  tft.getTextBounds(mtsSerie, 0, 0, &x, &y, &w, &h);
  tft.setTextColor(color, ILI9341_BLACK);
  tft.setCursor(ANCHO_PANTALLA - w, hContador);
  tft.print(mtsSerie);
}

void imprimeMetrosTotales(uint16_t color)
{
  int16_t x, y;
  uint16_t w, h;
  String mtsTotal = String(contador.total * LARGO_PILETA) + "m";

  tft.setTextSize(4);
  tft.getTextBounds(mtsTotal, 0, 0, &x, &y, &w, &h);
  tft.setTextColor(color, ILI9341_BLACK);
  tft.setCursor(ANCHO_PANTALLA - w, ALTO_PANTALLA - h);
  tft.print(mtsTotal);
}

void imprimeSeries()
{
  tft.setCursor(0, 150);
  tft.setTextSize(2);

  std::map<int, std::pair<int, int>> counts;  // Declare a map named 'counts'

  // Count occurrences of each unique 'piletas' value and store associated 'tiempo'
  for (int i = 0; i < RENGLONES_SERIE; ++i) {
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
    tft.print(creaReloj(totalTiempo));
    tft.print(" ");
    tft.println();
    tft.setCursor(0, tft.getCursorY()+5);
  }

}
