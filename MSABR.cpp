#include <string>
#include <algorithm>
#include <vector>
#include <math.h>
#include "cmat.h"


#include "MSABR.h"




int sgn(double num)
{
	if (num > 0)
		return 1;
	else if (num == 0)
		return 0;
	//else if (num < 0)
		return -1;
}


// ------------------------------------------------------
//   double SABR_BlackVol_InitialAlpha
//	 fwd = forward price
//	 k = strike price
//	 tau = expiry
//	 a0 = initial alpha
//	 bet = beta
//	 rho = rho
//	 nu = nu
// ------------------------------------------------------
double SABR_BlackVol_InitialAlpha(
	double fwd,
	double k,
	double tau,
	double a0,
	double bet,
	double rho,
	double nu,
	std::string model = "hagan2002")
{

	std::transform(model.begin(), model.end(), model.begin(), ::tolower);
	double h = 1 - bet;
	double p = pow((fwd * k), (h / 2));
	double q = log(fwd / k);
	double  v = pow(h, 2) * pow(a0, 2) / (24 * pow(p, 2)) +
		rho * bet * nu * a0 / (4 * p) +
		(2 - 3 * pow(rho, 2)) * pow(nu, 2) / 24;
	double  zeta = 0;
	double  zeta_chi = 0;
	double  eta = 0;
	//note that ((fwd ^ h - k ^ h) / (h * q)) = p * (1 + (h * q) ^ 2 / 24 + (h * q) ^ 4 / 1920 + ...)
	//note that p * q = ((fwd ^ h - k ^ h) / h) / (1 + (h * q) ^ 2 / 24 + (h * q) ^ 4 / 1920 + ...)
	if (abs(q) < eps){ // Then ' at-the-money, K = f	
		eta = p;
		zeta_chi = 1;	}
	else if (abs(h) < eps){ // Then ' beta = 1	
		eta = 1;
		zeta = nu / a0 * q;  //' p = 1 when beta = 1
		zeta_chi = zeta / Chi(zeta, rho);	}
	else   //' general formula
	{
		eta = (pow(fwd, h) - pow(k, h)) / (h * q);
		if (model == "hagan2002") { //Then ' original Hagan's method in Hagan et. al. 2002
			zeta = nu / a0 * p * q;
		}
		else if (model == "obloj2008") { // Then ' improved method in Obloj 2008
			zeta = nu / a0 * eta * q;
		}
		zeta_chi = zeta / Chi(zeta, rho);
	}
	
	return zeta_chi * (a0 * (1 + v * tau)) / eta;
}


double Chi(double z, double rho)
{
	return log((sqrt(1 - 2 * rho * z + pow(z, 2)) + z - rho) / (1 - rho));
}

// ------------------------------------------------------ 
// double AlphaInitial
//  fwd = forward
//  tau = expiry
//  atm = at-the-money volatility
//  bet = beta
//  rho = rho
//  nu = nu
// ------------------------------------------------------ 
double AlphaInitial(
	double fwd,
	double tau,
	double atm,
	double bet,
	double rho,
	double nu)
{
	double h = 1 - bet;
	double a = pow(h, 2) * tau / 24 / pow(fwd, (2 * h));
	double b = rho * bet * nu * tau / 4 / pow(fwd, h);
	double c = 1 + (2 - 3 * pow(rho, 2)) * pow(nu, 2) * tau / 24;
	double d = -atm * pow(fwd, h);

	// a*x^3 + b*x^2 + c*x + d = 0
	// take the smallest positive root.
	// When there are three real roots, they are of the order of -1000, 1 and +1000.
	// So we take the root of order 1.
	double param_norm = norm(a, b, c, d);
	if (abs(norm(0, 0, c, d) / param_norm - 1) < eps) //linear equation
	{
		//double ret = -d / c;
		//std::cout << " -d / c" << ret << std::endl;
		return  -d / c;
		//return ret;
	}
	else if (abs(norm(0, b, c, d) / param_norm - 1) < eps)  // quadratic equation
	{
		//double ret = QuadraticSolver(b, c, d);
		//std::cout << " QuadraticSolver " << ret << std::endl;
		//return ret;
		return QuadraticSolver(b, c, d);
	}
	else  // cubic equation
	{
		//double ret = CubicSolver(a, b, c, d);
		//std::cout << "CubicSolver " << ret << std::endl;
		//return ret;
		return CubicSolver(a, b, c, d);
	}

}

//==============================================================
// double SABR_Vol_By_ATM
// F = forward price
// X = strike price
// T = expiry
// ATM = at-the-money volatility
// B = beta
// R = rho
// V = nu
//==============================================================								
double SABR_Vol_By_ATM(double f,
	double x,
	double t,
	double atm,
	double b,
	double r,
	double v)
{

	return SABR_Vol(f, x, t, GetAlpha(f, t, atm, b, r, v), b, r, v);
}

