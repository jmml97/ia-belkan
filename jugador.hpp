#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <queue>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>

struct estado {
  int fila;
  int columna;
  int orientacion;
};

struct PosicionCuadricula
{
  int x, y;

  inline bool operator ==(const PosicionCuadricula& a) const {
    return a.x == x && a.y == y;
  }

  inline bool operator !=(const PosicionCuadricula& a) const {
    return a.x != x || a.y != y;
  }

  inline bool operator <(const PosicionCuadricula& a) const {
    return tie(a.x, a.y) < tie(x, y);
  }

  friend PosicionCuadricula operator+(const PosicionCuadricula &p1, const PosicionCuadricula &p2);
  friend PosicionCuadricula operator-(const PosicionCuadricula &p1, const PosicionCuadricula &p2);
  friend ostream& operator<<(ostream& os, const PosicionCuadricula& pos);
};

template<typename T, typename prioridad_t>
struct ColaPrioridad {
  typedef std::pair<prioridad_t, T> elemento_cp;
  priority_queue<elemento_cp, vector<elemento_cp>, greater<elemento_cp>>elementos;

  inline bool vacia() const {
     return elementos.empty();
  }

  inline void insertar(T elemento, prioridad_t prioridad) {
    elementos.emplace(prioridad, elemento);
  }

  T obtener() {
    T mejor_elemento = elementos.top().second;
    elementos.pop();
    return mejor_elemento;
  }
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hay_plan = false;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hay_plan = false;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    bool PosicionCorrecta(PosicionCuadricula pos);
    bool PosicionAtravesable(PosicionCuadricula pos);
    map<char, PosicionCuadricula> ObtenerVecinos(PosicionCuadricula pos);

    double Heuristica(PosicionCuadricula a, PosicionCuadricula b);
    void AEstrella(
        PosicionCuadricula inicio,
        PosicionCuadricula destino,
        map<PosicionCuadricula, tuple<PosicionCuadricula, list<Action>, int>>& plan
    );

    // Borrables/Revisables
    void ImprimirPasos(map<PosicionCuadricula,
                       PosicionCuadricula>& pasos,
                       PosicionCuadricula origen,
                       PosicionCuadricula final
    );
    void ImprimirPasosConPlan(
        map<PosicionCuadricula, tuple<PosicionCuadricula, list<Action>, int>>& plan,
        PosicionCuadricula origen,
        PosicionCuadricula final
    );

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado destino;
    list<Action> plan;
    bool hay_plan;

    bool pathFinding(const estado &origen, const estado &destino, list<Action> &plan);
    void PintaPlan(list<Action> plan);
};

#endif
