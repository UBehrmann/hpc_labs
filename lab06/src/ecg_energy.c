#include <stdio.h>
#include <string.h>
#include <time.h>

#include "csv_reader.h"
#include "ecg_processing.h"
#include "json_writer.h"
#include "output_structs.h"
#include "perf_manager.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.csv> <output.json>\n", argv[0]);
        return 1;
    }

    if (read_csv(argv[1]) != 0) {
        return 2;
    }

    ECG_Peaks peaks;
    ECG_Intervals intervals;
    memset(&peaks, 0, sizeof(peaks));
    memset(&intervals, 0, sizeof(intervals));

    ECG_Params params;
    memset(&params, 0, sizeof(params));
    params.sampling_rate_hz = SAMPLING_RATE;
    params.leads = LEADS;
    params.gain = 100.0;
    params.r_threshold_hint = 0.0;

    ECG_Context *ctx = ecg_create(&params);
    if (!ctx) {
        fprintf(stderr, "ecg_create failed\n");
        return 4;
    }

    const int lead_index = 1;
    PerfManager pmon;
    PerfManager_init(&pmon);

    PerfManager_resume(&pmon);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    ECG_Status st =
        ecg_analyze(ctx, ecg_data[lead_index], (size_t)sample_count, lead_index, &peaks, &intervals);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    PerfManager_pause(&pmon);

    double elapsed_ms = (t1.tv_sec - t0.tv_sec) * 1e3 + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    if (st != ECG_OK) {
        fprintf(stderr, "ecg_analyze error %d\n", (int)st);
        ecg_destroy(ctx);
        return 6;
    }

    printf("%d pics R, analyse %.3f ms\n", peaks.R_count, elapsed_ms);
    ecg_destroy(ctx);

    if (write_json(argv[2], &peaks, &intervals) != 0) {
        return 3;
    }

    return 0;
}
