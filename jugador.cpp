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

PosicionCuadricula operator+(const PosicionCuadricula &p1, const PosicionCuadricula &p2) {
	return PosicionCuadricula{p1.x + p2.x, p1.y + p2.y};
}

PosicionCuadricula operator-(const PosicionCuadricula &p1, const PosicionCuadricula &p2) {
	return PosicionCuadricula{p1.x - p2.x, p1.y - p2.y};
}

//----

bool ComportamientoJugador::posicion_correcta(PosicionCuadricula pos) {
	int ancho = mapaResultado.size();
	int largo = mapaResultado[0].size();
	return 0 <= pos.x && pos.x < ancho && 0 <= pos.y && pos.y < largo;
}

// Comprobamos si podemos cruzar una casilla
bool ComportamientoJugador::atravesable(PosicionCuadricula pos) {
	char casilla = mapaResultado[pos.x][pos.y];

	if (casilla == 'B' || casilla == 'A' || casilla == 'P' || casilla == 'M') {
		return false;
	} else {
		return true;
	}
}

map<char, PosicionCuadricula> ComportamientoJugador::calcular_vecinos(PosicionCuadricula pos) {

	// Posibles vecinos de una casilla
	map<char, PosicionCuadricula> direcciones = {
	    {'S', PosicionCuadricula{1,0}},
	    {'O', PosicionCuadricula{0, -1}},
	    {'N', PosicionCuadricula{-1, 0}},
	    {'E', PosicionCuadricula{0, 1}}
    };

	map<char, PosicionCuadricula> vecinos;

	for (pair<char, PosicionCuadricula> dir : direcciones) {
        PosicionCuadricula siguiente{pos.x + dir.second.x, pos.y + dir.second.y};
        if (posicion_correcta(siguiente) && atravesable(siguiente)) {
            vecinos[dir.first] = siguiente;
        }
	}

	return vecinos;
}

double ComportamientoJugador::heuristica(PosicionCuadricula a, PosicionCuadricula b) {
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

void ComportamientoJugador::a_estrella(
	PosicionCuadricula inicio,
	PosicionCuadricula destino,
	map<PosicionCuadricula, tuple<PosicionCuadricula, list<Action>, int>>& plan,
	vector<PosicionCuadricula>& explorados)
{
	ColaPrioridad<PosicionCuadricula, double> frontera;

	frontera.insertar(inicio, 0);
	list<Action> acciones_iniciales = {actIDLE};
	plan[inicio] = make_tuple(inicio, acciones_iniciales , brujula);

	while (!frontera.vacia()) {
		PosicionCuadricula actual = frontera.obtener();

		if (actual == destino)
		{
			break;
		}

		int orientacion_act = get<2>(plan[actual]);
		int orientacion_sig;

		for (pair<char, PosicionCuadricula> siguiente : calcular_vecinos(actual)) {

            PosicionCuadricula siguiente_pos = siguiente.second;
            char siguiente_dir = siguiente.first;

            if (find(explorados.begin(), explorados.end(), siguiente_pos) == explorados.end()) {
            	explorados.push_back(siguiente_pos);
            	double prioridad = heuristica(siguiente_pos, destino);
				frontera.insertar(siguiente_pos, prioridad);

				list<Action> acciones;

                // Reorientamos a nuestro personaje
                switch(siguiente_dir) {
                    case 'N':
                        switch (orientacion_act) {
                            case 1: acciones.push_front(actTURN_L);
                                    break;
                            // Tiene que ir al Norte pero está mirando al sur como si viniese del Norte
                            // ¿Ocurriría?
                            case 2: acciones.push_front(actTURN_L);
                                    acciones.push_front(actTURN_L);
                                    break;
                            case 3: acciones.push_front(actTURN_R);
                                    break;
                        }
                        orientacion_sig = 0;
                        break;
                    case 'E':
                        switch (orientacion_act) {
                            case 0: acciones.push_front(actTURN_R);
                                    break;
                            case 2: acciones.push_front(actTURN_L);
                                    break;
                            case 3: acciones.push_front(actTURN_R);
                                    acciones.push_front(actTURN_R);
                                    break;
                        }
                        orientacion_sig = 1;
                        break;
                    case 'S':
                        switch (orientacion_act) {
                            case 0: acciones.push_front(actTURN_R);
                                    acciones.push_front(actTURN_R);
                                    break;
                            case 1: acciones.push_front(actTURN_R);
                                    break;
                            case 3: acciones.push_front(actTURN_L);
                                    break;
                        }
                        orientacion_sig = 2;
                        break;
                    case 'O':
                        switch (orientacion_act) {
                            case 0: acciones.push_front(actTURN_L);
                                    break;
                            case 1: acciones.push_front(actTURN_L);
                                    acciones.push_front(actTURN_L);
                                    break;
                            case 2: acciones.push_front(actTURN_R);
                                    break;
                        }
                        orientacion_sig = 3;
                }
                // Lo movemos hacia adelante
                acciones.push_front(actFORWARD);

                char *Acciones[] = {
                    "Delante",
                    "Girar izquierda",
                    "Girar derecha",
                    "Quieto"
                };

				plan[siguiente_pos] = make_tuple(actual, acciones, orientacion_sig);
            }

		}
	}

}

void ComportamientoJugador::imprimirPasos(map<PosicionCuadricula, PosicionCuadricula>& pasos, PosicionCuadricula origen, PosicionCuadricula final) {
    cout << "Imprimir pasos" << endl;
    cout << "origen:" << origen << "final: " << final << endl;
	PosicionCuadricula pos = final;
	while (pos != origen) {
		cout << pos.x << ", " << pos.y << endl;
		pos = pasos[pos];
	}
}

void ComportamientoJugador::imprimirPasosConPlan(map<PosicionCuadricula, tuple<PosicionCuadricula, list<Action>, int>>& plan, PosicionCuadricula origen, PosicionCuadricula final) {
    cout << "Imprimir plan" << endl;
    cout << "origen:" << origen << "final: " << final << endl;
	PosicionCuadricula pos = final;

	char *Acciones[] = {
        "Delante",
        "Girar izquierda",
        "Girar derecha",
        "Quieto"
    };

    char *Direcciones[] = {
        "Norte",
        "Este",
        "Sur",
        "Oeste"
    };

	while (pos != origen) {
		cout << pos.x << ", " << pos.y << endl;

		for (Action accion : get<1>(plan[pos])) {
            cout << Acciones[accion] << endl;
		}

		cout << "orientacion " << Direcciones[get<2>(plan[pos])] << endl;

		pos = get<0>(plan[pos]);
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

bool ComportamientoJugador::pathFinding(const estado &origen, const estado &destino, list<Action> &plan) {

    plan.clear();

    map<PosicionCuadricula, tuple<PosicionCuadricula, list<Action>, int>> recorrido;
	vector<PosicionCuadricula> explorados;

	PosicionCuadricula pos_org = {origen.fila, origen.columna};
	PosicionCuadricula pos_dest = {destino.fila, destino.columna};

	a_estrella(pos_org, pos_dest, recorrido, explorados);

	PosicionCuadricula pos = pos_dest;
	while (pos != pos_org) {

        list<Action> acciones = get<1>(recorrido[pos]);

        for (Action act : acciones) {
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
