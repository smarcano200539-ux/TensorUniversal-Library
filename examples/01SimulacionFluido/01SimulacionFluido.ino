/*
 * Ejemplo 1: Simulación de Fluido con Tensor Universal
 * 
 * Este ejemplo demuestra la ecuación de difusión tensorial con amortiguamiento
 * no lineal (Ec. 1.10 del libro "La Nueva Física II").
 * 
 * Simula la evolución de un campo de presión u(x,y,t) bajo:
 * - Difusión anisótropa con tensor rotatorio T(t)
 * - Amortiguamiento no lineal γ(s) que estabiliza el sistema
 * - Forzado periódico ΩZ(t) con perfil espacial M(x)
 * 
 * Hardware requerido: ESP32-S3 N16R8 con PSRAM activada
 * 
 * Autor: Sergio Luis Marcano Ramos
 * Basado en: La Nueva Física II, Capítulo 1
 */

#include <TensorUniversal.h>

// Instancia del Tensor Universal
TensorUniversal tensor;

// Parámetros del Cuadro 1.2 del libro
const int N_MALLA = 64;           // Malla 64×64 celdas
const float L_DOMINIO = 1.0;      // Dominio [0,1]×[0,1]
const float U_REF = 50.0;         // Presión nominal (eje crítico s=0.5)
const float GAMMA0 = 2.0;         // Amortiguamiento máximo
const float P_ESC = 20.0;         // Escala de presión
const float D1 = 1.0;             // Difusividad principal 1
const float D2 = 0.25;            // Difusividad principal 2
const float THETA0 = 30.0;        // Ángulo inicial del tensor (grados)
const float OMEGA_THETA = 5.0;    // Velocidad de rotación (grados/s)
const float A_FORZ = 8.0;         // Amplitud del forzado
const float OMEGA_FORZ = 1.5;     // Frecuencia angular del forzado (rad/s)
const float PHI = 0.0;            // Fase del forzado
const float X0 = 0.10;            // Posición del perfil M(x) (fracción de L)
const float SIGMA_M = 0.05;       // Ancho del perfil M(x)
const float FS = 0.8;             // Factor de seguridad para Δt

// Variables de control
unsigned long lastPrint = 0;
unsigned long lastStep = 0;
int pasosTotales = 0;

void setup() {
  Serial.begin(115200);
  delay(2000); // Esperar a que el monitor serial abra
  
  Serial.println("╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║  SIMULACIÓN DE FLUIDO - TENSOR UNIVERSAL                  ║");
  Serial.println("║  Basado en: La Nueva Física II, Capítulo 1                ║");
  Serial.println("╚═══════════════════════════════════════════════════════════╝");
  Serial.println();
  
  // Inicializar el Tensor Universal con los parámetros del Cuadro 1.2
  Serial.println("Inicializando Tensor Universal...");
  tensor.inicializar(
    N_MALLA, 
    L_DOMINIO, 
    U_REF, 
    GAMMA0, 
    P_ESC,
    D1, 
    D2, 
    THETA0, 
    OMEGA_THETA,
    A_FORZ, 
    OMEGA_FORZ, 
    PHI, 
    X0, 
    SIGMA_M, 
    FS
  );
  
  Serial.println();
  Serial.println("Parámetros de simulación:");
  Serial.print("  Malla: "); Serial.print(N_MALLA); Serial.print("×"); Serial.println(N_MALLA);
  Serial.print("  Paso espacial h: "); Serial.println(tensor.getH(), 6);
  Serial.print("  Paso temporal Δt: "); Serial.println(tensor.getDeltaT(), 6);
  Serial.print("  Δt máximo estable: "); Serial.println(tensor.getDeltaTMax(), 6);
  Serial.print("  PSRAM utilizada: "); Serial.print((N_MALLA * N_MALLA * 4 * 2) / 1024); Serial.println(" KB");
  Serial.println();
  Serial.println("Iniciando simulación...");
  Serial.println("─────────────────────────────────────────────────────────────");
}

void loop() {
  // Ejecutar 100 pasos de simulación por ciclo del loop
  // Esto acelera la simulación sin saturar el Serial
  for(int i = 0; i < 100; i++) {
    tensor.paso();
    pasosTotales++;
  }
  
  // Imprimir telemetría cada segundo
  if (millis() - lastPrint > 1000) {
    float tiempo = tensor.getTiempoSimulacion();
    float E_tot = tensor.getEnergiaTotal();
    float E_fl = tensor.getEnergiaFluctuaciones();
    float gamma_med = tensor.getGammaMedio();
    
    Serial.print("t=");
    Serial.print(tiempo, 4);
    Serial.print("s | E_tot=");
    Serial.print(E_tot, 4);
    Serial.print(" | E_fl=");
    Serial.print(E_fl, 4);
    Serial.print(" | γ=");
    Serial.print(gamma_med, 6);
    Serial.print(" | pasos=");
    Serial.println(pasosTotales);
    
    // Mostrar estado del sistema
    if (E_fl < 1.0) {
      Serial.println("  ✓ Sistema estabilizado (E_fl < 1.0)");
    } else if (E_fl < 10.0) {
      Serial.println("  ◐ Sistema en relajación");
    } else {
      Serial.println("  ◉ Sistema en transiente");
    }
    
    lastPrint = millis();
  }
  
  // Simular un evento de Slug Flow cada 5 segundos
  // (perturbación abrupta en el campo de presión)
  if (millis() - lastStep > 5000) {
    Serial.println();
    Serial.println("⚡ Evento Slug Flow: Inyectando perturbación...");
    
    // Inyectar una perturbación en el centro del dominio
    int centro = (N_MALLA / 2) * N_MALLA + (N_MALLA / 2);
    tensor.inyectarPerturbacion(centro, 100.0); // +100 unidades de presión
    
    lastStep = millis();
  }
}