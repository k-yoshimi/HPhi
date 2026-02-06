#!/bin/sh
set -eu

WORKDIR="unit_sz_tjgc_expert_characterization_work"
HPHI_BIN="../../../src/HPhi"

rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

if [ ! -x "${HPHI_BIN}" ]; then
  echo "HPhi binary is not found: ${HPHI_BIN}" >&2
  exit 1
fi
HPHI_BIN="$(cd "$(dirname "${HPHI_BIN}")" && pwd)/$(basename "${HPHI_BIN}")"

cat > calcmod.def <<'EOF'
#CalcType = 0:Lanczos, 1:TPQCalc, 2:FullDiag, 3:CG, 4:Time-evolution 5:cTPQ
#CalcModel = 0:Hubbard, 1:Spin, 2:Kondo, 3:HubbardGC, 4:SpinGC, 5:KondoGC
#Restart = 0:None, 1:Save, 2:Restart&Save, 3:Restart
#CalcSpec = 0:None, 1:Normal, 2:No H*Phi, 3:Save, 4:Restart, 5:Restart&Save
CalcType   0
CalcModel   10
ReStart   0
CalcSpec   0
CalcEigenVec   0
InitialVecType   0
InputEigenVec   0
OutputEigenVec   0
InputHam   0
OutputHam   0
OutputExVec   0
EOF

cat > locspn.def <<'EOF'
================================
NlocalSpin     0
================================
========i_1LocSpn_0IteElc ======
================================
    0      0
    1      0
    2      0
    3      0
EOF

cat > trans.def <<'EOF'
========================
NTransfer      4
========================
========i_j_s_tijs======
========================
    1     0     0     0         1.0         0.0
    0     0     1     0         1.0         0.0
    1     1     0     1         1.0         0.0
    0     1     1     1         1.0         0.0
EOF

cat > modpara.def <<'EOF'
--------------------
Model_Parameters   0
--------------------
HPhi_Cal_Parameters
--------------------
CDataFileHead  zvo
CParaFileHead  zqp
--------------------
Nsite          4
Lanczos_max    200
initial_iv     -1
exct           1
LanczosEps     14
LanczosTarget  2
LargeValue     8.0
NumAve         5
ExpecInterval  20
NOmega         200
OmegaMax       3.2e1 8.0e-2
OmegaMin       -3.2e1 8.0e-2
OmegaOrg       0.0 0.0
PreCG          1
EOF

cat > namelist.def <<'EOF'
         ModPara  modpara.def
         LocSpin  locspn.def
         CalcMod  calcmod.def
           Trans  trans.def
EOF

"${HPHI_BIN}" -e namelist.def > tjgc_sz.log 2>&1
grep -q '^sdim=16 =2\^4$' output/CHECK_Sdim.dat
grep -Eq 'idim_max=81([[:space:]]|$)' output/CHECK_Memory.dat
test ! -f output/Err_sz.dat

echo "PASS: unit_sz_tjgc_expert_characterization"
