#pragma once
#include <vector>
#include "cmat.h"
double  MyFunction(std::vector<double>& py, bool Converged = false);
//int gnm_range_sumxmy2(Matrix<double> *xs, Matrix<double> *ys, int n, std::vector<double>& res, int row);
double gnm_range_sumxmy2(std::vector<double>& MktVol, std::vector<double>& ModelVol,
	 int row);
