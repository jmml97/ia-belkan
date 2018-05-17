#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <chrono>
#include <ctime>
#include <stdlib.h>

// Operadores sobrecargados de PosicionCuadricula ----

ostream& operator<<(ostream& os, const PosicionCuadricula& pos)
{
    os << pos.x << ", " << pos.y << endl;
    return os;
}

PosicionCuadricula operator+(const PosicionCuadricula &p1,
                             const PosicionCuadricula &p2) {

	return PosicionCuadricula{p1.x + p2.x, p1.y + p2.y};
}

PosicionCuadricula operator-(const PosicionCuadricula &p1,
                             const PosicionCuadricula &p2) {

	return PosicionCuadricula{p1.x - p2.x, p1.y - p2.y};
}

//----

bool ComportamientoJugador::PosicionCorrecta(PosicionCuadricula pos) {
	int ancho = mapaResultado.size();
	int largo = mapaResultado[0].size();

	return 0 <= pos.x && pos.x < ancho && 0 <= pos.y && pos.y < largo;
}

// Comprobamos si podemos cruzar una casilla
bool ComportamientoJugador::PosicionAtravesable(PosicionCuadricula pos) {
	char casilla = mapaResultado[pos.x][pos.y];

	if (casilla == 'B' || casilla == 'A' || casilla == 'P' || casilla == 'M') {
		return false;
	} else {
		return true;
	}
}

map<char, PosicionCuadricula> ComportamientoJugador::ObtenerVecinos(
    PosicionCuadricula pos) {

	// Posibles vecinos de una casilla
	map<char, PosicionCuadricula> direcciones = {
	    {'S', PosicionCuadricula{1,0}},
	    {'O', PosicionCuadricula{0, -1}},
	    {'N', PosicionCuadricula{-1, 0}},
	    {'E', PosicionCuadricula{0, 1}}
    };

	map<char, PosicionCuadricula> vecinos;

	for (pair<char, PosicionCuadricula> dir : direcciones) {
        PosicionCuadricula siguiente{pos.x + dir.second.x,
                                     pos.y + dir.second.y};

        if (PosicionCorrecta(siguiente) && PosicionAtravesable(siguiente)) {
            vecinos[dir.first] = siguiente;
        }
	}

	return vecinos;
}

// Utilizamos la distancia L_1, suma de las diferencias (absolutas) de las
// coordenadas
double ComportamientoJugador::Heuristica(PosicionCuadricula a,
                                         PosicionCuadricula b) {

    return abs(a.x - b.x) + abs(a.y - b.y);
}

///////////////////////////////////////////////////////////////////////////////
//
// Función principal del algoritmo
//
// Argumentos:
// pasos_anteriores: nos indica la casilla anterior a una en el recorrido
// explorados: almacena los nodos que ya hemos visitado
//

void ComportamientoJugador::AEstrella(
	PosicionCuadricula inicio,
	PosicionCuadricula destino,
	map<PosicionCuadricula, tuple<PosicionCuadricula, list<Action>,int>>& recorrido) {

  // La frontera está formada por los puntos más externos al área que hemos
  // explorado
  ColaPrioridad<PosicionCuadricula, double> frontera;

  map<PosicionCuadricula, double> coste_hasta_ahora;

  // Valores iniciales de nuestro recorrido de movimiento
  frontera.insertar(inicio, 0);
  list<Action> acciones_iniciales = {actIDLE};
  recorrido[inicio] = make_tuple(inicio, acciones_iniciales , brujula);
  coste_hasta_ahora[inicio] = 0;

  int coste = 0;

  while (!frontera.vacia()) {
    PosicionCuadricula actual = frontera.obtener();

    if (actual == destino) {
      break;
    }

    // Orientación del personaje
    int orientacion_actual = get<2>(recorrido[actual]);
    int orientacion_siguiente;

    map<char, PosicionCuadricula> vecinos = ObtenerVecinos(actual);

    // Iteramos entre todos los posibles vecinos de la casilla actual
    for (pair<char, PosicionCuadricula> siguiente : vecinos) {

      PosicionCuadricula siguiente_pos = siguiente.second;
      char siguiente_dir = siguiente.first;

      list<Action> acciones;

      // Para mover a nuestro personaje será necesario cambiar
      // su orientación
      switch(siguiente_dir) {
        case 'N':
          switch (orientacion_actual) {
            case 1: acciones.push_front(actTURN_L);
                    break;
            case 2: acciones.push_front(actTURN_L);
                    acciones.push_front(actTURN_L);
                    break;
            case 3: acciones.push_front(actTURN_R);
                    break;
          }
          orientacion_siguiente = 0;
          break;
        case 'E':
          switch (orientacion_actual) {
            case 0: acciones.push_front(actTURN_R);
                    break;
            case 2: acciones.push_front(actTURN_L);
                    break;
            case 3: acciones.push_front(actTURN_R);
                    acciones.push_front(actTURN_R);
                    break;
          }
          orientacion_siguiente = 1;
          break;
        case 'S':
          switch (orientacion_actual) {
            case 0: acciones.push_front(actTURN_R);
                    acciones.push_front(actTURN_R);
                    break;
            case 1: acciones.push_front(actTURN_R);
                    break;
            case 3: acciones.push_front(actTURN_L);
                    break;
          }
          orientacion_siguiente = 2;
          break;
        case 'O':
          switch (orientacion_actual) {
            case 0: acciones.push_front(actTURN_L);
                    break;
            case 1: acciones.push_front(actTURN_L);
                    acciones.push_front(actTURN_L);
                    break;
            case 2: acciones.push_front(actTURN_R);
                    break;
          }
          orientacion_siguiente = 3;
      }
      // Una vez orientado, lo movemos hacia adelante
      acciones.push_front(actFORWARD);

      double coste_siguiente = coste_hasta_ahora[actual] + acciones.size();

      // Tenemos que descartar las posiciones que ya hayamos visitado
      // anteriormente
      bool posicion_no_explorada = coste_hasta_ahora.find(siguiente_pos)
        == coste_hasta_ahora.end();
      bool coste_menor = coste_siguiente < coste_hasta_ahora[siguiente_pos];

      if (posicion_no_explorada || coste_menor) {

        coste_hasta_ahora[siguiente_pos] = coste_siguiente;

        double prioridad = coste_siguiente +
          Heuristica(siguiente_pos, destino);
        frontera.insertar(siguiente_pos, prioridad);

        recorrido[siguiente_pos] = make_tuple(actual, acciones, orientacion_siguiente);
      }

    }
	}

}

