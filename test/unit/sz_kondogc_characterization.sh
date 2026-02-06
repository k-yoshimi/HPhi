#!/bin/sh
set -eu

WORKDIR="unit_sz_kondogc_characterization_work"
HPHI_BIN="../../../src/HPhi"

rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi
HPHI_BIN="$(cd "$(dirname "${HPHI_BIN}")" && pwd)/$(basename "${HPHI_BIN}")"

cat > stan.in <<'EOT'
L = 4
model = "KondoGC"
method = "Lanczos"
lattice = "chain"
t = 1.0
J = 4.0
outputmode = "all"
EOT

"${HPHI_BIN}" -s stan.in > kondogc.log 2>&1

test -f output/CHECK_Sdim.dat
test -f output/CHECK_Memory.dat
grep -Eq '^8[[:space:]]+256([[:space:]]|$)' output/CHECK_Sdim.dat
grep -Eq 'idim_max=4096([[:space:]]|$)' output/CHECK_Memory.dat
test ! -f output/Err_sz.dat

echo "PASS: unit_sz_kondogc_characterization"
