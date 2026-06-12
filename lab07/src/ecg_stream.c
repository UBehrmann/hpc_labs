#include "ecg_stream.h"

#include <string.h>

int ecg_stream_num_packets(size_t total_samples) {
    if (total_samples == 0) {
        return 0;
    }
    if (total_samples <= PACKET_SIZE) {
        return 1;
    }
    return 1 + (int)((total_samples - PACKET_SIZE + STRIDE - 1) / STRIDE);
}

void ecg_stream_process_packet(
    ECG_Context *ctx,
    const double *signal,
    size_t total_samples,
    int packet_idx,
    int num_packets,
    int lead_index,
    ECG_Peaks *out_peaks
) {
    size_t start = (size_t)packet_idx * STRIDE;
    size_t len = PACKET_SIZE;
    if (start + len > total_samples) {
        len = total_samples - start;
    }

    double packet[PACKET_SIZE];
    memcpy(packet, signal + start, len * sizeof(double));

    ECG_Peaks local;
    ECG_Intervals intervals;
    memset(&local, 0, sizeof(local));
    memset(&intervals, 0, sizeof(intervals));

    if (ecg_analyze(ctx, packet, len, lead_index, &local, &intervals) != ECG_OK) {
        return;
    }

    /* Keep peaks in [n*750, (n+1)*750); overlap is analyzed but not double-counted. */
    size_t lo = start;
    size_t hi = (packet_idx == num_packets - 1) ? total_samples : start + STRIDE;

    for (int i = 0; i < local.R_count && out_peaks->R_count < MAX_BEATS; i++) {
        size_t global = start + (size_t)local.R[i];
        if (global >= lo && global < hi) {
            out_peaks->R[out_peaks->R_count++] = (int)global;
        }
    }
}

void ecg_stream_compute_intervals(const ECG_Peaks *peaks, ECG_Intervals *intervals, int sampling_rate_hz) {
    intervals->count = 0;
    const double fs = (double)sampling_rate_hz;
    for (int i = 1; i < peaks->R_count && intervals->count < MAX_BEATS; i++) {
        intervals->RR[intervals->count++] = (peaks->R[i] - peaks->R[i - 1]) / fs;
    }
}