//==============================================================
// double SABR_Vol
// F = forward price
// X = strike price
// T = expiry
// A = alpha
// B = beta
// R = rho
// V = nu
//==============================================================
double SABR_Vol(double f,
	double  x,
	double  t,
	double a,
	double b,
	double r,
	double v)
{
	double d = 1 - b;
	double p = f * x;

	double num = 1 + t * (pow(d, 2) / 24 * pow(a, 2) / pow(p, (2 * d))
		+ 0.25 * r * b * v * a / pow(f, d) + (2 - 3 * pow(r, 2)) * pow(v, 2) / 24);
	if (abs((f - x) / f) < 0.0000000001)
		return  a * num / pow(f, d);
	else
	{
		double q = log(f / x);
		double z = v / a * pow(p, (d / 2)) * q;
		double chi = log((pow((1 - 2 * r * z + pow(z, 2)), 0.5) + z - r) / (1 - r));
		double den = chi * pow(p, (d / 2)) * (1 + pow(d, 2) / 24 * pow(q, 2) + pow(d, 4) / 1920 * pow(q, 4));
		return z * a * num / den;
	}
}
//==============================================================
// double GetAlpha(double f,
// F = forward price
// T = expiry
// ATM = at-the-money volatility
// B = beta
// R = rho
// V = nu
//==============================================================
double GetAlpha(double f,
	double  t,
	double atm,
	double b,
	double r,
	double v)
{

	double d = 1 - b;
	double oo = pow(d , 2) * t / (24 * pow(f , (2 * d)));
	double pp = r * b * v * t / (4 * pow(f , d));
	double qq = 1 + (2 - 3 * pow(r , 2)) / 24 * pow(v , 2) * t;
	double rr = -atm * pow(f , d);
	// O*y^3 + P*y^2 + Q*y + R = 0
	return CubicSolver(oo, pp, qq, rr);
}

double norm(double a,
	double b,
	double c,
	double  d)
{
	return sqrt(pow(a, 2) + pow(b, 2) + pow(c, 2) + pow(d, 2));
}
// ------------------------------------------------------
// Solves a quadratic equation of the form:
// a*x^2 + b*x + c = 0 for real roots.
// ------------------------------------------------------
double QuadraticSolver(
	double a,
	double b,
	double c)
{
	double q = -0.5 * (b + sgn(b) * sqrt(pow(b, 2) - 4 * a * c));
	double x1 = q / a;
	double x2 = c / q;

	// take the smallest positive root.
	if (x1 * x2 < 0) //Then ' one positive
		return std::max(x1, x2);
	else if (x1 > 0) //Then ' both positive
		return std::min(x1, x2);
	else  //both negative
	//  Err.Raise 1, , "invalid solution for initial alpha"
		return -1;

}
// ------------------------------------------------------ 
//	 Solves a cubic equation of the form:
//	 x^3 + b*x^2 + c*x + d = 0 for real roots.
//	 Inputs:
//	 b,c,d: coefficients of polynomial.
//	
//	 Outputs:
//	 ROOT 3-vector containing only real roots.
//	 NROOTS The number of roots found. The real roots
//	 found will be in the first elements of ROOT.
//	
//	 Method: Closed form employing trigonometric and Cardan
//	 methods as appropriate.
//	
//	 Note: to transform equation:
//	 A*x^3 + B*x^2 + C*x + D = 0 
//	 into the form above, simply divide the coefficients thru by A
//	 i.e. b = B/A, c = C/A and d = D/A
// ------------------------------------------------------
double CubicSolver(
	double a,
	double b,
	double c,
	double d)
{

	b = b / a;
	c = c / a;
	d = d / a;

	// transform the equation into the form z^3 + p*z + q = 0
	double p = c - pow(b, 2.0) / 3.0;
	double q = b * (2.0 * pow(b, 2.0) - 9.0 * c) / 27.0 + d;

	const double deg = 2.09439510239319;  //' PI*2/3
	const double tol1 = 0.00001;
	const double tol2 = 1e-20;

	std::vector<double> z;
	long nr = 0;  //' number of roots
	double t1 = 0;
	double t2 = 0;
	double ratio = 0;
	if (sqrt(pow(p, 2) + pow(q, 2)) < tol2) // Then ' Z^3 ~= 0 : Three equal roots
	{
		nr = 3;
		for (int i = 0; i < nr; ++i)
			z.push_back(0);
	}
	else {
		double g = pow((p / 3), 3) + pow((q / 2), 2);
		if (g > 0)
		{
			t1 = -q / 2;
			t2 = sqrt(g);
			ratio = 1;
			if (q != 0)
				ratio = t2 / t1;
			if (abs(ratio) < tol1) // Then ' Three real roots, two (2 and 3) equal
			{
				nr = 3;
				z.push_back(2 * CubicRoot(t1));
				z.push_back(CubicRoot(-t1));
				z.push_back(z[1]);
			}
			else // ' One real root, two complex. Solve it using Cardan formula
			{
				nr = 1;
				z.push_back(CubicRoot(t1 + t2) + CubicRoot(t1 - t2));
			}

		}
		else  // ' Three real unequal roots. Solve using trigonometric method
		{
			nr = 3;
			double ad3 = p / 3.0;
			double e0 = 2.0 * sqrt(-ad3);
			double phi = -q / (2.0 * sqrt(pow(-ad3, 3.0)));
			double phi3 = cos(phi) / 3.0;
			z.push_back(e0 * cos(phi3));
			z.push_back(e0 * cos(phi3 + deg));
			z.push_back(e0 * cos(phi3 - deg));
		}

	}

	std::vector<double> root; // Now translate back to roots of original equation
	for (int i = 0; i < nr; ++i)
		root.push_back(z[i] - b / 3);

	return root[0];
}

double CubicRoot(double x)
{
	// Signed cube root function. Used by CubicSolver procedure.
	// repeating decimals to 18 places agrees with VB
	return pow(abs(x), .333333333333333333) * sgn(x);
}





