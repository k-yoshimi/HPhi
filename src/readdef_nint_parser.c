/* HPhi  -  Quantum Lattice Model Simulator */
/* Copyright (C) 2015 The University of Tokyo */

#include "Common.h"
#include "readdef.h"
#include "readdef_nint_parser.h"
#include "LogMessage.h"
#include "wrapperMPI.h"

int ReadDefFileError(const char *defname);
int ReadcalcmodFile(const char *defname, struct DefineList *X);

typedef struct {
  struct DefineList *X;
  struct BoostList *xBoost;
  const char *defname;
  int *iReadNCond;
} ReadNIntContext;

typedef int (*ReadNIntHandler)(FILE *fp, ReadNIntContext *ctx);

typedef struct {
  int keyword;
  ReadNIntHandler handler;
} ReadNIntDispatchEntry;

int IsRequiredNameListKeyword(const int keyword) {
  return keyword == KWCalcMod || keyword == KWModPara || keyword == KWLocSpin;
}

static int ReadSimpleKeywordCount(FILE *fp, unsigned int *out_count) {
  char ctmp[D_CharTmpReadDef];
  char ctmp2[256];
  if (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) == NULL) {
    return -1;
  }
  if (fgetsMPI(ctmp2, 256, fp) == NULL) {
    return -1;
  }
  if (sscanf(ctmp2, "%s %u\n", ctmp, out_count) != 2) {
    return -1;
  }
  return 0;
}

static int HandleNIntCalcMod(FILE *fp, ReadNIntContext *ctx) {
  (void)fp;
  if (ReadcalcmodFile(ctx->defname, ctx->X) != 0) {
    return ReadDefFileError(ctx->defname);
  }
  return 0;
}

static int HandleNIntModPara(FILE *fp, ReadNIntContext *ctx) {
  char ctmp[D_CharTmpReadDef];
  char ctmp2[256];
  int itmp;
  double dtmp;
  double dtmp2;
  struct DefineList *X = ctx->X;

  if (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) == NULL) return -1;
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  sscanf(ctmp2, "%s %d\n", ctmp, &itmp);
  if (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) == NULL) return -1;
  if (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) == NULL) return -1;
  if (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) == NULL) return -1;

  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  sscanf(ctmp2, "%s %s\n", ctmp, X->CDataFileHead);

  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  sscanf(ctmp2, "%s %s\n", ctmp, X->CParaFileHead);

  if (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) == NULL) return -1;
  X->read_hacker = 1;
  while (fgetsMPI(ctmp2, 256, fp) != NULL) {
    if (*ctmp2 == '\n') continue;
    sscanf(ctmp2, "%s %lf %lf\n", ctmp, &dtmp, &dtmp2);
    if (CheckWords(ctmp, "Nsite") == 0) {
      X->Nsite = (int)dtmp;
    } else if (CheckWords(ctmp, "Nup") == 0) {
      X->Nup = (int)dtmp;
    } else if (CheckWords(ctmp, "Ndown") == 0) {
      X->Ndown = (int)dtmp;
      X->Total2Sz = X->Nup - X->Ndown;
    } else if (CheckWords(ctmp, "2Sz") == 0) {
      X->Total2Sz = (int)dtmp;
      X->iFlgSzConserved = TRUE;
    } else if (CheckWords(ctmp, "Ncond") == 0) {
      if ((int)dtmp < 0) {
        fprintf(stdoutMPI, cErrNcond, ctx->defname);
        return -1;
      }
      X->NCond = (int)dtmp;
      *(ctx->iReadNCond) = TRUE;
    } else if (CheckWords(ctmp, "Lanczos_max") == 0) {
      X->Lanczos_max = (int)dtmp;
    } else if (CheckWords(ctmp, "initial_iv") == 0) {
      X->initial_iv = (int)dtmp;
    } else if (CheckWords(ctmp, "nvec") == 0) {
      X->nvec = (int)dtmp;
    } else if (CheckWords(ctmp, "exct") == 0) {
      X->k_exct = (int)dtmp;
    } else if (CheckWords(ctmp, "LanczosEps") == 0) {
      X->LanczosEps = (int)dtmp;
    } else if (CheckWords(ctmp, "LanczosTarget") == 0) {
      X->LanczosTarget = (int)dtmp;
    } else if (CheckWords(ctmp, "LargeValue") == 0) {
      LargeValue = dtmp;
    } else if (CheckWords(ctmp, "NumAve") == 0) {
      NumAve = (int)dtmp;
    } else if (strcmp(ctmp, "TimeSlice") == 0) {
      X->Param.TimeSlice = dtmp;
    } else if (strcmp(ctmp, "ExpandCoef") == 0) {
      X->Param.ExpandCoef = (int)dtmp;
    } else if (strcmp(ctmp, "OutputInterval") == 0) {
      X->Param.OutputInterval = (int)dtmp;
    } else if (CheckWords(ctmp, "ExpecInterval") == 0) {
      X->Param.ExpecInterval = (int)dtmp;
    } else if (strcmp(ctmp, "Tinit") == 0) {
      X->Param.Tinit = dtmp;
    } else if (CheckWords(ctmp, "CalcHS") == 0) {
      X->read_hacker = (int)dtmp;
    } else if (CheckWords(ctmp, "OmegaMax") == 0) {
      X->dcOmegaMax = dtmp + dtmp2 * I;
      X->iFlgSpecOmegaMax = TRUE;
    } else if (CheckWords(ctmp, "OmegaMin") == 0) {
      X->dcOmegaMin = dtmp + dtmp2 * I;
      X->iFlgSpecOmegaMin = TRUE;
    } else if (CheckWords(ctmp, "OmegaIm") == 0) {
      X->dcOmegaOrg += dtmp * I;
      X->iFlgSpecOmegaOrg = TRUE;
    } else if (CheckWords(ctmp, "OmegaOrg") == 0) {
      X->dcOmegaOrg += dtmp + dtmp2 * I;
      X->iFlgSpecOmegaOrg = TRUE;
    } else if (CheckWords(ctmp, "NOmega") == 0) {
      X->iNOmega = (int)dtmp;
    } else if (CheckWords(ctmp, "TargetTPQRand") == 0) {
      X->irand = (int)dtmp;
    } else if (CheckWords(ctmp, "PreCG") == 0) {
      X->PreCG = (int)dtmp;
    } else {
      return -1;
    }
  }
  return 0;
}

