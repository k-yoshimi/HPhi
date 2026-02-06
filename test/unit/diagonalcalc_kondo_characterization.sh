#!/bin/sh
set -eu

WORKDIR="unit_diagonalcalc_kondo_characterization_work"
HPHI_BIN="../../../src/HPhi"
PHYS_FILE="output/zvo_phys_Nup3_Ndown3.dat"

rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi
HPHI_BIN="$(cd "$(dirname "${HPHI_BIN}")" && pwd)/$(basename "${HPHI_BIN}")"

cat > stan.in <<'EOT'
L = 3
model = "Kondo"
method = "FullDiag"
lattice = "chain"
t = 1.0
J = 4.0
nelec = 3
2Sz = 0
EOT

"${HPHI_BIN}" -s stan.in > fulldiag_kondo.log 2>&1
test -f "${PHYS_FILE}"

# 56 basis states in this sector + 1 header line.
test "$(wc -l < "${PHYS_FILE}")" -eq 57

cat > reference_energy.dat <<'EOT'
-9.510820
-6.357216
-6.357216
EOT

awk 'NR>=2 && NR<=4 {print $1}' "${PHYS_FILE}" > actual_energy.dat
awk '
  NR==FNR {ref[NR]=$1; next}
  {
    d=$1-ref[FNR];
    if (d < 0) d = -d;
    if (d > 1.0e-6) exit 1;
  }
  END {
    if (FNR != 3) exit 1;
  }
' reference_energy.dat actual_energy.dat

echo "PASS: unit_diagonalcalc_kondo_characterization"
