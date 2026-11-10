#ifndef SUDOKU_TENSORIAL_H
#define SUDOKU_TENSORIAL_H

#include <Arduino.h>

class SudokuTensorial {
private:
    // Tablero de probabilidades P[i][j][n] (n de 0 a 8 representa números 1 a 9)
    float P[9][9][9]; 
    // Tablero final resuelto
    int tablero[9][9];
    // Celdas fijas (pistas iniciales)
    bool esFija[9][9];
    
    // Parámetros del algoritmo (Ecs. 3.1, 3.2, 3.3)
    float umbralBloqueo;           // 0.85 (Ec. 3.3)
    float coeficienteConfinamiento; // 0.45 (Inspector Unity)
    float factorCastigo;           // 5 * coeficienteConfinamiento (Ec. 3.2)
    float dt;                      // Paso de tiempo de relajación

    // Funciones auxiliares
    void normalizarCelda(int i, int j); // Ec. 3.1
    void aplicarCastigoRefuerzo(int i, int j); // Ec. 3.2
    bool verificarBloqueo(int i, int j); // Ec. 3.3
    int contarConflictos(int i, int j, int num) const;

public:
    SudokuTensorial();

    void inicializar();
    void cargarTablero(int inicial[9][9]);
    
    // Lazo principal
    bool iterar(); // Devuelve true si el Sudoku está completamente resuelto

    // Getters para telemetría
    float getUmbralBloqueo() const { return umbralBloqueo; }
    float getFactorCastigo() const { return factorCastigo; }
    float getCoeficienteConfinamiento() const { return coeficienteConfinamiento; }
    int getCeldasResueltas() const;
    
    // Validación y salida
    bool validarSolucion() const;
    void imprimirTableroResuelto() const;
};

#endif