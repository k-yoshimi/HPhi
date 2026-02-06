#!/bin/sh
set -eu

WORKDIR="unit_sz_calchs_validation_work"
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
2Sz = 0
EOF

"${HPHI_BIN}" -sdry stan.in > sdry.log 2>&1
printf 'CalcHS          2\n' >> modpara.def

set +e
"${HPHI_BIN}" -e namelist.def > invalid_calchs.log 2>&1
rc=$?
set -e

if [ "${rc}" -eq 0 ]; then
  echo "Expected failure with invalid CalcHS for Kondo model." >&2
  exit 1
fi
grep -q "CalcHS in ModPara file must be 0 or 1 for Kondo model" invalid_calchs.log

echo "PASS: unit_sz_calchs_validation"
