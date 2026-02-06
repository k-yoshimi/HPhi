/* HPhi  -  Quantum Lattice Model Simulator */
/* Copyright (C) 2015 The University of Tokyo */

#include "Common.h"
#include "readdef.h"
#include "readdef_idx_parser.h"
#include "LogMessage.h"
#include "wrapperMPI.h"

int ReadDefFileError(const char *defname);
int CheckTETransferHermite(struct DefineList *X, const int NTETrans, const int idx);
int CheckInterAllCondition(
        int iCalcModel,
        int Nsite,
        int iFlgGeneralSpin,
        int *iLocSpin,
        int isite1, int isigma1,
        int isite2, int isigma2,
        int isite3, int isigma3,
        int isite4, int isigma4
);
int InputInterAllInfo(
        int *icnt_interall,
        int **iInterAllInfo,
        double complex *cInterAllValue,
        int isite1, int isigma1,
        int isite2, int isigma2,
        int isite3, int isigma3,
        int isite4, int isigma4,
        double re_value, double im_value
);

static int ParseIdxLocSpinDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  int xitmp[2];
  char ctmp2[256];

  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->Nsite) {
      return ReadDefFileError(defname);
    }

    sscanf(ctmp2, "%d %d\n", &(xitmp[0]), &(xitmp[1]));
    X->LocSpn[xitmp[0]] = xitmp[1];
    X->SiteToBit[xitmp[0]] = (X->LocSpn[xitmp[0]] + 1); /* 2S+1 */
    if (CheckSite(xitmp[0], X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }
  if (CheckLocSpin(X) == FALSE) {
    return ReadDefFileError(defname);
  }
  return 0;
}

static int ParseIdxTransferDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int i;
  unsigned int idx = 0;
  int icnt_trans = 0;
  int iflg_trans = 0;
  int iboolLoc = 0;
  int isite1, isite2;
  int isigma1, isigma2;
  double dvalue_re, dvalue_im;
  char ctmp2[256];

  if (X->NTransfer <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NTransfer) {
      return ReadDefFileError(defname);
    }

    sscanf(ctmp2, "%d %d %d %d %lf %lf\n",
           &isite1,
           &isigma1,
           &isite2,
           &isigma2,
           &dvalue_re,
           &dvalue_im);

    if (CheckPairSite(isite1, isite2, X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }

    if (isite1 == isite2 && isigma1 == isigma2) {
      if (fabs(dvalue_im) > eps_CheckImag0) {
        fprintf(stdoutMPI, cErrNonHermiteTrans, isite1, isigma1, isite2, isigma2, dvalue_re, dvalue_im);
        return ReadDefFileError(defname);
      }
    }

    if (X->iCalcModel == Spin) {
      if (isite1 != isite2) {
        iboolLoc = 1;
        fprintf(stdoutMPI, cWarningIncorrectFormatForSpin2, isite1, isite2);
      }
    } else if (X->iCalcModel == Kondo) {
      if (X->LocSpn[isite1] != ITINERANT || X->LocSpn[isite2] != ITINERANT) {
        if (isite1 != isite2) {
          iboolLoc = 1;
          fprintf(stdoutMPI, cErrIncorrectFormatForKondoTrans, isite1, isite2);
        }
      }
    } else if (X->iCalcModel == SpinlessFermion || X->iCalcModel == SpinlessFermionGC) {
      if (isigma1 != 0 || isigma2 != 0) {
        fprintf(stderr, cErrNonHermiteTrans, isite1, isigma1, isite2, isigma2, dvalue_re, dvalue_im);
        return ReadDefFileError(defname);
      }
    }

    iflg_trans = 0;
    for (i = 0; i < (unsigned int)icnt_trans; i++) {
      if (isite1 == X->GeneralTransfer[i][0] && isite2 == X->GeneralTransfer[i][2] &&
          isigma1 == X->GeneralTransfer[i][1] && isigma2 == X->GeneralTransfer[i][3]) {
        X->ParaGeneralTransfer[i] += dvalue_re + dvalue_im * I;
        iflg_trans = 1;
        continue;
      }
    }

    if (iflg_trans == 0) {
      X->GeneralTransfer[icnt_trans][0] = isite1;
      X->GeneralTransfer[icnt_trans][1] = isigma1;
      X->GeneralTransfer[icnt_trans][2] = isite2;
      X->GeneralTransfer[icnt_trans][3] = isigma2;
      X->ParaGeneralTransfer[icnt_trans] = dvalue_re + dvalue_im * I;
      icnt_trans++;
    }
    idx++;
  }

  if (iboolLoc == 1) {
    return -1;
  }
  X->NTransfer = icnt_trans;

  if (CheckSpinIndexForTrans(X) == FALSE) {
    return -1;
  }
  if (CheckTransferHermite(X) != 0) {
    fprintf(stdoutMPI, "%s", cErrNonHermiteTransForAll);
    return -1;
  }
  return 0;
}

static int ParseIdxCoulombIntraDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  char ctmp2[256];

  if (X->NCoulombIntra <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NCoulombIntra) {
      return ReadDefFileError(defname);
    }
    sscanf(ctmp2, "%d %lf\n",
           &(X->CoulombIntra[idx][0]),
           &(X->ParaCoulombIntra[idx]));

    if (CheckSite(X->CoulombIntra[idx][0], X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }
  return 0;
}

static int ParseIdxCoulombInterDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  char ctmp2[256];

  if (X->NCoulombInter <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NCoulombInter) {
      return ReadDefFileError(defname);
    }
    sscanf(ctmp2, "%d %d %lf\n",
           &(X->CoulombInter[idx][0]),
           &(X->CoulombInter[idx][1]),
           &(X->ParaCoulombInter[idx]));

    if (CheckPairSite(X->CoulombInter[idx][0], X->CoulombInter[idx][1], X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }
  return 0;
}

static int ParseIdxHundDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  char ctmp2[256];

  if (X->NHundCoupling <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NHundCoupling) {
      return ReadDefFileError(defname);
    }
    sscanf(ctmp2, "%d %d %lf\n",
           &(X->HundCoupling[idx][0]),
           &(X->HundCoupling[idx][1]),
           &(X->ParaHundCoupling[idx]));

    if (CheckPairSite(X->HundCoupling[idx][0], X->HundCoupling[idx][1], X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }
  return 0;
}

static int ParseIdxPairHopDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  char ctmp2[256];

  if (X->iCalcModel == Spin || X->iCalcModel == SpinGC) {
    fprintf(stdoutMPI, "PairHop is not active in Spin and SpinGC.\n");
    return -1;
  }
  if (X->NPairHopping <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NPairHopping / 2) {
      return ReadDefFileError(defname);
    }
    sscanf(ctmp2, "%d %d %lf\n",
           &(X->PairHopping[2 * idx][0]),
           &(X->PairHopping[2 * idx][1]),
           &(X->ParaPairHopping[2 * idx]));

    if (CheckPairSite(X->PairHopping[2 * idx][0], X->PairHopping[2 * idx][1], X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    X->PairHopping[2 * idx + 1][0] = X->PairHopping[2 * idx][1];
    X->PairHopping[2 * idx + 1][1] = X->PairHopping[2 * idx][0];
    X->ParaPairHopping[2 * idx + 1] = X->ParaPairHopping[2 * idx];
    idx++;
  }
  return 0;
}

static int ParseIdxExchangeDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  char ctmp2[256];

  if (X->NExchangeCoupling <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NExchangeCoupling) {
      return ReadDefFileError(defname);
    }
    sscanf(ctmp2, "%d %d %lf\n",
           &(X->ExchangeCoupling[idx][0]),
           &(X->ExchangeCoupling[idx][1]),
           &(X->ParaExchangeCoupling[idx]));

    if (CheckPairSite(X->ExchangeCoupling[idx][0], X->ExchangeCoupling[idx][1], X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }
  return 0;
}

static int ParseIdxIsingDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  int isite1, isite2;
  double dvalue_re;
  char ctmp2[256];

  if (X->NIsingCoupling <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NIsingCoupling) {
      return ReadDefFileError(defname);
    }

    sscanf(ctmp2, "%d %d %lf\n", &isite1, &isite2, &dvalue_re);
    if (CheckPairSite(isite1, isite2, X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }

    X->HundCoupling[X->NHundCoupling + idx][0] = isite1;
    X->HundCoupling[X->NHundCoupling + idx][1] = isite2;
    X->ParaHundCoupling[X->NHundCoupling + idx] = -dvalue_re / 2.0;
    X->CoulombInter[X->NCoulombInter + idx][0] = isite1;
    X->CoulombInter[X->NCoulombInter + idx][1] = isite2;
    X->ParaCoulombInter[X->NCoulombInter + idx] = -dvalue_re / 4.0;
    idx++;
  }
  return 0;
}

