/* HPhi  -  Quantum Lattice Model Simulator */
/* Copyright (C) 2015 The University of Tokyo */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <bitcalc.h>
#include "common/setmemory.h"
#include "FileIO.h"
#include "sz.h"
#include "wrapperMPI.h"
#include "xsetmem.h"

static void InitializeGeneralSpinWorkArrays(
    const struct BindStruct *X,
    long int **list_2_1_Sz,
    long int **list_2_2_Sz
) {
    long unsigned int j;

    *list_2_1_Sz = NULL;
    *list_2_2_Sz = NULL;
    if (X->Def.iFlgGeneralSpin != TRUE) {
        return;
    }

    *list_2_1_Sz = li_1d_allocate(X->Check.sdim + 2);
    *list_2_2_Sz = li_1d_allocate((X->Def.Tpow[X->Def.Nsite - 1] * X->Def.SiteToBit[X->Def.Nsite - 1] / X->Check.sdim) + 2);
    for (j = 0; j < X->Check.sdim + 2; j++) {
        (*list_2_1_Sz)[j] = 0;
    }
    for (j = 0; j < (X->Def.Tpow[X->Def.Nsite - 1] * X->Def.SiteToBit[X->Def.Nsite - 1] / X->Check.sdim) + 2; j++) {
        (*list_2_2_Sz)[j] = 0;
    }
}

static long unsigned int *AllocateAndClearListJb(const struct BindStruct *X) {
    long unsigned int i;
    long unsigned int *list_jb;

    list_jb = lui_1d_allocate(X->Large.SizeOflistjb);
    for (i = 0; i < X->Large.SizeOflistjb; i++) {
        list_jb[i] = 0;
    }
    return list_jb;
}

static void SetupHilbertDimensionContext(
    struct BindStruct *X,
    unsigned int *N2,
    unsigned int *N,
    double *idim
) {
    long unsigned int j;

    switch (X->Def.iCalcModel) {
        case HubbardGC:
        case HubbardNConserved:
        case Hubbard:
        case tJGC:
        case tJNConserved:
        case tJ:
            *N2 = 2 * X->Def.Nsite;
            *idim = pow(2.0, *N2);
            break;
        case KondoGC:
        case Kondo:
        case KondoNConserved:
            *N2 = 2 * X->Def.Nsite;
            *N = X->Def.Nsite;
            *idim = pow(2.0, *N2);
            for (j = 0; j < *N; j++) {
                fprintf(stdoutMPI, cStateLocSpin, j, X->Def.LocSpn[j]);
            }
            break;
        case SpinGC:
        case Spin:
            *N = X->Def.Nsite;
            if (X->Def.iFlgGeneralSpin == FALSE) {
                *idim = pow(2.0, *N);
            } else {
                *idim = 1;
                for (j = 0; j < *N; j++) {
                    *idim *= X->Def.SiteToBit[j];
                }
            }
            break;
        default:
            break;
    }
}

static int SetupSplitBitContext(
    struct BindStruct *X,
    long unsigned int *irght,
    long unsigned int *ilft,
    long unsigned int *ihfbit
) {
    switch (X->Def.iCalcModel) {
        case HubbardNConserved:
        case Hubbard:
        case tJGC:
        case tJNConserved:
        case tJ:
        case KondoGC:
        case Kondo:
        case KondoNConserved:
        case Spin:
            if (X->Def.iFlgGeneralSpin == FALSE) {
                if (GetSplitBitByModel(X->Def.Nsite, X->Def.iCalcModel, irght, ilft, ihfbit) != 0) {
                    return -1;
                }
                X->Large.irght = *irght;
                X->Large.ilft = *ilft;
                X->Large.ihfbit = *ihfbit;
            } else {
                *ihfbit = X->Check.sdim;
            }
            break;
        default:
            break;
    }
    return 0;
}

static void ValidateSzDimensionOrAbort(const long unsigned int calculated_dim, const long unsigned int expected_dim) {
    FILE *fp_err;
    char sdt_err[D_FileNameMax];

    if (calculated_dim == expected_dim) {
        return;
    }

    fprintf(stderr, "%s", cErrSz);
    fprintf(stderr, cErrSz_ShowDim, calculated_dim, expected_dim);
    strcpy(sdt_err, cFileNameErrorSz);
    if (childfopenMPI(sdt_err, "a", &fp_err) != 0) {
        exitMPI(-1);
    }
    fprintf(fp_err, "%s", cErrSz_OutFile);
    fclose(fp_err);
    exitMPI(-1);
}

static void ConvertNConservedModelToNormalIfNeeded(struct BindStruct *X) {
    if (X->Def.iFlgCalcSpec != CALCSPEC_NOT) {
        return;
    }
    if (X->Def.iCalcModel == HubbardNConserved) {
        X->Def.iCalcModel = Hubbard;
    }
    if (X->Def.iCalcModel == KondoNConserved) {
        X->Def.iCalcModel = Kondo;
    }
    if (X->Def.iCalcModel == tJNConserved) {
        X->Def.iCalcModel = tJ;
    }
}

static void RecordOMPSzMid(struct BindStruct *X) {
    TimeKeeper(X, cFileNameSzTimeKeep, cOMPSzMid, "a");
    TimeKeeper(X, cFileNameTimeKeep, cOMPSzMid, "a");
}

static long unsigned int ComputeSzCountForTJFamily(
    struct BindStruct *X,
    long unsigned int ihfbit,
    unsigned int N2,
    long unsigned int *list_1_,
    long unsigned int *list_2_1_,
    long unsigned int *list_2_2_,
    long unsigned int *list_jb
) {
    long unsigned int ib;
    long unsigned int icnt = 0;

    switch (X->Def.iCalcModel) {
        case tJ:
            calculate_jb_tJ(X, list_jb, ihfbit, N2);
            break;
        case tJNConserved:
            calculate_jb_tJNConserved(X, list_jb, ihfbit, N2);
            break;
        case tJGC:
            calculate_jb_tJGC(X, list_jb, ihfbit, N2);
            break;
        default:
            return 0;
    }

    RecordOMPSzMid(X);
    for (ib = 0; ib < X->Check.sdim; ib++) {
        icnt += omp_sz_tJ(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
    }
    return icnt;
}

static int ComputeSzCountForHubbardFamily(
    struct BindStruct *X,
    long unsigned int ihfbit,
    unsigned int N2,
    long unsigned int *list_1_,
    long unsigned int *list_2_1_,
    long unsigned int *list_2_2_,
    long unsigned int *list_jb,
    long unsigned int *icnt_out
) {
    int hacker;
    long unsigned int ib;
    long unsigned int icnt = 0;

    hacker = X->Def.read_hacker;
    switch (X->Def.iCalcModel) {
        case Hubbard:
            if (hacker == 0) {
                calculate_jb_Hubbard(X, list_jb, ihfbit, N2);
                RecordOMPSzMid(X);
                for (ib = 0; ib < X->Check.sdim; ib++) {
                    icnt += omp_sz(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
                }
            } else if (hacker == 1) {
                calculate_jb_Hubbard_Hacker(X, list_jb, ihfbit, N2);
                RecordOMPSzMid(X);
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, X) shared(list_1_, list_2_1_, list_2_2_, list_jb)
                for (ib = 0; ib < X->Check.sdim; ib++) {
                    icnt += omp_sz_hacker(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
                }
            } else {
                fprintf(stderr, "Error: CalcHS in ModPara file must be 0 or 1 for Hubbard model.");
                return -1;
            }
            break;
        case HubbardNConserved:
            if (hacker == 0) {
                calculate_jb_HubbardNCoserved(X, list_jb, ihfbit, N2);
                RecordOMPSzMid(X);
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, N2, X) shared(list_1_, list_2_1_, list_2_2_, list_jb)
                for (ib = 0; ib < X->Check.sdim; ib++) {
                    icnt += omp_sz(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
                }
            } else if (hacker == 1) {
                calculate_jb_HubbardNCoserved_Hacker(X, list_jb, ihfbit, N2);
                RecordOMPSzMid(X);
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, N2, X) shared(list_1_, list_2_1_, list_2_2_, list_jb)
                for (ib = 0; ib < X->Check.sdim; ib++) {
                    icnt += omp_sz_hacker(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
                }
            } else {
                fprintf(stderr, "Error: CalcHS in ModPara file must be 0 or 1 for Hubbard model.");
                return -1;
            }
            break;
        default:
            return -1;
    }

    *icnt_out = icnt;
    return 0;
}

static int ComputeSzCountForKondoFamily(
    struct BindStruct *X,
    long unsigned int ihfbit,
    unsigned int N2,
    long unsigned int *list_1_,
    long unsigned int *list_2_1_,
    long unsigned int *list_2_2_,
    long unsigned int *list_jb,
    long unsigned int *icnt_out
) {
    int hacker;
    long unsigned int ib;
    long unsigned int num_loc;
    long unsigned int icnt = 0;

    switch (X->Def.iCalcModel) {
        case KondoGC:
            num_loc = count_localized_spins(X);
            calculate_jb_KondoGC(X, num_loc, list_jb, ihfbit);
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, N2, X) shared(list_1_, list_2_1_, list_2_2_, list_jb)
            for (ib = 0; ib < X->Check.sdim; ib++) {
                icnt += omp_sz_KondoGC(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
            }
            break;
        case KondoNConserved:
            calculate_jb_KondoNConserved(X, list_jb, ihfbit);
            RecordOMPSzMid(X);
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, N2, X) shared(list_1_, list_2_1_, list_2_2_, list_jb)
            for (ib = 0; ib < X->Check.sdim; ib++) {
                icnt += omp_sz_KondoNConserved(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
            }
            BarrierMPI();
            break;
        case Kondo:
            calculate_jb_Kondo(X, list_jb, ihfbit);
            RecordOMPSzMid(X);
            hacker = X->Def.read_hacker;
            if (hacker == 0) {
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, N2, X) shared(list_1_, list_2_1_, list_2_2_, list_jb)
                for (ib = 0; ib < X->Check.sdim; ib++) {
                    icnt += omp_sz_Kondo(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
                }
            } else if (hacker == 1) {
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, N2, X) shared(list_1_, list_2_1_, list_2_2_, list_jb)
                for (ib = 0; ib < X->Check.sdim; ib++) {
                    icnt += omp_sz_Kondo_hacker(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_jb);
                }
            } else {
                fprintf(stderr, "Error: CalcHS in ModPara file must be 0 or 1 for Kondo model.");
                return -1;
            }
            break;
        default:
            return -1;
    }

    *icnt_out = icnt;
    return 0;
}

static int ComputeSzCountForSpin(
    struct BindStruct *X,
    long unsigned int ihfbit,
    long unsigned int irght,
    long unsigned int ilft,
    long unsigned int ibpatn,
    unsigned int N,
    long unsigned int *list_1_,
    long unsigned int *list_2_1_,
    long unsigned int *list_2_2_,
    long unsigned int *list_jb,
    long int *list_2_1_Sz,
    long int *list_2_2_Sz,
    long unsigned int *icnt_io
) {
    int hacker;
    long unsigned int ib;
    long unsigned int icnt;

    icnt = *icnt_io;
    if (X->Def.iFlgGeneralSpin == FALSE) {
        hacker = X->Def.read_hacker;
        if (hacker == -1) {
            calculate_jb_Spin_m1(X, list_jb, list_1_, list_2_1_, list_2_2_, ihfbit, irght, ilft, ibpatn, N);
            /*
             * Legacy CalcHS=-1 path constructs the restricted basis directly in
             * calculate_jb_Spin_m1. Keep i_max consistent with the known target
             * Hilbert dimension for later validation.
             */
            icnt = X->Check.idim_max;
        } else if (hacker == 1) {
            calculate_jb_Spin_Hacker(X, list_jb, ihfbit, N);
            RecordOMPSzMid(X);
            icnt = 0;
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, N, X, list_1_, list_2_1_, list_2_2_, list_jb)
            for (ib = 0; ib < X->Check.sdim; ib++) {
                icnt += omp_sz_spin_hacker(ib, ihfbit, N, X, list_1_, list_2_1_, list_2_2_, list_jb);
            }
        } else if (hacker == 0) {
            calculate_jb_Spin_Old(X, list_jb, ihfbit, N);
            RecordOMPSzMid(X);
            icnt = 0;
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ihfbit, N, X) shared(list_1_, list_2_1_, list_2_2_, list_jb)
            for (ib = 0; ib < X->Check.sdim; ib++) {
                icnt += omp_sz_spin(ib, ihfbit, N, X, list_1_, list_2_1_, list_2_2_, list_jb);
            }
        } else {
            fprintf(stderr, "Error: CalcHS in ModPara file must be -1 or 0 or 1 for Spin model.");
            return -1;
        }
    } else {
        long unsigned int ilftdim = (X->Def.Tpow[X->Def.Nsite - 1] * X->Def.SiteToBit[X->Def.Nsite - 1]) / ihfbit;
        calculate_jb_GeneralSpin(X, list_jb, list_2_1_Sz, list_2_2_Sz, ihfbit, ilftdim, N);
        RecordOMPSzMid(X);

        icnt = 0;
#pragma omp parallel for default(none) reduction(+:icnt) private(ib) firstprivate(ilftdim, ihfbit, X) shared(list_1_, list_2_1_, list_2_2_, list_2_1_Sz, list_2_2_Sz, list_jb)
        for (ib = 0; ib < ilftdim; ib++) {
            icnt += omp_sz_GeneralSpin(ib, ihfbit, X, list_1_, list_2_1_, list_2_2_, list_2_1_Sz, list_2_2_Sz, list_jb);
        }
    }

    *icnt_io = icnt;
    return 0;
}

