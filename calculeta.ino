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
boolean contandoPiletas = false;
boolean descansando = false;
boolean seHaPresionadoElBoton = false;

unsigned long ultimoSegundo = 0;
unsigned long timestampBotonPresionado = 0;

typedef struct
{
  int piletas;
  int tiempo;
} datos_series;
datos_series series[CANT_MAX_SERIES];

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

  inicializaCalculeta();
}

void loop() 
{
  seHaPresionadoElBoton = boton.pressed(); // chequeo si se ha presionado el botón

  contandoPiletas = sePuedeIncrementarPileta();

  if ((millis() - ultimoSegundo >= UN_SEGUNDO) && contando) // cada vez que pasa un segundo
  {
    ultimoSegundo = millis();
    incrementaCronometros();

    pantallaCronometroDescanso();
    pantallaCronometroPileta();
    pantallaCronometroSerie();
    pantallaCronometroTotal();
  }

  if (contando // incremento pileta
      && contandoPiletas 
      && !descansando
      && seHaPresionadoElBoton) 
  {
    timestampBotonPresionado = millis(); //tomo el tiempo en el que se presionó el botón
    contandoPiletas = !contandoPiletas; // set en falso contandoPiletas.

    incrementaContadores(true, false, true); // piletas y total
    reseteaCronometros(true, false, false, false); // pileta
    pantallaContadorPiletas(); // imprime el contador en pantalla
    pantallaMetrosSerie(); // imprime los metros de la serie
    pantallaMetrosTotales(); // imprime los metros totales0
  }

  else if (contando
      && !contandoPiletas
      && !descansando
      && contador.piletas == 0
      && seHaPresionadoElBoton)
  {
    resumen();
  }

  else if (contando // incremento serie
      && !contandoPiletas 
      && !descansando
      && seHaPresionadoElBoton ) 
  {
    timestampBotonPresionado = millis(); //tomo el tiempo en el que se presionó el botón
    if(contador.piletas !=  0) // no tiene sentido una serie de 0 piletas
    {
      descansando = !descansando; // set en true descansando
    
      incrementaSeries(); // guarda los datos de la serie en la estructura
      incrementaContadores(false, true, false); // serie
      tft.fillRect(0, 0, 240, 145, ILI9341_BLACK); // borra el contador y el cronometro pileta 
      pantallaMetrosSerie(); // imprime los metros de la ultima serie
      pantallaCronometroSerie(); // imprime el tiempo de la ultima serie
      pantallaSeries(); // imprime las series
      reseteaContadores(true, false, false); // pileta
      reseteaCronometros(true, false, true, false); // pileta descanso
    }
  }
  
  else if (descansando // deja de descansar
      && seHaPresionadoElBoton ) 
  {
    timestampBotonPresionado = millis(); //tomo el tiempo en el que se presionó el botón
    descansando = !descansando; // set en false descansando
    tft.fillRect(0, 0, 240, 145, ILI9341_BLACK); // borra el contador y el cronometro pileta 
    pantallaContadorPiletas(); // imprime el contador
    pantallaMetrosSerie();
    reseteaCronometros(true, true, true, false);
  }

  else if (!contando 
      && seHaPresionadoElBoton) // esperando para empezar
  {
    timestampBotonPresionado = millis(); //tomo el tiempo en el que se presionó el botón
    contando = !contando; // se empieza a contar
    tft.fillScreen(0); // borra la pantalla a negro
    reseteaCronometros(true, true, true, true);
    reseteaContadores(true, true, true);
    pantallaContadorPiletas(); // imprime el contador
    pantallaMetrosSerie(); // imprime los metros serie
    pantallaMetrosTotales(); //imprime los metros totales
  }
}

void inicializaCalculeta()
{
  contando = false; 
  reseteaCronometros(true, true, true, true);
  reseteaContadores(true, true, true);
  reseteaSeries();

  tft.fillScreen(0x9E1E);
  tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, 0x002D);
  return;
}

void reseteaContadores(boolean piletas, boolean series, boolean total)
{
  if (piletas) contador.piletas = 0;
  if (series) contador.series = 0;
  if (total) contador.total = 0;
  return;
}

void incrementaContadores(boolean piletas, boolean series, boolean total)
{
  if (piletas) contador.piletas++;
  if (series) contador.series++;
  if (total) contador.total++;
  return;
}

void reseteaCronometros(boolean pileta, boolean serie, boolean descanso, boolean total)
{
  if (pileta) cronometro.pileta = 0;
  if (serie) cronometro.serie = 0;
  if (descanso) cronometro.descanso = 0;
  if (total) cronometro.total = 0;
  return;
}

void incrementaCronometros()
{
  cronometro.pileta++;
  cronometro.total++;
  if (descansando) cronometro.descanso++;
  else cronometro.serie++;
  return;
}

void reseteaSeries()
{
  for (int i = 0; i < CANT_MAX_SERIES; i++)
  {
    series[i].tiempo = 0;
    series[i].piletas = 0;
  }
  return;
}

void incrementaSeries()
{
  series[contador.series].piletas = contador.piletas;
  series[contador.series].tiempo = cronometro.serie;
  return;
}

