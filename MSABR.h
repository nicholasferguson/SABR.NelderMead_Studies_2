#pragma once


#include <string>
#include <algorithm>
#include <vector>
#include <math.h>
#include "cmat.h"

const double eps = 1e-8; //1e-20; // 1e-8

int sgn(double num);
double norm(double a, double b, double c, double  d);
double Chi(double z, double rho);
double CubicRoot(double x);
double QuadraticSolver(double a, double b, double c);
double CubicSolver(double a, double b, double c, double d);

//double SABR_BlackVol_By_atmvol(	double fwd,	double k,	double tau,	double atm,	double bet,	double rho,	double nu,std::string model);
double AlphaInitial(	double fwd,	double tau,	double atm,	double bet,	double rho,	double nu);
double GetAlpha(double f, double  t, double atm, double b, double r, double v);
double SABR_BlackVol_InitialAlpha(double fwd, double k, double tau, double a0, double bet, double rho, double nu, std::string model);
double SABR_Vol_By_ATM(double f, double x, double t, double atm, double b, double r, double v);
double SABR_Vol(double f, double  x, double  t, double a, double b, double r, double v);
double CubicSolver(double Op, double pp, double Qp, double Rp);