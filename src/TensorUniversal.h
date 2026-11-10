#ifndef TENSOR_UNIVERSAL_H
#define TENSOR_UNIVERSAL_H

#include <Arduino.h>
#include <esp_heap_caps.h> // Necesario para asignar memoria en la PSRAM

class TensorUniversal {
private:
    // Parámetros de la malla y física
    int N;
    float h, L;
    float u_ref, gamma0, P_esc;
    float D1, D2;
    float theta0, omega_theta; // en grados y grados/s
    float A_forz, omega_forz, phi_forz;
    float x0, sigmaM;
    float fs;
    float D_max;

    // Variables de tiempo
    float dt, dt_max, t_sim;

    // Matrices en PSRAM (Usamos arrays 1D aplanados para mejor rendimiento de caché)
    float* u;      // Estado actual
    float* u_next; // Estado siguiente

    // Funciones auxiliares matemáticas (Ecs. 1.2, 1.4, 1.5, 1.8)
    inline float get_K(float s) const;
    inline float get_gamma(float s) const;
    inline float get_s(float u_val) const;
    inline float get_M(float x) const;
    inline float get_Omega_Z(float t) const;

    // Condición de contorno de Neumann (Ec. 1.9)
    inline int wrap(int i) const {
        if (i < 0) return 1;
        if (i >= N) return N - 2;
        return i;
    }

public:
    TensorUniversal();
    ~TensorUniversal();

    // Inicialización con los parámetros del Cuadro 1.2
    void inicializar(int n, float L_val, float uref, float g0, float Pesc,
                     float d1, float d2, float th0, float w_th,
                     float Amp, float w_forz, float ph, float x_0, float sigM, float f_s);

    // Cálculo del paso de tiempo máximo (Ec. 1.12)
    void calcularDeltaTMax();

    // El corazón del algoritmo: Un paso de tiempo (Ec. 1.10)
    void paso();

    // Telemetría (Ecs. 1.13, 1.14)
    float getEnergiaTotal() const;
    float getEnergiaFluctuaciones() const;
    float getGammaMedio() const;
    
    // Getters
    float getTiempoSimulacion() const { return t_sim; }
    float getDeltaT() const { return dt; }
};

#endif