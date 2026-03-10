#include "ecg_processing.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ecg_utils.h"

ECG_Context* ecg_create(const ECG_Params* params) {
    if (!params) return NULL;

    ECG_Context* ctx = (ECG_Context*)malloc(sizeof(ECG_Context));
    if (!ctx) return NULL;

    /* Copier les paramètres dans le contexte */
    ctx->params = *params;

    return ctx;
}

void ecg_destroy(ECG_Context* ctx) {
    if (ctx) {
        /* Libérer les ressources internes */
        free(ctx);
    }
}

ECG_Status ecg_analyze(ECG_Context* ctx, const double* signal, size_t n_samples, int lead_idx, ECG_Peaks* peaks,
                       ECG_Intervals* intervals) {
    if (!ctx || !signal || !peaks) return ECG_ERR_NULL;
    if (n_samples == 0 || lead_idx < 0 || lead_idx >= LEADS) return ECG_ERR_PARAM;

    /* Copie modifiable du signal */
    double s[n_samples];
    for (size_t i = 0; i < n_samples; i++) {
        s[i] = signal[i];
    }

    /* 1. Suppression DC (offset de ligne de base) */

    ecg_remove_dc(s, n_samples);

    /* 2. Calcul de la dérivée du signal */

    ecg_derivative_1(s, s, n_samples);

    /* 3. Mise au carré (accentuation des pentes) */

    ecg_square(s, s, n_samples);

    /* 4. Intégration sur fenêtre glissante */

    ecg_mwi(s, s, n_samples, (size_t)(ctx->params.sampling_rate_hz * 0.150));  // Fenêtre de 150 ms

    /* 5. Seuil flottant : moyenne glissante + k * écart-type glissant */

    /*
     * Fenêtre de contexte : 600 ms de chaque côté.
     * threshold[i] = mean(s, i±half_win) + K * std(s, i±half_win)
     * K = 1.5 donne un bon compromis sensibilité / spécificité.
     */
    const int half_win = (int)(ctx->params.sampling_rate_hz * 0.300);
    const double K = 1.0;

    double* threshold = (double*)malloc(n_samples * sizeof(double));
    if (!threshold) return ECG_ERR_ALLOC;

    for (size_t i = 0; i < n_samples; i++) {
        int lo = (int)i - half_win;
        if (lo < 0) lo = 0;
        int hi = (int)i + half_win;
        if (hi >= (int)n_samples) hi = (int)n_samples - 1;
        size_t w = (size_t)(hi - lo + 1);

        /* Moyenne */
        double sum = 0.0;
        for (int j = lo; j <= hi; j++) sum += s[j];
        double mean = sum / (double)w;

        /* Écart-type */
        double var = 0.0;
        for (int j = lo; j <= hi; j++) {
            double d = s[j] - mean;
            var += d * d;
        }
        double std = sqrt(var / (double)w);

        threshold[i] = mean + K * std;
    }

    peaks->R_count = 0;

    for (size_t i = 1; i + 1 < n_samples && peaks->R_count < MAX_BEATS; i++) {
        /* Pic local au-dessus du seuil flottant */
        if (s[i] > threshold[i] && s[i] >= s[i - 1] && s[i] >= s[i + 1]) {
            /* Chercher le vrai pic R dans le signal original (±75 ms) */
            int hw = (int)(ctx->params.sampling_rate_hz * 0.075);
            int start = (int)i - hw;
            if (start < 0) start = 0;
            int end = (int)i + hw;
            if (end >= (int)n_samples) end = (int)n_samples - 1;

            int r_idx = start;
            for (int j = start + 1; j <= end; j++) {
                if (signal[j] > signal[r_idx]) r_idx = j;
            }

            peaks->R[peaks->R_count++] = r_idx;
        }
    }

    free(threshold);

    /* Calcul des intervalles RR */
    if (intervals) {
        intervals->count = 0;
        double fs = (double)ctx->params.sampling_rate_hz;
        for (int i = 1; i < peaks->R_count && intervals->count < MAX_BEATS; i++) {
            intervals->RR[intervals->count++] = (peaks->R[i] - peaks->R[i - 1]) / fs;
        }
    }

    return ECG_OK;
}
