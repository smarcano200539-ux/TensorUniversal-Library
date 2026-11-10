#include "SudokuTensorial.h"

SudokuTensorial::SudokuTensorial() {
    // Inicialización por defecto
    umbralBloqueo = 0.85;
    coeficienteConfinamiento = 0.45;
    factorCastigo = 5.0 * coeficienteConfinamiento; // Ec. 3.2 (5x más agresivo)
    dt = 0.1;
}

void SudokuTensorial::inicializar() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            esFija[i][j] = false;
            tablero[i][j] = 0;
            for (int n = 0; n < 9; n++) {
                P[i][j][n] = 1.0 / 9.0; // Distribución uniforme inicial
            }
        }
    }
}

void SudokuTensorial::cargarTablero(int inicial[9][9]) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (inicial[i][j] != 0) {
                esFija[i][j] = true;
                tablero[i][j] = inicial[i][j];
                // Fijar probabilidad a 1.0 para el número dado, 0 para el resto
                for (int n = 0; n < 9; n++) {
                    P[i][j][n] = (n + 1 == inicial[i][j]) ? 1.0 : 0.0;
                }
            } else {
                esFija[i][j] = false;
                tablero[i][j] = 0;
            }
        }
    }
}

int SudokuTensorial::contarConflictos(int i, int j, int num) const {
    int conflictos = 0;
    // Fila y Columna
    for (int k = 0; k < 9; k++) {
        if (k != j && tablero[i][k] == num) conflictos++;
        if (k != i && tablero[k][j] == num) conflictos++;
    }
    // Bloque 3x3
    int bi = (i / 3) * 3;
    int bj = (j / 3) * 3;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            int ci = bi + x, cj = bj + y;
            if (ci != i || cj != j) {
                if (tablero[ci][cj] == num) conflictos++;
            }
        }
    }
    return conflictos;
}

void SudokuTensorial::normalizarCelda(int i, int j) {
    // Ec. 3.1: Normalización local de probabilidades
    float suma = 0.0;
    for (int n = 0; n < 9; n++) {
        if (P[i][j][n] < 0) P[i][j][n] = 0; // Evitar probabilidades negativas
        suma += P[i][j][n];
    }
    if (suma > 1e-6) {
        for (int n = 0; n < 9; n++) {
            P[i][j][n] /= suma;
        }
    }
}

void SudokuTensorial::aplicarCastigoRefuerzo(int i, int j) {
    // Ec. 3.2: Castigo y refuerzo diferenciado
    for (int n = 0; n < 9; n++) {
        int num = n + 1;
        int conflictos = contarConflictos(i, j, num);
        
        if (conflictos > 0) {
            // Penalización
            P[i][j][n] -= factorCastigo * dt * conflictos;
        } else {
            // Refuerzo usando cos(π(s - 0.5)). 
            // Mapeamos s a la probabilidad actual: s = 0.5 + (P - 1/9)
            float s = 0.5 + (P[i][j][n] - (1.0/9.0));
            float refuerzo = coeficienteConfinamiento * cosf(PI * (s - 0.5));
            P[i][j][n] += refuerzo * dt;
        }
    }
}

bool SudokuTensorial::verificarBloqueo(int i, int j) {
    // Ec. 3.3: Umbral de bloqueo post-normalización
    float maxP = 0;
    int mejorN = -1;
    for (int n = 0; n < 9; n++) {
        if (P[i][j][n] > maxP) {
            maxP = P[i][j][n];
            mejorN = n;
        }
    }
    
    if (maxP > umbralBloqueo) {
        tablero[i][j] = mejorN + 1;
        // Fijar la celda
        for (int n = 0; n < 9; n++) {
            P[i][j][n] = (n == mejorN) ? 1.0 : 0.0;
        }
        esFija[i][j] = true; // Bloquear permanentemente
        return true;
    }
    return false;
}

bool SudokuTensorial::iterar() {
    bool todasResueltas = true;
    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (!esFija[i][j]) {
                todasResueltas = false;
                aplicarCastigoRefuerzo(i, j);
                normalizarCelda(i, j);
                verificarBloqueo(i, j);
            }
        }
    }
    return todasResueltas;
}

int SudokuTensorial::getCeldasResueltas() const {
    int count = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (tablero[i][j] != 0) count++;
        }
    }
    return count;
}

bool SudokuTensorial::validarSolucion() const {
    // Validación matemática (Ec. 3.4)
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (tablero[i][j] == 0) return false;
            
            int num = tablero[i][j];
            // Chequear fila y columna
            for (int k = 0; k < 9; k++) {
                if (k != j && tablero[i][k] == num) return false;
                if (k != i && tablero[k][j] == num) return false;
            }
            // Chequear bloque 3x3
            int bi = (i / 3) * 3;
            int bj = (j / 3) * 3;
            for (int x = 0; x < 3; x++) {
                for (int y = 0; y < 3; y++) {
                    if ((bi+x != i || bj+y != j) && tablero[bi+x][bj+y] == num) return false;
                }
            }
        }
    }
    return true;
}

void SudokuTensorial::imprimirTableroResuelto() const {
    Serial.println("┌───────┬───────┬───────┐");
    for (int i = 0; i < 9; i++) {
        Serial.print("│ ");
        for (int j = 0; j < 9; j++) {
            Serial.print(tablero[i][j]); Serial.print(" ");
            if ((j + 1) % 3 == 0 && j < 8) Serial.print("│ ");
        }
        Serial.println("│");
        if ((i + 1) % 3 == 0 && i < 8) Serial.println("├───────┼───────┼───────┤");
    }
    Serial.println("└───────┴───────┴───────┘");
}