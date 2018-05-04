/*
In the spreadsheet, both the original method proposed by Hagan et. al. 2002 
and the fine tuned correction proposed by Obloj 2008 are implemented.
constrained Nelder-Mead Simplex method 
*/

#pragma once

#include <string>
#include <math.h>
#include <algorithm>
#include <vector>
#include <array>
#include <type_traits>
#include "CConstraints.h"
#include "MSABR.h"
#include "02.vb.h"

static int IDD = 0;

class Apex
{
public:

	Apex() : type(""), id(IDD++) {}
	std::vector<double> x;
	double f;
	std::string type;
	int id;
};

class CNelderMead
{

private:

	const double alpha = 1; // reflection
	const double gamma = 2; // expansion
	const double rho = 0.5; // contraction
	const double sigma = 0.5; // shrinkage

	const long MAXITER = 1000; // maximal # of iterations
	const double XTOL = 0.00001; //simplex size convergence
	const double FTOL = 0.0000000001; //function value convergence
	const double eps = 1e-20; //function value convergence
	Constraints constraint;
	std::string functionname;
	double previousfunctionvalue;
	double sse = 0;

public:
	// SolveMinimum calls CalculateNewApexToReplaceWorstApex or calls GetInitialSimplex
	// which then calls void ApexEvaluate(Apex* a) which cals RunFunction.
	// ApexEvaluate is only function that calls RunFunction.
	double   RunFunction(std::vector<double>& params)
	{
		if (functionname == "")
			return pow(params[0] - 4, 2) + pow(params[1] - 2.7, 4) * sin(params[1] - 2) + pow(params[2] - 6.7, 4)* cos(params[2] - 6.7);
		else
			//SolveMinimum(x) gets parameters then passes them to MyFunction.
			sse = MyFunction(params);
		return sse;

	}


	CNelderMead():functionname(""), previousfunctionvalue(0)
	{	}


	void SetFunctionName(std::string function_name)
	{
		functionname = function_name;
	}

	void AddSingleConstraint(long index,std::string dir,double bound)
	{
		long coef;
		coef = 1;
		constraint.AddConstraint(index, coef, dir, bound);
	}

	void AddSumConstraint(std::vector<double> index,std::vector<double> coef,std::string dir,
		double bound)
	{
		for (int i = 0; i < index.size(); ++i)
			constraint.AddConstraint(index[i], coef[i], dir, bound);
	}

	//------------------------------------------------------------------------------
	//    Nelder-Mead method loops as follows:
	//  
	//    order ascendingly the n apexes by function value (n = dimension + 1, smallest f(x) ==> best)
	//    check convergence (must be under TOL=>diff between best value and others, both for function and parameters)
	//    calculate xo (i.e. the centroid of the apexes excluding the worst apex)
	//    reflextion: xr = xo + alpha * (xo - xn)    
	//    if xr better than x1 then do "expansion":
	//        xe = xo + gamma * (xr - xo)
	//        if f(xe) < f(xr) then xe --> xn otherwise xr --> xn
	//    elseif xr better than x(n-1) then do "reflection":
	//        xr --> xn
	//    elseif xr better than xn then do "outside contraction":
	//        xoc = xo + rho * (xr - xo)
	//        if f(xoc) < f(xr) then xoc --> xn
	//        else do "shrinkage": compute xi = x1 + sigma * (xi - x1) for i = 2 to n
	//    else do "inside contraction":
	//        xic = xo - rho * (xr - xo)   <==> xic = xo + rho * (xn - xo) if alpha = 1 and rho = 1/2
	//        if f(xic) < f(xn) then xic --> xn
	//       else do "shrinkage": compute xi = x1 + sigma * (xi - x1) for i = 2 to n
	//   end if
	//    ------------------------------------------------------------------------------
	//    
	//     alpha > 0; gamma > 1; 0 < rho, sigma < 1
	//     alpha = 1; gamma = 2; rho = sigma = 1/2
	//------------------------------------------------------------------------------  
	std::vector<double> SolveMinimum(std::vector<double>& pv)
	{
		int n = (int)pv.size() + 1 ;

		//std::cout << "called SolveMinimum " << std::endl;
		std::vector<Apex*> simplex = GetInitialSimplex(pv);

		for (long iter = 0; iter < MAXITER; ++iter)
		{
			//std::cout << "New iteration \t" << iter << std::endl;
			SortSimplex(simplex);  // ' step (1) sorting
			if (CheckConvergence(simplex) == true) // step (2) convergence check
			{
				//return simplex[0]->x; // return the 1st apex (i.e. optimal, best)
				//do cleanup for next SolveMinimum...
				std::vector<double> vv(simplex[0]->x);
				for (int i = 0; i < simplex.size(); ++i)
					delete	simplex[i];
				simplex.empty();
				return vv;

			}
	

			Apex* xo = GetCentroidExcludingWorstApex(simplex);  // step (3) obtaining xo
			xo->type = "c";

			// step (4) reflection: reflect the worst to go farthest away from it
			Apex* xr = CalculateNewApexToReplaceWorstApex(xo, simplex[n-1], alpha); //  xr = xo - alpha * (xo - simplex[n-1])
			xr->type = "r";

			if (xr->f < simplex[0]->f)  // xr is better than x1 (i.e. better than the best)
			{
				// expansion: xr better than x1, so go more and expand in this direction
				Apex* xe = CalculateNewApexToReplaceWorstApex(xo, xr, -gamma); // xe = xo + -1* gamma * (xo - xr)
				xe->type = "e";
				if (xe->f < xr->f)  simplex[n-1] = xe; else simplex[n-1] = xr;
			}
			else if (xr->f < simplex[n - 2]->f) // xr is better than 2nd worst
			{
				// reflection:
				simplex[n-1] = xr;  //  good in moving towards this direction
			}
			else if (xr->f < simplex[n-1]->f)  // xr is better than the worst
			{
				// outside contraction:
				Apex* xoc = CalculateNewApexToReplaceWorstApex(xo, xr, -rho); //  xoc = xo + rho * (xo - xr)
				xoc->type = "xoc";
				if (xoc->f < xr->f) simplex[n-1] = xoc; else Shrink(simplex);
			}
			else // xr is NOT better than any apex
			{
				// inside contraction:
				Apex* xic = CalculateNewApexToReplaceWorstApex(xo, xr, rho); // xic = xo - rho * (xr - xo)
				xic->type = "xic";
														  // Dim xic As Apex: xic = CalculateNewApexToReplaceWorstApex(xo, simplex(n), -rho) ' xic = xo - rho * (xo - xn)
				if (xic->f < simplex[n-1]->f) simplex[n-1] = xic; else Shrink(simplex);
			}
		}
		std::string msg("iterations did not converge");
		std::cout << msg << std::endl;
	}


