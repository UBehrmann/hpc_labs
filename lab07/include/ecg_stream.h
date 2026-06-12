#ifndef ECG_STREAM_H
#define ECG_STREAM_H

#include <stddef.h>

#include "ecg_processing.h"
#include "output_structs.h"

#define PACKET_SIZE 1000
#define OVERLAP     250
#define STRIDE      (PACKET_SIZE - OVERLAP)

int ecg_stream_num_packets(size_t total_samples);

void ecg_stream_process_packet(
    ECG_Context *ctx,
    const double *signal,
    size_t total_samples,
    int packet_idx,
    int num_packets,
    int lead_index,
    ECG_Peaks *out_peaks
);

void ecg_stream_compute_intervals(const ECG_Peaks *peaks, ECG_Intervals *intervals, int sampling_rate_hz);

#endif
