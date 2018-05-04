#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>
struct ConItem
{
	//'e.g. 1.1x + 2.2y - 3.3z > 4.4
	std::vector<double> index;  // As Long ' index of parameters, e.g. (1, 2, 3)
	std::vector<double> coef; //As Double ' coefficients, e.g. (1.1, 2.2, 3.3)
	long dir; 	// As Long ' direction, e.g. 1 for >, -1 for < and 0 for =
	double bound;  //As Double ' boundary, e.g 4.4
	long n;		//As Long ' number of parameters involved

	ConItem(): dir(0),bound(1),n(1) {}

	ConItem(const ConItem& lhs)
	{
		dir = lhs.dir;
		bound = lhs.bound;
		n = lhs.n;
		for (int i = 0; i < lhs.index.size() ; ++i)
			index.push_back(lhs.index[i]);
		for (int i = 0; i < lhs.coef.size() ; ++i)
			coef.push_back(lhs.coef[i]);

	}

};

enum string_code {
	eq = 0, gt,
	gteq,
	eqgt,
	lt,
	lteq,
	eqlt
};



class Constraints
{

private:
	long size;		// As Long
	long count;		// As Long
	const double PWEIGHT = 1e30;// ' penalty weight
	const double CONTOLERANCE = 0.001;// ' 0.0001 ' ConItem tolerance
	std::vector<ConItem*>  ConArray;
	std::map<std::string, string_code> s_mapStringValues;
public:
	Constraints()
	{
		size = 8;
		
		count = 0;
		s_mapStringValues["="] = eq;
		s_mapStringValues[">"] = gt;
		s_mapStringValues[">="] = gteq;
		s_mapStringValues["=>"] = eqgt;
		s_mapStringValues["<"] = lt;
		s_mapStringValues["<="] = lteq;
		s_mapStringValues["=<"] = eqlt;
	}


	void AddConstraint(long index, long coef, std::string dir,double bound)
	{
		ConItem* con = new ConItem();


		con->n = 1;

		con->index.push_back(index);
     	con->coef.push_back(coef);


		switch (s_mapStringValues[dir])
		{
			case eq:
				con->dir = 0;
				break;
			case  gt:
			case gteq:
			case eqgt:
				con->dir = 1;
				break;
			case lt:
			case lteq:
			case eqlt:
				con->dir = -1;
				break;
			default:
				break;
				// Err.Raise 1, , "Invalid relationship in constraint.";
		}

		con->bound = bound;

		if (count >= size)
		{
			//size = size * 2;
			// ReDim Preserve ConArray(1 To size) As ConItem
			// ConArray = new ConItem[size];
			//ConArray.resize(ConArray.size() * 2);
			//size = (long) ConArray.size() * 2;
		}
		count += 1;
		ConArray.push_back(con);
	}


	double CalculatePenalty(std::vector<double>& pv)
	{
		//std::cout << " CalculatePenalty " << std::endl;
		double total = 0;
		double diff = 0;
		for (int i = 0; i < count; ++i)
		{
			ConItem* c = ConArray.at(i);
			double val = 0;
			for (int k = 0; k < c->n; ++k)
			{
				double a = c->coef[k];
				double a1 = c->index[k];
				double a2 = pv[a1] ;
				if( a2 < pv.size())
					val = val + c->coef[k] * pv[c->index[k]];

			}

			diff = val - c->bound;
			if (c->dir == 0)
			{
				if (abs(diff) > CONTOLERANCE) //Then ' invalid value			
					total = total + PWEIGHT * (abs(diff) - pow(CONTOLERANCE, 2));
			}
			else if (c->dir * diff < 0) // Then ' invalid value
				total = total + PWEIGHT * pow(diff, 2);

		}
		return total;    // ' if total > 0 then boundary breached

	}

};



