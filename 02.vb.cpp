// 02.vb.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include "MSABR.h"
#include "CConstraints.h"
#include "CNealderMead.h"
#include "cmat.h"
#include "xlsread.h"
#include "ofmt.h"
#include "Black76.h"
//==============================================================================
// Containers to hold excel data in C++ app.
//==============================================================================
Matrix<double> cm_FixedRates;
std::vector<std::string > row_FixedRates;
std::vector<std::string > row_tenor_FixedRates;
std::vector<std::string > col_FixedRates;
Matrix<double> cm_MktVol;
Matrix<double> cm_K;
Matrix<double> cm_Expiry;
Matrix<double> cm_Fwd;
Matrix<double> cm_StrikeRates_K;
Matrix<double> cm_ModelVol;
Matrix<double> cm_ATMmktVol; // rows are Expiry and ols are Tenor  1 2 3 4 5 7 10 15 20 25 30
Matrix<double> cm_BetRhoNu;
std::map<int, std::vector<int> >ATMmktVolKeys;  // Key:Row Val:Exiry Tenor. 
std::map<int, int >ATMmktVolKeysRow; /// Key: Expiry Val: Row
std::map<int, int >ATMmktVolKeysCol;  //Key: Tenor Val: Col

// Some hard coded transfer of data from excel to this app
void populateATMmktVolKeysRow(std::map<int, int >& map)
{
	// Expiry - Key is Expiry. Val is Row
	// To get access to Matrix for Market ATM Vol
	map[1] = 0;
	map[2] = 1;
	map[3] = 2;
	map[4] = 3;
	map[5] = 4;
	map[7] = 5;
	map[10] = 6;
	map[15] = 7;
	map[20] = 8;
	map[25] = 9;
	map[30] = 10;

}
void populateATMmktVolKeysCol(std::map<int, int >& map)
{
	//L"Data", "E39:N39"
	//Row values of ATM Vol Tenor

	map[1] = 0;
	map[2] = 1;
	map[3] = 2;
	map[4] = 3;
	map[5] = 4;
	map[7] = 5;
	map[10] = 6;
	map[20] = 7;
	map[30] = 8;

}
// To do .....collect this info in a Matrix as opposed to hard copy.
void populateATMmktVolKeys(std::map<int, std::vector<int> >& map)
{//Market implied swaption volatilities
 // L"Data", L"P40:Q59"
 //Expiry	Tenor

	map[0] = { 5,2 };
	map[1] = { 10,2 };
	map[2] = { 20,2 };
	map[3] = { 30,2 };
	map[4] = { 5,5 };
	map[5] = { 10,5 };
	map[6] = { 20,5 };
	map[7] = { 30,5 };
	map[8] = { 5,10 };
	map[9] = { 10,10 };
	map[10] = { 20,10 };
	map[11] = { 30,10 };
	map[12] = { 5,20 };
	map[13] = { 10,20 };
	map[14] = { 20,20 };
	map[15] = { 30,20 };
	map[16] = { 5,30 };
	map[17] = { 10,30 };
	map[18] = { 20,30 };
	map[19] = { 30,30 };

}
void initLocalCalibration(Matrix<double>& matrix, int M, int N)
{
	vector<double> first = matrix.slice(0);
	matrix = matrix.newsize(M, N);
	for (int i = 0; i < M; ++i) {
		matrix.setRow(i, 0, first[0]);
		matrix.setRow(i, 1, first[1]);
		matrix.setRow(i, 2, first[2]);
	}
}
//==============================================================================
// Variables and containers for SABR related computations.
//==============================================================================

std::vector<double> Alphas;
double fwd = 0;  // forward rate.
double tau = 0;   // years
double atm = 0;
double bet = 0;
double rho = 0;
double nu  = 0;
int rows = 0;
int cols = 0;

//==============================================================================
// Functions called from NealderMead algorithm.
// This is called from MyFunction.
//==============================================================================
double gnm_range_sumxmy2(std::vector<double>& MktVol, std::vector<double>& ModelVol,
	int row)
{
	double s = 0;
	int i;

	for (i = 0; i < MktVol.size(); i++) {
		s += pow((ModelVol[i] - MktVol[i]), 2);
	}
	return s;
}

