#include "TensorUniversal.h"
#include <math.h>

#define PI 3.14159265359f

TensorUniversal::TensorUniversal() {
    u = nullptr;
    u_next = nullptr;
}

TensorUniversal::~TensorUniversal() {
    if (u != nullptr) free(u);
    if (u_next != nullptr) free(u_next);
}

void TensorUniversal::inicializar(int n, float L_val, float uref, float g0, float Pesc,
                                  float d1, float d2, float th0, float w_th,
                                  float Amp, float w_forz, float ph, float x_0, float sigM, float f_s) {
    N = n;
    L = L_val;
    h = L / (N - 1);
    u_ref = uref;
    gamma0 = g0;
    P_esc = Pesc;
    D1 = d1;
    D2 = d2;
    theta0 = th0;
    omega_theta = w_th;
    A_forz = Amp;
    omega_forz = w_forz;
    phi_forz = ph;
    x0 = x_0 * L;
    sigmaM = sigM * L;
    fs = f_s;
    D_max = (D1 > D2) ? D1 : D2;
    t_sim = 0.0;

    // Verificar PSRAM
    if (ESP.getPsramSize() == 0) {
        Serial.println("¡ERROR CRÍTICO! PSRAM no detectada. Revisa la configuración en Arduino IDE.");
        while (true); // Detener ejecución
    }

    // Reservar memoria en PSRAM (8 MB disponibles en N16R8)
    size_t tamano = N * N * sizeof(float);
    u = (float*)heap_caps_malloc(tamano, MALLOC_CAP_SPIRAM);
    u_next = (float*)heap_caps_malloc(tamano, MALLOC_CAP_SPIRAM);

    if (u == nullptr || u_next == nullptr) {
        Serial.println("¡ERROR! No hay suficiente PSRAM para la matriz.");
        while (true);
    }

    // Inicializar campo a u_ref
    for (int i = 0; i < N * N; i++) {
        u[i] = u_ref;
        u_next[i] = u_ref;
    }

    calcularDeltaTMax();
    Serial.printf("Tensor inicializado: N=%d, h=%.4f, dt=%.5f, PSRAM usada: %d KB\n", 
                  N, h, dt, (tamano * 2) / 1024);
}

void TensorUniversal::calcularDeltaTMax() {
    // Ec. 1.12
    float denominador = (8.0 * D_max) / (h * h) + 1.4 * gamma0;
    dt_max = 2.0 / denominador;
    dt = fs * dt_max;
}

// --- Funciones Matemáticas Privadas ---

inline float TensorUniversal::get_K(float s) const {
    // Ec. 1.2: Núcleo de estabilidad
    float diff = s - 0.5;
    return 1.0 / sqrtf(diff * diff + 1.0);
}

inline float TensorUniversal::get_gamma(float s) const {
    // Ec. 1.4: Amortiguamiento no lineal
    return gamma0 * (1.0 - get_K(s));
}

inline float TensorUniversal::get_s(float u_val) const {
    // Ec. 1.5: Coordenada de control
    return 0.5 + (u_val - u_ref) / P_esc;
}

inline float TensorUniversal::get_M(float x) const {
    // Ec. 1.8: Perfil espacial (Gaussiana)
    float diff = (x - x0) / sigmaM;
    return expf(-0.5 * diff * diff);
}

inline float TensorUniversal::get_Omega_Z(float t) const {
    // Ec. 1.8: Forzado de sincronización
    return A_forz * sinf(omega_forz * t + phi_forz);
}

// --- El Lazo Principal ---

void TensorUniversal::paso() {
    // 1. Calcular componentes del Tensor T(t) (Ecs. 1.6, 1.7)
    // Convertir grados a radianes
    float theta_rad = (theta0 + omega_theta * t_sim) * (PI / 180.0);
    float cos_t = cosf(theta_rad);
    float sin_t = sinf(theta_rad);
    
    float Txx = D1 * cos_t * cos_t + D2 * sin_t * sin_t;
    float Tyy = D1 * sin_t * sin_t + D2 * cos_t * cos_t;
    float Txy = (D1 - D2) * cos_t * sin_t;
    float dos_Txy = 2.0 * Txy; // Optimización

    // 2. Calcular forzado temporal
    float Omega_Z = get_Omega_Z(t_sim);
    float h2 = h * h;
    float h2_4 = 4.0 * h2; // Para la derivada cruzada

    // 3. Integración de Euler Explícito (Ec. 1.10)
    for (int i = 0; i < N; i++) {
        float x_i = i * h;
        float M_x = get_M(x_i);
        float forz_total = Omega_Z * M_x;

        for (int j = 0; j < N; j++) {
            int idx = i * N + j;
            float u_val = u[idx];

            // Vecinos con condición de Neumann (Ec. 1.9)
            int i_p = wrap(i + 1);
            int i_m = wrap(i - 1);
            int j_p = wrap(j + 1);
            int j_m = wrap(j - 1);

            // Derivadas finitas centradas
            float u_xx = (u[i_p * N + j] - 2.0 * u_val + u[i_m * N + j]) / h2;
            float u_yy = (u[i * N + j_p] - 2.0 * u_val + u[i * N + j_m]) / h2;
            float u_xy = (u[i_p * N + j_p] - u[i_p * N + j_m] - u[i_m * N + j_p] + u[i_m * N + j_m]) / h2_4;

            // Términos de amortiguamiento (Ecs. 1.4, 1.5)
            float s_val = get_s(u_val);
            float gamma_val = get_gamma(s_val);
            float amortiguamiento = gamma_val * (u_val - u_ref);

            // Ecuación de evolución completa (Ec. 1.10)
            float du_dt = Txx * u_xx + dos_Txy * u_xy + Tyy * u_yy - amortiguamiento + forz_total;
            
            u_next[idx] = u_val + dt * du_dt;
        }
    }

    // 4. Intercambiar punteros (O(1), mucho más rápido que copiar arrays)
    float* temp = u;
    u = u_next;
    u_next = temp;

    t_sim += dt;
}

// --- Telemetría ---

float TensorUniversal::getEnergiaTotal() const {
    // Ec. 1.13
    float suma = 0.0;
    float h2 = h * h;
    for (int i = 0; i < N * N; i++) {
        suma += u[i] * u[i];
    }
    return 0.5 * h2 * suma;
}

float TensorUniversal::getEnergiaFluctuaciones() const {
    // Ec. 1.14
    float suma = 0.0;
    float h2 = h * h;
    for (int i = 0; i < N * N; i++) {
        float diff = u[i] - u_ref;
        suma += diff * diff;
    }
    return 0.5 * h2 * suma;
}

float TensorUniversal::getGammaMedio() const {
    float suma = 0.0;
    for (int i = 0; i < N * N; i++) {
        suma += get_gamma(get_s(u[i]));
    }
    return suma / (N * N);
}