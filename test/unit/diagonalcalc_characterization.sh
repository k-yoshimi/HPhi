#!/bin/sh
set -eu

WORKDIR="unit_diagonalcalc_characterization_work"
HPHI_BIN="../../../src/HPhi"
PHYS_FILE="output/zvo_phys_Nup2_Ndown2.dat"

rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi
HPHI_BIN="$(cd "$(dirname "${HPHI_BIN}")" && pwd)/$(basename "${HPHI_BIN}")"

cat > stan.in <<'EOF'
L = 4
model = "FermionHubbard"
method = "FullDiag"
lattice = "chain"
t = 1.0
U = 4.0
nelec = 4
2Sz = 0
EOF

"${HPHI_BIN}" -s stan.in > fulldiag.log 2>&1
test -f "${PHYS_FILE}"

# 36 basis states in this sector + 1 header line.
test "$(wc -l < "${PHYS_FILE}")" -eq 37

cat > reference_energy.dat <<'EOF'
-2.102748
-1.806424
-1.068140
EOF

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

echo "PASS: unit_diagonalcalc_characterization"
