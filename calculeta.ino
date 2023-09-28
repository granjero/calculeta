#include <map>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Button.h"
#include "Logo.h"

#include <Fonts/FreeSans9pt7b.h>

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
#define CANT_PILETAS            255
#define ALTO_FUENTE_UNITARIA    8
#define ANCHO_FUENTE_UNITARIA   6
#define TAM_FUENTE_CONTADOR     15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Button boton(BOTON);

boolean seHaPresionadoElBoton = false;
boolean seHaLiberadoElBoton = false;

unsigned long ultimoSegundo = 0;
unsigned long timestampBotonPresionado = 0;

enum Calculeta { // estados del programa
    AUMENTA_PILETAS,
    AUMENTA_SERIES,
    ESPERANDO_INICIO,
    DESCANSANDO,
    VER_RESUMEN
  };

Calculeta calculeta = ESPERANDO_INICIO;

typedef struct
{
  boolean pileta; // 1 tiempo pileta | 0 tiempo descanso
  unsigned int tiempo;
} cuenta_piletas;
cuenta_piletas piletas[CANT_PILETAS];

typedef struct
{
  unsigned int piletas;
  unsigned int tiempo;
  unsigned int descanso;
} cuenta_series;
cuenta_series series[CANT_MAX_SERIES];

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
  unsigned int totalConDescansos;
} contadores;
contadores contador;

unsigned int hContador = TAM_FUENTE_CONTADOR * ALTO_FUENTE_UNITARIA; // altura del contador para posicionar otras cosas

void setup() 
{
  Serial.begin(115200);
  boton.begin();  // inicializa el boton
  tft.begin();    // inicializa la pantalla

  inicializaCalculeta(&calculeta);
}

void loop() 
{
  seHaPresionadoElBoton = boton.pressed(); 
  seHaLiberadoElBoton = boton.released();  

  queEstamosAumentando(&calculeta, calculeta);

  if ((millis() - ultimoSegundo >= UN_SEGUNDO)) // cada vez que pasa un segundo
  {
    switch (calculeta) {
      case ESPERANDO_INICIO:
        break;
      
      case VER_RESUMEN:
        break;

      default:
          ultimoSegundo = millis();
          incrementaCronometros(calculeta);

          if (calculeta == DESCANSANDO) pantallaCronometroDescanso();
          if (calculeta != DESCANSANDO) pantallaCronometroPileta();
          pantallaCronometroSerie();
          pantallaCronometroTotal();
        break;
    }
  }

  if (seHaPresionadoElBoton)
  {
    timestampBotonPresionado = millis(); //tomo el tiempo en el que se presionó el botón

    switch (calculeta) {
      case ESPERANDO_INICIO:
        calculeta = AUMENTA_PILETAS;
        tft.fillScreen(0); // pinta la pantalla de negro
        reseteaCronometros(true, true, true, true);
        reseteaContadores(true, true, true, true);
        reseteaSeries();
        reseteaPiletas();
        pantallaContadorPiletas(); // imprime el contador
        pantallaMetrosSerie(); // imprime los metros serie
        pantallaMetrosTotales(); //imprime los metros totales
        break;

      case AUMENTA_PILETAS:
        calculeta = AUMENTA_SERIES;
        guardaDatosPileta();
        incrementaContadores(true, false, true, true); // piletas total totalConDescansos
        reseteaCronometros(true, false, false, false); // pileta
        pantallaContadorPiletas(); // imprime el contador en pantalla
        pantallaCronometroPileta();
        pantallaMetrosSerie(); // imprime los metros de la serie
        pantallaMetrosTotales(); // imprime los metros totales0
        break;

      case AUMENTA_SERIES:
        if (contador.piletas !=  0) // no tiene sentido una serie de 0 piletas
        {
          calculeta = DESCANSANDO;
          guardaDatosSerie(); // guarda los datos de la serie en la estructura
          tft.fillRect(0, 0, 240, 145, ILI9341_BLACK); // borra el contador y el cronometro pileta 
          pantallaMetrosSerie(); // imprime los metros de la ultima serie
          pantallaCronometroSerie(); // imprime el tiempo de la ultima serie
          pantallaSeries(); // imprime las series
          reseteaContadores(true, false, false, false); // pileta
          reseteaCronometros(true, false, true, false); // pileta descanso
        }
        else 
        {
          calculeta = VER_RESUMEN;
          pantallaResumen(2);
        }
        break;

      case DESCANSANDO:
        calculeta = AUMENTA_PILETAS;
        tft.fillRect(0, 0, 240, 145, ILI9341_BLACK); // borra el contador y el cronometro pileta 
        pantallaContadorPiletas(); // imprime el contador
        pantallaMetrosSerie();
        guardaDescansoPileta();
        guardaDescansoSerie();
        incrementaContadores(false, true, false, true); // serie totalConDescansos
        reseteaCronometros(true, true, true, false);
        break;

      case VER_RESUMEN:
        pantallaResumen(1);
        break;
    }
  }
  if (seHaLiberadoElBoton)
  {
    if(millis() - timestampBotonPresionado >= TIEMPO_PULSO_RESET)
    inicializaCalculeta(&calculeta);
  }
}

