#pragma once

#include <stdio.h>
#include "readdef.h"

int ParseReadDefIdxKeyword(const int keyword,
                           FILE *fp,
                           const char *defname,
                           struct DefineList *X,
                           struct BoostList *xBoost);

int IsGeneralSpinForbiddenKeyword(const int keyword);
