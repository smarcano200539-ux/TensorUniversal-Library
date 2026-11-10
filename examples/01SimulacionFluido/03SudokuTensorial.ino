/*
 * Ejemplo 3: Resolvedor de Sudoku con Tensor Universal
 * 
 * Este ejemplo demuestra el algoritmo de convergencia probabilística
 * (Ecs. 3.1, 3.2, 3.3) del libro "La Nueva Física II".
 * 
 * Resuelve un Sudoku 9×9 usando:
 * - Normalización local de probabilidades (Ec. 3.1)
 * - Castigo y refuerzo diferenciado (Ec. 3.2)
 * - Umbral de bloqueo post-normalización (Ec. 3.3)
 * 
 * Características:
 * - Tiempo de resolución: ~1-2 segundos (vs 3:51 de la Versión 1)
 * - Validación matemática perfecta
 * - Sin deriva numérica gracias a normalización
 * 
 * Hardware requerido: ESP32-S3 N16R8 (funciona en cualquier ESP32)
 * 
 * Autor: Sergio Luis Marcano Ramos
 * Basado en: La Nueva Física II, Capítulo 3
 */

#include <SudokuTensorial.h>

// Instancia del resolvedor
SudokuTensorial sudoku;

// Sudoku de ejemplo (0 = celda vacía)
// Este es el Sudoku de la Figura 3.3 del libro
int tableroInicial[9][9] = {
  {5, 3, 0, 0, 7, 0, 0, 0, 0},
  {6, 0, 0, 1, 9, 5, 0, 0, 0},
  {0, 9, 8, 0, 0, 0, 0, 6, 0},
  {8, 0, 0, 0, 6, 0, 0, 0, 3},
  {4, 0, 0, 8, 0, 3, 0, 0, 1},
  {7, 0, 0, 0, 2, 0, 0, 0, 6},
  {0, 6, 0, 0, 0, 0, 2, 8, 0},
  {0, 0, 0, 4, 1, 9, 0, 0, 5},
  {0, 0, 0, 0, 8, 0, 0, 7, 9}
};

// Variables de control
unsigned long tiempoInicio = 0;
bool resuelto = false;
int iteraciones = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║  RESOLVEDOR DE SUDOKU - TENSOR UNIVERSAL                  ║");
  Serial.println("║  Basado en: La Nueva Física II, Capítulo 3                ║");
  Serial.println("╚═══════════════════════════════════════════════════════════╝");
  Serial.println();
  
  // Mostrar tablero inicial
  Serial.println("Tablero inicial (0 = vacío):");
  imprimirTablero(tableroInicial);
  Serial.println();
  
  // Inicializar el resolvedor
  Serial.println("Inicializando resolvedor tensorial...");
  sudoku.inicializar();
  
  // Cargar el tablero
  sudoku.cargarTablero(tableroInicial);
  
  Serial.println();
  Serial.println("Parámetros del algoritmo:");
  Serial.print("  Umbral de bloqueo: "); Serial.println(sudoku.getUmbralBloqueo(), 2);
  Serial.print("  Factor de castigo: "); Serial.println(sudoku.getFactorCastigo(), 2);
  Serial.print("  Coeficiente de confinamiento: "); Serial.println(sudoku.getCoeficienteConfinamiento(), 2);
  Serial.println();
  Serial.println("Iniciando resolución...");
  Serial.println("─────────────────────────────────────────────────────────────");
  
  tiempoInicio = millis();
}

void loop() {
  if (!resuelto) {
    // Ejecutar 100 iteraciones del algoritmo
    for(int i = 0; i < 100; i++) {
      bool completado = sudoku.iterar();
      iteraciones++;
      
      if (completado) {
        resuelto = true;
        unsigned long tiempoFinal = millis();
        float tiempoSegundos = (tiempoFinal - tiempoInicio) / 1000.0;
        
        Serial.println();
        Serial.println("╔═══════════════════════════════════════════════════════════╗");
        Serial.println("║  ¡SUDOKU RESUELTO!                                        ║");
        Serial.println("╚═══════════════════════════════════════════════════════════╝");
        Serial.println();
        Serial.print("Tiempo de resolución: "); Serial.print(tiempoSegundos, 2); Serial.println(" segundos");
        Serial.print("Iteraciones totales: "); Serial.println(iteraciones);
        Serial.println();
        
        // Mostrar tablero resuelto
        Serial.println("Tablero resuelto:");
        sudoku.imprimirTableroResuelto();
        Serial.println();
        
        // Validar solución
        if (sudoku.validarSolucion()) {
          Serial.println("✓ Solución matemáticamente correcta");
          Serial.println("  - Todas las filas contienen 1-9 sin repeticiones");
          Serial.println("  - Todas las columnas contienen 1-9 sin repeticiones");
          Serial.println("  - Todos los bloques 3×3 contienen 1-9 sin repeticiones");
          Serial.println("  - Se respetan todas las pistas iniciales");
        } else {
          Serial.println("✗ Solución inválida");
        }
        
        Serial.println();
        Serial.println("Comparativa con versiones anteriores:");
        Serial.println("  Versión 1 (Glóbulo Rojo):  231 segundos");
        Serial.println("  Versión 2 (Windows Forms): 3.34 segundos (inválida)");
        Serial.println("  Versión 3 (Unity 6):       1.76 segundos ✓");
        Serial.print("  Esta versión (ESP32-S3):   "); Serial.print(tiempoSegundos, 2); Serial.println(" segundos");
        
        while(true); // Detener ejecución
      }
    }
    
    // Mostrar progreso cada segundo
    if (millis() % 1000 < 20) {
      int celdasResueltas = sudoku.getCeldasResueltas();
      float porcentaje = (celdasResueltas / 81.0) * 100.0;
      
      Serial.print("Progreso: "); Serial.print(porcentaje, 1); Serial.print("%");
      Serial.print(" ("); Serial.print(celdasResueltas); Serial.print("/81 celdas)");
      Serial.print(" | Iteraciones: "); Serial.println(iteraciones);
    }
  }
}

// Función auxiliar para imprimir tablero
void imprimirTablero(int tablero[9][9]) {
  Serial.println("┌───────┬───────┬───────┐");
  for (int i = 0; i < 9; i++) {
    Serial.print("│ ");
    for (int j = 0; j < 9; j++) {
      if (tablero[i][j] == 0) {
        Serial.print(". ");
      } else {
        Serial.print(tablero[i][j]); Serial.print(" ");
      }
      
      if ((j + 1) % 3 == 0 && j < 8) {
        Serial.print("│ ");
      }
    }
    Serial.println("│");
    
    if ((i + 1) % 3 == 0 && i < 8) {
      Serial.println("├───────┼───────┼───────┤");
    }
  }
  Serial.println("└───────┴───────┴───────┘");
}