static int ParseIdxPairLiftDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  char ctmp2[256];

  if (X->NPairLiftCoupling <= 0) {
    return 0;
  }
  if (X->iCalcModel != SpinGC) {
    fprintf(stdoutMPI, "PairLift is active only in SpinGC.\n");
    return -1;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NPairLiftCoupling) {
      return ReadDefFileError(defname);
    }

    sscanf(ctmp2, "%d %d %lf\n",
           &(X->PairLiftCoupling[idx][0]),
           &(X->PairLiftCoupling[idx][1]),
           &(X->ParaPairLiftCoupling[idx]));

    if (CheckPairSite(X->PairLiftCoupling[idx][0], X->PairLiftCoupling[idx][1], X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }
  return 0;
}

static int ParseIdxOneBodyGDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  int isite1, isite2;
  int isigma1, isigma2;
  char ctmp2[256];

  if (X->NCisAjt <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NCisAjt) {
      return ReadDefFileError(defname);
    }
    sscanf(ctmp2, "%d %d %d %d\n", &isite1, &isigma1, &isite2, &isigma2);

    if (X->iCalcModel == Spin) {
      if (isite1 != isite2) {
        fprintf(stdoutMPI, cWarningIncorrectFormatForSpin2, isite1, isite2);
        X->NCisAjt--;
        continue;
      }
    }

    X->CisAjt[idx][0] = isite1;
    X->CisAjt[idx][1] = isigma1;
    X->CisAjt[idx][2] = isite2;
    X->CisAjt[idx][3] = isigma2;

    if (CheckPairSite(isite1, isite2, X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }
  return 0;
}

static int ParseIdxTwoBodyGDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  int isite1, isite2, isite3, isite4;
  int isigma1, isigma2, isigma3, isigma4;
  char ctmp2[256];

  if (X->NCisAjtCkuAlvDC <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NCisAjtCkuAlvDC) {
      return ReadDefFileError(defname);
    }

    sscanf(ctmp2, "%d %d %d %d %d %d %d %d\n",
           &isite1,
           &isigma1,
           &isite2,
           &isigma2,
           &isite3,
           &isigma3,
           &isite4,
           &isigma4);

    if (X->iCalcModel == Spin || X->iCalcModel == SpinGC) {
      if (CheckFormatForSpinInt(isite1, isite2, isite3, isite4) != 0) {
        exitMPI(-1);
      }
    }

    X->CisAjtCkuAlvDC[idx][0] = isite1;
    X->CisAjtCkuAlvDC[idx][1] = isigma1;
    X->CisAjtCkuAlvDC[idx][2] = isite2;
    X->CisAjtCkuAlvDC[idx][3] = isigma2;
    X->CisAjtCkuAlvDC[idx][4] = isite3;
    X->CisAjtCkuAlvDC[idx][5] = isigma3;
    X->CisAjtCkuAlvDC[idx][6] = isite4;
    X->CisAjtCkuAlvDC[idx][7] = isigma4;

    if (CheckQuadSite(isite1, isite2, isite3, isite4, X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }
  return 0;
}

