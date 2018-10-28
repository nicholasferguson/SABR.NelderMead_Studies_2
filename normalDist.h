#pragma once

const double pi = 3.14159265358979323846;


class normalDist
{
public:
	normalDist(double _mu = 0.0, double _sigma = 1.0) : mu(_mu), sigma(_sigma) {}
	inline double pdf(double x);
	double cdf(double x);
private:
	double mu;
	double sigma;
};
