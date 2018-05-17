# ia-belkan
Repositorio para la práctica 2 de la asignatura Inteligencia Artificial del Doble Grado en Ingeniería Informática y Matemáticas de la Universidad de Granada.

Solución de "Los Mundos de BelKan" utilizando A*. El programa es capaz de resolver todos los niveles:
- Nivel 1: el personaje tiene total conocimiento del mapa del juego. Se resuelve utilizando únicamente A*.
- Nivel 2: igual que el nivel anterior pero hay "aldeanos" que se mueven por el mapa. Estos obstáculos se salvan únicamente esperando a que se muevan.
- Nivel 3: el personaje no conoce el mapa del juego. En una primera fase el programa se mueve aleatoriamente por el mapa, favoreciendo los movimietos hacia delante frente a los cambios se sentido, hasta que visualiza un punto de referencia con el que obtiene sus coordenadas. Una vez visualizado, traza un camino utilizando una versión local de A*. Conocidas las coordenadas, traza una ruta usando A*, actualizando el mapa y la ruta según lo va descubriendo.

La práctica estaba enfocada de modo competitivo: obtenía más puntuación quien más objetivos consiguiese. Algunos de mis compañeros y yo, entendiendo que este modo de evaluación no es el más adecuado pues no rompe las dinámicas de grupo, decidimos "boicotear" la competición, haciendo que el programa se detuviese al alcanzar el primer objetivo.

**La educación no es una competición.**
