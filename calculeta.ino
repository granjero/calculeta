#include "Button.h" // asegurarse de tener la ultima version madleech/Button
#include "Chrono.h"
#include "DataSeries.h"
#include "Pantalla.h"
#include "Conexion.h"


// debug
#define DEBUG false

#if DEBUG
#define debug(msg) Serial.print(msg)
#define debugln(msg) Serial.println(msg)
#else
#define debug(msg)
#define debugln(msg)
#endif

#define BOTON D1
#define CANT_MAX_SERIES 128
#define CANT_MAX_PILETAS 128

// #define TIEMPO_PULSO_RESET      3000

bool seHaPresionadoElBoton = false;
bool puedoPonerCelesteElContador = false;

Button boton(BOTON, 750); 

Chrono cronoUnSegundo;
Chrono cronoPileta(Chrono::SECONDS);
Chrono cronoSerie(Chrono::SECONDS);
Chrono cronoTotal(Chrono::SECONDS);

Pantalla pantalla;

Conexion conexion;

DataSeries dataSerie[CANT_MAX_SERIES];

enum Calculeta { // estados del programa
  INICIO,
  PILETAS,
  SERIES,
  DESCANSANDO,
  RESUMEN
};
Calculeta calculeta = INICIO;

typedef struct {
  boolean pileta; // 1 tiempo pileta | 0 tiempo descanso
  unsigned int tiempo;
} DataPiletas;
DataPiletas pileta[CANT_MAX_PILETAS]; // 6400 mts

typedef struct {
  unsigned int piletas;
  unsigned int series;
  unsigned int total;
  unsigned int totalConDescansos;
} Contadores;
Contadores contador;

void setup(){
  if (DEBUG) Serial.begin(115200);
  debugln("");
  debugln("Calculeta");
  debugln(calculeta);

  boton.begin();  // inicializa el boton
  inicializaCalculeta(&calculeta);
}

void loop() {
  seHaPresionadoElBoton = boton.pressed();

  calculeta = contandoPiletasOSeries(calculeta, cronoPileta.hasPassed(10));

  if (cronoUnSegundo.hasPassed(1000)) { // cada vez que pasa un segundo
    cronoUnSegundo.restart();

    switch (calculeta) {
      case INICIO:
        break;

      case RESUMEN:
        break;

      default:
        if (calculeta != DESCANSANDO) {
          // pantalla.cronos(cronoPileta.elapsed(), AN_CHAR_CRON, AL_CHAR_CRON * 2, contador.piletas < 10 ? 3 : 2);
          pantalla.cronos(cronoSerie.elapsed(), AN_CHAR_CRON * 2, AL_CHAR_CONTADOR, 3);
        } else {
          pantalla.cronoDescanso(cronoPileta.elapsed());
        }
        pantalla.cronos(cronoTotal.elapsed(), 5, AL_PANTALLA - AL_FUENTE_UNITARIA * 3, 3);
        break;
    }

    if (puedoPonerCelesteElContador) // barra de progreso
    {
      pantalla.barraProgreso(cronoPileta.elapsed());
    }
  }

  if (puedoPonerCelesteElContador && cronoPileta.hasPassed(11))
  {
    poneCelesteElContador();
    puedoPonerCelesteElContador = false;
  }

  if (seHaPresionadoElBoton) {
    debug(F("boton presionado -> estado: "));
    debugln(calculeta);

    switch (calculeta) {
      case INICIO: // se pulsa en la pantalla de Inicio
        debugln(" pulso pantalla de inicio");

        calculeta = PILETAS;

        cronoUnSegundo.restart();
        cronoPileta.restart();
        cronoSerie.restart();
        cronoTotal.restart();

        reseteaSeries();
        reseteaPiletas();

        pantalla.inicio();
        // pantalla.cronos(cronoPileta.elapsed(), AN_CHAR_CRON, AL_CHAR_CRON * 2 , contador.piletas < 10 ? 3 : 2);
        pantalla.cronos(cronoSerie.elapsed(), AN_CHAR_CRON * 2, AL_CHAR_CONTADOR, 3);
        pantalla.cronos(cronoTotal.elapsed(), 5, AL_PANTALLA - AL_FUENTE_UNITARIA * 3, 3);
        pantalla.contadorPiletas(contador.piletas, CELESTE); // imprime el contador
        pantalla.metrosSerie(contador.piletas); // imprime los metros  
        pantalla.metrosTot(contador.total); // imprime los metros totales 
        break;

      case PILETAS: // se pulsa mientras se está nadando
        calculeta = SERIES;
        puedoPonerCelesteElContador = true;
        guardaPileta(cronoPileta.elapsed());
        incrementaContadores(true, false, true, true); // piletas[*] series[] total[*] totalConDescansos[*]
        cronoPileta.restart();
        // pantalla.cronos(cronoPileta.elapsed(), AN_CHAR_CRON, AL_CHAR_CRON * 2 , contador.piletas < 10 ? 3 : 2);
        pantalla.contadorPiletas(contador.piletas, ILI9341_GREEN); // imprime el contador
        pantalla.metrosSerie(contador.piletas); // imprime los metros  
        pantalla.metrosTot(contador.total); // imprime los metros totales 

        debugln(" pulso contando piletas");

        break;

      case SERIES: // el contador estaba rosa 
        puedoPonerCelesteElContador = false;
        if (contador.piletas !=  0) { // comienza el descanso
          calculeta = DESCANSANDO;
          guardaSerie(cronoSerie.elapsed() - cronoPileta.elapsed()); // guarda los datos de la serie en la estructura
          pantalla.borraContador();
          pantalla.cronos(cronoSerie.elapsed() - cronoPileta.elapsed(), AN_CHAR_CRON * 2, AL_CHAR_CONTADOR, 3);
          pantalla.cronoDescanso(cronoPileta.elapsed());
          pantalla.metrosSerie(contador.piletas); // imprime los metros  
          pantalla.anotaSeries(dataSerie); // imprime las series
          reseteaContadores(true, false, false, false); // pileta[*] serie[] total[] totalConDescansos[]
        } else {
          calculeta = RESUMEN;
          pantalla.resumen(2, dataSerie);
          pantalla.metrosTot(contador.total); // imprime los metros totales 
          pantalla.cronos(cronoTotal.elapsed(), 5, AL_PANTALLA - AL_FUENTE_UNITARIA * 3, 3);

          debugln("Resumen");
          debugln(piletasParaDB());

          pantalla.esperandoWiFi();
          conexion.conectar();
          pantalla.conectadoWiFi();
          int httpCode = conexion.pileta( piletasParaDB() );
          if( httpCode == 201 ) pantalla.finalizadoWiFi();
          else pantalla.errorWiFi(httpCode);
        }

        debugln(" pulso con el contador rosa para cerrar la serie");
        debug(" tiempo pileta ");
        debugln(cronoPileta.elapsed());

        break;

      case DESCANSANDO:
        calculeta = PILETAS;
        puedoPonerCelesteElContador = true;
        guardaDescanso(cronoPileta.elapsed());
        guardaSerieDescanso(cronoPileta.elapsed());
        pantalla.borraContador();
        pantalla.contadorPiletas(contador.piletas, CELESTE); // imprime el contador
        pantalla.metrosSerie(contador.piletas); // imprime los metros de la serie 
        incrementaContadores(false, true, false, true); // piletas[] series[*] total[] totalConDescansos[*]
        cronoPileta.restart();
        cronoSerie.restart();
        debugln(" pulso mientra se estaba descansando");
        break;

      case RESUMEN:
        debugln(F("resumen"));
        pantalla.resumen(1, dataSerie);
        pantalla.metrosTot(contador.total); // imprime los metros totales 
        pantalla.cronos(cronoTotal.elapsed(), 5, AL_PANTALLA - AL_FUENTE_UNITARIA * 3, 3);
        break;
    }
  }
}


