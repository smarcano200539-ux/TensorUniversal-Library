#include "LeyGSIZ.h"
#include <math.h>

#define PI 3.14159265359f

LeyGSIZ::LeyGSIZ() {
    M = 0;
    epsilon = 0;
    G = 0;
}

void LeyGSIZ::inicializar(float masa, float eps, float constanteG) {
    M = masa;
    epsilon = eps;
    G = constanteG;
}

float LeyGSIZ::calcularPotencial(float r) const {
    // Ec. 2.3: Potencial de Plummer modificado
    return - (G * M) / sqrtf(r * r + epsilon * epsilon);
}

float LeyGSIZ::calcularCampo(float r) const {
    // Ec. 2.4: Magnitud del campo de aceleración
    float denom = powf(r * r + epsilon * epsilon, 1.5);
    return - (G * M * r) / denom;
}

void LeyGSIZ::calcularAceleracion(float x, float y, float &ax, float &ay) const {
    float r = sqrtf(x * x + y * y);
    if (r < 1e-6) {
        ax = 0; ay = 0; // Fuerza nula en el ancla (r=0)
        return;
    }
    float g_mag = calcularCampo(r);
    // Vector unitario radial
    ax = g_mag * (x / r);
    ay = g_mag * (y / r);
}

float LeyGSIZ::calcularEnergia(float x, float y, float vx, float vy) const {
    float r = sqrtf(x * x + y * y);
    float v2 = vx * vx + vy * vy;
    float E_cinetica = 0.5 * v2; // Asumiendo masa de prueba = 1
    float E_potencial = calcularPotencial(r);
    return E_cinetica + E_potencial;
}

float LeyGSIZ::calcularMomentoAngular(float x, float y, float vx, float vy) const {
    // L = x*vy - y*vx (producto cruz en 2D)
    return (x * vy) - (y * vx);
}

float LeyGSIZ::calcularPeriodo(float r) const {
    // Periodo orbital T(r) = 2π(r²+ε²)^(3/4) / √(GM)
    float numerador = 2.0 * PI * powf(r * r + epsilon * epsilon, 0.75);
    float denominador = sqrtf(G * M);
    return numerador / denominador;
}

void LeyGSIZ::integrarPasoLeapfrog(float &x, float &y, float &vx, float &vy, float dt) const {
    // Esquema Leapfrog (Kick-Drift-Kick) - Sección 2.4
    // 1. Kick (medio paso de velocidad)
    float ax0, ay0;
    calcularAceleracion(x, y, ax0, ay0);
    vx += 0.5 * ax0 * dt;
    vy += 0.5 * ay0 * dt;

    // 2. Drift (paso completo de posición)
    x += vx * dt;
    y += vy * dt;

    // 3. Kick (segundo medio paso de velocidad)
    float ax1, ay1;
    calcularAceleracion(x, y, ax1, ay1);
    vx += 0.5 * ax1 * dt;
    vy += 0.5 * ay1 * dt;
}