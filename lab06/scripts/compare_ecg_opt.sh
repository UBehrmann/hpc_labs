#!/usr/bin/env bash
# Compare energie avant/apres (2000 x ecg_analyze, 1 lecture CSV).
# Usage: sudo ./compare_ecg_opt.sh [csv] [iterations]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
CSV="${1:-${ROOT}/data/test_ecg.csv}"
ITERS="${2:-2000}"
EVENTS="power/energy-pkg/,power_core/energy-core/"

for bin in ecg_repeat_avant ecg_repeat_apres; do
    if [[ ! -x "${BUILD}/${bin}" ]]; then
        echo "Manquant: ${BUILD}/${bin} — fais: cd build && cmake .. && make" >&2
        exit 1
    fi
done

run_one() {
    local label="$1"
    local bin="$2"
    echo ""
    echo "=== ${label} (${bin}, ${ITERS} iterations) ==="
    perf stat -e "${EVENTS}" "${BUILD}/${bin}" "${CSV}" "${ITERS}"
}

run_one "AVANT (malloc + VLA)" ecg_repeat_avant
run_one "APRES (tampons ECG_Context)" ecg_repeat_apres
