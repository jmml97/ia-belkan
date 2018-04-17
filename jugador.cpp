#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
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

    vector<PosicionCuadricula> explorados;

    // Valores iniciales de nuestro recorrido de movimiento
	frontera.insertar(inicio, 0);
	list<Action> acciones_iniciales = {actIDLE};
	recorrido[inicio] = make_tuple(inicio, acciones_iniciales , brujula);

    int cont = 0;
    int coste = 0;

	while (!frontera.vacia()) {
		PosicionCuadricula actual = frontera.obtener();

        cont++;

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

            // Tenemos que descartar las posiciones que ya hayamos visitado
            // anteriormente
            bool posicion_no_explorada = find(explorados.begin(),
                                         explorados.end(),
                                         siguiente_pos)
                                         == explorados.end();

            if (posicion_no_explorada) {

            	explorados.push_back(siguiente_pos);

            	double prioridad = Heuristica(siguiente_pos, destino);
            	coste += prioridad;
				frontera.insertar(siguiente_pos, prioridad);

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

				recorrido[siguiente_pos] = make_tuple(actual, acciones, orientacion_siguiente);
            }

		}
	}

    cout << "iteraciones: " << cont << endl;

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
        cout << "n_acciones: " << n_acciones << endl;
        cout << "n_pasos: " << n_pasos << endl;
        return true;
	}



  return false;
}

Action ComportamientoJugador::think(Sensores sensores) {

    if (sensores.mensajeF != -1) {
        fil = sensores.mensajeF;
        col = sensores.mensajeC;
    }

    if (hay_plan && (sensores.destinoF != destino.fila || sensores.destinoC != destino.columna)) {
        cout << "El destino ha cambiado" << endl;
        hay_plan = false;
    }

    if (!hay_plan) {
        estado origen = {fil, col, brujula};
        destino = {sensores.destinoF, sensores.destinoC, 0};
        hay_plan = pathFinding(origen, destino, plan);
    }

    Action sig_action;

    if (hay_plan && plan.size() > 0) {
        if (sensores.superficie[2] == 'a') {
            cout << "he detectado un aldeano" << endl;
            sig_action = actIDLE;
            PintaPlan(plan);
        } else {
            sig_action = plan.front();
            plan.erase(plan.begin());
        }
    } else {
        sig_action = actIDLE;
    }

    // Actualizamos las variables de estado del personaje

    switch(sig_action) {
        case actTURN_R: brujula = (brujula + 1)% 4; break;
        case actTURN_L: brujula = (brujula + 3)% 4; break;
        case actFORWARD:
            switch(brujula) {
                case 0: fil--; break;
                case 1: col++; break;
                case 2: fil++; break;
                case 3: col--; break;
            }
    }

  return sig_action;
}

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
