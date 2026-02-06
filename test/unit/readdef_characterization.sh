#!/bin/sh
set -eu

WORKDIR="unit_readdef_characterization_work"
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

# Prepare a minimal valid expert-mode input set.
"${HPHI_BIN}" -sdry stan.in > baseline_sdry.log 2>&1
"${HPHI_BIN}" -e namelist.def > baseline_eval.log 2>&1

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

# Case 1: duplicate keyword in namelist should fail.
cp namelist.def namelist_dup.def
echo "CalcMod calcmod.def" >> namelist_dup.def
expect_fail duplicate_keyword "${HPHI_BIN}" -e namelist_dup.def
grep -q "Same keywords exist" duplicate_keyword.log

# Case 2: invalid CalcType in calcmod should fail.
awk '
  $1=="CalcType" { print "CalcType 99"; next }
  { print }
' calcmod.def > calcmod_invalid.def

awk '
  $1=="CalcMod" { print "CalcMod calcmod_invalid.def"; next }
  { print }
' namelist.def > namelist_invalid_calcmod.def

expect_fail invalid_calcmod "${HPHI_BIN}" -e namelist_invalid_calcmod.def
grep -q "CalcType" invalid_calcmod.log

# Case 3: missing required LocSpin entry should fail.
awk '$1!="LocSpin" { print }' namelist.def > namelist_missing_locspin.def
expect_fail missing_locspin "${HPHI_BIN}" -e namelist_missing_locspin.def
grep -q "Need to make a def file for LocSpin" missing_locspin.log

echo "PASS: unit_readdef_characterization"