static int ComputeSzCountByModel(
    struct BindStruct *X,
    const long unsigned int ihfbit,
    const long unsigned int irght,
    const long unsigned int ilft,
    const long unsigned int ibpatn,
    const unsigned int N,
    const unsigned int N2,
    long unsigned int *list_1_,
    long unsigned int *list_2_1_,
    long unsigned int *list_2_2_,
    long unsigned int *list_jb,
    long int *list_2_1_Sz,
    long int *list_2_2_Sz,
    long unsigned int *icnt
) {
    switch (X->Def.iCalcModel) {
        case HubbardGC:
            *icnt = X->Def.Tpow[2 * X->Def.Nsite - 1] * 2 + 0;
            break;
        case SpinGC:
            if (X->Def.iFlgGeneralSpin == FALSE) {
                *icnt = X->Def.Tpow[X->Def.Nsite - 1] * 2 + 0;
            } else {
                *icnt = X->Def.Tpow[X->Def.Nsite - 1] * X->Def.SiteToBit[X->Def.Nsite - 1];
            }
            break;
        case HubbardNConserved:
        case Hubbard:
            if (ComputeSzCountForHubbardFamily(X, ihfbit, N2, list_1_, list_2_1_, list_2_2_, list_jb, icnt) != 0) {
                return -1;
            }
            break;
        case KondoGC:
        case KondoNConserved:
        case Kondo:
            if (ComputeSzCountForKondoFamily(X, ihfbit, N2, list_1_, list_2_1_, list_2_2_, list_jb, icnt) != 0) {
                return -1;
            }
            break;
        case tJ:
        case tJNConserved:
        case tJGC:
            *icnt = ComputeSzCountForTJFamily(X, ihfbit, N2, list_1_, list_2_1_, list_2_2_, list_jb);
            break;
        case Spin:
            if (ComputeSzCountForSpin(X, ihfbit, irght, ilft, ibpatn, N, list_1_, list_2_1_, list_2_2_, list_jb, list_2_1_Sz, list_2_2_Sz, icnt) != 0) {
                return -1;
            }
            break;
        default:
            return -1;
    }
    return 0;
}

/**
 * @file   sz.c
 * 
 * @brief  Generating Hilbert spaces
 * 
 * @version 0.2
 * @details 
 *
 * @version 0.1
 *
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 * 
 */


/** 
 * 
 * @brief generating Hilbert space
 * 
 * @param[inout] X 
 * @param[out] list_1_   list_1[icnt] = i (index of full Hilbert space) : icnt = index in the restricted Hilbert space 
 * @param[out] list_2_1_ icnt=list_2_1[]+list_2_2[] 
 * @param[out] list_2_2_ 
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
int sz(
    struct BindStruct *X,
    long unsigned int *list_1_,
    long unsigned int *list_2_1_,
    long unsigned int *list_2_2_
){
    FILE *fp;
    char sdt[D_FileNameMax];
    long unsigned int irght = 0;
    long unsigned int ilft = 0;
    long unsigned int ihfbit = 0;
    long unsigned int i_max = 0;
    long unsigned int icnt = 1;
    long unsigned int ibpatn = 0;
    unsigned int num_threads;
    unsigned int N2 = 0;
    unsigned int N = 0;
    double idim = 0.0;
    long int **comb;

    /*[s] for general spin*/
    long int *list_2_1_Sz = NULL;
    long int *list_2_2_Sz = NULL;
    InitializeGeneralSpinWorkArrays(X, &list_2_1_Sz, &list_2_2_Sz);
    /*[e] for general spin*/

    long unsigned int *list_jb;
    list_jb = AllocateAndClearListJb(X);

    fprintf(stdoutMPI, "%s", cProStartCalcSz);
    TimeKeeper(X, cFileNameSzTimeKeep, cInitalSz, "w");
    TimeKeeper(X, cFileNameTimeKeep, cInitalSz, "a");
    if(X->Check.idim_max!=0){
        /*[s] calculating the maximum size of Hilbert dimensions*/
        SetupHilbertDimensionContext(X, &N2, &N, &idim);
        /*[e] calculating the maximum size of Hilbert dimensions*/

        comb  = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1); /*comb=Binomial coefficient */
        i_max = X->Check.idim_max; /*i_max, idim_max : Hilbert dimensions*/
    
        /*[s] calculating irght, ilht, ihfbit*/
        if (SetupSplitBitContext(X, &irght, &ilft, &ihfbit) != 0) {
            exitMPI(-1);
        }
        /*[e] calculating irght, ilht, ihfbit*/
        
        icnt=1;

        if(X->Def.READ==1){
            if(Read_sz(X, irght, ilft, ihfbit, &i_max)!=0){
                exitMPI(-1);
            }
        }else{ 
            sprintf(sdt, cFileNameSzTimeKeep, X->Def.CDataFileHead);
            #ifdef _OPENMP
                num_threads  = omp_get_max_threads();
            #else
                num_threads  = 1;
            #endif
            childfopenMPI(sdt,"a", &fp);
            fprintf(fp, "num_threads==%d\n",num_threads);
            fclose(fp);
          
            //*[s] omp parallel

            TimeKeeper(X, cFileNameSzTimeKeep, cOMPSzStart, "a");
            TimeKeeper(X, cFileNameTimeKeep, cOMPSzStart, "a");
            if (ComputeSzCountByModel(
                    X,
                    ihfbit,
                    irght,
                    ilft,
                    ibpatn,
                    N,
                    N2,
                    list_1_,
                    list_2_1_,
                    list_2_2_,
                    list_jb,
                    list_2_1_Sz,
                    list_2_2_Sz,
                    &icnt) != 0) {
                return -1;
            }
              i_max=icnt;
              //printf("BBB i_max %ld icnt=%ld\n",i_max,icnt);
              //fprintf(stdoutMPI, "Debug: Xicnt=%ld \n",icnt);
              TimeKeeper(X, cFileNameSzTimeKeep, cOMPSzFinish, "a");
              TimeKeeper(X, cFileNameTimeKeep, cOMPSzFinish, "a");
          }
          /* NConserved -> Normal */
          // this part is move to the ene of sz.c so that this change is made even 
          // when idim_max=0
          /*
          if(X->Def.iFlgCalcSpec == CALCSPEC_NOT){
              if(X->Def.iCalcModel  == HubbardNConserved){
                  X->Def.iCalcModel =  Hubbard;
              }
              if(X->Def.iCalcModel  == KondoNConserved){
                  X->Def.iCalcModel  = Kondo;
              }
              if(X->Def.iCalcModel  == tJNConserved){
                  X->Def.iCalcModel  = tJ;
              }
          }
          */
          ValidateSzDimensionOrAbort(i_max, X->Check.idim_max);
          free_li_2d_allocate(comb);
    }
    fprintf(stdoutMPI, "%s", cProEndCalcSz);
    free(list_jb);

    /* NConserved -> Normal */
    ConvertNConservedModelToNormalIfNeeded(X);

    if(X->Def.iFlgGeneralSpin==TRUE){
        free(list_2_1_Sz);
        free(list_2_2_Sz);
    }
    BarrierMPI();
    return 0;    
}

