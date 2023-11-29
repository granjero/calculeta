# CALCULETA

Por algun motivo no soy capaz de llevar la cuenta cuando voy a nadar. 

Por eso tuve que hacer este aparato (?)

La calculeta es un dispositivo fundamental y fenomenal a la hora de nadar. Lleva la cuenta de piletas y series que es imposible llevar con el cerebro.

<a href="https://www.hackster.io/user3330224/swimming-pool-lap-counter-d9196f" target="_blank">Inspiración</a>

#### Convenciones

  Se diferencia entre __pileta__ y __largo__

  __largo__: El largo de la piscina. (a la que voy mide 25mts)

  __pileta__: El largo de ida + el largo de vuelta. (50mts*)

  La Calculeta® lleva la cuenta *series* de *piletas*.

### Funcionamiento

  La calculeta se deja de un lado de la piscina. Con la primer pulsación se inicia la cuenta de la primer serie.
  Cada pulsación añade una *pileta*. 
  Para terminar una *serie* y comenzar el *descanso* se debe volver a pulsar antes que pasen 10 segundos.
  Cuando finaliza una serie se actualiza el resumen parcial de series y comienza el cronómetro del descanso.
  Para comenzar otra serie se debe volver a realizar una pulsación.
  Para finalizar se debe cerrar una serie, salir del descanso y finalizar la serie con cero piletas.

<div style="display: flex; justify-content: space-between;">
  <img src="https://calculeta.estonoesunaweb.com.ar/calculeta_ini.png" alt="calculeta pantalla de inicio" width="20%">
  <img src="https://calculeta.estonoesunaweb.com.ar/calculeta_contando_piles.png" alt="calculeta contando piletas" width="20%">
  <img src="https://calculeta.estonoesunaweb.com.ar/calculeta_descansando.png" alt="calculeta descansando" width="20%">
  <img src="https://calculeta.estonoesunaweb.com.ar/calculeta_res.png" alt="calculeta resumen" width="20%">
</div>



* TODO tener un selector de hardware de 2bits.
