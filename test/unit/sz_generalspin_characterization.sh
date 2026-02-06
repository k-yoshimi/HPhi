#!/bin/sh
set -eu

WORKDIR="unit_sz_generalspin_characterization_work"
HPHI_BIN="../../../src/HPhi"

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
model = "Spin"
method = "Lanczos"
lattice = "chain"
J = 1.0
2S = 3
2Sz = 0
EOF

"${HPHI_BIN}" -s stan.in > generalspin.log 2>&1

test -f output/CHECK_Sdim.dat
test -f output/CHECK_Memory.dat
grep -Eq '^3[[:space:]]+64([[:space:]]|$)' output/CHECK_Sdim.dat
grep -Eq 'idim_max=44([[:space:]]|$)' output/CHECK_Memory.dat
test ! -f output/Err_sz.dat

echo "PASS: unit_sz_generalspin_characterization"
