#!/bin/sh
set -eu

WORKDIR="unit_te_interall_diagonal_characterization_work"
HPHI_BIN="../../../src/HPhi"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
TE_TOOL="${SCRIPT_DIR}/../testTECalc.py"
FLCT_FILE="output/Flct.dat"

rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi
if [ ! -f "${TE_TOOL}" ]; then
  echo "TE test tool is not found: ${TE_TOOL}" >&2
  exit 1
fi
HPHI_BIN="$(cd "$(dirname "${HPHI_BIN}")" && pwd)/$(basename "${HPHI_BIN}")"
TE_TOOL="$(cd "$(dirname "${TE_TOOL}")" && pwd)/$(basename "${TE_TOOL}")"

python3 "${TE_TOOL}" -p "${HPHI_BIN}" -mpi "${MPIRUN:-}" -t "Diagonal" > te_diagonal.log 2>&1
test -f "${FLCT_FILE}"

awk 'NR==2 || NR==101 {print $1, $2, $3, $4, $5}' "${FLCT_FILE}" > actual.dat
cat > reference.dat <<'EOF'
0.0000000000000000 4.0000000000000009 16.0000000000000036 0.2873253726900541 0.3082093228771928
0.9900000000000000 4.0000000000000018 16.0000000000000071 0.1264957142842115 0.1324722838647145
EOF

awk '
  NR==FNR {
    for (i=1; i<=5; i++) ref[FNR, i]=$i;
    next;
  }
  {
    for (i=1; i<=5; i++) {
      d=$i-ref[FNR, i];
      if (d < 0) d = -d;
      if (d > 1.0e-6) exit 1;
    }
  }
  END {
    if (FNR != 2) exit 1;
  }
' reference.dat actual.dat

echo "PASS: unit_te_interall_diagonal_characterization"