static int HandleNIntLocSpin(FILE *fp, ReadNIntContext *ctx) {
  ctx->X->iFlgGeneralSpin = FALSE;
  return ReadSimpleKeywordCount(fp, &(ctx->X->NLocSpn));
}

static int HandleNIntTrans(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NTransfer));
}

static int HandleNIntCoulombIntra(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NCoulombIntra));
}

static int HandleNIntCoulombInter(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NCoulombInter));
}

static int HandleNIntHund(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NHundCoupling));
}

static int HandleNIntPairHop(FILE *fp, ReadNIntContext *ctx) {
  if (ReadSimpleKeywordCount(fp, &(ctx->X->NPairHopping)) != 0) {
    return -1;
  }
  ctx->X->NPairHopping *= 2;
  return 0;
}

static int HandleNIntExchange(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NExchangeCoupling));
}

static int HandleNIntIsing(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NIsingCoupling));
}

static int HandleNIntPairLift(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NPairLiftCoupling));
}

static int HandleNIntInterAll(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NInterAll));
}

static int HandleNIntOneBodyG(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NCisAjt));
}

static int HandleNIntTwoBodyG(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NCisAjtCkuAlvDC));
}

static int HandleNIntThreeBodyG(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NTBody));
}

static int HandleNIntFourBodyG(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NFBody));
}

static int HandleNIntSixBodyG(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NSBody));
}

static int HandleNIntInvTemp(FILE *fp, ReadNIntContext *ctx) {
  (void)fp;
  ctx->X->flag_read_invtemp = 1;
  strcpy(ctx->X->file_invtemp, ctx->defname);
  return 0;
}

static int HandleNIntLaser(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NLaser));
}

static int HandleNIntTEOneBody(FILE *fp, ReadNIntContext *ctx) {
  char ctmp[D_CharTmpReadDef];
  char ctmp2[256];
  double dtmp;
  int itmp;
  int iTETransMax = 0;
  unsigned int i;

  if (ctx->X->iCalcType != TimeEvolution) {
    return 0;
  }

  if (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) == NULL) return -1;
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  sscanf(ctmp2, "%s %d\n", ctmp, &(ctx->X->NTETimeSteps));
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;

  if (ctx->X->NTETimeSteps > 0) {
    while (fgetsMPI(ctmp2, 256, fp) != NULL) {
      sscanf(ctmp2, "%lf %d \n", &dtmp, &itmp);
      for (i = 0; i < (unsigned int)itmp; ++i) {
        fgetsMPI(ctmp2, 256, fp);
      }
      if (iTETransMax < itmp) iTETransMax = itmp;
    }
  }
  ctx->X->NTETransferMax = iTETransMax;
  return 0;
}