bool ComportamientoJugador::PosicionLocalCorrecta(PosicionCuadricula pos) {
	int ancho = mapa_local.size();
	int largo = mapa_local[0].size();

	return 0 <= pos.x && pos.x < ancho && 0 <= pos.y && pos.y < largo;
}

// Comprobamos si podemos cruzar una casilla
bool ComportamientoJugador::PosicionLocalAtravesable(PosicionCuadricula pos) {
	char casilla = mapa_local[pos.x][pos.y];

	if (casilla == 'B' || casilla == 'A' || casilla == 'P' || casilla == 'M') {
		return false;
	} else {
		return true;
	}
}

map<char, PosicionCuadricula> ComportamientoJugador::ObtenerVecinosLocales(
    PosicionCuadricula pos) {

	// Posibles vecinos de una casilla
	map<char, PosicionCuadricula> direcciones = {
	    {'S', PosicionCuadricula{1,0}},
	    {'O', PosicionCuadricula{0, -1}},
	    {'N', PosicionCuadricula{-1, 0}},
	    {'E', PosicionCuadricula{0, 1}}
    };

	map<char, PosicionCuadricula> vecinos;

	for (pair<char, PosicionCuadricula> dir : direcciones) {
        PosicionCuadricula siguiente{pos.x + dir.second.x,
                                     pos.y + dir.second.y};

        if (PosicionLocalCorrecta(siguiente) && PosicionLocalAtravesable(siguiente)) {
            vecinos[dir.first] = siguiente;
        }
	}

	return vecinos;
}