/** 
 * 
 * @file   sz.c
 * @brief  calculating binomial coefficients
 * 
 * @param[in] n      n for @f$_nC_k = \frac{n!}{(n-k)!k!}@f$
 * @param[in] k      k for @f$_nC_k = \frac{n!}{(n-k)!k!}@f$
 * @param[out] comb   binomial coefficients @f$_nC_k@f$
 * @param[in] Nsite  # of sites
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
long int Binomial(int n,int k,long int **comb,int Nsite){
  // nCk, Nsite=max(n)
  int tmp_i,tmp_j;

  if(n==0 && k==0){
      return 1;
  } else if(n<0 || k<0 || n<k){
      return 0;
  }
  
  for(tmp_i=0;tmp_i<=Nsite;tmp_i++){
      for(tmp_j=0;tmp_j<=Nsite;tmp_j++){
          comb[tmp_i][tmp_j] = 0;
      }
  }

  comb[0][0] = 1;
  comb[1][0] = 1;
  comb[1][1] = 1;
  for(tmp_i=2;tmp_i<=n;tmp_i++){
      for(tmp_j=0;tmp_j<=tmp_i;tmp_j++){
          if(tmp_j==0){
              comb[tmp_i][tmp_j] = 1;
          }else if(tmp_j==tmp_i){
              comb[tmp_i][tmp_j] = 1;
          }else{
              comb[tmp_i][tmp_j] = comb[tmp_i-1][tmp_j-1]+comb[tmp_i-1][tmp_j];
          }
      }
  }
  return comb[n][k];
}

/** 
 * @brief calculating restricted Hilbert space for the tJ systems
 * 
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia) 
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[in] list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 */
int omp_sz_tJ(
                 long unsigned int ib,    //!<[in]
                 long unsigned int ihfbit, //!<[in]
                 struct BindStruct *X,     //!<[in]
                 long unsigned int *list_1_, //!<[out]
                 long unsigned int *list_2_1_,//!<[out]
                 long unsigned int *list_2_2_,//!<[out]
                 long unsigned int *list_jb_ //!<[in]
                 )
{
  long unsigned int i,j; 
  long unsigned int ia,ja,jb;
  long unsigned int div_down, div_up;
  long unsigned int num_up,num_down;
  long unsigned int tmp_num_up,tmp_num_down;
  int check_doublon;
    
  jb = list_jb_[ib];
  i  = ib*ihfbit;
    
  num_up          = 0;
  num_down        = 0;
  check_doublon   = 0;
  ja              = 1;
  for(j=0;j< X->Def.Nsite ;j++){
    div_up          = i & X->Def.Tpow[2*j];
    div_up          = div_up/X->Def.Tpow[2*j];
    div_down        = i & X->Def.Tpow[2*j+1];
    div_down        = div_down/X->Def.Tpow[2*j+1];
    check_doublon   = div_up*div_down;
    //printf("Ns %d i %ld j %ld  div_up %ld div_down %ld \n",X->Def.Nsite,i,j,div_up,div_down);
    if (check_doublon==1){
      break;
    }
    num_up   += div_up;
    num_down += div_down;
  }
  
  if(check_doublon==0){
    tmp_num_up   = num_up;
    tmp_num_down = num_down;

    if(X->Def.iCalcModel==tJ){
      for(ia=0;ia<X->Check.sdim;ia++){
        i               = ia;
        num_up          = tmp_num_up;
        num_down        = tmp_num_down;
        check_doublon   = 0;
        for(j=0;j<X->Def.Nsite;j++){
          div_up    = i & X->Def.Tpow[2*j];
          div_up    = div_up/X->Def.Tpow[2*j];
          div_down  = i & X->Def.Tpow[2*j+1];
          div_down  = div_down/X->Def.Tpow[2*j+1];
          check_doublon = div_up*div_down;
          if (check_doublon==1){
            break;
          }
          num_up   += div_up;
          num_down += div_down;
        }
        if(check_doublon==0){
          if(num_up == X->Def.Nup && num_down == X->Def.Ndown ){
            list_1_[ja+jb]=ia+ib*ihfbit;
            list_2_1_[ia]=ja+1;
            list_2_2_[ib]=jb+1;
            ja+=1;
          } 
        }
      }
    }else if(X->Def.iCalcModel==tJNConserved){
      for(ia=0;ia<X->Check.sdim;ia++){
        i               =  ia;
        num_up          =  tmp_num_up;
        num_down        =  tmp_num_down;
        for(j=0;j<X->Def.Nsite;j++){
          div_up    = i & X->Def.Tpow[2*j];
          div_up    = div_up/X->Def.Tpow[2*j];
          div_down  = i & X->Def.Tpow[2*j+1];
          div_down  = div_down/X->Def.Tpow[2*j+1];
          check_doublon = div_up*div_down;
          if (check_doublon==1){
            break;
          }
          num_up   += div_up;
          num_down += div_down;
        }
        if(check_doublon==0){
          if( (num_up+num_down) == X->Def.Ne){
            list_1_[ja+jb]=ia+ib*ihfbit;
            list_2_1_[ia]=ja+1;
            list_2_2_[ib]=jb+1;
            ja+=1;
          } 
        }
      }  
    }else if(X->Def.iCalcModel==tJGC){
      for(ia=0;ia<X->Check.sdim;ia++){
        i               =  ia;
        num_up          =  tmp_num_up;
        num_down        =  tmp_num_down;
        for(j=0;j<X->Def.Nsite;j++){
          div_up        = i & X->Def.Tpow[2*j];
          div_up        = div_up/X->Def.Tpow[2*j];
          div_down      = i & X->Def.Tpow[2*j+1];
          div_down      = div_down/X->Def.Tpow[2*j+1];
          check_doublon = div_up*div_down;
          if (check_doublon==1){
            break;
          }
          num_up   += div_up;
          num_down += div_down;
        }
        if(check_doublon==0){
          list_1_[ja+jb]=ia+ib*ihfbit;
          list_2_1_[ia]=ja+1;
          list_2_2_[ib]=jb+1;
          ja+=1;
        }
      }  
    }
  }
  ja=ja-1;    
  return ja; 
}


/** 
 * @brief calculating restricted Hilbert space for Hubbard systems
 * 
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia) 
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[in] list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
int omp_sz(
                 long unsigned int ib,    //!<[in]
                 long unsigned int ihfbit, //!<[in]
                 struct BindStruct *X,     //!<[in]
                 long unsigned int *list_1_, //!<[out]
                 long unsigned int *list_2_1_,//!<[out]
                 long unsigned int *list_2_2_,//!<[out]
                 long unsigned int *list_jb_ //!<[in]
                 )
{
  long unsigned int i,j; 
  long unsigned int ia,ja,jb;
  long unsigned int div_down, div_up;
  long unsigned int num_up,num_down;
  long unsigned int tmp_num_up,tmp_num_down;
    
  jb = list_jb_[ib];
  i  = ib*ihfbit;
    
  num_up   = 0;
  num_down = 0;
  for(j=0;j< X->Def.Nsite ;j++){
    div_up    = i & X->Def.Tpow[2*j];
    div_up    = div_up/X->Def.Tpow[2*j];
    div_down  = i & X->Def.Tpow[2*j+1];
    div_down  = div_down/X->Def.Tpow[2*j+1];
    num_up += div_up;
    num_down += div_down;
  }
  
  ja=1;
  tmp_num_up   = num_up;
  tmp_num_down = num_down;

  if(X->Def.iCalcModel==Hubbard){
    for(ia=0;ia<X->Check.sdim;ia++){
      i=ia;
      num_up =  tmp_num_up;
      num_down =  tmp_num_down;
      
      for(j=0;j<X->Def.Nsite;j++){
        div_up    = i & X->Def.Tpow[2*j];
        div_up    = div_up/X->Def.Tpow[2*j];
        div_down  = i & X->Def.Tpow[2*j+1];
        div_down  = div_down/X->Def.Tpow[2*j+1];
        num_up += div_up;
        num_down += div_down;
      }
      if(num_up == X->Def.Nup && num_down == X->Def.Ndown){
        list_1_[ja+jb]=ia+ib*ihfbit;
        list_2_1_[ia]=ja+1;
        list_2_2_[ib]=jb+1;
        ja+=1;
      } 
    }
  }
  else if(X->Def.iCalcModel==HubbardNConserved){
    for(ia=0;ia<X->Check.sdim;ia++){
      i=ia;
      num_up =  tmp_num_up;
      num_down =  tmp_num_down;
      
      for(j=0;j<X->Def.Nsite;j++){
        div_up    = i & X->Def.Tpow[2*j];
        div_up    = div_up/X->Def.Tpow[2*j];
        div_down  = i & X->Def.Tpow[2*j+1];
        div_down  = div_down/X->Def.Tpow[2*j+1];
        num_up += div_up;
        num_down += div_down;
      }
      if( (num_up+num_down) == X->Def.Ne){
        list_1_[ja+jb]=ia+ib*ihfbit;
        list_2_1_[ia]=ja+1;
        list_2_2_[ib]=jb+1;
        ja+=1;
      } 
    }  
  }
  ja=ja-1;    
  return ja; 
}
/** 
 * @brief efficient version of calculating restricted Hilbert space for Hubbard systems  using snoob
 * details of snoob is found in S.H. Warren, Hacker's delight(Bs Delight, second ed., Addison-Wesley, ISBN: 0321842685, 2012.
 *
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia) 
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[in] list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 */
int omp_sz_hacker(long unsigned int ib,
                        long unsigned int ihfbit,
                        struct BindStruct *X,
                        long unsigned int *list_1_,
                        long unsigned int *list_2_1_,
                        long unsigned int *list_2_2_,
                        long unsigned int *list_jb_
                        )
{
  long unsigned int i,j; 
  long unsigned int ia,ja,jb;
  long unsigned int div_down, div_up;
  long unsigned int num_up,num_down;
  long unsigned int tmp_num_up,tmp_num_down;
    
  jb = list_jb_[ib];
  i  = ib*ihfbit;
    
  num_up   = 0;
  num_down = 0;
  for(j=0;j< X->Def.Nsite ;j++){
    div_up    = i & X->Def.Tpow[2*j];
    div_up    = div_up/X->Def.Tpow[2*j];
    div_down  = i & X->Def.Tpow[2*j+1];
    div_down  = div_down/X->Def.Tpow[2*j+1];
    num_up += div_up;
    num_down += div_down;
  }
  
  ja=1;
  tmp_num_up   = num_up;
  tmp_num_down = num_down;

  if(X->Def.iCalcModel==Hubbard){
    if(tmp_num_up <= X->Def.Nup && tmp_num_down <= X->Def.Ndown){ //do not exceed Nup and Ndown
      ia = X->Def.Tpow[X->Def.Nup+X->Def.Ndown-tmp_num_up-tmp_num_down]-1;
      if(ia < X->Check.sdim){
        num_up   =  tmp_num_up;
        num_down =  tmp_num_down;
        for(j=0;j<X->Def.Nsite;j++){
          div_up    = ia & X->Def.Tpow[2*j];
          div_up    = div_up/X->Def.Tpow[2*j];
          div_down  = ia & X->Def.Tpow[2*j+1];
          div_down  = div_down/X->Def.Tpow[2*j+1];
          num_up   += div_up;
          num_down += div_down;
        }
        if(num_up == X->Def.Nup && num_down == X->Def.Ndown){
          list_1_[ja+jb]=ia+ib*ihfbit;
          list_2_1_[ia]=ja+1;
          list_2_2_[ib]=jb+1;
          ja+=1;
        }
        if(ia!=0){
          ia = snoob(ia);
          while(ia < X->Check.sdim){
            num_up   =  tmp_num_up;
            num_down =  tmp_num_down;
            for(j=0;j<X->Def.Nsite;j++){
              div_up    = ia & X->Def.Tpow[2*j];
              div_up    = div_up/X->Def.Tpow[2*j];
              div_down  = ia & X->Def.Tpow[2*j+1];
              div_down  = div_down/X->Def.Tpow[2*j+1];
              num_up   += div_up;
              num_down += div_down;
            }
            if(num_up == X->Def.Nup && num_down == X->Def.Ndown){
              list_1_[ja+jb]=ia+ib*ihfbit;
              list_2_1_[ia]=ja+1;
              list_2_2_[ib]=jb+1;
              ja+=1;
            }
            ia = snoob(ia);
          }
        } 
      } 
    }
  }
  else if(X->Def.iCalcModel==HubbardNConserved){
    if(tmp_num_up+tmp_num_down <= X->Def.Ne){ //do not exceed Ne
      ia = X->Def.Tpow[X->Def.Ne-tmp_num_up-tmp_num_down]-1;
      if(ia < X->Check.sdim){
        list_1_[ja+jb]=ia+ib*ihfbit;
        list_2_1_[ia]=ja+1;
        list_2_2_[ib]=jb+1;
        ja+=1;
        if(ia!=0){
          ia = snoob(ia);
          while(ia < X->Check.sdim){
            list_1_[ja+jb]=ia+ib*ihfbit;
            list_2_1_[ia]=ja+1;
            list_2_2_[ib]=jb+1;
            ja+=1;
            ia = snoob(ia);
          }
        } 
      }  
    }
  }
  ja=ja-1;    
  return ja; 
}