void inicializaCalculeta(Calculeta* nuevoEstado)
{
  *nuevoEstado = ESPERANDO_INICIO;
  reseteaCronometros(true, true, true, true);
  reseteaContadores(true, true, true, true);
  reseteaSeries();

  tft.fillScreen(0x9E1E);
  tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, 0x002D);
  return;
}

void reseteaContadores(boolean piletas, boolean series, boolean total, boolean totalConDescansos)
{
  if (piletas) contador.piletas = 0;
  if (series) contador.series = 0;
  if (total) contador.total = 0;
  if (totalConDescansos) contador.totalConDescansos = 0;
  return;
}

void incrementaContadores(boolean piletas, boolean series, boolean total, boolean totalConDescansos)
{
  if (piletas) contador.piletas++;
  if (series) contador.series++;
  if (total) contador.total++;
  if (totalConDescansos) contador.totalConDescansos++;
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

void incrementaCronometros(Calculeta estado)
{
  cronometro.pileta++;
  cronometro.total++;
  if (estado == DESCANSANDO) cronometro.descanso++;
  else cronometro.serie++;
  return;
}

void reseteaSeries()
{
  for (int i = 0; i < CANT_MAX_SERIES; i++)
  {
    series[i].tiempo = 0;
    series[i].piletas = 0;
    series[i].descanso = 0;
  }
  return;
}

void guardaDatosSerie()
{
  series[contador.series].piletas = contador.piletas;
  series[contador.series].tiempo = cronometro.serie;
  return;
}

void guardaDescansoSerie()
{
  series[contador.series].descanso = cronometro.descanso;
  return;
}

void reseteaPiletas()
{
  for (int i = 0; i < CANT_PILETAS; i++)
  {
    piletas[i].tiempo = 0;
    piletas[i].pileta = 0;
  }
  return;
}

void guardaDatosPileta()
{
  piletas[contador.totalConDescansos].pileta = true;
  piletas[contador.totalConDescansos].tiempo = cronometro.pileta;
  return;
}

void guardaDescansoPileta()
{
  piletas[contador.totalConDescansos].pileta = false;
  piletas[contador.totalConDescansos].tiempo = cronometro.pileta;
  return;
}

void queEstamosAumentando(Calculeta* nuevoEstado, Calculeta estado ) 
{
  switch(estado){
    case ESPERANDO_INICIO:
      break;

    case DESCANSANDO:
      break;

    case VER_RESUMEN:
      break;

    default:
      if (millis() - timestampBotonPresionado >= TIEMPO_INCREMENTO_SERIE)
      {
        *nuevoEstado = AUMENTA_PILETAS;
      }
      else
      {
        *nuevoEstado = AUMENTA_SERIES;
      }
      break;
  }
  return;
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
  char reloj[6];
  dibujaReloj(cronometro.pileta, reloj, sizeof(reloj));

  if (contador.piletas == 10) tft.fillRect(0, 0, 60, 60, ILI9341_BLACK);
  tft.setTextColor(calculeta == AUMENTA_PILETAS ? ILI9341_WHITE : 0xFB54, ILI9341_BLACK);
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

void pantallaResumen(int tamanioFuente)
{
  char reloj[6];
  int tam = tamanioFuente;

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(tamanioFuente + 1);
  // tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.println(F("Resumen"));
  tft.setCursor(0, tft.getCursorY() + 5);
  tft.setTextSize(tamanioFuente);
  for (int i = 0; i < CANT_MAX_SERIES; ++i)
  {
    if (series[i].piletas == 0) continue; // cuando no hay mas datos...
    tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    tft.write(0x10); // sibolo triangulo de costado
    // tft.print(F(">"));
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    // tft.print(F(" "));
    tft.print(series[i].piletas);
    tft.print(F(" "));
    tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
    tft.print(series[i].piletas * LARGO_PILETA);
    tft.print(F("m "));
    tft.setTextColor(0x90D5, ILI9341_BLACK);
    dibujaReloj(series[i].tiempo, reloj, sizeof(reloj));
    tft.print(reloj);
    tft.print(F(" "));
    tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    dibujaReloj(series[i].descanso, reloj, sizeof(reloj));
    tft.println(reloj);
    tft.setCursor(0, tft.getCursorY() + 3);
  }
  // tft.setFont();
  pantallaCronometroTotal();
  pantallaMetrosTotales();
  // for (int i = 0; i < CANT_PILETAS; i++)
  // {
  //   if(piletas[i].tiempo > 0)
  //   {
  //     if (piletas[i].pileta) 
  //     Serial.print(F("P: "));
  //     else
  //     Serial.print(F("D: "));
  //     Serial.println(piletas[i].tiempo);
  //   }
  // }
  return;
}
