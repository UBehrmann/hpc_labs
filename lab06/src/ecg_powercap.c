#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv_reader.h"
#include "ecg_processing.h"
#include "json_writer.h"
#include "output_structs.h"
#include "powercap-rapl.h"

static const char *zone_name(powercap_rapl_zone zone) {
    switch (zone) {
        case POWERCAP_RAPL_ZONE_PACKAGE:
            return "package";
        case POWERCAP_RAPL_ZONE_CORE:
            return "core";
        case POWERCAP_RAPL_ZONE_DRAM:
            return "dram";
        case POWERCAP_RAPL_ZONE_PSYS:
            return "psys";
        default:
            return "uncore";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.csv> <output.json>\n", argv[0]);
        return 1;
    }

    if (read_csv(argv[1]) != 0) {
        return 2;
    }

    const uint32_t npackages = powercap_rapl_get_num_instances();
    if (npackages == 0) {
        fprintf(stderr, "Aucune instance RAPL\n");
        return 2;
    }

    powercap_rapl_pkg *pkg = calloc(npackages, sizeof(*pkg));
    uint64_t *e1 = calloc(npackages, sizeof(*e1));
    uint64_t *e2 = calloc(npackages, sizeof(*e2));
    if (!pkg || !e1 || !e2) {
        free(pkg);
        free(e1);
        free(e2);
        return 2;
    }

    const powercap_rapl_zone zone = POWERCAP_RAPL_ZONE_PACKAGE;
    int has_supported = 0;
    for (uint32_t i = 0; i < npackages; ++i) {
        if (powercap_rapl_init(i, &pkg[i], 1) != 0) {
            fprintf(stderr, "init package %u failed\n", i);
            continue;
        }
        if (powercap_rapl_is_zone_supported(&pkg[i], zone)) {
            has_supported = 1;
        } else {
            fprintf(stderr, "zone %s non supportee sur package %u\n", zone_name(zone), i);
        }
    }

    if (!has_supported) {
        fprintf(stderr, "Aucun package compatible\n");
        return 2;
    }

    for (uint32_t j = 0; j < npackages; ++j) {
        if (powercap_rapl_is_zone_supported(&pkg[j], zone)) {
            if (powercap_rapl_get_energy_uj(&pkg[j], zone, &e1[j]) != 0) {
                fprintf(stderr, "lecture energie avant echec (pkg %u)\n", j);
                return 3;
            }
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

    ECG_Context *ctx = ecg_create(&params);
    if (!ctx) {
        return 4;
    }

    const int lead_index = 1;
    ECG_Status st =
        ecg_analyze(ctx, ecg_data[lead_index], (size_t)sample_count, lead_index, &peaks, &intervals);
    ecg_destroy(ctx);

    if (st != ECG_OK) {
        return 6;
    }

    for (uint32_t j = 0; j < npackages; ++j) {
        if (powercap_rapl_is_zone_supported(&pkg[j], zone)) {
            if (powercap_rapl_get_energy_uj(&pkg[j], zone, &e2[j]) != 0) {
                fprintf(stderr, "lecture energie apres echec (pkg %u)\n", j);
                return 3;
            }
        }
    }

    double total_j = 0.0;
    for (uint32_t i = 0; i < npackages; ++i) {
        if (powercap_rapl_is_zone_supported(&pkg[i], zone)) {
            const double delta_j = (double)(e2[i] - e1[i]) / 1e6;
            total_j += delta_j;
            printf("package %u %s: %.6f J\n", i, zone_name(zone), delta_j);
        }
        powercap_rapl_destroy(&pkg[i]);
    }

    printf("%d pics R, energie totale %s: %.6f J\n", peaks.R_count, zone_name(zone), total_j);
    write_json(argv[2], &peaks, &intervals);

    free(pkg);
    free(e1);
    free(e2);
    return 0;
}
