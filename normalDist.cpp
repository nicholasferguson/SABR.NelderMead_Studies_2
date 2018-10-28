

#include "normalDist.h"
#include <math.h>

inline double normalDist::pdf(double x)
{  //You will change this function if you want another type of distribution
	return exp(-1 * (x - mu) * (x - mu) / (2 * sigma * sigma)) / (sigma * sqrt(2 * pi));
}

double normalDist::cdf(double x)
{
	// Integral from a to x;
	const double ninf = mu - 10 * sigma; // Dependent on the type of distribution, tune as appropriate
	double sum = 0;
	double n = 1e5; // tune for speed/accuracy
	double c = (x - ninf) / n;

	for (double k = 1.; k < n - 1; k++)
		sum += pdf(ninf + k*c);

	return c * ((pdf(x) + pdf(ninf)) / 2 + sum);
}