/** 
 * @brief calculating restricted Hilbert space for KondoNConserved systems
 *
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia) 
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[in]  list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 */
int omp_sz_KondoNConserved(
    long unsigned int ib,        //[in]
    long unsigned int ihfbit,    //[in]
    struct BindStruct *X,        //[in]
    long unsigned int *list_1_,  //[out]
    long unsigned int *list_2_1_,//[out]
    long unsigned int *list_2_2_,//[out]
    long unsigned int *list_jb_  //[in]
)
{
    long unsigned int i,j; 
    long unsigned int ia,ja,jb;
    long unsigned int div_down, div_up;
    long unsigned int num_up,num_down;
    long unsigned int tmp_num_up,tmp_num_down;
    int icheck_loc;

  jb = list_jb_[ib];
  i  = ib*ihfbit;
  num_up     = 0;
  num_down   = 0;
  icheck_loc = 1;
  for(j=X->Def.Nsite/2; j< X->Def.Nsite ;j++){
    div_up    = i & X->Def.Tpow[2*j];
    div_up    = div_up/X->Def.Tpow[2*j];
    div_down  = i & X->Def.Tpow[2*j+1];
    div_down  = div_down/X->Def.Tpow[2*j+1];

    if(X->Def.LocSpn[j] == ITINERANT){
      num_up   += div_up;        
      num_down += div_down;  
    }else{    
      num_up   += div_up;        
      num_down += div_down;
      if(!(X->Def.Nsite%2==1 && j==(X->Def.Nsite/2))){
        icheck_loc   = icheck_loc*(div_up^div_down);// exclude doubllly ocupited site
      }
    }
  }

  ja=1;
  tmp_num_up   = num_up;
  tmp_num_down = num_down;
  if(icheck_loc ==1){
    for(ia=0;ia<X->Check.sdim;ia++){
      i        =  ia;
      num_up   =  tmp_num_up;
      num_down =  tmp_num_down;
      icheck_loc=1;
      for(j=0;j<(X->Def.Nsite+1)/2;j++){
        div_up    = i & X->Def.Tpow[2*j];
        div_up    = div_up/X->Def.Tpow[2*j];
        div_down  = i & X->Def.Tpow[2*j+1];
        div_down  = div_down/X->Def.Tpow[2*j+1];

        if(X->Def.LocSpn[j] ==  ITINERANT){
          num_up   += div_up;        
          num_down += div_down;  
        }else{    
          num_up   += div_up;        
          num_down += div_down;  
          if(!(X->Def.Nsite%2==1 && j==(X->Def.Nsite/2))){
            icheck_loc   = icheck_loc*(div_up^div_down);// exclude doubllly ocupited site
          }
        }
      }
      
      if(icheck_loc == 1 && X->Def.LocSpn[X->Def.Nsite/2] != ITINERANT && X->Def.Nsite%2==1){
        div_up    = ia & X->Def.Tpow[X->Def.Nsite-1];
        div_up    = div_up/X->Def.Tpow[X->Def.Nsite-1];
        div_down  = (ib*ihfbit) & X->Def.Tpow[X->Def.Nsite];
        div_down  = div_down/X->Def.Tpow[X->Def.Nsite];
        icheck_loc= icheck_loc*(div_up^div_down);
      }
      
      if(num_up+num_down == X->Def.Ne && icheck_loc==1){
        list_1_[ja+jb]=ia+ib*ihfbit;
        /*
        list_2_1_[ia]=ja;
        list_2_2_[ib]=jb;
         */
        list_2_1_[ia]=ja+1;
        list_2_2_[ib]=jb+1;
        //printf("DEBUG: rank=%d, list_1[%d]=%d, list_2_1_[%d]=%d, list_2_2_[%d]=%d\n", myrank, ja+jb, list_1_[ja+jb], ia, list_2_1[ia], ib, list_2_2[ib]);
        ja+=1;
      }
    }
  }
  //printf("XXX ib  %lu ja %lu\n",ib,ja);
  ja=ja-1;    
  return ja; 
}



/** 
 * @brief calculating restricted Hilbert space for Kondo systems
 *
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia) 
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[in] list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 */
int omp_sz_Kondo(
                       long unsigned int ib,        //[in]
                       long unsigned int ihfbit,    //[in]
                       struct BindStruct *X,        //[in]
                       long unsigned int *list_1_,  //[out]
                       long unsigned int *list_2_1_,//[out]
                       long unsigned int *list_2_2_,//[out]
                       long unsigned int *list_jb_  //[in]
                       )
{
  long unsigned int i,j; 
  long unsigned int ia,ja,jb;
  long unsigned int div_down, div_up;
  long unsigned int num_up,num_down;
  long unsigned int tmp_num_up,tmp_num_down;
  int icheck_loc;
    
  jb = list_jb_[ib];
  i  = ib*ihfbit;
    
  num_up   = 0;
  num_down = 0;
  icheck_loc=1;
  for(j=X->Def.Nsite/2; j< X->Def.Nsite ;j++){
    div_up    = i & X->Def.Tpow[2*j];
    div_up    = div_up/X->Def.Tpow[2*j];
    div_down  = i & X->Def.Tpow[2*j+1];
    div_down  = div_down/X->Def.Tpow[2*j+1];

    if(X->Def.LocSpn[j] == ITINERANT){
      num_up   += div_up;        
      num_down += div_down;  
    }else{    
      num_up   += div_up;        
      num_down += div_down;
      if(!(X->Def.Nsite%2==1 && j==(X->Def.Nsite/2))){
        icheck_loc   = icheck_loc*(div_up^div_down);// exclude doubllly ocupited site
      }
    }
  }
  
  ja=1;
  tmp_num_up   = num_up;
  tmp_num_down = num_down;
  if(icheck_loc ==1){
    for(ia=0;ia<X->Check.sdim;ia++){
      i=ia;
      num_up =  tmp_num_up;
      num_down =  tmp_num_down;
      icheck_loc=1;
      for(j=0;j<(X->Def.Nsite+1)/2;j++){
        div_up    = i & X->Def.Tpow[2*j];
        div_up    = div_up/X->Def.Tpow[2*j];
        div_down  = i & X->Def.Tpow[2*j+1];
        div_down  = div_down/X->Def.Tpow[2*j+1];

        if(X->Def.LocSpn[j] ==  ITINERANT){
          num_up   += div_up;        
          num_down += div_down;  
        }else{    
          num_up   += div_up;        
          num_down += div_down;  
          if(!(X->Def.Nsite%2==1 && j==(X->Def.Nsite/2))){
            icheck_loc   = icheck_loc*(div_up^div_down);// exclude doubllly ocupited site
          }
        }
      }
      
      if(icheck_loc == 1 && X->Def.LocSpn[X->Def.Nsite/2] != ITINERANT && X->Def.Nsite%2==1){
        div_up    = ia & X->Def.Tpow[X->Def.Nsite-1];
        div_up    = div_up/X->Def.Tpow[X->Def.Nsite-1];
        div_down  = (ib*ihfbit) & X->Def.Tpow[X->Def.Nsite];
        div_down  = div_down/X->Def.Tpow[X->Def.Nsite];
        icheck_loc= icheck_loc*(div_up^div_down);
      }
      
      if(num_up == X->Def.Nup && num_down == X->Def.Ndown && icheck_loc==1){
        list_1_[ja+jb]=ia+ib*ihfbit;
        /*
        list_2_1_[ia]=ja;
        list_2_2_[ib]=jb;
         */
        list_2_1_[ia]=ja+1;
        list_2_2_[ib]=jb+1;
        //printf("DEBUG: rank=%d, list_1[%d]=%d, list_2_1_[%d]=%d, list_2_2_[%d]=%d\n", myrank, ja+jb, list_1_[ja+jb], ia, list_2_1[ia], ib, list_2_2[ib]);
        ja+=1;
      }
    }
  }
  ja=ja-1;    
  return ja; 
}
/** 
 * @brief calculating restricted Hilbert space for Kondo-GC systems
 *
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia) 
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[in] list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 */
int omp_sz_Kondo_hacker(
                       long unsigned int ib,
                       long unsigned int ihfbit,
                       struct BindStruct *X,
                       long unsigned int *list_1_,
                       long unsigned int *list_2_1_,
                       long unsigned int *list_2_2_,
                       long unsigned int *list_jb_
                       )
{
  long unsigned int i,j; 
  long unsigned int ia,ja,jb;
  long unsigned int div_down, div_up;
  long unsigned int num_up,num_down;
  long unsigned int tmp_num_up,tmp_num_down;
  int icheck_loc;
    
  jb = list_jb_[ib];
  i  = ib*ihfbit;
    
  num_up   = 0;
  num_down = 0;
  icheck_loc=1;
  for(j=X->Def.Nsite/2; j< X->Def.Nsite ;j++){
    div_up    = i & X->Def.Tpow[2*j];
    div_up    = div_up/X->Def.Tpow[2*j];
    div_down  = i & X->Def.Tpow[2*j+1];
    div_down  = div_down/X->Def.Tpow[2*j+1];

    if(X->Def.LocSpn[j] == ITINERANT){
      num_up   += div_up;        
      num_down += div_down;  
    }else{    
      num_up   += div_up;        
      num_down += div_down;
      if(!(X->Def.Nsite%2==1 && j==(X->Def.Nsite/2))){
        icheck_loc   = icheck_loc*(div_up^div_down);// exclude doubly occupied site
      }
    }
  }
//[s] get ja  
  ja           = 1;
  tmp_num_up   = num_up;
  tmp_num_down = num_down;
  if(icheck_loc ==1){
    //for(ia=0;ia<X->Check.sdim;ia++){
    ia = X->Def.Tpow[X->Def.Nup+X->Def.Ndown-tmp_num_up-tmp_num_down]-1;
    //ia = 1;
    //if(ia < X->Check.sdim && ia!=0){
    //ia = snoob(ia);
    while(ia < X->Check.sdim && ia!=0){
    // for(ia=0;ia<X->Check.sdim;ia++){
        //[s] proceed ja
        i        = ia;
        num_up   =  tmp_num_up;
        num_down =  tmp_num_down;
        icheck_loc=1;
        for(j=0;j<(X->Def.Nsite+1)/2;j++){
          div_up    = i & X->Def.Tpow[2*j];
          div_up    = div_up/X->Def.Tpow[2*j];
          div_down  = i & X->Def.Tpow[2*j+1];
          div_down  = div_down/X->Def.Tpow[2*j+1];

          if(X->Def.LocSpn[j] ==  ITINERANT){
            num_up   += div_up;        
            num_down += div_down;  
          }else{    
            num_up   += div_up;        
            num_down += div_down;  
            if(!(X->Def.Nsite%2==1 && j==(X->Def.Nsite/2))){
              icheck_loc   = icheck_loc*(div_up^div_down);// exclude doubllly ocupited site
            }
          }
        }
        
        if(icheck_loc == 1 && X->Def.LocSpn[X->Def.Nsite/2] != ITINERANT && X->Def.Nsite%2==1){
          div_up    = ia & X->Def.Tpow[X->Def.Nsite-1];
          div_up    = div_up/X->Def.Tpow[X->Def.Nsite-1];
          div_down  = (ib*ihfbit) & X->Def.Tpow[X->Def.Nsite];
          div_down  = div_down/X->Def.Tpow[X->Def.Nsite];
          icheck_loc= icheck_loc*(div_up^div_down);
        }
        
        if(num_up == X->Def.Nup && num_down == X->Def.Ndown && icheck_loc==1){
          //printf("ia=%ud ja=%ud \n",ia,ja);
          list_1_[ja+jb]=ia+ib*ihfbit;
          list_2_1_[ia]=ja+1;
          list_2_2_[ib]=jb+1;
          ja+=1;
        }
        ia = snoob(ia);
        //[e] proceed ja
        //ia+=1;
      //}
    }
  }
//[e] get ja
  ja=ja-1;    
  return ja; 
}


