#define MAT_READER_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "csv_reader.h"
#include "ecg_processing.h"
#include "json_writer.h"
#include "mat_reader.h"
#include "output_structs.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.csv|input.mat> <output_json>\n", argv[0]);
        return 1;
    }

    const char *ext = strrchr(argv[1], '.');
    if (ext && strcmp(ext, ".mat") == 0) {
        if (read_mat(argv[1]) != 0) {
            fprintf(stderr, "Erreur lecture .mat.\n");
            return 2;
        }
    } else {
        if (read_csv(argv[1]) != 0) {
            fprintf(stderr, "Erreur lecture CSV.\n");
            return 2;
        }
    }

    ECG_Peaks peaks;
    ECG_Intervals intervals;
    memset(&peaks, 0, sizeof(peaks));
    memset(&intervals, 0, sizeof(intervals));

    ECG_Params params;
    memset(&params, 0, sizeof(params));
    params.sampling_rate_hz = SAMPLING_RATE;
    params.leads = LEADS;
    params.gain = 100.0;            // <-- Ajuster le gain si nécessaire
    params.r_threshold_hint = 0.0;  // <-- Optionnel, peut être 0.0, et peut-être adaptatif au long du code.

    ECG_Context* ctx = ecg_create(&params);
    if (!ctx) {
        fprintf(stderr, "Erreur: ecg_create() a échoué.\n");
        return 4;
    }

    int lead_index = 1;  // Analyser la LEAD II (index 1)
    if (lead_index < 0 || lead_index >= LEADS) {
        fprintf(stderr, "Erreur: lead_index invalide.\n");
        ecg_destroy(ctx);
        return 5;
    }

    /* Ici vous êtes libre de déconstruire en chunk ou d'analyser le signal dans son entiéreté
       Dans la réalité vous serez plus ammené a avoir un flux continus plutôt qu'un gros chunk de données */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    ECG_Status st = ecg_analyze(ctx, ecg_data[lead_index], (size_t)sample_count, lead_index, &peaks, &intervals);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed_ms = (t1.tv_sec - t0.tv_sec) * 1e3 + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    if (st != ECG_OK) {
        fprintf(stderr, "Erreur: ecg_analyze() a retourné %d.\n", (int)st);
        ecg_destroy(ctx);
        return 6;
    }

    printf("%d pics R détectés.\n", peaks.R_count);
    printf("Durée de l'analyse : %.3f ms\n", elapsed_ms);

    ecg_destroy(ctx);

    if (write_json(argv[2], &peaks, &intervals) != 0) {
        fprintf(stderr, "Erreur écriture JSON.\n");
        return 3;
    }

    printf("Analyse terminée. Résultats sauvegardés dans %s\n", argv[2]);
    return 0;
}
