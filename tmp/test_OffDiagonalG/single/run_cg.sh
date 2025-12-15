#!/bin/bash
# Script to calculate c_0^dagger c_1 Green's function using CG method
# 1D Hubbard 4-site (U=4, t=1, half-filling)
# G(z) = <gs| c_0^dagger (1/(z-H)) c_1 |gs>

HPHI=../../../build/src/HPhi

# Clear output directory
rm -rf output
mkdir -p output

echo "=== Step 1: Ground state calculation (CG) ==="
$HPHI -s stan_gs.in

if [ ! -f output/zvo_eigenvec_0_rank_0.dat ]; then
    echo "Error: Eigenvector file was not generated"
    exit 1
fi

echo ""
echo "=== Step 2: Spectrum calculation (BiCG) ==="
echo "A = c_{1,up} (annihilation at site 1, spin-up)"
echo "B = c_{0,up} (annihilation at site 0, spin-up)"
echo "Calculating: <gs| c_0^dagger (1/(z-H)) c_1 |gs>"
echo ""

$HPHI -e namelist_cg.def

echo ""
echo "=== Calculation completed ==="
echo "Result: output/zvo_DynamicalGreen.dat"

if [ -f output/zvo_DynamicalGreen.dat ]; then
    echo ""
    echo "=== First 10 lines ==="
    head -10 output/zvo_DynamicalGreen.dat
fi