void inicializaCalculeta(Calculeta* nuevoEstado) {
  *nuevoEstado = INICIO;

  cronoUnSegundo.restart();
  cronoPileta.restart();
  cronoSerie.restart();
  cronoTotal.restart();

  reseteaContadores(true, true, true, true);
  reseteaSeries();
  pantalla.logoCalculeta();
  pantalla.macAddress(conexion.macAddress);
  return;
}

/*VARIABLES*/
void incrementaContadores(boolean piletas, boolean series, boolean total, boolean totalConDescansos) {
  if (piletas) contador.piletas++;
  if (series) contador.series++;
  if (total) contador.total++;
  if (totalConDescansos) contador.totalConDescansos++;
  return;
}

void reseteaContadores(boolean piletas, boolean series, boolean total, boolean totalConDescansos) {
  if (piletas) contador.piletas = 0;
  if (series) contador.series = 0;
  if (total) contador.total = 0;
  if (totalConDescansos) contador.totalConDescansos = 0;
  return;
}

void reseteaSeries() {
  for (int i = 0; i < CANT_MAX_SERIES; i++) {
    dataSerie[i].tiempo = 0;
    dataSerie[i].piletas = 0;
    dataSerie[i].descanso = 0;
  }
  return;
}

void reseteaPiletas() {
  for (int i = 0; i < CANT_MAX_PILETAS; i++) {
    pileta[i].tiempo = 0;
    pileta[i].pileta = 0;
  }
  return;
}

void guardaPileta(unsigned long tiempo) {
  pileta[contador.totalConDescansos].pileta = true;
  pileta[contador.totalConDescansos].tiempo = tiempo;
  return;
}

void guardaDescanso(unsigned long tiempo) {
  pileta[contador.totalConDescansos].pileta = false;
  pileta[contador.totalConDescansos].tiempo = tiempo;
  return;
}

void guardaSerie(unsigned long tiempo) {
  dataSerie[contador.series].piletas = contador.piletas;
  dataSerie[contador.series].tiempo = tiempo;
  return;
}

void guardaSerieDescanso(unsigned long tiempo) {
  dataSerie[contador.series].descanso = tiempo;
  return;
}

Calculeta contandoPiletasOSeries(Calculeta estado, bool pasaronDiezSegundos) {
  switch(estado) {
    case INICIO:
      break;

    case DESCANSANDO:
      break;

    case RESUMEN:
      break;

    default:
      if (pasaronDiezSegundos)
      {
        return PILETAS;
      }
      else
      {
        return SERIES;
      }
      break;
  }
  return estado;
}

void poneCelesteElContador()
{
  pantalla.contadorPiletas(contador.piletas, CELESTE);
}

String piletasParaDB() {
  String datos;
  // STRING TEST
  for (int i = 0; i < CANT_MAX_PILETAS; i++) {
    if(pileta[i].tiempo > 0) {
      if (pileta[i].pileta) datos += "P:";
      else datos += "D:";
      datos += pileta[i].tiempo;
      if (pileta[i].pileta) datos += ",";
      else if (pileta[i+1].tiempo > 0) datos += "|";
    }
  }
  return datos; 
}
