#!/bin/sh
set -eu

WORKDIR="unit_sz_characterization_work"
HPHI_BIN="../../../src/HPhi"

rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi
HPHI_BIN="$(cd "$(dirname "${HPHI_BIN}")" && pwd)/$(basename "${HPHI_BIN}")"

mkdir -p spin
cd spin
cat > stan.in <<'EOF'
L = 4
model = "Spin"
method = "Lanczos"
lattice = "chain"
J = 1.0
2Sz = 0
EOF

"${HPHI_BIN}" -s stan.in > spin.log 2>&1
test -f output/CHECK_Sdim.dat
test -f output/CHECK_Memory.dat
grep -q '^sdim=4 =2\^2$' output/CHECK_Sdim.dat
grep -Eq 'idim_max=6([[:space:]]|$)' output/CHECK_Memory.dat
cd ..

mkdir -p hubbard
cd hubbard
cat > stan.in <<'EOF'
L = 4
model = "FermionHubbard"
method = "Lanczos"
lattice = "chain"
t = 1.0
U = 4.0
nelec = 4
2Sz = 0
EOF

"${HPHI_BIN}" -s stan.in > hubbard.log 2>&1
test -f output/CHECK_Sdim.dat
test -f output/CHECK_Memory.dat
grep -q '^sdim=16 =2\^4$' output/CHECK_Sdim.dat
grep -Eq 'idim_max=36([[:space:]]|$)' output/CHECK_Memory.dat

echo "PASS: unit_sz_characterization"
