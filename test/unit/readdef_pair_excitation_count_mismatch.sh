#!/bin/sh
set -eu

WORKDIR="unit_readdef_pair_excitation_count_mismatch_work"
HPHI_BIN="../../../src/HPhi"
rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi

cat > stan.in <<'EOT'
W = 4
L = 2
model = "FermionHubbard"
method = "Lanczos"
lattice = "Tetragonal"
t = 1.0
U = 4.0
nelec = 8
2Sz = 0
EOT

"${HPHI_BIN}" -sdry stan.in > baseline_sdry.log 2>&1

# Force PairExcitation count mismatch: header says 1 but no data line exists.
awk 'NR==2{$2=1} NR<=5{print}' pair.def > pair_bad.def
awk '
  $1=="PairExcitation" { print "PairExcitation pair_bad.def"; next }
  { print }
' namelist.def > namelist_pair_bad.def

expect_fail() {
  name="$1"
  shift
  set +e
  "$@" > "${name}.log" 2>&1
  rc=$?
  set -e
  if [ "${rc}" -eq 0 ]; then
    echo "Expected failure but command succeeded: ${name}" >&2
    exit 1
  fi
}

expect_fail pair_excitation_count_mismatch "${HPHI_BIN}" -e namelist_pair_bad.def
grep -q "pair_bad.def" pair_excitation_count_mismatch.log

echo "PASS: unit_readdef_pair_excitation_count_mismatch"
