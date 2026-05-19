#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
CSV="${1:-${ROOT}/data/test_ecg.csv}"
OUT="${2:-/tmp/ecg_out.json}"
BIN="${BUILD}/ecg_energy"

if [[ ! -x "${BIN}" ]]; then
    echo "Binaire manquant: ${BIN}" >&2
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
    -- "${BIN}" "${CSV}" "${OUT}"
