#ifndef LEY_GSIZ_H
#define LEY_GSIZ_H

#include <Arduino.h>

class LeyGSIZ {
private:
    float M;          // Masa central
    float epsilon;    // Radio de suavizado (Ec. 2.2)
    float G;          // Constante gravitacional

    // Funciones auxiliares (Ecs. 2.2, 2.3, 2.4)
    float calcularPotencial(float r) const;
    float calcularCampo(float r) const;

public:
    LeyGSIZ();

    // Inicialización
    void inicializar(float masa, float eps, float constanteG);

    // Cálculos físicos (Ecs. 2.3, 2.4, 2.5)
    void calcularAceleracion(float x, float y, float &ax, float &ay) const;
    float calcularEnergia(float x, float y, float vx, float vy) const;
    float calcularMomentoAngular(float x, float y, float vx, float vy) const;
    float calcularPeriodo(float r) const; // T(r) = 2π(r²+ε²)^(3/4) / √(GM)

    // Integración numérica (Esquema Leapfrog - Sección 2.4)
    void integrarPasoLeapfrog(float &x, float &y, float &vx, float &vy, float dt) const;
};

#endif