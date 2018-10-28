



// Finds the Black(1976) price
// By Fabrice Douglas Rouah
// Ported to C++ by Nick Ferguson

#include <math.h>
#include <string.h>
#include <iostream>
#include "normalDist.h"
// F = futures price
// K = strike price
// T = maturity in years
// v = volatility
// rf = risk free rate
// PutCall = 'Call' or 'Put'

double Black76(double F, double K, double  T, double v, double rf, char* PutCall)
{
	if (strlen(PutCall) < 1)
		return -1;
	double d1 = (log(F / K) + (( pow(v , 2) / 2) *T )) / (v * sqrt(T));
	double d2 = d1 - v*sqrt(T);
	normalDist Gauss;
	double Nd1;
	double Nd2;
	double y;

	if (strcmp(PutCall, "Call") == 0 ) {
		Nd1 = Gauss.cdf(d1);
		 Nd2 = Gauss.cdf(d2);
		y = exp(-rf*T)*(F*Nd1 - K*Nd2);
	}
	else if (strcmp(PutCall, "Put") == 0)
	{
		Nd1 = Gauss.cdf(-1 * d1);
		Nd2 = Gauss.cdf(-1 * d2);
		y = exp(-rf*T)*(K*Nd2 - F*Nd1);
	}

	return y;
}

double diffInPutCallParity(double C, double P, double F, double K, double r, double T)
{

	double L= C - P;
	double R = (F - K)*exp(-r*T);
	return L - R;


}