static int ParseIdxInterAllDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  int isite1, isite2, isite3, isite4;
  int isigma1, isigma2, isigma3, isigma4;
  int icnt_interall = 0;
  int icnt_diagonal = 0;
  double dvalue_re, dvalue_im;
  char ctmp2[256];

  X->NInterAll_Diagonal = 0;
  X->NInterAll_OffDiagonal = 0;
  if (X->NInterAll > 0) {
    while (fgetsMPI(ctmp2, 256, fp) != NULL) {
      if (idx == X->NInterAll) {
        return ReadDefFileError(defname);
      }
      sscanf(ctmp2, "%d %d %d %d %d %d %d %d %lf %lf\n",
             &isite1,
             &isigma1,
             &isite2,
             &isigma2,
             &isite3,
             &isigma3,
             &isite4,
             &isigma4,
             &dvalue_re,
             &dvalue_im);

      if (CheckInterAllCondition(X->iCalcModel, X->Nsite, X->iFlgGeneralSpin, X->LocSpn,
                                 isite1, isigma1, isite2, isigma2,
                                 isite3, isigma3, isite4, isigma4) != 0) {
        return ReadDefFileError(defname);
      }

      if (InputInterAllInfo(&icnt_interall,
                            X->InterAll,
                            X->ParaInterAll,
                            isite1, isigma1,
                            isite2, isigma2,
                            isite3, isigma3,
                            isite4, isigma4,
                            dvalue_re, dvalue_im) != 0) {
        icnt_diagonal += 1;
      }
      idx++;
    }
  }

  X->NInterAll = icnt_interall;
  X->NInterAll_Diagonal = icnt_diagonal;
  X->NInterAll_OffDiagonal = X->NInterAll - X->NInterAll_Diagonal;

  if (GetDiagonalInterAll_simple(
          X->InterAll, X->ParaInterAll, X->NInterAll,
          X->InterAll_Diagonal, X->ParaInterAll_Diagonal,
          X->InterAll_OffDiagonal, X->ParaInterAll_OffDiagonal,
          X->EDChemi, X->EDSpinChemi, X->EDParaChemi, &X->EDNChemi,
          X->iCalcModel) != 0) {
    return -1;
  }

  if (CheckInterAllHermite_simple(
          X->InterAll, X->ParaInterAll,
          X->InterAll_OffDiagonal, X->ParaInterAll_OffDiagonal,
          X->NInterAll_OffDiagonal, X->iCalcModel) != 0) {
    fprintf(stdoutMPI, "%s", cErrNonHermiteInterAllForAll);
    return -1;
  }

  if (ArrangeInterAllOffDiagonal(
          X->NInterAll_OffDiagonal,
          X->InterAll_OffDiagonal, X->ParaInterAll_OffDiagonal,
          X->iCalcModel) != 0) {
    return -1;
  }

  return 0;
}

static int ParseIdxThreeBodyGDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  int isite1, isite2, isite3, isite4, isite5, isite6;
  int isigma1, isigma2, isigma3, isigma4, isigma5, isigma6;
  char ctmp2[256];

  if (X->NTBody <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NTBody) {
      return ReadDefFileError(defname);
    }

    sscanf(ctmp2, "%d %d %d %d %d %d %d %d %d %d %d %d\n",
           &isite1,
           &isigma1,
           &isite2,
           &isigma2,
           &isite3,
           &isigma3,
           &isite4,
           &isigma4,
           &isite5,
           &isigma5,
           &isite6,
           &isigma6);

    X->TBody[idx][0] = isite1;
    X->TBody[idx][1] = isigma1;
    X->TBody[idx][2] = isite2;
    X->TBody[idx][3] = isigma2;
    X->TBody[idx][4] = isite3;
    X->TBody[idx][5] = isigma3;
    X->TBody[idx][6] = isite4;
    X->TBody[idx][7] = isigma4;
    X->TBody[idx][8] = isite5;
    X->TBody[idx][9] = isigma5;
    X->TBody[idx][10] = isite6;
    X->TBody[idx][11] = isigma6;
    idx++;
  }
  return 0;
}

static int ParseIdxFourBodyGDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  int isite1, isite2, isite3, isite4, isite5, isite6, isite7, isite8;
  int isigma1, isigma2, isigma3, isigma4, isigma5, isigma6, isigma7, isigma8;
  char ctmp2[256];

  if (X->NFBody <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NFBody) {
      return ReadDefFileError(defname);
    }

    sscanf(ctmp2, "%d %d %d %d %d %d %d %d %d %d %d %d  %d %d %d %d\n",
           &isite1,
           &isigma1,
           &isite2,
           &isigma2,
           &isite3,
           &isigma3,
           &isite4,
           &isigma4,
           &isite5,
           &isigma5,
           &isite6,
           &isigma6,
           &isite7,
           &isigma7,
           &isite8,
           &isigma8);

    X->FBody[idx][0] = isite1;
    X->FBody[idx][1] = isigma1;
    X->FBody[idx][2] = isite2;
    X->FBody[idx][3] = isigma2;
    X->FBody[idx][4] = isite3;
    X->FBody[idx][5] = isigma3;
    X->FBody[idx][6] = isite4;
    X->FBody[idx][7] = isigma4;
    X->FBody[idx][8] = isite5;
    X->FBody[idx][9] = isigma5;
    X->FBody[idx][10] = isite6;
    X->FBody[idx][11] = isigma6;
    X->FBody[idx][12] = isite7;
    X->FBody[idx][13] = isigma7;
    X->FBody[idx][14] = isite8;
    X->FBody[idx][15] = isigma8;
    idx++;
  }
  return 0;
}

static int ParseIdxSixBodyGDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  int isite1, isite2, isite3, isite4, isite5, isite6, isite7, isite8, isite9, isite10, isite11, isite12;
  int isigma1, isigma2, isigma3, isigma4, isigma5, isigma6, isigma7, isigma8, isigma9, isigma10, isigma11, isigma12;
  char ctmp2[256];

  if (X->NSBody <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (idx == X->NSBody) {
      return ReadDefFileError(defname);
    }

    sscanf(ctmp2, "%d %d %d %d %d %d %d %d %d %d %d %d  %d %d %d %d  %d %d %d %d %d %d %d %d\n",
           &isite1,
           &isigma1,
           &isite2,
           &isigma2,
           &isite3,
           &isigma3,
           &isite4,
           &isigma4,
           &isite5,
           &isigma5,
           &isite6,
           &isigma6,
           &isite7,
           &isigma7,
           &isite8,
           &isigma8,
           &isite9,
           &isigma9,
           &isite10,
           &isigma10,
           &isite11,
           &isigma11,
           &isite12,
           &isigma12);

    X->SBody[idx][0] = isite1;
    X->SBody[idx][1] = isigma1;
    X->SBody[idx][2] = isite2;
    X->SBody[idx][3] = isigma2;
    X->SBody[idx][4] = isite3;
    X->SBody[idx][5] = isigma3;
    X->SBody[idx][6] = isite4;
    X->SBody[idx][7] = isigma4;
    X->SBody[idx][8] = isite5;
    X->SBody[idx][9] = isigma5;
    X->SBody[idx][10] = isite6;
    X->SBody[idx][11] = isigma6;
    X->SBody[idx][12] = isite7;
    X->SBody[idx][13] = isigma7;
    X->SBody[idx][14] = isite8;
    X->SBody[idx][15] = isigma8;
    X->SBody[idx][16] = isite9;
    X->SBody[idx][17] = isigma9;
    X->SBody[idx][18] = isite10;
    X->SBody[idx][19] = isigma10;
    X->SBody[idx][20] = isite11;
    X->SBody[idx][21] = isigma11;
    X->SBody[idx][22] = isite12;
    X->SBody[idx][23] = isigma12;
    idx++;
  }
  return 0;
}

static int ParseIdxLaserDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  char ctmp[D_CharTmpReadDef];
  char ctmp2[256];

  if (X->NLaser <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    sscanf(ctmp2, "%s %lf\n", &(ctmp[0]), &(X->ParaLaser[idx]));
    idx++;
  }
  if (idx != X->NLaser) {
    return ReadDefFileError(defname);
  }
  return 0;
}

static int ParseIdxTEOneBodyDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int i;
  unsigned int idx = 0;
  int isite1, isite2;
  int isigma1, isigma2;
  double dvalue_re, dvalue_im;
  char ctmp2[256];

  if (X->NTETimeSteps <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    sscanf(ctmp2, "%lf %d\n", &(X->TETime[idx]), &(X->NTETransfer[idx]));
    for (i = 0; i < (unsigned int)X->NTETransfer[idx]; ++i) {
      fgetsMPI(ctmp2, 256, fp);
      sscanf(ctmp2, "%d %d %d %d %lf %lf\n",
             &isite1,
             &isigma1,
             &isite2,
             &isigma2,
             &dvalue_re,
             &dvalue_im);
      X->TETransfer[idx][i][0] = isite1;
      X->TETransfer[idx][i][1] = isigma1;
      X->TETransfer[idx][i][2] = isite2;
      X->TETransfer[idx][i][3] = isigma2;
      X->ParaTETransfer[idx][i] = dvalue_re + dvalue_im * I;
    }
    if (CheckTETransferHermite(X, X->NTETransfer[idx], idx) != 0) {
      return ReadDefFileError(defname);
    }
    idx++;
  }

  if (idx != X->NTETimeSteps) {
    return ReadDefFileError(defname);
  }
  return 0;
}

static int ParseIdxTETwoBodyDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int i;
  unsigned int idx = 0;
  int isite1, isite2, isite3, isite4;
  int isigma1, isigma2, isigma3, isigma4;
  int icnt_interall, icnt_diagonal;
  double dvalue_re, dvalue_im;
  char ctmp2[256];

  if (X->NTETimeSteps <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    sscanf(ctmp2, "%lf %d\n", &(X->TETime[idx]), &(X->NTEInterAll[idx]));
    icnt_interall = 0;
    icnt_diagonal = 0;
    for (i = 0; i < (unsigned int)X->NTEInterAll[idx]; ++i) {
      fgetsMPI(ctmp2, 256, fp);
      sscanf(ctmp2, "%d %d %d %d %d %d %d %d %lf %lf\n",
             &isite1,
             &isigma1,
             &isite2,
             &isigma2,
             &isite3,
             &isigma3,
             &isite4,
             &isigma4,
             &dvalue_re,
             &dvalue_im);
      if (CheckInterAllCondition(X->iCalcModel, X->Nsite, X->iFlgGeneralSpin, X->LocSpn,
                                 isite1, isigma1, isite2, isigma2,
                                 isite3, isigma3, isite4, isigma4) != 0) {
        return ReadDefFileError(defname);
      }
      if (InputInterAllInfo(&icnt_interall,
                            X->TEInterAll[idx],
                            X->ParaTEInterAll[idx],
                            isite1, isigma1,
                            isite2, isigma2,
                            isite3, isigma3,
                            isite4, isigma4,
                            dvalue_re, dvalue_im) != 0) {
        icnt_diagonal += 1;
      }
    }

    X->NTEInterAll[idx] = icnt_interall;
    X->NTEInterAllDiagonal[idx] = icnt_diagonal;
    X->NTEInterAllOffDiagonal[idx] = icnt_interall - icnt_diagonal;

    if (GetDiagonalInterAll_simple(
            X->TEInterAll[idx], X->ParaTEInterAll[idx], X->NTEInterAll[idx],
            X->TEInterAllDiagonal[idx], X->ParaTEInterAllDiagonal[idx],
            X->TEInterAllOffDiagonal[idx], X->ParaTEInterAllOffDiagonal[idx],
            X->TEChemi[idx], X->SpinTEChemi[idx], X->ParaTEChemi[idx], &X->NTEChemi[idx], X->iCalcModel) != 0) {
      return -1;
    }

    if (CheckInterAllHermite_simple(
            X->TEInterAll[idx], X->ParaTEInterAll[idx],
            X->TEInterAllOffDiagonal[idx], X->ParaTEInterAllOffDiagonal[idx],
            X->NTEInterAllOffDiagonal[idx], X->iCalcModel) != 0) {
      fprintf(stdoutMPI, "%s", cErrNonHermiteInterAllForAll);
      return -1;
    }

    if (ArrangeInterAllOffDiagonal(
            X->NTEInterAllOffDiagonal[idx],
            X->TEInterAllOffDiagonal[idx], X->ParaTEInterAllOffDiagonal[idx],
            X->iCalcModel) != 0) {
      return -1;
    }
    idx++;
  }

  if (idx != X->NTETimeSteps) {
    return ReadDefFileError(defname);
  }
  return 0;
}

