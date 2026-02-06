#!/bin/sh
set -eu

WORKDIR="unit_readdef_missing_ncond_error_work"
HPHI_BIN="../../../src/HPhi"
rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi

cat > stan.in <<'EOF'
W = 4
L = 2
model = "FermionHubbard"
method = "Lanczos"
lattice = "Tetragonal"
t = 1.0
U = 4.0
nelec = 8
2Sz = 0
EOF

"${HPHI_BIN}" -sdry stan.in > baseline_sdry.log 2>&1

# Remove all particle-number related inputs from modpara to trigger the model-rule error path.
awk '
  $1!="Ncond" && $1!="Nup" && $1!="Ndown" && $1!="2Sz" { print }
' modpara.def > modpara_missing_ncond.def

awk '
  $1=="ModPara" { print "ModPara modpara_missing_ncond.def"; next }
  { print }
' namelist.def > namelist_missing_ncond.def

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

expect_fail missing_ncond "${HPHI_BIN}" -e namelist_missing_ncond.def
grep -q "NCond is not defined" missing_ncond.log

echo "PASS: unit_readdef_missing_ncond_error"
