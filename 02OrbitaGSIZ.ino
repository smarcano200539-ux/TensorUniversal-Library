/*
 * Ejemplo 2: Simulación Orbital con Ley GSIZ
 * 
 * Este ejemplo demuestra el potencial gravitacional regularizado (Ec. 2.3)
 * y la ecuación de movimiento (Ec. 2.4) del libro "La Nueva Física II".
 * 
 * Simula la órbita de un cuerpo de prueba alrededor de una masa central
 * usando el potencial de Plummer modificado por la Ley GSIZ.
 * 
 * Características:
 * - Fuerza nula en el ancla (r=0): g(0)=0
 * - Fuerza máxima acotada en r=ε/√2
 * - Recuperación de Newton para r≫ε
 * - Conservación de energía y momento angular
 * 
 * Hardware requerido: ESP32-S3 N16R8 con PSRAM activada
 * 
 * Autor: Sergio Luis Marcano Ramos
 * Basado en: La Nueva Física II, Capítulo 2
 */

#include <LeyGSIZ.h>

// Instancia de la Ley GSIZ
LeyGSIZ gravedad;

// Parámetros de la simulación
const float MASA_CENTRAL = 1e6;      // Masa central M (unidades arbitrarias)
const float EPSILON = 0.1;           // Radio de suavizado ε
const float G_CONST = 6.674e-11;     // Constante gravitacional
const float R_INICIAL = 3.0;         // Distancia inicial r/ε = 3
const float DT = 0.001;              // Paso de tiempo (s)

// Estado del cuerpo de prueba
float x, y;           // Posición
float vx, vy;         // Velocidad
float energia_inicial;
float momento_inicial;

// Variables de control
unsigned long lastPrint = 0;
int pasosTotales = 0;
float tiempoSimulacion = 0.0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║  SIMULACIÓN ORBITAL - LEY GSIZ                            ║");
  Serial.println("║  Basado en: La Nueva Física II, Capítulo 2                ║");
  Serial.println("╚═══════════════════════════════════════════════════════════╝");
  Serial.println();
  
  // Inicializar el campo gravitacional
  Serial.println("Inicializando campo GSIZ...");
  gravedad.inicializar(MASA_CENTRAL, EPSILON, G_CONST);
  
  Serial.println();
  Serial.println("Parámetros de simulación:");
  Serial.print("  Masa central M: "); Serial.println(MASA_CENTRAL, 0);
  Serial.print("  Radio de suavizado ε: "); Serial.println(EPSILON, 4);
  Serial.print("  Distancia inicial r/ε: "); Serial.println(R_INICIAL, 2);
  Serial.print("  Paso temporal Δt: "); Serial.println(DT, 4);
  Serial.println();
  
  // Calcular velocidad orbital circular teórica
  // v = sqrt(G*M*r / (r²+ε²)^(3/2))
  float r = R_INICIAL * EPSILON;
  float v_circular = sqrtf(G_CONST * MASA_CENTRAL * r / powf(r*r + EPSILON*EPSILON, 1.5));
  
  Serial.println("Condiciones iniciales:");
  Serial.print("  Posición inicial: ("); Serial.print(r, 4); Serial.print(", 0)");
  Serial.print(" | r/ε = "); Serial.println(R_INICIAL, 2);
  Serial.print("  Velocidad tangencial: "); Serial.println(v_circular, 6);
  Serial.println();
  
  // Posicionar el cuerpo en (r, 0) con velocidad tangencial
  x = r;
  y = 0.0;
  vx = 0.0;
  vy = v_circular;
  
  // Calcular energía y momento angular iniciales
  energia_inicial = gravedad.calcularEnergia(x, y, vx, vy);
  momento_inicial = gravedad.calcularMomentoAngular(x, y, vx, vy);
  
  Serial.println("Valores de referencia:");
  Serial.print("  Energía total E0: "); Serial.println(energia_inicial, 10);
  Serial.print("  Momento angular L0: "); Serial.println(momento_inicial, 10);
  Serial.println();
  
  // Calcular periodo orbital teórico
  float T_teorico = gravedad.calcularPeriodo(r);
  Serial.print("Periodo orbital teórico T: "); Serial.print(T_teorico, 4); Serial.println(" s");
  Serial.print("Periodo de Kepler TK: "); Serial.print(2.0 * PI * sqrtf(powf(r, 3) / (G_CONST * MASA_CENTRAL)), 4); Serial.println(" s");
  Serial.print("Razón T/TK: "); Serial.println(T_teorico / (2.0 * PI * sqrtf(powf(r, 3) / (G_CONST * MASA_CENTRAL))), 6);
  Serial.println();
  Serial.println("Iniciando simulación orbital...");
  Serial.println("─────────────────────────────────────────────────────────────");
}

void loop() {
  // Ejecutar 1000 pasos de integración por ciclo
  for(int i = 0; i < 1000; i++) {
    // Esquema leapfrog (kick-drift-kick) para conservación de energía
    gravedad.integrarPasoLeapfrog(x, y, vx, vy, DT);
    tiempoSimulacion += DT;
    pasosTotales++;
  }
  
  // Imprimir telemetría cada segundo
  if (millis() - lastPrint > 1000) {
    float r = sqrtf(x*x + y*y);
    float v = sqrtf(vx*vx + vy*vy);
    float energia_actual = gravedad.calcularEnergia(x, y, vx, vy);
    float momento_actual = gravedad.calcularMomentoAngular(x, y, vx, vy);
    
    // Calcular errores relativos
    float error_E = fabsf((energia_actual - energia_inicial) / energia_inicial);
    float error_L = fabsf((momento_actual - momento_inicial) / momento_inicial);
    
    Serial.print("t=");
    Serial.print(tiempoSimulacion, 3);
    Serial.print("s | r=");
    Serial.print(r, 4);
    Serial.print(" | v=");
    Serial.print(v, 4);
    Serial.print(" | ΔE/E0=");
    Serial.print(error_E, 10);
    Serial.print(" | ΔL/L0=");
    Serial.print(error_L, 10);
    Serial.println();
    
    // Verificar conservación
    if (error_E < 1e-6 && error_L < 1e-6) {
      Serial.println("  ✓ Conservación excelente (error < 10⁻⁶)");
    } else if (error_E < 1e-4 && error_L < 1e-4) {
      Serial.println("  ◐ Conservación buena (error < 10⁻⁴)");
    } else {
      Serial.println("  ⚠ Conservación degradada");
    }
    
    // Mostrar posición actual
    Serial.print("  Posición: ("); Serial.print(x, 4); Serial.print(", "); Serial.print(y, 4); Serial.println(")");
    Serial.println();
    
    lastPrint = millis();
  }
}