#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv_reader.h"
#include "ecg_processing.h"
#include "output_structs.h"
#include "perf_manager.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.csv> [iterations]\n", argv[0]);
        return 1;
    }

    int iterations = 2000;
    if (argc >= 3) {
        iterations = atoi(argv[2]);
        if (iterations < 1) {
            fprintf(stderr, "iterations invalides\n");
            return 1;
        }
    }

    if (read_csv(argv[1]) != 0) {
        return 2;
    }

    ECG_Params params;
    memset(&params, 0, sizeof(params));
    params.sampling_rate_hz = SAMPLING_RATE;
    params.leads = LEADS;

    ECG_Context *ctx = ecg_create(&params);
    if (!ctx) {
        return 4;
    }

    const int lead_index = 1;
    ECG_Peaks peaks;
    ECG_Intervals intervals;

    PerfManager pmon;
    PerfManager_init(&pmon);
    PerfManager_resume(&pmon);

    for (int k = 0; k < iterations; ++k) {
        memset(&peaks, 0, sizeof(peaks));
        memset(&intervals, 0, sizeof(intervals));
        ECG_Status st = ecg_analyze(ctx, ecg_data[lead_index], (size_t)sample_count, lead_index, &peaks,
                                    &intervals);
        if (st != ECG_OK) {
            fprintf(stderr, "ecg_analyze error %d at iter %d\n", (int)st, k);
            ecg_destroy(ctx);
            return 6;
        }
    }

    PerfManager_pause(&pmon);

    printf("%d iterations, %d pics R (dernier run)\n", iterations, peaks.R_count);
    ecg_destroy(ctx);
    return 0;
}