/** 
 * 
 * 
 * @param ib 
 * @param ihfbit 
 * @param N2 
 * @param X 
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
int omp_sz_KondoGC(
                         long unsigned int ib,  //!<[in]
                         long unsigned int ihfbit,//!<[in]
                         struct BindStruct *X,    //!<[in]
                         long unsigned int *list_1_, //!<[out]
                         long unsigned int *list_2_1_,//!<[out]
                         long unsigned int *list_2_2_,//!<[out]
                         long unsigned int *list_jb_//!<[in]
                         )
{
  long unsigned int i,j; 
  long unsigned int ia,ja,jb;
  long unsigned int div_down, div_up;
  int icheck_loc;
    
  jb = list_jb_[ib];
  i  = ib*ihfbit;
  icheck_loc=1;
  for(j=X->Def.Nsite/2; j< X->Def.Nsite ;j++){
    div_up    = i & X->Def.Tpow[2*j];
    div_up    = div_up/X->Def.Tpow[2*j];
    div_down  = i & X->Def.Tpow[2*j+1];
    div_down  = div_down/X->Def.Tpow[2*j+1];
    if(X->Def.LocSpn[j] !=  ITINERANT){
      if(!(X->Def.Nsite%2==1 && j==(X->Def.Nsite/2))){
        icheck_loc   = icheck_loc*(div_up^div_down);// exclude doubllly ocupited site
      }
    }
  }

  ja=1;
  if(icheck_loc ==1){
    for(ia=0;ia<X->Check.sdim;ia++){
      i=ia;
      icheck_loc =1;
      for(j=0;j<(X->Def.Nsite+1)/2;j++){
        div_up    = i & X->Def.Tpow[2*j];
        div_up    = div_up/X->Def.Tpow[2*j];
        div_down  = i & X->Def.Tpow[2*j+1];
        div_down  = div_down/X->Def.Tpow[2*j+1];
        if(X->Def.LocSpn[j] !=  ITINERANT){
          if(!(X->Def.Nsite%2==1 && j==(X->Def.Nsite/2))){
            icheck_loc   = icheck_loc*(div_up^div_down);// exclude doubllly ocupited site
          }
        }
      }

      if(icheck_loc == 1 && X->Def.LocSpn[X->Def.Nsite/2] != ITINERANT && X->Def.Nsite%2==1){
        div_up    = ia & X->Def.Tpow[X->Def.Nsite-1];
        div_up    = div_up/X->Def.Tpow[X->Def.Nsite-1];
        div_down  = (ib*ihfbit) & X->Def.Tpow[X->Def.Nsite];
        div_down  = div_down/X->Def.Tpow[X->Def.Nsite];
        icheck_loc= icheck_loc*(div_up^div_down);
      }
      
      if(icheck_loc==1){
        list_1_[ja+jb]=ia+ib*ihfbit;
        list_2_1_[ia]=ja+1;
        list_2_2_[ib]=jb+1;
        ja+=1;
      }
    }
  }
  ja=ja-1;
    
  return ja; 
}

/** 
 * @brief calculating restricted Hilbert space for spin-1/2 systems
 *
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X          
 * @param[in] N ???
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia)
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[in] list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 */
int omp_sz_spin(
                      long unsigned int ib, 
                      long unsigned int ihfbit,
                      unsigned int N,
                      struct BindStruct *X,
                      long unsigned int *list_1_,
                      long unsigned int *list_2_1_,
                      long unsigned int *list_2_2_,
                      long unsigned int *list_jb_
                      )
{
  long unsigned int i,j,div; 
  long unsigned int ia,ja,jb;
  long unsigned int num_up;
  unsigned int tmp_num_up;
  
  jb = list_jb_[ib];
  i  = ib*ihfbit;
  num_up=0;
  for(j=0;j<N;j++){
    div=i & X->Def.Tpow[j];
    div=div/X->Def.Tpow[j];
    num_up+=div;
  }
  ja=1;
  tmp_num_up   = num_up;
  
  for(ia=0;ia<ihfbit;ia++){
    i=ia;
    num_up =  tmp_num_up;
    for(j=0;j<N;j++){
      div=i & X->Def.Tpow[j];
      div=div/X->Def.Tpow[j];
      num_up+=div;
    }

    if(num_up == X->Def.Ne){
      list_1_[ja+jb]=ia+ib*ihfbit;
      list_2_1_[ia]=ja+1;
      list_2_2_[ib]=jb+1;
      ja+=1;
    } 
  }
  ja=ja-1;
  return ja; 
}


/** 
 * @brief efficient version of calculating restricted Hilbert space for spin-1/2 systems 
 * details of snoob is found in S.H. Warren, Hacker's delight(Bs Delight, second ed., Addison-Wesley, ISBN: 0321842685, 2012.
 *
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X          
 * @param[in] N ???
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia)
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[in] list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 */
int omp_sz_spin_hacker(
                             long unsigned int ib, 
                             long unsigned int ihfbit,
                             unsigned int N,
                             struct BindStruct *X,
                             long unsigned int *list_1_,
                             long unsigned int *list_2_1_,
                             long unsigned int *list_2_2_,
                             long unsigned int *list_jb_
                             )
{
  long unsigned int i,j,div; 
  long unsigned int ia,ja,jb;
  long unsigned int num_up;
  unsigned int tmp_num_up;
  
  jb = list_jb_[ib];
  i  = ib*ihfbit;
  num_up=0;
  for(j=0;j<N;j++){
    div=i & X->Def.Tpow[j];
    div=div/X->Def.Tpow[j];
    num_up+=div;
  }
  ja=1;
  tmp_num_up   = num_up;
  
  // using hacker's delight
  if(tmp_num_up<=X->Def.Ne && (X->Def.Ne-tmp_num_up)<= X->Def.Nsite-1){ // do not exceed Ne
    ia = X->Def.Tpow[X->Def.Ne-tmp_num_up]-1;
    if(ia<ihfbit ){          // do not exceed Ne
      list_1_[ja+jb] = ia+ib*ihfbit;
      list_2_1_[ia]  = ja+1;
      list_2_2_[ib]  = jb+1;
      ja           += 1;

      if(ia!=0){
        ia = snoob(ia);
        while(ia < ihfbit){
          //fprintf(stdoutMPI, " X: ia= %ld ia=%ld \n", ia,ia);
          list_1_[ja+jb]    = ia+ib*ihfbit;
          list_2_1_[ia]     = ja+1;
          list_2_2_[ib]     = jb+1;
          ja+=1;
          ia = snoob(ia);
        }
      }
    }
  }
  ja=ja-1;
  return ja; 
}

