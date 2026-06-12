#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <omp.h>

#include "csv_reader.h"
#include "ecg_processing.h"
#include "ecg_stream.h"
#include "json_writer.h"
#include "output_structs.h"

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void merge_packet_peaks(const ECG_Peaks *src, ECG_Peaks *dest) {
    for (int i = 0; i < src->R_count && dest->R_count < MAX_BEATS; i++) {
        dest->R[dest->R_count++] = src->R[i];
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.csv> <output.json>\n", argv[0]);
        return 1;
    }

    if (read_csv(argv[1]) != 0) {
        fprintf(stderr, "Erreur lecture CSV.\n");
        return 2;
    }

    const int lead_index = 1;
    const double *signal = ecg_data[lead_index];
    const size_t total_samples = (size_t)sample_count;

    ECG_Params params;
    memset(&params, 0, sizeof(params));
    params.sampling_rate_hz = SAMPLING_RATE;
    params.leads = LEADS;
    params.gain = 100.0;
    params.r_threshold_hint = 0.0;

    const int num_packets = ecg_stream_num_packets(total_samples);
    int nthreads = num_packets;
    if (nthreads > omp_get_max_threads()) {
        nthreads = omp_get_max_threads();
    }
    if (nthreads < 1) {
        nthreads = 1;
    }
    omp_set_num_threads(nthreads);

    ECG_Context **ctxs = calloc((size_t)nthreads, sizeof(ECG_Context *));
    ECG_Peaks *packet_peaks = calloc((size_t)num_packets, sizeof(ECG_Peaks));
    if (!ctxs || !packet_peaks) {
        fprintf(stderr, "Erreur d'allocation.\n");
        free(ctxs);
        free(packet_peaks);
        return 4;
    }

    for (int t = 0; t < nthreads; t++) {
        ctxs[t] = ecg_create(&params);
        if (!ctxs[t]) {
            fprintf(stderr, "Erreur: ecg_create() a échoué.\n");
            for (int i = 0; i < t; i++) {
                ecg_destroy(ctxs[i]);
            }
            free(ctxs);
            free(packet_peaks);
            return 4;
        }
    }

    double t1 = now_sec();

#pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        ECG_Context *ctx = ctxs[tid];

#pragma omp for schedule(static)
        for (int k = 0; k < num_packets; k++) {
            memset(&packet_peaks[k], 0, sizeof(ECG_Peaks));
            ecg_stream_process_packet(ctx, signal, total_samples, k, num_packets, lead_index, &packet_peaks[k]);
        }
    }

    double t2 = now_sec();

    ECG_Peaks peaks;
    ECG_Intervals intervals;
    memset(&peaks, 0, sizeof(peaks));
    memset(&intervals, 0, sizeof(intervals));

    for (int k = 0; k < num_packets; k++) {
        merge_packet_peaks(&packet_peaks[k], &peaks);
    }

    ecg_stream_compute_intervals(&peaks, &intervals, SAMPLING_RATE);

    for (int t = 0; t < nthreads; t++) {
        ecg_destroy(ctxs[t]);
    }
    free(ctxs);
    free(packet_peaks);

    printf("%d paquets traités (%d échantillons, chevauchement %d, %d threads).\n",
           num_packets, sample_count, OVERLAP, nthreads);
    printf("%d pics R détectés.\n", peaks.R_count);
    printf("Time = %.6f s\n", t2 - t1);

    if (write_json(argv[2], &peaks, &intervals) != 0) {
        fprintf(stderr, "Erreur écriture JSON.\n");
        return 3;
    }

    printf("Résultats sauvegardés dans %s\n", argv[2]);
    return 0;
}