	void Shrink(std::vector<Apex*>& simplex)
	{
		// the shrink transformations almost never happens in practice
		long size = (long) simplex.size();
		for (long i = 0; i < size; ++i) // xi = x1 + sigma * (xi - x1) for i = 2 to n
			simplex[i] = CalculateNewApexToReplaceWorstApex(simplex[i], simplex[i], -sigma);
	}


	std::vector<Apex*> GetInitialSimplex(std::vector<double>&  pv)
	{
		   // An n dimensional search would have a simplex
			//with  n + 1  vertices.This  is  because  a  one
			//dimensional  search  space  would  need  two
			//vertices to cover a range, and each additional
			//dimension would need one more vertex to cover it.
		double shift = 0;
		int n = (int)pv.size() +1;
		std::vector<Apex*> simplex;
		for (int i = 0; i < n; ++i) 
			simplex.push_back(new Apex());
	
		//set first row vector simply to initial parameters
		simplex[0]->x = pv;
		ApexEvaluate(simplex[0]);
	//	for (int i = 0; i < n - 1; ++i)
			//	std::cout << "GetInitialSimplex #1 " << i << " \t" << simplex[0]->x[i] << std::endl;

			//calculate shifting factor by taking hightest value of coordinates of initial guess
			//shift = max entry of initial guess, if shift < 1 then shift = 1
		shift = Max(1, MaxVec(pv));

	
		double factor = 0;
		long count = 0;

		for (long i = 1; i < n; ++i)  //loop over remaining row vectors (i.e. apexes)
		{
			simplex[i]->x = pv;
			factor = 1;
			do { // new apex by shifting the coordinates of initial guess
				 // if out of boundary then shrinks apex to x1
				simplex[i]->x[i - 1] = pv[i - 1] + shift * factor;
			//	std::cout << "GetInitialSimplex #2 " << i << " \t" << " pv[i - 1] " << pv[i - 1] << " shift*factor " << shift*factor << "\t" << simplex[i]->x[i - 1] << std::endl;
				factor = factor / 2;
			//	std::cout << "InitialSimplex loop: " << count++ << std::endl;
			} while (constraint.CalculatePenalty(simplex[i]->x) > eps);
			// hack for row number
		//	simplex[i]->x[3] = simplex[0]->x[3];
			ApexEvaluate(simplex[i]);
		}
		return simplex;
	}

	bool CheckConvergence(std::vector< Apex*>& simplex)
	{
		long n = (long)simplex.size();

		double xdiff = 0;// xdiff = inf-norm between other apexes and x1
		double fdiff = 0;

		for (long i = 1; i < n; ++i) // loop over apexes except the 1st
			for (long j = 0; j < n - 1; ++j) // loop over coordinates of an apex
				xdiff = Max(xdiff, abs(simplex[i]->x[j] - simplex[0]->x[j]));

		xdiff = xdiff / Max(1, MaxVec(simplex[0]->x)); //normalized by inf-norm of x1
		fdiff = abs(simplex[n-1]->f - simplex[0]->f) / Max(1, abs(simplex[0]->f));
		if (xdiff < XTOL || fdiff < FTOL)
			return true;
		return false;
	}

