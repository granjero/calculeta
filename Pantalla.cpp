#include "Pantalla.h"

Pantalla::Pantalla() : 
  tft(TFT_CS, TFT_DC),
  canvas_contador_piletas(AN_CHAR_CONTADOR * 2, AL_CHAR_CONTADOR * 1),
  canvas_crono(AN_CHAR_CRON * 5, AL_CHAR_CRON * 1),
  canvas_metros_serie(AN_CHAR_MTS * 5, AL_CHAR_MTS * 1),
  canvas_metros_tot(AN_CHAR_MTS_TOT * 5, AL_CHAR_MTS_TOT * 1) {  
  tft.begin();
}

char* Pantalla::reloj(int segundos) {
  int mins = segundos / 60;
  int secs = segundos % 60;
  static char relojFormateado[6];
  snprintf(relojFormateado, sizeof(relojFormateado), "%02d:%02d", mins, secs);
  return relojFormateado;
}

void Pantalla::inicio() {
  tft.fillScreen(0); // pinta la pantalla de negro
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, AL_CHAR_CONTADOR);
  tft.write(0xE3); // sibolo del reloj
  // tft.write(0x0F); // sibolo del reloj
}

void Pantalla::borraContador(){
  tft.fillRect(0, 0, AN_PANTALLA, AL_CHAR_CONTADOR, ILI9341_BLACK); // borra el contador y el cronometro pileta 
}

void Pantalla::logoCalculeta(){
  tft.fillScreen(0x9E1E);
  tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, 0x002D);
}

void Pantalla::contadorPiletas(unsigned int contador) {
  int cantDigitos = contador < 10 ? 1 : 2;
  canvas_contador_piletas.fillScreen(0);    // Clear canvas (not display)
  canvas_contador_piletas.setCursor(0, 0); // Pos. is BASE LINE when using fonts!
  canvas_contador_piletas.setTextSize(TAM_FUENTE_CONTADOR);
  canvas_contador_piletas.print(contador);
  tft.drawBitmap(AN_PANTALLA - AN_CHAR_CONTADOR * cantDigitos, 0,
  canvas_contador_piletas.getBuffer(), canvas_contador_piletas.width(), canvas_contador_piletas.height(),
  0x653E, ILI9341_BLACK);
  return;
}

void Pantalla::cronos(int numero, int x, int y, int tamFuente) {
  int X = x;
  uint16_t color = ILI9341_WHITE;
  if((numero <= TIEMPO_INCREMENTO_SERIE / 1000) && (y < AL_CHAR_CONTADOR)){
    color = 0xFB54;
  }
  if(y == AL_CHAR_CRON * 3){
    color = 0x07E3;
  }
  if (tamFuente == TAM_FUENTE_CRON_CHICO) {
    canvas_crono.setCursor(canvas_crono.width() - AN_CHAR_CRON_CHICO * 5, 0); // Pos. is BASE LINE when using fonts!
    X = AN_CHAR_CRON_CHICO * 5 - canvas_crono.width() ;
  } else canvas_crono.setCursor(0, 0);
  canvas_crono.fillScreen(0);    // Clear canvas (not display)
  canvas_crono.setTextSize(tamFuente);
  canvas_crono.print(reloj(numero));
  tft.drawBitmap(X, y,
  canvas_crono.getBuffer(), canvas_crono.width(), canvas_crono.height(), 
  color, ILI9341_BLACK);
  return;
}

void Pantalla::cronoDescanso(int numero) {
  char* relos = reloj(numero);
  tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  tft.setTextSize(8);
  tft.setCursor(0, 25);
  tft.println(relos);
return;
}

void Pantalla::metrosSerie(unsigned int largos) {
  int mts = largos * LARGO_PILETA;
  int espacios =  mts > 999 ? 0 : (mts > 99 ? 1 : (mts > 0 ? 2 : 3));
  canvas_metros_serie.fillScreen(0);    // Clear canvas (not display)
  canvas_metros_serie.setCursor(0, 0); // Pos. is BASE LINE when using fonts!
  canvas_metros_serie.setTextSize(TAM_FUENTE_MTS);
  for (int i = 0; i < espacios; i++) {
    canvas_metros_serie.print(" ");
  }
  canvas_metros_serie.print(mts);
  canvas_metros_serie.print("m");
  tft.drawBitmap(AN_PANTALLA - canvas_metros_serie.width(), AL_CHAR_CONTADOR, 
  canvas_metros_serie.getBuffer(), canvas_metros_serie.width(), canvas_metros_serie.height(),
  ILI9341_GREEN, ILI9341_BLACK);
  return;
}

void Pantalla::metrosTot(unsigned int total) {
  int mts = total * LARGO_PILETA;
  int espacios =  mts > 999 ? 0 : (mts > 99 ? 1 : (mts > 0 ? 2 : 3));
  canvas_metros_tot.fillScreen(0);    // Clear canvas (not display)
  canvas_metros_tot.setCursor(0, 0); // Pos. is BASE LINE when using fonts!
  canvas_metros_tot.setTextSize(TAM_FUENTE_MTS_TOT);
  for (int i = 0; i < espacios; i++) {
    canvas_metros_tot.print(" ");
  }
  canvas_metros_tot.print(mts);
  canvas_metros_tot.print("m");

  tft.drawBitmap(AN_PANTALLA - canvas_metros_tot.width(),  AL_PANTALLA - AL_CHAR_MTS_TOT, 
  canvas_metros_tot.getBuffer(), canvas_metros_tot.width(), canvas_metros_tot.height(), 
  ILI9341_GREENYELLOW, ILI9341_BLACK);
  return;
}

void Pantalla::anotaSeries(DataSeries* data) {
  tft.setCursor(0, 150);
  tft.setTextSize(2);
  tft.fillRect(0, 150, 240, 135, ILI9341_BLACK); // borra las series

  std::map<int, std::pair<int, int>> counts;  // Declara un map llamdo counts

  // Count occurrences of each unique 'piletas' value and store associated 'tiempo'
  for (int i = 0; i < CANT_MAX_SERIES; ++i) {
    if (data[i].piletas == 0) continue;
    int piletas = data[i].piletas;
    int tiempo = data[i].tiempo;

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
  tft.print("d");
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print(piletas * LARGO_PILETA);
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
return;
}

void Pantalla::resumen(int tamanioFuente, DataSeries* data) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(tamanioFuente + 1);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.println("Resumen");
  tft.setCursor(0, tft.getCursorY() + 5);
  tft.setTextSize(tamanioFuente);
  for (int i = 0; i < CANT_MAX_SERIES; ++i) {
    if (data[i].piletas == 0) continue; // cuando no hay mas datos...
    tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    tft.write(0x10); // sibolo triangulo de costado
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.print(data[i].piletas);
    tft.print(" ");
    tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
    tft.print(data[i].piletas * LARGO_PILETA);
    tft.print("m ");
    tft.setTextColor(0x90D5, ILI9341_BLACK);
    tft.print(reloj(data[i].tiempo));
    tft.print(data[i].piletas >= 20 ? "" : " ");
    tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    tft.println(reloj(data[i].descanso));
    tft.setCursor(0, tft.getCursorY() + 3);
  }
  // cronos(cronometro.total, 5, AL_PANTALLA - AL_FUENTE_UNITARIA * 3, 3);
  // metrosTot(); // imprime los metros totales 

  return;
}
