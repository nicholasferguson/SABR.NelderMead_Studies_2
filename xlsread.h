#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include "cmat.h"

//std::vector<double> xlsread(wchar_t* szWorkBookPath, wchar_t* szSheet, wchar_t* szRange);
int xlsread(wchar_t* szWorkBookPath, wchar_t* szSheet, wchar_t* szRange, Matrix<double>& ret);
int xlsreadLabels(wchar_t* szWorkBookPath, wchar_t* szSheet, wchar_t* szRange, std::vector<std::string>& ret);
HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...);