static int ParseIdxBoostDef(FILE *fp, struct BoostList *xBoost) {
  unsigned int iline, iloop;
  int ilineIn, ilineIn2, itmp;
  double dArrayValue_re[3];
  char ctmp2[256];

  fgetsMPI(ctmp2, 256, fp);
  sscanf(ctmp2, "%lf %lf %lf\n",
         &dArrayValue_re[0],
         &dArrayValue_re[1],
         &dArrayValue_re[2]);
  for (iline = 0; iline < 3; iline++) {
    xBoost->vecB[iline] = dArrayValue_re[iline];
  }

  fgetsMPI(ctmp2, 256, fp);

  if (xBoost->NumarrayJ > 0) {
    for (iline = 0; iline < (unsigned int)xBoost->NumarrayJ; iline++) {
      for (ilineIn = 0; ilineIn < 3; ilineIn++) {
        fgetsMPI(ctmp2, 256, fp);
        sscanf(ctmp2, "%lf %lf %lf\n",
               &dArrayValue_re[0],
               &dArrayValue_re[1],
               &dArrayValue_re[2]);
        for (ilineIn2 = 0; ilineIn2 < 3; ilineIn2++) {
          xBoost->arrayJ[iline][ilineIn][ilineIn2] = dArrayValue_re[ilineIn2];
        }
      }
    }
  }

  fgetsMPI(ctmp2, 256, fp);

  if (xBoost->num_pivot > 0) {
    for (iline = 0; iline < (unsigned int)xBoost->num_pivot; iline++) {
      fgetsMPI(ctmp2, 256, fp);
      sscanf(ctmp2, "%d %d %d %d %d %d %d\n",
             &xBoost->list_6spin_star[iline][0],
             &xBoost->list_6spin_star[iline][1],
             &xBoost->list_6spin_star[iline][2],
             &xBoost->list_6spin_star[iline][3],
             &xBoost->list_6spin_star[iline][4],
             &xBoost->list_6spin_star[iline][5],
             &xBoost->list_6spin_star[iline][6]);
      for (iloop = 0; iloop < (unsigned int)xBoost->R0; iloop++) {
        for (itmp = 0; itmp < 7; itmp++) {
          xBoost->list_6spin_star[iloop * xBoost->num_pivot + iline][itmp] =
            xBoost->list_6spin_star[iline][itmp];
        }
      }
    }
  }

  if (xBoost->num_pivot > 0) {
    for (iline = 0; iline < (unsigned int)xBoost->num_pivot; iline++) {
      for (ilineIn2 = 0; ilineIn2 < xBoost->list_6spin_star[iline][0]; ilineIn2++) {
        fgetsMPI(ctmp2, 256, fp);
        sscanf(ctmp2, "%d %d %d %d %d %d %d\n",
               &xBoost->list_6spin_pair[iline][0][ilineIn2],
               &xBoost->list_6spin_pair[iline][1][ilineIn2],
               &xBoost->list_6spin_pair[iline][2][ilineIn2],
               &xBoost->list_6spin_pair[iline][3][ilineIn2],
               &xBoost->list_6spin_pair[iline][4][ilineIn2],
               &xBoost->list_6spin_pair[iline][5][ilineIn2],
               &xBoost->list_6spin_pair[iline][6][ilineIn2]);

        for (iloop = 0; iloop < (unsigned int)xBoost->R0; iloop++) {
          for (itmp = 0; itmp < 7; itmp++) {
            xBoost->list_6spin_pair[iloop * xBoost->num_pivot + iline][itmp][ilineIn2] =
              xBoost->list_6spin_pair[iline][itmp][ilineIn2];
          }
        }
      }
    }
  }

  return 0;
}

static int ParseIdxSingleExcitationDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  unsigned int itype;
  int isite1, isigma1;
  double dvalue_re, dvalue_im;
  char ctmp2[256];

  if (X->NSingleExcitationOperator <= 0) {
    return 0;
  }
  if (X->iCalcModel == Spin || X->iCalcModel == SpinGC) {
    fprintf(stderr, "SingleExcitation is not allowed for spin system.\n");
    return ReadDefFileError(defname);
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    sscanf(ctmp2, "%d %d %d %lf %lf\n",
           &isite1,
           &isigma1,
           &itype,
           &dvalue_re,
           &dvalue_im);

    if (CheckSite(isite1, X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }

    X->SingleExcitationOperator[idx][0] = isite1;
    X->SingleExcitationOperator[idx][1] = isigma1;
    X->SingleExcitationOperator[idx][2] = (int)itype;
    X->ParaSingleExcitationOperator[idx] = dvalue_re + I * dvalue_im;
    idx++;
  }

  if (idx != X->NSingleExcitationOperator) {
    return ReadDefFileError(defname);
  }
  return 0;
}

static int ParseIdxPairExcitationDef(FILE *fp, const char *defname, struct DefineList *X) {
  unsigned int idx = 0;
  unsigned int itype;
  int isite1, isigma1, isite2, isigma2;
  double dvalue_re, dvalue_im;
  char ctmp2[256];

  if (X->NPairExcitationOperator <= 0) {
    return 0;
  }
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    sscanf(ctmp2, "%d %d %d %d %d %lf %lf\n",
           &isite1,
           &isigma1,
           &isite2,
           &isigma2,
           &itype,
           &dvalue_re,
           &dvalue_im);
    if (CheckPairSite(isite1, isite2, X->Nsite) != 0) {
      return ReadDefFileError(defname);
    }

    if (itype == 1) {
      X->PairExcitationOperator[idx][0] = isite1;
      X->PairExcitationOperator[idx][1] = isigma1;
      X->PairExcitationOperator[idx][2] = isite2;
      X->PairExcitationOperator[idx][3] = isigma2;
      X->PairExcitationOperator[idx][4] = (int)itype;
      X->ParaPairExcitationOperator[idx] = dvalue_re + I * dvalue_im;
    } else {
      X->PairExcitationOperator[idx][0] = isite2;
      X->PairExcitationOperator[idx][1] = isigma2;
      X->PairExcitationOperator[idx][2] = isite1;
      X->PairExcitationOperator[idx][3] = isigma1;
      X->PairExcitationOperator[idx][4] = (int)itype;
      X->ParaPairExcitationOperator[idx] = -(dvalue_re + I * dvalue_im);
    }

    idx++;
  }

  if (idx != X->NPairExcitationOperator) {
    return ReadDefFileError(defname);
  }
  return 0;
}

