#!/bin/sh
set -eu

WORKDIR="unit_sz_hubbard_calchs_equivalence_work"
HPHI_BIN="../../../src/HPhi"
TOL="1.0e-12"

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
model = "FermionHubbard"
method = "Lanczos"
lattice = "chain"
t = 1.0
U = 4.0
nelec = 4
2Sz = 0
EOF

"${HPHI_BIN}" -sdry stan.in > sdry.log 2>&1

# Fix initial vector for stable scalar comparisons.
awk '
  $1=="initial_iv" { print "initial_iv     1"; next }
  { print }
' modpara.def > modpara_base.def

cp modpara_base.def modpara_0.def
cp modpara_base.def modpara_1.def
printf 'CalcHS          0\n' >> modpara_0.def
printf 'CalcHS          1\n' >> modpara_1.def

awk '
  $1=="ModPara" { print "ModPara modpara_0.def"; next }
  { print }
' namelist.def > namelist_0.def

awk '
  $1=="ModPara" { print "ModPara modpara_1.def"; next }
  { print }
' namelist.def > namelist_1.def

"${HPHI_BIN}" -e namelist_0.def > run_0.log 2>&1
awk '/^Energy/{print $2}/^Doublon/{print $2}/^Sz/{print $2}' output/zvo_energy.dat > energy_0.dat
sed -nE 's/.*idim_max=([0-9]+).*/\1/p' output/CHECK_Memory.dat > idim_0.dat
test ! -f output/Err_sz.dat

rm -rf output
"${HPHI_BIN}" -e namelist_1.def > run_1.log 2>&1
awk '/^Energy/{print $2}/^Doublon/{print $2}/^Sz/{print $2}' output/zvo_energy.dat > energy_1.dat
sed -nE 's/.*idim_max=([0-9]+).*/\1/p' output/CHECK_Memory.dat > idim_1.dat
test ! -f output/Err_sz.dat

awk -v tol="${TOL}" '
  NR==FNR { ref[FNR]=$1; next }
  {
    d = $1 - ref[FNR];
    if (d < 0) d = -d;
    if (d > tol) exit 1;
  }
  END {
    if (FNR != 3) exit 1;
  }
' energy_0.dat energy_1.dat

cmp -s idim_0.dat idim_1.dat

echo "PASS: unit_sz_hubbard_calchs_equivalence"
