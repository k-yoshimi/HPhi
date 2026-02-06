#!/bin/sh
set -eu

WORKDIR="unit_readdef_teonebody_count_mismatch_work"
HPHI_BIN="../../../src/HPhi"
rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi

cat > stan.in <<'EOT'
model = "Hubbard"
method = "Time-Evolution"
lattice = "square"
a0w = 3
a0l = 0
a1w = 0
a1l = 3
t = 1.0
u = 10.0
nelec = 8
2sz = 0
lanczos_max = 40
initial_iv = 1
eigenvecio = "in"
dt = 0.01
tshift = 0.2
freq = 10.0
pumptype = "ACLaser"
vecpotw = 0.5
vecpotl = 0.5
EOT

"${HPHI_BIN}" -sdry stan.in > baseline_sdry.log 2>&1

# Force TEOneBody timestep mismatch: header says 2 but there are no step lines.
awk 'NR==2{$2=2} NR<=5{print}' teone.def > teone_bad.def
awk '
  $1=="TEOneBody" { print "TEOneBody teone_bad.def"; next }
  { print }
' namelist.def > namelist_teone_bad.def

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

expect_fail teonebody_count_mismatch "${HPHI_BIN}" -e namelist_teone_bad.def
grep -q "teone_bad.def" teonebody_count_mismatch.log

echo "PASS: unit_readdef_teonebody_count_mismatch"
