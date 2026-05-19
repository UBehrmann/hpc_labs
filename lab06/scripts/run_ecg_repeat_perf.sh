#!/usr/bin/env bash
# Mesure ciblée : N appels à ecg_analyze (hors I/O JSON).
# Usage: sudo ./run_ecg_repeat_perf.sh [avant|apres] [csv] [iterations]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
VARIANT="${1:-apres}"
CSV="${2:-${ROOT}/data/test_ecg.csv}"
ITERS="${3:-2000}"

case "${VARIANT}" in
    avant) BIN="${BUILD}/ecg_repeat_avant" ;;
    apres) BIN="${BUILD}/ecg_repeat_apres" ;;
    *)
        echo "Variante: avant ou apres" >&2
        exit 1
        ;;
esac

if [[ ! -x "${BIN}" ]]; then
    echo "Compile d'abord: cd ${BUILD} && cmake .. && make" >&2
    exit 1
fi

cd "${BUILD}"
rm -f perf_fifo.ctl perf_fifo.ack
mkfifo perf_fifo.ctl perf_fifo.ack

exec {perf_ctl_fd}<>perf_fifo.ctl
exec {perf_ack_fd}<>perf_fifo.ack
export PERF_CTL_FD=${perf_ctl_fd}
export PERF_ACK_FD=${perf_ack_fd}

perf stat \
    -e power/energy-pkg/,power_core/energy-core/ \
    --delay=-1 \
    --control "fd:${perf_ctl_fd},${perf_ack_fd}" \
    -- "${BIN}" "${CSV}" "${ITERS}"