void ComportamientoJugador::AEstrellaLocal(
	PosicionCuadricula inicio,
	PosicionCuadricula destino,
	map<PosicionCuadricula, tuple<PosicionCuadricula, list<Action>,int>>& recorrido) {

  // La frontera está formada por los puntos más externos al área que hemos
  // explorado
  ColaPrioridad<PosicionCuadricula, double> frontera;

  map<PosicionCuadricula, double> coste_hasta_ahora;

  // Valores iniciales de nuestro recorrido de movimiento
  frontera.insertar(inicio, 0);
  list<Action> acciones_iniciales = {actIDLE};
  recorrido[inicio] = make_tuple(inicio, acciones_iniciales , brujula);
  coste_hasta_ahora[inicio] = 0;

  int coste = 0;

  while (!frontera.vacia()) {
    PosicionCuadricula actual = frontera.obtener();

    if (actual == destino) {
      break;
    }

    // Orientación del personaje
    int orientacion_actual = get<2>(recorrido[actual]);
    int orientacion_siguiente;

    map<char, PosicionCuadricula> vecinos = ObtenerVecinosLocales(actual);

    // Iteramos entre todos los posibles vecinos de la casilla actual
    for (pair<char, PosicionCuadricula> siguiente : vecinos) {

      PosicionCuadricula siguiente_pos = siguiente.second;
      char siguiente_dir = siguiente.first;

      list<Action> acciones;

      // Para mover a nuestro personaje será necesario cambiar
      // su orientación
      switch(siguiente_dir) {
        case 'N':
          switch (orientacion_actual) {
            case 1: acciones.push_front(actTURN_L);
                    break;
            case 2: acciones.push_front(actTURN_L);
                    acciones.push_front(actTURN_L);
                    break;
            case 3: acciones.push_front(actTURN_R);
                    break;
          }
          orientacion_siguiente = 0;
          break;
        case 'E':
          switch (orientacion_actual) {
            case 0: acciones.push_front(actTURN_R);
                    break;
            case 2: acciones.push_front(actTURN_L);
                    break;
            case 3: acciones.push_front(actTURN_R);
                    acciones.push_front(actTURN_R);
                    break;
          }
          orientacion_siguiente = 1;
          break;
        case 'S':
          switch (orientacion_actual) {
            case 0: acciones.push_front(actTURN_R);
                    acciones.push_front(actTURN_R);
                    break;
            case 1: acciones.push_front(actTURN_R);
                    break;
            case 3: acciones.push_front(actTURN_L);
                    break;
          }
          orientacion_siguiente = 2;
          break;
        case 'O':
          switch (orientacion_actual) {
            case 0: acciones.push_front(actTURN_L);
                    break;
            case 1: acciones.push_front(actTURN_L);
                    acciones.push_front(actTURN_L);
                    break;
            case 2: acciones.push_front(actTURN_R);
                    break;
          }
          orientacion_siguiente = 3;
      }
      // Una vez orientado, lo movemos hacia adelante
      acciones.push_front(actFORWARD);

      double coste_siguiente = coste_hasta_ahora[actual] + acciones.size();

      // Tenemos que descartar las posiciones que ya hayamos visitado
      // anteriormente
      bool posicion_no_explorada = coste_hasta_ahora.find(siguiente_pos)
        == coste_hasta_ahora.end();
      bool coste_menor = coste_siguiente < coste_hasta_ahora[siguiente_pos];

      if (posicion_no_explorada || coste_menor) {

        coste_hasta_ahora[siguiente_pos] = coste_siguiente;

        double prioridad = coste_siguiente +
          Heuristica(siguiente_pos, destino);
        frontera.insertar(siguiente_pos, prioridad);

        recorrido[siguiente_pos] = make_tuple(actual, acciones, orientacion_siguiente);
      }

    }
	}

}

void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}

void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}

void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}

Action ComportamientoJugador::Deambular(Sensores sensores) {
  char casilla = sensores.terreno[2];

  PosicionCuadricula actual = {3, 3};
  PosicionCuadricula siguiente, cambio;
  int indice = 0;

  for (int i = 0; i <= 3; i++) {
    for (int j = -i; j <= i; j++) {
      switch (brujula) {
        case 0:
          cambio = {-i, j};
          break;
        case 1:
          cambio = {j, i};
          break;
        case 2:
          cambio = {i, -j};
          break;
        case 3:
          cambio = {-j, -i};
          break;
      }

      siguiente = actual + cambio;

      mapa_local[siguiente.x][siguiente.y] = sensores.terreno[indice];
      if (sensores.terreno[indice] == 'K'){
        cout << "he visto un pk" << endl;
        pk = siguiente;
      }
      indice++;
    }
  }

  if (ultima_accion == actTURN_L || ultima_accion == actTURN_R) {
    return actFORWARD;
  } else {

    srand(time(nullptr));
    int aleatorio = rand() % 8;

    switch (aleatorio) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
        return actFORWARD;
      case 6:
        return actTURN_L;
      case 7:
        return actTURN_R;
    }

  }

}

bool ComportamientoJugador::pathFinding(const estado &origen, const estado &destino, list<Action> &plan) {

  plan.clear();

  map<PosicionCuadricula,
    tuple<PosicionCuadricula,
    list<Action>,
    int>> recorrido;

  PosicionCuadricula pos_origen = {origen.fila, origen.columna};
  PosicionCuadricula pos_destino = {destino.fila, destino.columna};

  AEstrella(pos_origen, pos_destino, recorrido);

  PosicionCuadricula pos = pos_destino;

  int n_acciones = 0;
  int n_pasos = 0;

  // Vemos el recorrido desde el destino hasta el origen y vamos añadiendo
  // las acciones al plan de movimiento
  while (pos != pos_origen) {
    n_pasos++;

    list<Action> acciones = get<1>(recorrido[pos]);

    for (Action act : acciones) {
      n_acciones++;
      plan.push_front(act);
    }

    pos = get<0>(recorrido[pos]);

  }

  if (!plan.empty()) {
    PintaPlan(plan);
    VisualizaPlan(origen, plan);
    return true;
  }

  return false;
}