bool sePuedeIncrementarPileta() // true si pasao el tiempo sin que se incremente una serie. 
{
  return millis() - timestampBotonPresionado >= TIEMPO_INCREMENTO_SERIE;
}

void pantallaCronometroTotal()
{
  int16_t x, y;
  uint16_t w, h;
  char reloj[6];
  dibujaReloj(cronometro.total, reloj, sizeof(reloj));

  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.getTextBounds(reloj, 0, 0, &x, &y, &w, &h);
  tft.setCursor(0, ALTO_PANTALLA - h);
  tft.print(" ");
  tft.println(reloj);
  return;
}

void pantallaCronometroSerie()
{
  char reloj[6];
  dibujaReloj(cronometro.serie, reloj, sizeof(reloj));

  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, hContador);
  tft.write(0x0F); // sibolo del reloj
  tft.print(" ");
  tft.println(reloj);
  return;
}

void pantallaCronometroDescanso()
{
  if (!descansando) return;

  char reloj[6];
  dibujaReloj(cronometro.descanso, reloj, sizeof(reloj));

  tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  tft.setTextSize(8);
  tft.setCursor(0, 25);
  tft.println(reloj);
  return;
}

void pantallaCronometroPileta()
{
  if (descansando) return;
  
  char reloj[6];
  dibujaReloj(cronometro.pileta, reloj, sizeof(reloj));

  if (contador.piletas == 10) tft.fillRect(0, 0, 60, 60, ILI9341_BLACK);
  tft.setTextColor(contandoPiletas ? ILI9341_WHITE : 0xFB54, ILI9341_BLACK);
  tft.setTextSize(contador.piletas >= 10 ? 2 : 3);
  tft.setCursor(0, 0);
  tft.println(reloj);
  return;
}

void dibujaReloj(unsigned int tiempo, char* buffer, size_t bufferSize)
{
  unsigned int segundos = tiempo % 60;
  unsigned int minutos = tiempo / 60;

  snprintf(buffer, bufferSize, "%02lu:%02lu", minutos, segundos);
  return;
}

void pantallaContadorPiletas()
{
  int16_t x, y;
  uint16_t w, h;

  char numero[3];
  sprintf(numero, "%u", contador.piletas);

  tft.setTextWrap(false);
  tft.setTextSize(TAM_FUENTE_CONTADOR);
  tft.getTextBounds(numero, 0, 0, &x, &y, &w, &h);
  tft.setCursor(ANCHO_PANTALLA - w, 0);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  if (contador.piletas == 0)
    tft.fillRect( 240 - TAM_FUENTE_CONTADOR * ANCHO_FUENTE_UNITARIA * 2,
        0,
        TAM_FUENTE_CONTADOR * ANCHO_FUENTE_UNITARIA * 2,
        TAM_FUENTE_CONTADOR * ALTO_FUENTE_UNITARIA, ILI9341_BLACK);
  tft.print(numero);
  return;
}

void pantallaMetrosSerie()
{
  int16_t x, y;
  uint16_t w, h;

  char mts[5];
  snprintf(mts, sizeof(mts), "%lum", contador.piletas * LARGO_PILETA);

  tft.setTextSize(3);
  tft.getTextBounds(mts, 0, 0, &x, &y, &w, &h);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(ANCHO_PANTALLA - w, hContador);
  tft.print(mts);
  return;
}

void pantallaMetrosTotales()
{
  int16_t x, y;
  uint16_t w, h;
  char mts[6];
  snprintf(mts, sizeof(mts), "%lum", contador.total * LARGO_PILETA);

  tft.setTextSize(4);
  tft.getTextBounds(mts, 0, 0, &x, &y, &w, &h);
  tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
  tft.setCursor(ANCHO_PANTALLA - w, ALTO_PANTALLA - h);
  tft.print(mts);
  return;
}

void pantallaSeries()
{
  tft.setCursor(0, 150);
  tft.setTextSize(2);
  tft.fillRect(0, 150, 240, 135, ILI9341_BLACK); // borra las series

  std::map<int, std::pair<int, int>> counts;  // Declara un map llamdo counts

  // Count occurrences of each unique 'piletas' value and store associated 'tiempo'
  for (int i = 0; i < CANT_MAX_SERIES; ++i) {
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
    dibujaReloj(totalTiempo, reloj, sizeof(reloj));

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
  return;
}

void resumen()
{
  char reloj[6];

  contando = false;
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.println(F("Resumen"));
  tft.setCursor(0, tft.getCursorY()+10);
  tft.setTextSize(2);
  for (int i = 0; i < CANT_MAX_SERIES; ++i)
  {
    dibujaReloj(series[i].tiempo, reloj, sizeof(reloj));
    if (series[i].piletas == 0) continue;
    tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    // tft.print(i);
    tft.write(0x10); // sibolo del reloj
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.print(F(" "));
    tft.print(series[i].piletas);
    tft.print(F(" "));
    tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
    tft.print(series[i].piletas * LARGO_PILETA);
    tft.print(F("m "));
    tft.setTextColor(0x90D5, ILI9341_BLACK);
    tft.println(reloj);
    tft.setCursor(0, tft.getCursorY()+5);
  }
  pantallaCronometroTotal();
  pantallaMetrosTotales();
  return;
}