static int HandleNIntTETwoBody(FILE *fp, ReadNIntContext *ctx) {
  char ctmp[D_CharTmpReadDef];
  char ctmp2[256];
  double dtmp;
  int itmp;
  int iTEInterAllMax = 0;
  unsigned int i;

  if (ctx->X->iCalcType != TimeEvolution) {
    return 0;
  }

  if (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) == NULL) return -1;
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  sscanf(ctmp2, "%s %d\n", ctmp, &(ctx->X->NTETimeSteps));
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;
  if (fgetsMPI(ctmp2, 256, fp) == NULL) return -1;

  if (ctx->X->NTETimeSteps > 0) {
    while (fgetsMPI(ctmp2, 256, fp) != NULL) {
      sscanf(ctmp2, "%lf %d \n", &dtmp, &itmp);
      for (i = 0; i < (unsigned int)itmp; ++i) {
        fgetsMPI(ctmp2, 256, fp);
      }
      if (iTEInterAllMax < itmp) iTEInterAllMax = itmp;
    }
  }
  ctx->X->NTEInterAllMax = iTEInterAllMax;
  return 0;
}

static int HandleNIntBoost(FILE *fp, ReadNIntContext *ctx) {
  char ctmp2[256];
  unsigned int iline;

  ctx->xBoost->NumarrayJ = 0;
  ctx->xBoost->W0 = 0;
  ctx->xBoost->R0 = 0;
  ctx->xBoost->num_pivot = 0;
  ctx->xBoost->ishift_nspin = 0;
  ctx->xBoost->flgBoost = TRUE;
  fgetsMPI(ctmp2, 256, fp);
  fgetsMPI(ctmp2, 256, fp);
  sscanf(ctmp2, "%d\n", &(ctx->xBoost->NumarrayJ));
  for (iline = 0; iline < (unsigned int)(ctx->xBoost->NumarrayJ * 3); iline++) {
    fgetsMPI(ctmp2, 256, fp);
  }
  fgetsMPI(ctmp2, 256, fp);
  sscanf(ctmp2, "%ld %ld %ld %ld\n",
         &(ctx->xBoost->W0),
         &(ctx->xBoost->R0),
         &(ctx->xBoost->num_pivot),
         &(ctx->xBoost->ishift_nspin));
  return 0;
}

static int HandleNIntSingleExcitation(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NSingleExcitationOperator));
}

static int HandleNIntPairExcitation(FILE *fp, ReadNIntContext *ctx) {
  return ReadSimpleKeywordCount(fp, &(ctx->X->NPairExcitationOperator));
}

int ParseReadDefNIntKeyword(const int keyword,
                            FILE *fp,
                            const char *defname,
                            struct DefineList *X,
                            struct BoostList *xBoost,
                            int *iReadNCond) {
  static const ReadNIntDispatchEntry kDispatchTable[] = {
      {KWCalcMod, HandleNIntCalcMod},
      {KWModPara, HandleNIntModPara},
      {KWLocSpin, HandleNIntLocSpin},
      {KWTrans, HandleNIntTrans},
      {KWCoulombIntra, HandleNIntCoulombIntra},
      {KWCoulombInter, HandleNIntCoulombInter},
      {KWHund, HandleNIntHund},
      {KWPairHop, HandleNIntPairHop},
      {KWExchange, HandleNIntExchange},
      {KWIsing, HandleNIntIsing},
      {KWPairLift, HandleNIntPairLift},
      {KWInterAll, HandleNIntInterAll},
      {KWOneBodyG, HandleNIntOneBodyG},
      {KWTwoBodyG, HandleNIntTwoBodyG},
      {KWThreeBodyG, HandleNIntThreeBodyG},
      {KWFourBodyG, HandleNIntFourBodyG},
      {KWSixBodyG, HandleNIntSixBodyG},
      {KWInvTemp, HandleNIntInvTemp},
      {KWLaser, HandleNIntLaser},
      {KWTEOneBody, HandleNIntTEOneBody},
      {KWTETwoBody, HandleNIntTETwoBody},
      {KWBoost, HandleNIntBoost},
      {KWSingleExcitation, HandleNIntSingleExcitation},
      {KWPairExcitation, HandleNIntPairExcitation},
  };
  const unsigned int nDispatch = sizeof(kDispatchTable) / sizeof(kDispatchTable[0]);
  unsigned int i;
  ReadNIntContext ctx;

  ctx.X = X;
  ctx.xBoost = xBoost;
  ctx.defname = defname;
  ctx.iReadNCond = iReadNCond;

  for (i = 0; i < nDispatch; i++) {
    if (kDispatchTable[i].keyword == keyword) {
      return kDispatchTable[i].handler(fp, &ctx);
    }
  }
  fprintf(stdoutMPI, "%s", cErrIncorrectDef);
  return -1;
}