/** 
 * @brief calculating restricted Hilbert space for general spin systems (S>1/2)
 *
 * @param[in] ib   upper half bit of i    
 * @param[in] ihfbit 2^(Ns/2) 
 * @param[in] X          
 * @param[out] list_1_    list_1_[icnt] = i : i is divided into ia and ib (i=ib*ihfbit+ia) 
 * @param[out] list_2_1_  list_2_1_[ib] = jb  
 * @param[out] list_2_2_  list_2_2_[ia] = ja  : icnt=jb+ja
 * @param[out] list_2_1_Sz_  
 * @param[out] list_2_2_Sz_  
 * @param[in] list_jb_   list_jb_[ib]  = jb  
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 */
int omp_sz_GeneralSpin(
                             long unsigned int ib, 
                             long unsigned int ihfbit,
                             struct BindStruct *X,
                             long unsigned int *list_1_,
                             long unsigned int *list_2_1_,
                             long unsigned int *list_2_2_,
                             long int *list_2_1_Sz_,
                             long int *list_2_2_Sz_,
                             long unsigned int *list_jb_
                             )
{
  long unsigned int ia,ja,jb;  
  int list_2_2_Sz_ib=0;
  int tmp_2Sz=0;
  jb = list_jb_[ib];
  list_2_2_Sz_ib =list_2_2_Sz_[ib];
  ja=1;
  for(ia=0;ia<ihfbit;ia++){
    tmp_2Sz=list_2_1_Sz_[ia]+list_2_2_Sz_ib;
    if(tmp_2Sz == X->Def.Total2Sz){
      list_1_[ja+jb]=ia+ib*ihfbit;
      list_2_1_[ia]=ja+1;
      list_2_2_[ib]=jb+1;
      ja+=1;
    } 
  }
  ja=ja-1;
  return ja; 
}

/** 
 * @brief reading the list of the restricted Hilbert space
 * 
 * @param[in] X 
 * @param[in] irght 
 * @param[in] ilft 
 * @param[in] ihfbit 
 * @param[in] i_max 
 * 
 * @return 
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
int Read_sz
(
 struct BindStruct *X,
 const long unsigned int irght,
 const long unsigned int ilft,
 const long unsigned int ihfbit,
 long unsigned int *i_max
 )
{
  FILE *fp,*fp_err;
  char sdt[D_FileNameMax];
  char buf[D_FileNameMax];
    
  long unsigned int icnt=0; 
  long unsigned int ia,ib;
  long unsigned int ja=0;
  long unsigned int jb=0;
  long unsigned int ibpatn=0;
  long unsigned int dam; 

  TimeKeeper(X,cFileNameSzTimeKeep,cReadSzStart, "a");
  TimeKeeper(X,cFileNameTimeKeep,cReadSzStart, "a");

  switch(X->Def.iCalcModel){
  case Hubbard:
  case HubbardGC:
  case Spin:
  case SpinGC:
    sprintf(sdt,cFileNameListModel, X->Def.Nsite, X->Def.Nup, X->Def.Ndown);
    break;
  case Kondo:
    sprintf(sdt,"ListForKondo_Ns%d_Ncond%d.dat",X->Def.Nsite,X->Def.Ne);
    break;
  case KondoNConserved:
    sprintf(sdt,"ListForKondo_Ns%d_Ncond%d.dat",X->Def.Nsite,X->Def.Ne);
    break;
  }
  if(childfopenMPI(sdt,"r", &fp)!=0){
    exitMPI(-1);
  }  

  if(fp == NULL){
    if(childfopenMPI(cFileNameErrorSz,"a",&fp_err)!=0){
      exitMPI(-1);
    }
    fprintf(fp_err, "%s", cErrSz_NoFile);
    fprintf(stderr, "%s", cErrSz_NoFile);
    fprintf(fp_err, cErrSz_NoFile_Show,sdt);
    fprintf(stderr, cErrSz_NoFile_Show, sdt);
    fclose(fp_err);
  }else{
    while(NULL != fgetsMPI(buf,sizeof(buf),fp)){  
      dam=atol(buf);  
      list_1[icnt]=dam;
            
      ia= dam & irght;
      ib= dam & ilft;
      ib=ib/ihfbit; 
            
      if(ib==ibpatn){
        ja=ja+1;
      }else{
        ibpatn=ib;
        ja=1;
        jb=icnt-1;
      }
            
      list_2_1[ia]=ja;
      list_2_2[ib]=jb;
      icnt+=1;
                
    }
    fclose(fp);
    *i_max=icnt-1;
  }

  TimeKeeper(X, cFileNameSzTimeKeep, cReadSzEnd, "a");
  TimeKeeper(X, cFileNameTimeKeep, cReadSzEnd, "a");

  return 0;
}

int count_localized_spins(struct BindStruct *X){
    int num_loc = 0;
    for (int j = X->Def.Nsite / 2; j < X->Def.Nsite; j++) {  /*counting # of localized spins*/
        if(X->Def.LocSpn[j] != ITINERANT) {/*ITINERANT ==0 -> itinerant*/
            num_loc += 1;
        }
    }
    return num_loc;
}

void calculate_jb_GeneralSpin(struct BindStruct *X, long unsigned int *list_jb, long int *list_2_1_Sz,long int *list_2_2_Sz, long unsigned int ihfbit,long unsigned int ilftdim,unsigned int N){
    unsigned int Max2Sz=0;
    unsigned int irghtsite=1;
    long unsigned int itmpSize=1;
    int i2Sz=0;
    long unsigned int *HilbertNumToSz;
    long unsigned int ib,jb,j;

    for(j=0; j<X->Def.Nsite; j++){
        itmpSize *= X->Def.SiteToBit[j];
        if(itmpSize==ihfbit){
            break;
        }
        irghtsite++;
    }
    for(j=0; j<X->Def.Nsite; j++){
        Max2Sz += X->Def.LocSpn[j];
    }
            
    HilbertNumToSz = lui_1d_allocate(2*Max2Sz+1);
    for(ib=0; ib<2*Max2Sz+1; ib++){
        HilbertNumToSz[ib]=0;
    }
            
    for(ib =0; ib<ihfbit; ib++){
        i2Sz=0;
        for(j=1; j<= irghtsite; j++){
            i2Sz += GetLocal2Sz(j,ib, X->Def.SiteToBit, X->Def.Tpow);
        }
        list_2_1_Sz[ib]=i2Sz;
        HilbertNumToSz[i2Sz+Max2Sz]++;
    }
    jb = 0;
    for(ib=0;ib<ilftdim;ib++){
        list_jb[ib]=jb;
        i2Sz=0;
        for(j=1;j<=(N-irghtsite); j++){
            i2Sz += GetLocal2Sz(j+irghtsite,ib*ihfbit, X->Def.SiteToBit, X->Def.Tpow);
        }
        list_2_2_Sz[ib]=i2Sz;
        if((X->Def.Total2Sz- i2Sz +(int)Max2Sz)>=0 && (X->Def.Total2Sz- i2Sz) <= (int)Max2Sz){
            jb += HilbertNumToSz[X->Def.Total2Sz- i2Sz +Max2Sz];
        }
    }
    free_lui_1d_allocate(HilbertNumToSz);
}

void calculate_jb_Spin_m1(struct BindStruct *X, long unsigned int *list_jb, long unsigned int *list_1_, long unsigned int *list_2_1_,long unsigned int *list_2_2_,\
long unsigned int ihfbit,long unsigned int irght,long unsigned int ilft,long unsigned int ibpatn, unsigned int N){
    long unsigned int ia, ja, ib, jb;
    long unsigned int tmp_pow, tmp_i, tmp_j, icnt, max_tmp_i;

    icnt    = 1;
    tmp_pow = 1;
    tmp_i   = 0;
    jb      = 0;
    ja      = 0;
    while(tmp_pow < X->Def.Tpow[X->Def.Ne]){
        tmp_i   += tmp_pow;
        tmp_pow  = tmp_pow*2;
    }
    if(X->Def.Nsite%2==0){
        max_tmp_i = X->Check.sdim*X->Check.sdim;
    }else{
        max_tmp_i = X->Check.sdim*X->Check.sdim*2-1;
    }  
    while(tmp_i<max_tmp_i){
        list_1_[icnt]=tmp_i;
                  
        ia= tmp_i & irght;
        ib= tmp_i & ilft;
        ib= ib/ihfbit; 
        if(ib==ibpatn){
            ja=ja+1;
        }else{
            ibpatn = ib;
            ja     = 1;
            jb     = icnt-1;
        }
                  
        list_2_1_[ia] = ja+1;
        list_2_2_[ib] = jb+1;
        tmp_j         = snoob(tmp_i);
        tmp_i         = tmp_j;
        icnt         += 1;
    }
    icnt = icnt-1;
    // old version + hacker's delight
}

void calculate_jb_Spin_Hacker(struct BindStruct *X, long unsigned int *list_jb, long unsigned int ihfbit,unsigned int N){
    long unsigned int ib,jb,div_up,i,j,tmp_1;
    int num_up,all_up;
    long int **comb;
    comb = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);
    jb   = 0;
    for(ib=0;ib<X->Check.sdim;ib++){
        list_jb[ib] = jb;
        i           = ib*ihfbit;
        num_up      = 0;
        for(j=0;j<N; j++){
            div_up =  i & X->Def.Tpow[j];
            div_up =  div_up/X->Def.Tpow[j];
            num_up += div_up;
        }
        all_up = (X->Def.Nsite+1)/2;
        tmp_1  = Binomial(all_up,X->Def.Ne-num_up,comb,all_up);
        jb    += tmp_1;
    }
    free_li_2d_allocate(comb);
}

void calculate_jb_Spin_Old(struct BindStruct *X, long unsigned int *list_jb, long unsigned int ihfbit,unsigned int N){
    long unsigned int ib,jb,div_up,i,j,tmp_1;
    int num_up,all_up;
    long int **comb;
    comb = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);
    jb   = 0;
    for(ib=0;ib<X->Check.sdim;ib++){
        list_jb[ib] = jb;
        i           = ib*ihfbit;
        num_up=0;
        for(j=0;j<N; j++){
            div_up  = i & X->Def.Tpow[j];
            div_up  = div_up/X->Def.Tpow[j];
            num_up += div_up;
        }
        all_up   = (X->Def.Nsite+1)/2;
        tmp_1    = Binomial(all_up,X->Def.Ne-num_up,comb,all_up);
        jb      += tmp_1;
    }
    free_li_2d_allocate(comb);
}

static void GetKondoSiteOccupations(const struct BindStruct *X,
                                    const long unsigned int state,
                                    const long unsigned int site,
                                    long unsigned int *div_up,
                                    long unsigned int *div_down) {
    *div_up = state & X->Def.Tpow[2 * site];
    *div_up = *div_up / X->Def.Tpow[2 * site];
    *div_down = state & X->Def.Tpow[2 * site + 1];
    *div_down = *div_down / X->Def.Tpow[2 * site + 1];
}