	// sample mean.  Some literature say calculate mean.
	Apex*   GetCentroidExcludingWorstApex(std::vector<Apex*>& simplex)
	{
		// calculate the centroid of the apexes excluding the worst apex
		int n = (long)simplex.size();

		Apex* centroid = new Apex();
		//centroid->x.resize(n);

		for (int i = 0; i < n-1; ++i)  //loop over coordinates
		{
			double sum = 0;
			for (long j = 0; j < n - 1; ++j) {  //loop over apexes except for the last (i.e. worst)
			//	std::cout << "getcentroid #1  apex: " << simplex[j]->id <<  " simplex# j "<< j << " pv x " << i << "\tstart sum " << sum << "\tsimplex[j]->x[i]  " << simplex[j]->x[i] << std::endl;
				sum = sum + simplex[j]->x[i];
			//	std::cout << "getcentroid #2 apex: " << simplex[j]->id  << " simplex# j " << j << " pv x " << i << "\tnext sum " << sum << "\tsimplex[j]->x[i]  " << simplex[j]->x[i] << std::endl;
			} 
			centroid->x.push_back(sum / (n - 1));
		//	std::cout << "getcentroid #3 " << i << "\tend sum " << sum  << " avg " << sum/ (n - 1) << std::endl;
		}
		//  ApexEvaluate centroid ' evaluate function value at centroid
		return centroid;
	}


	void ApexEvaluate(Apex* a)
	{
		double penalty = constraint.CalculatePenalty(a->x);
		if (penalty == 0)  //no boundary breaches
			previousfunctionvalue = RunFunction(a->x);
		a->f = previousfunctionvalue + penalty;
	}

	Apex* CalculateNewApexToReplaceWorstApex(	Apex*a,Apex* b,double c)
	{
		// xnew = a + c * (a - b)
		int size = (long)a->x.size();
		Apex* xnew = new Apex();
		for (int i = 0; i < size; ++i) {
	//		std::cout << ":CalculateNewApexToReplaceWorstApex 1 Apex: " << xnew->id << "\t" << i << "\ta->x[i] + c * (a->x[i] - b->x[i])\t" << a->type << "\t" << b->type << "\t" << " coef " << c << "\t" <<  "a->x[i] " << a->x[i] << " b->x[i] " << b->x[i] << std::endl;
			double DD = a->x[i] + c * (a->x[i] - b->x[i]);
	//		std::cout << ":CalculateNewApexToReplaceWorstApex 1 Apex: " << xnew->id << "\t" << i << "\ta->x[i] + c * (a->x[i] - b->x[i])\t" << a->type << "\t" << b->type << "\t" << " coef " << c << "\t" << DD << "a->x[i] " << a->x[i] <<  " b->x[i] " << b->x[i] << std::endl;
			xnew->x.push_back(DD);

		}
		ApexEvaluate(xnew);
		return xnew;
	}
	void printSimplex(std::vector<Apex*>& simplex)
	{
		long size = (long)simplex.size();
		long sizex = (long)simplex[0]->x.size();
		for (long i = 0; i < size; ++i) //  for each apex upto the 2nd last
		{
		//	std::cout << i << "f\t" << simplex[i]->f << "\t" << std::endl;
			for (long j = 0; j < sizex;  ++j)
			{
			//	std::cout << j << "\t"  << simplex[i]->x[j] << std::endl;
			}
		}

	}

	void SortSimplex(std::vector<Apex*>& simplex)
	{
		// Sorts apexes of simplex in an ascending order
		// the 1st has the smallest function value, while the last has the largest.
	//	std::cout << "Before sort\t" << std::endl;
	//	printSimplex(simplex);
		long size = (long)simplex.size();
		for (long i = 0; i < size; ++i) //  for each apex upto the 2nd last
		{
			for (long j = i + 1; j < size; ++j)
			{
				if (simplex[i]->f > simplex[j]->f) // swap apex i with j
				{
					//  std::swap(simplex[i],simplex[j]);
					Apex* tmp = simplex[i];
					simplex[i] = simplex[j];
					simplex[j] = tmp;
				}
			}
		}
	//	std::cout << "After sort\t" << std::endl;
	//	printSimplex(simplex);
	}
	double Max(double a, double  b)
	{
		if (a > b) return  a; else return b;
	}
	double MaxVec(std::vector<double>& vec)
	{
		double maxval = 0;
		int size = (int)vec.size();
		for (int i = 0; i < size; ++i)
			maxval = Max(maxval, abs(vec[i]));
		return maxval;
	}


	long Factorial(long n)
	{
		if (n <= 1) //  Factorial = 1: Exit Function
			return 1;
		return n * Factorial(n - 1);
	}
};