// main calls MyFunction on a row of data to calculate alpha, beta, rho, nu.
//     Each column of this row represents a different strike K.
// MyFunction calls NealderMead algorithm
// In NealderMead algorithm:
// SolveMinium via iteration calls CheckConvergence.
// Shrink calls CalculateNewApex
// SolveMinimum via iteration calls CalculateNewApex
// CalculateNewApex, GetInitialSimplex calls ApexEvaluate
// ApexEvaluate calls RunFUnction
// RunFunction calls MyFunction
// When converged, it goes onto next row of data to calculate alpha beta, rho, nu
int row = 0;
int Iter = 0;
double  MyFunction(std::vector<double>& py, bool Converged ) // default Converged = false
{

	double bet	= py[0];
	double rho	= py[1];
	double nu	= py[2];
	double Fwd = cm_FixedRates[row][0];
	double error;
	double y = 0.0;
	// calculate initial alpha
	double tau = cm_Expiry[row][0];
	int Expiry = ATMmktVolKeys[row][0];
	int Tenor =  ATMmktVolKeys[row][1];
	int atmRow = ATMmktVolKeysRow[Expiry];
	int atmCol = ATMmktVolKeysCol[Tenor];
	double atm = cm_ATMmktVol[atmRow][atmCol];
	double a0 = AlphaInitial(Fwd,tau,atm,bet,rho,nu);
	Alphas[row] = a0;

	// caculate row of SABR Vol
	// cols is from dimension of MktVol
	for (int i = 0; i < cols; ++i) {		

		double strikeRates = cm_StrikeRates_K[row][i];
		double vol = SABR_BlackVol_InitialAlpha(Fwd, strikeRates, tau, a0, bet, rho, nu, "Obloj2008");
		cm_ModelVol[row][i] = vol;

	}
	cm_BetRhoNu[row][0] = bet;
	cm_BetRhoNu[row][1] = rho;
	cm_BetRhoNu[row][2] = nu;
	std::vector<double> MktVol = cm_MktVol.slice(row);
	std::vector<double> ModelVol = cm_ModelVol.slice(row);
	y = gnm_range_sumxmy2(MktVol, ModelVol, row);
	Iter++;

	// Impose the constraint that - 1 <= rho <= +1 and that v>0
	if (abs(rho) > 1 || nu < 0)
		y = 1e100;
	if (Converged == true) {
		//std::cout << " row " << row << "\t" << " error " << y << "\t" << " beta " << bet << "\t" << " rho " << rho << "\t" << " nu " << nu << "\t" << " a0 " << a0 << "\t";
		//std::cout << " Fwd " << Fwd << "\t" << " tau " << tau << "\t" << " Expiry " << Expiry << "\t" << " Tenor " << Tenor << "\t" << " atm " << atm << std::endl;
		//std::cout << " row " << row << " beta " << bet << "\t" << " rho " << rho << "\t" << " nu " << nu << "\t" << " a0 " << a0 <<  std::endl;
	}
	if (Converged == true) {
		std::cout << "Row " << row << " Count " << Iter << std::endl;
		++row;
		Iter = 0;

	}

	return y;
}
//==============================================================================
// End of functions called from NealderMead
//==============================================================================