static int CountKondoRightHalfOccupations(const struct BindStruct *X,
                                          const long unsigned int state,
                                          int *num_up,
                                          int *num_down) {
    long unsigned int j;
    long unsigned int div_up;
    long unsigned int div_down;
    int icheck_loc = 1;

    *num_up = 0;
    *num_down = 0;
    for (j = X->Def.Nsite / 2; j < X->Def.Nsite; j++) {
        GetKondoSiteOccupations(X, state, j, &div_up, &div_down);
        if (X->Def.LocSpn[j] == ITINERANT) {
            *num_up += (int)div_up;
            *num_down += (int)div_down;
            continue;
        }

        *num_up += (int)div_up;
        *num_down += (int)div_down;
        if (X->Def.Nsite % 2 == 1 && j == (X->Def.Nsite / 2)) {
            if (div_down == 0) {
                *num_up += 1;
            }
        } else {
            icheck_loc *= (int)(div_up ^ div_down); /* exclude empty or doubly occupied site */
        }
    }
    return icheck_loc;
}

static void GetKondoHalfBitCombinationWindow(const struct BindStruct *X,
                                             const int num_loc,
                                             int *all_loc,
                                             int *all_up,
                                             int *all_down) {
    int tmp_res = X->Def.Nsite % 2;

    *all_loc = X->Def.NLocSpn - num_loc;
    *all_up = (X->Def.Nsite + tmp_res) / 2 - *all_loc;
    *all_down = (X->Def.Nsite - tmp_res) / 2 - *all_loc;
    if (X->Def.Nsite % 2 == 1 && X->Def.LocSpn[X->Def.Nsite / 2] != ITINERANT) {
        *all_up = X->Def.Nsite / 2 - *all_loc;
        *all_down = X->Def.Nsite / 2 - *all_loc;
    }
}

static long unsigned int ComputeKondoFixedParticleContribution(const struct BindStruct *X,
                                                               const int num_up,
                                                               const int num_down,
                                                               const int all_loc,
                                                               const int all_up,
                                                               const int all_down,
                                                               long int **comb) {
    int num_loc_up;
    long unsigned int tmp_1;
    long unsigned int tmp_2;
    long unsigned int tmp_3;
    long unsigned int contribution = 0;

    for (num_loc_up = 0; num_loc_up <= all_loc; num_loc_up++) {
        tmp_1 = Binomial(all_loc, num_loc_up, comb, all_loc);
        tmp_2 = Binomial(all_up, X->Def.Nup - num_up - num_loc_up, comb, all_up);
        tmp_3 = Binomial(all_down, X->Def.Ndown - num_down - (all_loc - num_loc_up), comb, all_down);
        contribution += tmp_1 * tmp_2 * tmp_3;
    }
    return contribution;
}

static int HasValidKondoGCLocalizedConfiguration(const struct BindStruct *X,
                                                 const long unsigned int state) {
    long unsigned int j;
    long unsigned int div_up;
    long unsigned int div_down;
    int icheck_loc = 1;

    for (j = (X->Def.Nsite + 1) / 2; j < X->Def.Nsite; j++) {
        GetKondoSiteOccupations(X, state, j, &div_up, &div_down);
        if (X->Def.LocSpn[j] != ITINERANT) {
            if (!(X->Def.Nsite % 2 == 1 && j == (X->Def.Nsite / 2))) {
                icheck_loc *= (int)(div_up ^ div_down); /* exclude doubly occupied site */
            }
        }
    }
    return icheck_loc;
}

void calculate_jb_Kondo(struct BindStruct *X, long unsigned int *list_jb, long unsigned int ihfbit){
    long unsigned int jb = 0;
    long unsigned int i;
    long unsigned int ib;
    int num_up;
    int num_down;
    int num_loc;
    int all_loc;
    int all_up;
    int all_down;
    long int **comb;

    comb = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);
    fprintf(stdoutMPI, cStateNupNdown, X->Def.Nup, X->Def.Ndown);

    num_loc = count_localized_spins(X);
    GetKondoHalfBitCombinationWindow(X, num_loc, &all_loc, &all_up, &all_down);
    for(ib=0;ib<X->Check.sdim;ib++){ //sdim = 2^(N/2)
        list_jb[ib] = jb;
        i           = ib*ihfbit; // ihfbit=pow(2,((Nsite+1)/2))

        if (CountKondoRightHalfOccupations(X, i, &num_up, &num_down) == 1) {
            jb += ComputeKondoFixedParticleContribution(X, num_up, num_down, all_loc, all_up, all_down, comb);
        }
    }
    free_li_2d_allocate(comb);
}

void calculate_jb_KondoNConserved(struct BindStruct *X, long unsigned int *list_jb, long unsigned int ihfbit){
    long unsigned int jb = 0;
    long unsigned int i;
    long unsigned int ib;
    int num_up;
    int num_down;
    int all_loc;
    int num_loc;

    num_loc = count_localized_spins(X);
    all_loc = X->Def.NLocSpn-num_loc;
    for(ib=0;ib<X->Check.sdim;ib++){ //sdim = 2^(N/2)
        list_jb[ib] = jb;
        i           = ib*ihfbit; // ihfbit=pow(2,((Nsite+1)/2))

        if (CountKondoRightHalfOccupations(X, i, &num_up, &num_down) == 1) {
            if (num_up + num_down == X->Def.Ne - all_loc) {
                jb += X->Def.Tpow[all_loc];
            }
        }
    }
}

void calculate_jb_KondoGC(struct BindStruct *X, int num_loc, long unsigned int *list_jb, long unsigned int ihfbit){
    long unsigned int jb = 0;
    long unsigned int gc_increment;

    if(X->Def.Nsite%2==1 && X->Def.LocSpn[X->Def.Nsite/2] != ITINERANT){
        gc_increment = X->Def.Tpow[X->Def.Nsite-1-(X->Def.NLocSpn-num_loc)];
    }else{
        gc_increment = X->Def.Tpow[X->Def.Nsite-(X->Def.NLocSpn-num_loc)];
    }

    for(long unsigned int ib=0;ib < X->Check.sdim;ib++){
        long unsigned int i = ib*ihfbit;
        list_jb[ib]         = jb;

        if(HasValidKondoGCLocalizedConfiguration(X, i) == 1){
            jb += gc_increment;
        }
    }
}

static void GetHalfBitSiteCounts(const struct BindStruct *X, int *all_up, int *all_down) {
    int tmp_res = X->Def.Nsite % 2;
    *all_up = (X->Def.Nsite + tmp_res) / 2;
    *all_down = (X->Def.Nsite - tmp_res) / 2;
}

static void CountHalfBitSpinOccupations(const struct BindStruct *X,
                                        const unsigned int N2,
                                        const long unsigned int state,
                                        int *num_up,
                                        int *num_down) {
    long unsigned int div;
    long unsigned int j;

    *num_up = 0;
    for (j = 0; j <= N2 - 2; j += 2) {
        div = state & X->Def.Tpow[j];
        div = div / X->Def.Tpow[j];
        *num_up += (int)div;
    }

    *num_down = 0;
    for (j = 1; j <= N2 - 1; j += 2) {
        div = state & X->Def.Tpow[j];
        div = div / X->Def.Tpow[j];
        *num_down += (int)div;
    }
}

static long unsigned int ComputeHubbardFixedParticleContribution(const struct BindStruct *X,
                                                                 const int num_up,
                                                                 const int num_down,
                                                                 long int **comb,
                                                                 const int all_up,
                                                                 const int all_down) {
    long unsigned int tmp_1;
    long unsigned int tmp_2;

    tmp_1 = Binomial(all_up, X->Def.Nup - num_up, comb, all_up);
    tmp_2 = Binomial(all_down, X->Def.Ndown - num_down, comb, all_down);
    return tmp_1 * tmp_2;
}

static long unsigned int ComputeHubbardNConservedContribution(const struct BindStruct *X,
                                                              const int num_up,
                                                              const int num_down,
                                                              long int **comb,
                                                              const int all_up,
                                                              const int all_down,
                                                              const int iMinup,
                                                              const int iAllup) {
    int iSpnup;
    long unsigned int tmp_1;
    long unsigned int tmp_2;
    long unsigned int contribution = 0;

    for (iSpnup = iMinup; iSpnup <= iAllup; iSpnup++) {
        tmp_1 = Binomial(all_up, iSpnup - num_up, comb, all_up);
        tmp_2 = Binomial(all_down, X->Def.Ne - iSpnup - num_down, comb, all_down);
        contribution += tmp_1 * tmp_2;
    }
    return contribution;
}


void calculate_jb_Hubbard(struct BindStruct *X,long unsigned int *list_jb, long unsigned int ihfbit, unsigned int N2){
    /*[s] this part can not be parallelized*/
    long unsigned int jb = 0, i;
    long int **comb;
    int num_up, num_down;
    int all_up, all_down;

    GetHalfBitSiteCounts(X, &all_up, &all_down);
    comb = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);
    for(long unsigned ib=0;ib<X->Check.sdim;ib++){ // sdim = 2^(N/2)
        list_jb[ib] = jb;
        i           = ib*ihfbit;

        CountHalfBitSpinOccupations(X, N2, i, &num_up, &num_down);
        jb += ComputeHubbardFixedParticleContribution(X, num_up, num_down, comb, all_up, all_down);
    }
    free_li_2d_allocate(comb);
    /*[e] this part can not be parallelized*/
}

void calculate_jb_Hubbard_Hacker(struct BindStruct *X,long unsigned int *list_jb, long unsigned int ihfbit, unsigned int N2){
    long unsigned int sdim_div,sdim_rest,ib_start,ib_end;
    long unsigned int i,ib,j,jb;
    int  mythread,num_up,num_down,all_up,all_down;
    long int **comb2;
    long unsigned int *jbthread;

    GetHalfBitSiteCounts(X, &all_up, &all_down);
    jbthread = lui_1d_allocate(nthreads);
    #pragma omp parallel default(none) \
    shared(X,list_jb,ihfbit,N2,nthreads,jbthread,all_up,all_down) \
    private(ib,i,j,num_up,num_down,jb, \
    comb2,mythread,sdim_div,sdim_rest,ib_start,ib_end)
    {
        jb = 0;
        #ifdef _OPENMP
            mythread = omp_get_thread_num();
        #else
            mythread = 0;
        #endif
        comb2 = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);
        /* explict loop decomposition is nessesary to fix the asignment to each thread*/
        sdim_div  = X->Check.sdim / nthreads;
        sdim_rest = X->Check.sdim % nthreads;
        if(mythread < sdim_rest){
            ib_start = sdim_div*mythread + mythread;
            ib_end = ib_start + sdim_div + 1;
        }else{
            ib_start = sdim_div*mythread + sdim_rest;
            ib_end = ib_start + sdim_div;
        }
        for(ib=ib_start;ib<ib_end;ib++){
            list_jb[ib] = jb;
            i           = ib*ihfbit;

            CountHalfBitSpinOccupations(X, N2, i, &num_up, &num_down);
            jb += ComputeHubbardFixedParticleContribution(X, num_up, num_down, comb2, all_up, all_down);
        }
        free_li_2d_allocate(comb2);
        if(mythread != nthreads-1) jbthread[mythread+1] = jb;
        #pragma omp barrier
        #pragma omp single
        {
            jbthread[0] = 0;
            for(j=1;j<nthreads;j++){
                jbthread[j] += jbthread[j-1];
            }
        }
        for(ib = ib_start;ib < ib_end;ib++){
            list_jb[ib] += jbthread[mythread];
        }
    }
    free_lui_1d_allocate(jbthread);
}

