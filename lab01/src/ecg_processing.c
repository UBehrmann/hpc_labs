#include "ecg_processing.h"
#include <stdlib.h>

ECG_Context *ecg_create(const ECG_Params *params)
{
    if (!params) return NULL;

    ECG_Context *ctx = (ECG_Context *)malloc(sizeof(ECG_Context));
    if (!ctx) return NULL;

    /* Initialiser les ressources internes selon les paramètres */
    /* ... */

    return ctx;
}

void ecg_destroy(ECG_Context *ctx)
{
    if (ctx) {
        /* Libérer les ressources internes */
        free(ctx);
    }
}

ECG_Status ecg_analyze(
    ECG_Context *ctx,
    const double *signal,
    size_t n_samples,
    int lead_idx,
    ECG_Peaks *peaks,
    ECG_Intervals *intervals)
{
    if (!ctx || !signal || !peaks) return ECG_ERR_NULL;
    if (n_samples == 0 || lead_idx < 0 || lead_idx >= LEADS) return ECG_ERR_PARAM;

    /* TODO: implémenter la détection des pics R, P, Q, S, T */
    (void)intervals;

    return ECG_OK;
}