typedef struct {
  struct DefineList *X;
  struct BoostList *xBoost;
  const char *defname;
} ReadIdxContext;

typedef int (*ReadIdxHandler)(FILE *fp, ReadIdxContext *ctx);

typedef struct {
  int keyword;
  ReadIdxHandler handler;
} ReadIdxDispatchEntry;

static int HandleIdxLocSpin(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxLocSpinDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxTrans(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxTransferDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxCoulombIntra(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxCoulombIntraDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxCoulombInter(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxCoulombInterDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxHund(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxHundDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxPairHop(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxPairHopDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxExchange(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxExchangeDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxIsing(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxIsingDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxPairLift(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxPairLiftDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxInterAll(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxInterAllDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxOneBodyG(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxOneBodyGDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxTwoBodyG(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxTwoBodyGDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxThreeBodyG(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxThreeBodyGDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxFourBodyG(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxFourBodyGDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxSixBodyG(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxSixBodyGDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxLaser(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxLaserDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxTEOneBody(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxTEOneBodyDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxTETwoBody(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxTETwoBodyDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxBoost(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxBoostDef(fp, ctx->xBoost);
}

static int HandleIdxSingleExcitation(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxSingleExcitationDef(fp, ctx->defname, ctx->X);
}

static int HandleIdxPairExcitation(FILE *fp, ReadIdxContext *ctx) {
  return ParseIdxPairExcitationDef(fp, ctx->defname, ctx->X);
}

static int DispatchReadIdxKeyword(const int keyword, FILE *fp, ReadIdxContext *ctx) {
  static const ReadIdxDispatchEntry kDispatchTable[] = {
      {KWLocSpin, HandleIdxLocSpin},
      {KWTrans, HandleIdxTrans},
      {KWCoulombIntra, HandleIdxCoulombIntra},
      {KWCoulombInter, HandleIdxCoulombInter},
      {KWHund, HandleIdxHund},
      {KWPairHop, HandleIdxPairHop},
      {KWExchange, HandleIdxExchange},
      {KWIsing, HandleIdxIsing},
      {KWPairLift, HandleIdxPairLift},
      {KWInterAll, HandleIdxInterAll},
      {KWOneBodyG, HandleIdxOneBodyG},
      {KWTwoBodyG, HandleIdxTwoBodyG},
      {KWThreeBodyG, HandleIdxThreeBodyG},
      {KWFourBodyG, HandleIdxFourBodyG},
      {KWSixBodyG, HandleIdxSixBodyG},
      {KWLaser, HandleIdxLaser},
      {KWTEOneBody, HandleIdxTEOneBody},
      {KWTETwoBody, HandleIdxTETwoBody},
      {KWBoost, HandleIdxBoost},
      {KWSingleExcitation, HandleIdxSingleExcitation},
      {KWPairExcitation, HandleIdxPairExcitation},
  };
  unsigned int i;
  const unsigned int nDispatch = sizeof(kDispatchTable) / sizeof(kDispatchTable[0]);
  for (i = 0; i < nDispatch; i++) {
    if (kDispatchTable[i].keyword == keyword) {
      return kDispatchTable[i].handler(fp, ctx);
    }
  }
  return 0;
}

int IsGeneralSpinForbiddenKeyword(const int keyword) {
  switch (keyword) {
  case KWCoulombIntra:
  case KWCoulombInter:
  case KWHund:
  case KWPairHop:
  case KWExchange:
  case KWIsing:
  case KWPairLift:
    return TRUE;
  default:
    return FALSE;
  }
}


int ParseReadDefIdxKeyword(const int keyword,
                           FILE *fp,
                           const char *defname,
                           struct DefineList *X,
                           struct BoostList *xBoost) {
  ReadIdxContext ctx;
  ctx.X = X;
  ctx.xBoost = xBoost;
  ctx.defname = defname;
  return DispatchReadIdxKeyword(keyword, fp, &ctx);
}