bool ComportamientoJugador::pathFindingLocal(PosicionCuadricula origen, PosicionCuadricula destino, list<Action> &plan) {

  plan.clear();

  map<PosicionCuadricula,
    tuple<PosicionCuadricula,
    list<Action>,
    int>> recorrido;

  AEstrellaLocal(origen, destino, recorrido);

  PosicionCuadricula pos = destino;

  int n_acciones = 0;
  int n_pasos = 0;

  // Vemos el recorrido desde el destino hasta el origen y vamos añadiendo
  // las acciones al plan de movimiento
  while (pos != origen) {

    n_pasos++;

    list<Action> acciones = get<1>(recorrido[pos]);

    for (Action act : acciones) {
      n_acciones++;
      plan.push_front(act);
    }

    pos = get<0>(recorrido[pos]);

  }

  if (!plan.empty()) {
    return true;
  }

  return false;
}

void ComportamientoJugador::ActualizarMapa(Sensores sensores) {

  PosicionCuadricula actual = {fil, col};
  PosicionCuadricula siguiente, cambio;
  int indice = 0;

  for (int i = 0; i <= 3; i++) {
    for (int j = -i; j <= i; j++) {
      switch (brujula) {
        case 0:
          cambio = {-i, j};
          break;
        case 1:
          cambio = {j, i};
          break;
        case 2:
          cambio = {i, -j};
          break;
        case 3:
          cambio = {-j, -i};
          break;
      }

      siguiente = actual + cambio;
      if (PosicionCorrecta(siguiente) && mapaResultado[siguiente.x][siguiente.y] == '?') {

        mapaResultado[siguiente.x][siguiente.y] = sensores.terreno[indice];

      }
      indice++;
    }
  }

}

Action ComportamientoJugador::think(Sensores sensores) {

    if (fil == sensores.destinoF && col == sensores.destinoC || parar == true) {
      cout << "Hasta aquí hemos llegado. La educación no es una competición." << endl;
      parar = true;
      return actIDLE;
    }

    Action sig_action;

    if (sensores.mensajeF != -1 && sensores.mensajeC != -1) {
        fil = sensores.mensajeF;
        col = sensores.mensajeC;
        tengo_posicion = true;
    }

    if (nivel == 3 && tengo_posicion) {
      ActualizarMapa(sensores);
      hay_plan = false;
    }

    if (hay_plan && (sensores.destinoF != destino.fila || sensores.destinoC != destino.columna) && nivel != 3) {
        hay_plan = false;
    }

    if (!hay_plan && nivel != 3) {
        estado origen = {fil, col, brujula};
        destino = {sensores.destinoF, sensores.destinoC, 0};
        hay_plan = pathFinding(origen, destino, plan);
    }

    if (!hay_plan && nivel == 3) {

      if (!tengo_posicion) {
        sig_action = Deambular(sensores);
        char casilla = sensores.terreno[2];

        bool puedo_avanzar = (casilla == 'S' || casilla == 'T' || casilla == 'K');
        if (sig_action == actFORWARD && !puedo_avanzar) {
          int aleatorio = rand() % 2;
          switch (aleatorio) {
            case 0: sig_action = actTURN_L; break;
            case 1: sig_action = actTURN_R;
          }
        }

        PosicionCuadricula pos_vacia = {0,0};
        if (pk != pos_vacia) {
          PosicionCuadricula pos_origen = {3,3};
          cout << "he avistado un pk" << endl;
          hay_plan = pathFindingLocal(pos_origen, pk, plan);
        }
      } else {
        estado origen = {fil, col, brujula};
        destino = {sensores.destinoF, sensores.destinoC, 0};
        hay_plan = pathFinding(origen, destino, plan);
      }

    }

    if (hay_plan && plan.size() > 0) {
        if (sensores.superficie[2] == 'a') {
            sig_action = actIDLE;
            PintaPlan(plan);
        } else {
            sig_action = plan.front();
            plan.erase(plan.begin());
        }
    } else if (nivel != 3) {
        sig_action = actIDLE;
    } else if (hay_plan && plan.size() == 0 && nivel == 3) {
      hay_plan = false;
      sig_action = Deambular(sensores);
    }

    // Actualizamos las variables de estado del personaje

    switch(sig_action) {
      case actTURN_R: brujula = (brujula + 1)% 4; break;
      case actTURN_L: brujula = (brujula + 3)% 4; break;
      case actFORWARD:
        if (tengo_posicion) {
          switch(brujula) {
            case 0: fil--; break;
            case 1: col++; break;
            case 2: fil++; break;
            case 3: col--; break;
          }
        }
    }

  ultima_accion = sig_action;
  return sig_action;
}

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
