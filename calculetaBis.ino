#include "Button.h"
#include "DataSeries.h"
#include "Pantalla.h"

#define BOTON     D1

#define UN_SEGUNDO  1000
// #define TIEMPO_INCREMENTO_SERIE 5000 // 10 segundos
#define TIEMPO_PULSO_RESET      3000

unsigned long ultimoSegundo = 0;
unsigned long millisAhora = 0;
unsigned long timestampBotonPresionado = 0;

bool seHaPresionadoElBoton = false;

Button boton(BOTON, 750); // asegurarse de tener la ultima version madleech/Button

Pantalla pantalla;

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

typedef struct {
  unsigned int pileta;
  unsigned int serie;
  unsigned int descanso;
  unsigned int total;
} Cronometros;
Cronometros cronometro;

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println("CalculetaBis");
  Serial.println(calculeta);
  boton.begin();  // inicializa el boton
  inicializaCalculeta(&calculeta);
}

void loop() {
  millisAhora = millis();

  seHaPresionadoElBoton = boton.pressed();

  pasaElTiempoParaIncrementarSeries(&calculeta, calculeta);

  if ((millisAhora - ultimoSegundo >= UN_SEGUNDO)) { // cada vez que pasa un segundo
    ultimoSegundo = millisAhora;

    switch (calculeta) {
      case INICIO:
        break;

      case RESUMEN:
        break;

      default:
        incrementaCronometros(calculeta);

        if (calculeta != DESCANSANDO) {
          pantalla.cronos(cronometro.pileta, AN_FUENTE_UNITARIA * 3, AL_FUENTE_UNITARIA * 3 * 2, contador.piletas < 10 ? 3 : 2);
          pantalla.cronos(cronometro.serie, AN_FUENTE_UNITARIA * 3 * 2, TAM_FUENTE_CONTADOR * AL_FUENTE_UNITARIA, 3);
        } else {
          pantalla.cronoDescanso(cronometro.descanso);
        }
        pantalla.cronos(cronometro.total, 5, AL_PANTALLA - AL_FUENTE_UNITARIA * 3, 3);
        break;
    }
  }

  if (seHaPresionadoElBoton) {
    timestampBotonPresionado = millisAhora; //tomo el tiempo en el que se presionó el botón

    Serial.print(F("boton presionado -> estado: "));
    Serial.print(calculeta);

    switch (calculeta) {
      case INICIO:
        Serial.println(F(" inicio"));
        calculeta = PILETAS;
        pantalla.inicio();
        reseteaCronometros(true, true, true, true); // pileta[*] serie[*] descanso[*] total[*]
        reseteaContadores(true, true, true, true); // pileta[*] serie[*] total[*] totalConDescansos[*]
        reseteaSeries();
        reseteaPiletas();
        pantalla.contadorPiletas(contador.piletas); // imprime el contador
        pantalla.metrosSerie(contador.piletas); // imprime los metros  
        pantalla.metrosTot(contador.total); // imprime los metros totales 
        break;

      case PILETAS:
        Serial.println(F(" piletas"));
        calculeta = SERIES;
        guardaPileta();
        reseteaCronometros(true, false, false, false); // pileta[*] serie[] descanso[] total[]
        incrementaContadores(true, false, true, true); // piletas[*] series[] total[*] totalConDescansos[*]
        pantalla.cronos(cronometro.pileta, AN_CHAR_CRON, AL_CHAR_CRON * 2, contador.piletas < 10 ? 3 : 2);
        pantalla.contadorPiletas(contador.piletas); // imprime el contador
        pantalla.metrosSerie(contador.piletas); // imprime los metros  
        pantalla.metrosTot(contador.total); // imprime los metros totales 
        break;

      case SERIES:
        Serial.println(F(" series"));
        if (contador.piletas !=  0) { // no tiene sentido una serie de 0 piletas
          calculeta = DESCANSANDO;
          pantalla.borraContador();
          guardaSerie(); // guarda los datos de la serie en la estructura
          pantalla.metrosSerie(contador.piletas); // imprime los metros  
          pantalla.anotaSeries(dataSerie); // imprime las series
          reseteaCronometros(true, false, true, false); // pileta[*] serie[] descanso[*] total[]
          reseteaContadores(true, false, false, false); // pileta[*] serie[] total[] totalConDescansos[]
        } else {
          calculeta = RESUMEN;
          pantalla.resumen(2, dataSerie);
          pantalla.metrosTot(contador.total); // imprime los metros totales 
          pantalla.cronos(cronometro.total, 5, AL_PANTALLA - AL_FUENTE_UNITARIA * 3, 3);
        //
        //   Serial.println(stringDatos());
        //   if (wifi) {
        //     if(!conectadoWifi) conectarWifi(setupWifi);
        //     if(!tengoApiKey()) registraCalculeta(apiUrlRegistro); 
        //     else enviaDatos("calculeta", apiUrlPiletas, key);
        //   }
        }
        break;

      case DESCANSANDO:
        Serial.println(F(" descansando"));
        calculeta = PILETAS;
        pantalla.borraContador();
        pantalla.contadorPiletas(contador.piletas); // imprime el contador
        pantalla.metrosSerie(contador.piletas); // imprime los metros de la serie 
        guardaDescanso();
        guardaSerieDescanso();
        incrementaContadores(false, true, false, true); // piletas[] series[*] total[] totalConDescansos[*]
        reseteaCronometros(true, true, true, false); // pileta[*] serie[*] descanso[*] total[]
        break;

      case RESUMEN:
        Serial.println(F("resumen"));
        pantalla.resumen(1, dataSerie);
        pantalla.metrosTot(contador.total); // imprime los metros totales 
        pantalla.cronos(cronometro.total, 5, AL_PANTALLA - AL_FUENTE_UNITARIA * 3, 3);
        break;
    }
  }
}


void inicializaCalculeta(Calculeta* nuevoEstado) {
  *nuevoEstado = INICIO;
  reseteaCronometros(true, true, true, true);
  reseteaContadores(true, true, true, true);
  reseteaSeries();
  pantalla.logoCalculeta();
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

void incrementaCronometros(Calculeta estado) {
  cronometro.pileta++;
  cronometro.total++;
  if (estado == DESCANSANDO) cronometro.descanso++;
  else cronometro.serie++;
  return;
}

void reseteaCronometros(boolean pileta, boolean serie, boolean descanso, boolean total) {
  if (pileta) cronometro.pileta = 0;
  if (serie) cronometro.serie = 0;
  if (descanso) cronometro.descanso = 0;
  if (total) cronometro.total = 0;
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

void guardaPileta() {
  pileta[contador.totalConDescansos].pileta = true;
  pileta[contador.totalConDescansos].tiempo = cronometro.pileta;
  return;
}

void guardaDescanso() {
  pileta[contador.totalConDescansos].pileta = false;
  pileta[contador.totalConDescansos].tiempo = cronometro.pileta;
  return;
}

void guardaSerie() {
  dataSerie[contador.series].piletas = contador.piletas;
  dataSerie[contador.series].tiempo = cronometro.serie;
  return;
}

void guardaSerieDescanso() {
  dataSerie[contador.series].descanso = cronometro.descanso;
  return;
}

void pasaElTiempoParaIncrementarSeries(Calculeta* nuevoEstado, Calculeta estado ) {
  switch(estado) {
    case INICIO:
      break;

    case DESCANSANDO:
      break;

    case RESUMEN:
      break;

    default:
      if (millis() - timestampBotonPresionado >= TIEMPO_INCREMENTO_SERIE) *nuevoEstado = PILETAS;
      else *nuevoEstado = SERIES;
      break;
  }
  return;
}