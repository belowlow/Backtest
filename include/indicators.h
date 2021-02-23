#ifndef INDICATORS_H
#define INDICATORS_H

#include "backtrader.h"
#include <math.h>

double get_sd(void* vectorName, ColumnType colType, int period = 0, int start = 0);
double get_movingAVG(void* vectorName, ColumnType colType, int period);
double round(double x, int num);

#endif // INDICATORS_H