int main(int argc, char * argv[])
{
	//wchar_t* XLSPATH = (wchar_t*)argv[1];
	wchar_t* XLSPATH = L"H:\\github\\workdir.2018\\SABR.NelderMead_Studies_2\\Swaption_Volatility_SABR_Calibration.xls";
	std::cout << "Fetching data from spreadsheet.  Tenor and Exp are on s/h" << std::endl;
	// Fetch data from spreadsheets
	xlsread(XLSPATH, L"Data", L"T13:T32", cm_FixedRates);
	xlsreadLabels(XLSPATH, L"Data", L"I12:I32", row_FixedRates);

	xlsreadLabels(XLSPATH, L"Data", L"J12:J32", row_tenor_FixedRates);
	xlsreadLabels(XLSPATH, L"Data", L"T11", col_FixedRates);
	xlsread(XLSPATH, L"SABR(Implied Alpha)", L"D9:L28", cm_MktVol);
	xlsread(XLSPATH, L"SABR(Implied Alpha)", L"D8:L8", cm_K);
	xlsread(XLSPATH, L"SABR(Implied Alpha)", L"N9:N28", cm_Expiry);
	xlsread(XLSPATH, L"Data", L"X13:AF32", cm_StrikeRates_K);
	xlsread(XLSPATH, L"Data", L"F40:N50", cm_ATMmktVol);
	xlsread(XLSPATH, L"SABR(Implied Alpha)", L"O37:Q37", cm_BetRhoNu);
	std::cout << "=================Excel Input: Local Calibration for NelderMead: Beta Rho Nu ===========" << std::endl;
	initLocalCalibration(cm_BetRhoNu, cm_MktVol.num_rows(),3 );
	print_2Dmatrix(cm_BetRhoNu);


	std::cout << "=================Excel Input: Fixed Rates ===========" << std::endl;
	print_2Dmatrix(cm_FixedRates);
	std::cout << "================= Excel Input: MktVol ===========" << std::endl;
	print_2Dmatrix(cm_MktVol);
	std::cout << "================= Excel Input: K ===========" << std::endl;
	print_2Dmatrix(cm_K);
	std::cout << "================= Excel Input: StrikeRates_K ===========" << std::endl;
	print_2Dmatrix(cm_StrikeRates_K);
	std::cout << "================= Excel Input: ATMmktVol ===========" << std::endl;
	print_2Dmatrix(cm_ATMmktVol);


	// Redo this three 'populate' functions to use xlsread.
	populateATMmktVolKeys(ATMmktVolKeys);
	populateATMmktVolKeysRow(ATMmktVolKeysRow);
	populateATMmktVolKeysCol(ATMmktVolKeysCol);
	// Resize needed Matrices based on fetched data from spreadsheets
	rows = cm_MktVol.num_rows();
	cols = cm_MktVol.num_cols();
	cm_ModelVol.newsize(rows, cols);
	Alphas.resize(rows);

	Matrix<double> blackP; blackP.newsize(rows, cols);
	Matrix<double> blackC; blackC.newsize(rows, cols);
	Matrix<double> parity; parity.newsize(rows, cols);


	// Add Constraints.. how used?
	CNelderMead nmopt;
//	nmopt.AddSingleConstraint(0, "=", 0);
	nmopt.AddSingleConstraint(0, "<", 1);
	nmopt.AddSingleConstraint(0, ">", -1);
	nmopt.AddSingleConstraint(1, "<=", 1);
	nmopt.AddSingleConstraint(1, ">=", -1);
	nmopt.AddSingleConstraint(2, ">=", -1);
	nmopt.AddSingleConstraint(2, "=>", -1);
	nmopt.SetFunctionName("MyFunction");

	std::cout << "================= SABR Model Output: NelderMead Iterations to Converge ========== = " << std::endl; 
	std::cout << "================= 	for each row of SABR parameters  ===========" << std::endl;
	// iterate through rows to calculate SABR pertubation parameters.
	std::vector<double> initguess = cm_BetRhoNu.slice(0);
	for (int i = 0; i < rows; ++i)
	{
		// Step 1.  Get initial beta, rho and nu.  
	
		// initguess.push_back(i);
		// Step 2.  Run Nelder Mead to get a converged beta rho and nu
        // SolveMinimum will call MyFunction via RunFunction, via ApexEvaluate
		// When SolveMinimum returns it will have a set of converged parameters
		// and also a tested return for MyFunction.
		double rem = MyFunction(nmopt.SolveMinimum(initguess), true);

	}
	// iterate through rows to calculate Black-76 Put Call and Parity
	double r = .02;
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j)
		{

			blackP[i][j] = Black76(cm_FixedRates(i, j), cm_StrikeRates_K(i, j), cm_Expiry(i, 0), cm_ModelVol(i, j), r, "Put");
			blackC[i][j] = Black76(cm_FixedRates(i, j), cm_StrikeRates_K(i, j), cm_Expiry(i, 0), cm_ModelVol(i, j), r, "Call");
			parity[i][j] = diffInPutCallParity(blackC(i, j), blackP(i, j), cm_FixedRates(i, j), cm_StrikeRates_K(i, j), r, cm_Expiry(i, 0));
		}
	}
	std::cout << "================= SABR Model Output: Beta Rho Nu (via NelderMead's RunFunction/MyFunction) ===========" << std::endl;
	print_2Dmatrix(cm_BetRhoNu);
	std::cout << "================= SABR Model Output: Alpha (via NelderMead's RunFunction/MyFunction)  ===========" << std::endl;
	print_vector(Alphas);
	std::cout << "================= SABR Model Output: SABR volatilities  (via NelderMead's RunFunction/MyFunction)  ===========" << std::endl;
	print_2Dmatrix(cm_ModelVol);
	std::cout << "================= Black 76 Puts ===========" << std::endl;
	print_2Dmatrix(blackP);
	std::cout << "================= Black 76 Calls ===========" << std::endl;
	print_2Dmatrix(blackC);
	std::cout << "================= Black 76 Diffs in Put Call Parity ===========" << std::endl;
	print_2Dmatrix(parity);
	return 0;
}

