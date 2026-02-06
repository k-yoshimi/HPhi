#pragma once

#include <stdio.h>
#include "readdef.h"

int IsRequiredNameListKeyword(const int keyword);

int ParseReadDefNIntKeyword(const int keyword,
                            FILE *fp,
                            const char *defname,
                            struct DefineList *X,
                            struct BoostList *xBoost,
                            int *iReadNCond);
