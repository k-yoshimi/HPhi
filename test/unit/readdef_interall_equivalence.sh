#!/bin/sh
set -eu

WORKDIR="unit_readdef_interall_equivalence_work"
HPHI_BIN="../../../src/HPhi"
TOL="1.0e-12"

rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi

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

"${HPHI_BIN}" -sdry stan.in > sdry.log 2>&1

# Make the Lanczos start deterministic for stable cross-case comparison.
awk '
  $1=="initial_iv" { print "initial_iv     1"; next }
  { print }
' modpara.def > modpara_fixed.def

awk '
  $1=="ModPara" { print "ModPara modpara_fixed.def"; next }
  { print }
' namelist.def > namelist_base.def

cp namelist_base.def namelist_a.def
cp namelist_base.def namelist_b.def
echo "InterAll interall_a.def" >> namelist_a.def
echo "InterAll interall_b.def" >> namelist_b.def

# Case A: canonical off-diagonal representation.
cat > interall_a.def <<'EOF'
========================
NInterAll 2
========================
========zInterAll=======
========================
0 0 3 0 2 1 1 1 -0.2 0.0
1 1 2 1 3 0 0 0 -0.2 0.0
EOF

# Case B: exchange-form representation equivalent to case A.
cat > interall_b.def <<'EOF'
========================
NInterAll 2
========================
========zInterAll=======
========================
0 0 1 1 2 1 3 0 0.2 0.0
3 0 2 1 1 1 0 0 0.2 0.0
EOF

"${HPHI_BIN}" -e namelist_a.def > run_a.log 2>&1
awk '/^Energy/{print $2}/^Doublon/{print $2}' output/zvo_energy.dat > energy_a.dat

rm -rf output
"${HPHI_BIN}" -e namelist_b.def > run_b.log 2>&1
awk '/^Energy/{print $2}/^Doublon/{print $2}' output/zvo_energy.dat > energy_b.dat

awk -v tol="${TOL}" '
  NR==FNR { a[FNR]=$1; next }
  {
    d=$1-a[FNR];
    if (d < 0) d = -d;
    if (d > tol) {
      exit 1;
    }
  }
  END {
    if (FNR != 2) exit 1;
  }
' energy_a.dat energy_b.dat

echo "PASS: unit_readdef_interall_equivalence"
