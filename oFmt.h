#pragma once


#include <iostream>
#include <iomanip>
#include <locale>
using namespace std;

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include "cmat.h"

class FMT
{
public:
	explicit FMT(const char* fmt) : m_fmt(fmt) {}
private:
	class fmter //actual worker class
	{
	public:
		explicit fmter(std::ostream& strm, const FMT& fmt) : m_strm(strm), m_fmt(fmt.m_fmt) {}
		//output next object (any type) to stream:
		template<typename TYPE>
		std::ostream& operator<<(const TYPE& value)
		{
			//            return m_strm << "FMT(" << m_fmt << "," << value << ")";
			char buf[40]; //enlarge as needed
			snprintf(buf, sizeof(buf), m_fmt, value);
			return m_strm << buf;
		}
	private:
		std::ostream& m_strm;
		const char* m_fmt;
	};
	const char* m_fmt; //save fmt string for inner class
					   //kludge: return derived stream to allow operator overloading:
	friend FMT::fmter operator<<(std::ostream& strm, const FMT& fmt)
	{
		return FMT::fmter(strm, fmt);
	}
};


void print_vector(vector<double>& m) {

		for (int j = 0; j < m.size(); ++j) {
			cout << FMT("%4.20f") << m[j] << endl;
		}
		cout << endl;
}


void print_2Dmatrix(Matrix<double>& matrix) {
	int rows = matrix.num_rows(); 
	int cols = matrix.num_cols(); 

	for (int i = 0; i < rows; ++i) {
		cout << "\t\t";
		for (int j = 0; j < cols; ++j) {
			cout  << FMT("%4.20f") << matrix(i,j) << "\t";
		}
		cout << endl;
	}
}
