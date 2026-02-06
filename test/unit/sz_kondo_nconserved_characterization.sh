#!/bin/sh
set -eu

WORKDIR="unit_sz_kondo_nconserved_characterization_work"
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
model = "Kondo"
method = "Lanczos"
lattice = "chain"
t = 1.0
J = 4.0
nelec = 4
EOF

"${HPHI_BIN}" -s stan.in > kondo_nconserved.log 2>&1
test -f output/CHECK_Sdim.dat
test -f output/CHECK_Memory.dat
grep -q '^sdim=256 =2\^8$' output/CHECK_Sdim.dat
grep -Eq 'idim_max=1120([[:space:]]|$)' output/CHECK_Memory.dat
test ! -f output/Err_sz.dat

echo "PASS: unit_sz_kondo_nconserved_characterization"