void calculate_jb_HubbardNCoserved(struct BindStruct *X,long unsigned int *list_jb, long unsigned int ihfbit, unsigned int N2){
    long unsigned int i,ib,jb;
    int num_up,num_down,all_up,all_down;
    long int **comb;
    int iMinup,iAllup;

    comb = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);
    // this part can not be parallelized
    jb     = 0;
    iMinup = 0;
    iAllup = X->Def.Ne;
    if(X->Def.Ne > X->Def.Nsite){
        iMinup = X->Def.Ne-X->Def.Nsite;
        iAllup = X->Def.Nsite;
    }
    GetHalfBitSiteCounts(X, &all_up, &all_down);
    for(ib=0;ib<X->Check.sdim;ib++){
        list_jb[ib] = jb;
        i           = ib*ihfbit;

        CountHalfBitSpinOccupations(X, N2, i, &num_up, &num_down);
        jb += ComputeHubbardNConservedContribution(X, num_up, num_down, comb, all_up, all_down, iMinup, iAllup);
    }
    free_li_2d_allocate(comb);
}

void calculate_jb_HubbardNCoserved_Hacker(struct BindStruct *X,long unsigned int *list_jb, long unsigned int ihfbit, unsigned int N2){
    long unsigned int sdim_div,sdim_rest,ib_start,ib_end;
    long unsigned int i,ib,j,jb;
    int  mythread,num_up,num_down,all_up,all_down;
    long int **comb2;
    long unsigned int *jbthread;
    int iMinup,iAllup;

    iMinup = 0;
    iAllup = X->Def.Ne;
    if(X->Def.Ne > X->Def.Nsite){
        iMinup = X->Def.Ne-X->Def.Nsite;
        iAllup = X->Def.Nsite;
    }
    GetHalfBitSiteCounts(X, &all_up, &all_down);
    jbthread = lui_1d_allocate(nthreads);
    #pragma omp parallel default(none) \
    shared(X,iMinup,iAllup,all_up,all_down,list_jb,ihfbit,N2,nthreads,jbthread) \
    private(ib,i,j,num_up,num_down,jb,comb2, \
    mythread,sdim_rest,sdim_div,ib_start,ib_end)
    {
        jb = 0;
    #ifdef _OPENMP
        mythread = omp_get_thread_num();
    #else
        mythread = 0;
    #endif
        comb2 = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);
        /* explict loop decomposition is nessesary to fix the asignment to each thread*/
        sdim_div  = X->Check.sdim / nthreads;
        sdim_rest = X->Check.sdim % nthreads;
        if(mythread < sdim_rest){
            ib_start = sdim_div*mythread + mythread;
            ib_end = ib_start + sdim_div + 1;
        }else{
            ib_start = sdim_div*mythread + sdim_rest;
            ib_end = ib_start + sdim_div;
        }
        for(ib=ib_start;ib<ib_end;ib++){
            list_jb[ib] = jb;
            i       = ib*ihfbit;

            CountHalfBitSpinOccupations(X, N2, i, &num_up, &num_down);
            jb += ComputeHubbardNConservedContribution(X, num_up, num_down, comb2, all_up, all_down, iMinup, iAllup);
        }
        free_li_2d_allocate(comb2);
        if(mythread != nthreads-1) jbthread[mythread+1] = jb;
        #pragma omp barrier
        #pragma omp single
        {
            jbthread[0] = 0;
            for(j=1;j<nthreads;j++){
                jbthread[j] += jbthread[j-1];
            }
        }
        for(ib=ib_start;ib<ib_end;ib++){
            list_jb[ib] += jbthread[mythread];
        }
    }//omp parallel
    free_lui_1d_allocate(jbthread);
}

static int CountTJHalfBitOccupations(const struct BindStruct *X,
                                     const unsigned int N2,
                                     const long unsigned int state,
                                     int *num_up,
                                     int *num_down) {
    long unsigned int j;
    long unsigned int div_up;
    long unsigned int div_down;
    int check_doublon = 0;

    *num_up = 0;
    *num_down = 0;
    for (j = 0; j < (N2 / 2); j++) {
        div_up = state & X->Def.Tpow[2 * j];
        div_up = div_up / X->Def.Tpow[2 * j];
        div_down = state & X->Def.Tpow[2 * j + 1];
        div_down = div_down / X->Def.Tpow[2 * j + 1];
        check_doublon = div_up * div_down;
        if (check_doublon == 1) {
            break;
        }
        *num_up += (int)div_up;
        *num_down += (int)div_down;
    }
    return check_doublon;
}

void calculate_jb_tJ(struct BindStruct *X,long unsigned int *list_jb, long unsigned int ihfbit, unsigned int N2){
    /*[s] this part can not be parallelized*/
    long unsigned int jb = 0, i, tmp_1, tmp_2;
    long int **comb;
    int num_up, num_down, check_doublon;
    int all_up, all_down;
    comb          = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);

    for(long unsigned ib=0;ib<X->Check.sdim;ib++){ // sdim = 2^(N/2)
        list_jb[ib]   = jb;
        i             = ib*ihfbit;
        check_doublon = CountTJHalfBitOccupations(X, N2, i, &num_up, &num_down);
          
        /* eg of even sites: 4site-> |DU|DU || |DU|DU|*/
        /* eg of odd  sites: 3site-> |DU|D  ||  |U|DU|*/
        /* all_up   -> # of up   sites in the lower half of bits*/
        /* all_down -> # of down sites in the lower half of bits*/
        if (check_doublon==0){
            GetHalfBitSiteCounts(X, &all_up, &all_down);
            tmp_1    = Binomial(all_up,X->Def.Nup-num_up,comb,all_up);
            tmp_2    = Binomial(all_down-(X->Def.Nup-num_up),X->Def.Ndown-num_down,comb,all_down);/* tJ all_down-(X->Def.Nup-num_up)*/
            jb       += tmp_1*tmp_2;
            //printf("DBB jb=%ld %ld %ld: num_up %d num_down %d : %d %d\n",jb,tmp_1,tmp_2,num_up,num_down,(X->Def.Nup-num_up),X->Def.Ndown-num_down);
        }
    }
    free_li_2d_allocate(comb);
    /*[e] this part can not be parallelized*/
}

void calculate_jb_tJNConserved(struct BindStruct *X,long unsigned int *list_jb, long unsigned int ihfbit, unsigned int N2){
    /*[s] this part can not be parallelized*/
    long unsigned int jb = 0, i, tmp_1, tmp_2;
    long int **comb;
    int num_up, num_down, check_doublon;
    int all_up, all_down;
    int iSpnup, iMinup,iAllup;

    comb          = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);
    iMinup        = 0;
    iAllup        = X->Def.Ne;
    if(X->Def.Ne > X->Def.Nsite){
        iMinup = X->Def.Ne-X->Def.Nsite;
        iAllup = X->Def.Nsite;
    }

    for(long unsigned ib=0;ib<X->Check.sdim;ib++){ // sdim = 2^(N/2)
        list_jb[ib]   = jb;
        i             = ib*ihfbit;
        check_doublon = CountTJHalfBitOccupations(X, N2, i, &num_up, &num_down);
          
        /* eg of even sites: 4site-> |DU|DU || |DU|DU|*/
        /* eg of odd  sites: 3site-> |DU|D  ||  |U|DU|*/
        /* all_up   -> # of up   sites in the lower half of bits*/
        /* all_down -> # of down sites in the lower half of bits*/
        if (check_doublon==0){
            GetHalfBitSiteCounts(X, &all_up, &all_down);

            for(iSpnup=iMinup; iSpnup<= iAllup; iSpnup++){
                tmp_1    = Binomial(all_up,iSpnup-num_up,comb,all_up);
                tmp_2    = Binomial(all_down-(iSpnup-num_up),X->Def.Ne-(iSpnup+num_down),comb,all_down);/* tJ all_down-(iSpnup-num_up)*/
                jb      += tmp_1*tmp_2;
            }
        }
    }
    free_li_2d_allocate(comb);
    /*[e] this part can not be parallelized*/
}

void calculate_jb_tJGC(struct BindStruct *X,long unsigned int *list_jb, long unsigned int ihfbit, unsigned int N2){
    /*[s] this part can not be parallelized*/
    long unsigned int jb = 0, i, tmp_1, tmp_2;
    long int **comb;
    int num_up, num_down, check_doublon;
    int all_up, all_down;
    int iSpnup,iSpndown;

    comb          = li_2d_allocate(X->Def.Nsite+1,X->Def.Nsite+1);

    for(long unsigned ib=0;ib<X->Check.sdim;ib++){ // sdim = 2^(N/2)
        list_jb[ib]   = jb;
        i             = ib*ihfbit;
        check_doublon = CountTJHalfBitOccupations(X, N2, i, &num_up, &num_down);
        (void)num_up;
        (void)num_down;
          
        /* eg of even sites: 4site-> |DU|DU || |DU|DU|*/
        /* eg of odd  sites: 3site-> |DU|D  ||  |U|DU|*/
        /* all_up   -> # of up   sites in the lower half of bits*/
        /* all_down -> # of down sites in the lower half of bits*/
        if (check_doublon==0){
            GetHalfBitSiteCounts(X, &all_up, &all_down);
            for(iSpnup=0; iSpnup<= all_up; iSpnup++){
                tmp_1   = Binomial(all_up,iSpnup,comb,all_up);
                for(iSpndown=0; iSpndown<= all_down; iSpndown++){
                    tmp_2   = Binomial(all_down-iSpnup,iSpndown,comb,all_down-iSpnup);
                    jb     += tmp_1*tmp_2;
                }
            }
        }
    }
    free_li_2d_allocate(comb);
    /*[e] this part can not be parallelized*/
}
