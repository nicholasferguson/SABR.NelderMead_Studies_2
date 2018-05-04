
// g++ Main.cpp -oXL2.exe -m32 -lole32 -loleaut32 -luuid -Os -s
// cl Main.cpp kernel32.lib ole32.lib oleaut32.lib uuid.lib /O1 /Os /MT /FeXL2.exe
#ifndef   UNICODE
#define   UNICODE
#endif
#ifndef   _UNICODE
#define   _UNICODE
#endif
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <regex>
#include <string>
#include <codecvt>

#include "cmat.h"
#include "xlsread.h"

std::string BSTRtoString( BSTR& bbstr)
{
	assert(bbstr != nullptr);
	std::wstring bstr(bbstr, SysStringLen(bbstr));
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.to_bytes(bstr);

}
HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp,
	LPOLESTR ptName, int cArgs...)
{
	// Begin variable-argument list
	va_list marker;
	va_start(marker, cArgs);

	if (!pDisp)
	{
		_putws(L"NULL IDispatch passed to AutoWrap()");
		_exit(0);
		return E_INVALIDARG;
	}

	// Variables used
	DISPPARAMS dp = { NULL, NULL, 0, 0 };
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	DISPID dispID;
	HRESULT hr;

	// Get DISPID for name passed
	hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
	if (FAILED(hr))
	{
		wprintf(L"IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx\n",
			ptName, hr);
		_exit(0);
		return hr;
	}

	// Allocate memory for arguments
	VARIANT *pArgs = new VARIANT[cArgs + 1];
	// Extract arguments...
	for (int i = 0; i < cArgs; i++)
	{
		pArgs[i] = va_arg(marker, VARIANT);
	}

	// Build DISPPARAMS
	dp.cArgs = cArgs;
	dp.rgvarg = pArgs;

	// Handle special-case for property-puts
	if (autoType & DISPATCH_PROPERTYPUT)
	{
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	// Make the call
	hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT,
		autoType, &dp, pvResult, NULL, NULL);
	if (FAILED(hr))
	{
		wprintf(L"IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx\n",
			ptName, dispID, hr);
		_exit(0);
		return hr;
	}

	// End variable-argument section
	va_end(marker);

	delete[] pArgs;

	return hr;
}

VARIANT  CheckExcelArray(VARIANT& ExcelArray)
{
	VARIANT dvout;
	switch (ExcelArray.vt)
	{
	case VT_DISPATCH:
		// if Array is a Range object expose its values through the 
		//IDispatch interface
	{
		EXCEPINFO excep;
		DISPPARAMS dispparams;
		unsigned int uiArgErr;
		DISPID dispidValue;
		LPOLESTR XName = L"Value";

		ExcelArray.pdispVal->GetIDsOfNames(IID_NULL, &XName,
			1, LOCALE_SYSTEM_DEFAULT, &dispidValue);

		dispparams.cArgs = 0;
		dispparams.cNamedArgs = 0;

		// Invoke PropertyGet and store values in dvout
		ExcelArray.pdispVal->Invoke(dispidValue, IID_NULL,
			LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET,
			&dispparams, &dvout, &excep, &uiArgErr);

		ExcelArray.pdispVal->Release();

		return dvout;
	}
	break;

	default:
		//if the Array is a VBA SAFEARRAY return it as such
		return ExcelArray;
		break;
	}
	VariantClear(&dvout);
	VariantClear(&ExcelArray);
}
//////////////////////////////////////////////////////
// Takes a cell value and inputs to a single cell Matrix
// 
void PrintVariant(VARIANT &var, Matrix<double>& ret)
{
	ret.newsize(1, 1);

	if (var.vt == VT_UI1)
	{
	//	wprintf(L"VT_UI1:%u\n", var.bVal);
	}
	else if (var.vt == VT_I2)
	{
	//	wprintf(L"VT_I2:%d\n", var.iVal);
	}
	else if (var.vt == VT_I4)
	{
	//	wprintf(L"I4:%d\n", var.lVal);
	}
	else if (var.vt == VT_R4)
	{
	//	wprintf(L"VT_R4:%f\n", var.fltVal);
	}
	else if (var.vt == VT_R8)
	{
	//	wprintf(L"VT_R8:%lf\n", var.dblVal);
		ret(0, 0) = var.dblVal;
	}
	else if (var.vt == VT_BOOL)
	{
	//	wprintf(L"VT_BOOL:");
	//	if (var.boolVal == 0)
	//	{
	//		wprintf(L"false\n");
	//	}
	//	else
	//	{
			wprintf(L"true\n");
	//	}
	}
	else if (var.vt == VT_BSTR)
	{
		//wprintf(L"VT_BSTR:%s\n", var.bstrVal);
	}
	return;
}
//////////////////////////////////////////////////////
// Takes cell label and inputs to string vector.
// Expects to with with a range of cells
void PrintVariant(VARIANT &var, std::vector<std::string>& ret)
{
	

	if (var.vt == VT_UI1)
	{
		//	wprintf(L"VT_UI1:%u\n", var.bVal);
	}
	else if (var.vt == VT_I2)
	{
		//	wprintf(L"VT_I2:%d\n", var.iVal);
	}
	else if (var.vt == VT_I4)
	{
		//	wprintf(L"I4:%d\n", var.lVal);
	}
	else if (var.vt == VT_R4)
	{
		//	wprintf(L"VT_R4:%f\n", var.fltVal);
	}
	else if (var.vt == VT_R8)
	{
		//	wprintf(L"VT_R8:%lf\n", var.dblVal);
		 //ret(0, 0) = var.dblVal;
	}
	else if (var.vt == VT_BOOL)
	{
		//	wprintf(L"VT_BOOL:");
		//	if (var.boolVal == 0)
		//	{
		//		wprintf(L"false\n");
		//	}
		//	else
		//	{
		wprintf(L"true\n");
		//	}
	}
	else if (var.vt == VT_BSTR)
	{
		//wprintf(L"VT_BSTR:%s\n", var.bstrVal);
		ret.push_back(BSTRtoString(var.bstrVal));
	}
	return;
}
//////////////////////////////////////////////////////
// Takes cell value and inputs into matrix row col.
// Expects to work with a range of cells
void PrintVariant(VARIANT &var, Matrix<double>& ret, int row, int col)
{
	if (var.vt == VT_UI1)
	{
		//wprintf(L"VT_UI1:%u\n", var.bVal);
		ret(row, col) = var.bVal;
	}
	else if (var.vt == VT_I2)
	{
	//	wprintf(L"VT_I2:%d\n", var.iVal);
		ret(row, col) = var.iVal;
	}
	else if (var.vt == VT_I4)
	{
		//wprintf(L"I4:%d\n", var.lVal);
		ret(row, col) = var.lVal;
	}
	else if (var.vt == VT_R4)
	{
	//	wprintf(L"VT_R4:%f\n", var.fltVal);
		ret(row, col) = var.fltVal;
	}
	else if (var.vt == VT_R8)
	{
		//wprintf(L"VT_R8:%lf\n", var.dblVal);
		ret(row, col) = var.dblVal;
	}
	else if (var.vt == VT_BOOL)
	{
		//wprintf(L"VT_BOOL:");
		if (var.boolVal == 0)
		{
		//	wprintf(L"false\n");
			ret(row, col) = 0;
		}
		else
		{
		//	wprintf(L"true\n");
			ret(row, col) = 1;
		}
	}
	else if (var.vt == VT_BSTR)
	{
		//wprintf(L"VT_BSTR:%s\n", var.bstrVal);
		ret(row, col) = wcstod(var.bstrVal, NULL);

	}

	return;

}


//////////////////////////////////////////////////////

void  PrintArray(VARIANT sourceArray, std::vector<std::string>& ret)
{

	//check if Array is a Range object

	if (sourceArray.vt == VT_DISPATCH)
		sourceArray = CheckExcelArray(sourceArray);

	long ncols, nrows, i, j;

	//get the number columns and rows

	ncols = (sourceArray.parray)->rgsabound[0].cElements;
	nrows = (sourceArray.parray)->rgsabound[1].cElements;


	//dynamically allocate memory for an array to store values

	//	VARIANT *dArray = new VARIANT[nrows*ncols];
	VARIANT data;

	for (i = 0; i < nrows; i++)
	{
		for (j = 0; j < ncols; j++)
		{
			long indi[] = { i + 1,j + 1 };
			//store in a VARIANT array for other uses 

			//	SafeArrayGetElement(sourceArray.parray, indi, &dArray[(i*ncols) + j]);
			SafeArrayGetElement(sourceArray.parray, indi, &data);
			PrintVariant(data, ret);
		}
	}
	return;
}

//////////////////////////////////////////////////////

void  PrintArray(VARIANT sourceArray, Matrix<double>& ret)
{

	//check if Array is a Range object

	if (sourceArray.vt == VT_DISPATCH)
		sourceArray = CheckExcelArray(sourceArray);

	long ncols, nrows, i, j;

	//get the number columns and rows

	ncols = (sourceArray.parray)->rgsabound[0].cElements;
	nrows = (sourceArray.parray)->rgsabound[1].cElements;

	ret.newsize(nrows, ncols);

	//dynamically allocate memory for an array to store values

	//	VARIANT *dArray = new VARIANT[nrows*ncols];
	VARIANT data;

	for (i = 0; i < nrows; i++)
	{
		for (j = 0; j < ncols; j++)
		{
			long indi[] = { i + 1,j + 1 };
			//store in a VARIANT array for other uses 

			//	SafeArrayGetElement(sourceArray.parray, indi, &dArray[(i*ncols) + j]);
			SafeArrayGetElement(sourceArray.parray, indi, &data);
			PrintVariant(data, ret, i, j);
		//	PrintVariant(*dArray, ret, i, j);
		}
	}
//	delete[] dArray;
	return;
}
// not needed
// R1C1 = /^R(\d+)C(\d+)$/

std::string convertA1toR1C1(std::string& word_A1)
{
	char sep = ',';
	// regex
	std::regex token_A1("^([A-Z]+)(\\d+)$");
	std::string new_A1 = std::regex_replace(word_A1, token_A1, "$1,$2");

	std::vector<std::string> tokens;

	// tokenize
	for (size_t p = 0, q = 0; p != new_A1.npos; p = q)
		tokens.push_back(new_A1.substr(p + (p != 0), (q = new_A1.find(sep, p + 1)) - p - (p != 0)));

	// get chars
	char* str = new char[tokens[0].length() + 1];
	strcpy(str, tokens[0].c_str());

	int column = 0;

	// convert to numeric
	for (char* it = str; *it; ++it)
		column = 26 * column + *it - 64;

	std::stringstream s;
	s << "R" << tokens[1] << "C" << column;

	return s.str();

}



/////////////////////////////////////////////////////

HRESULT SetVisible(IDispatch* pObject, LCID lcid) // Visible is one of the members of the _Application object ...
{                                                 //
	VARIANT         vArgArray[1];                    // [odl, uuid(000208D5-0000-0000-C000-000000000046), helpcontext(0x00020001), dual, oleautomation] interface _Application : IDispatch
	DISPPARAMS      DispParams;                      // {
	DISPID          dispidNamed;                     //   ...
	VARIANT         vResult;                         //   [id(0x0000022e), propget, helpcontext(0x0001022e)] HRESULT Visible([in, lcid] long lcid, [out, retval] VARIANT_BOOL* RHS);
	HRESULT         hr;                              //   [id(0x0000022e), propput, helpcontext(0x0001022e)] HRESULT Visible([in, lcid] long lcid, [in] VARIANT_BOOL RHS);
													 // };
	VariantInit(&vArgArray[0]);                      //
	vArgArray[0].vt = VT_BOOL;         // This information can be obtained by dumping the object's typelib with OleView.exe.  You'll note the hexidicimal number 0x22e in
	vArgArray[0].boolVal = TRUE;                 // both the propget and propput Visible _Application::Visible() members.  That number is a dispatch id or dispid for short.
	dispidNamed = DISPID_PROPERTYPUT;   // Since here we want to set the property to TRUE we use the DISPATCH_PROPERTYPUT as the wFlags parameter of the
	DispParams.rgvarg = vArgArray;            // IDispatch::Invoke() call. Note above that the Excel Application Object is listed as being a dual interface.  The unfortunate
	DispParams.rgdispidNamedArgs = &dispidNamed;    // fact of the matter is that the direct VTable part of the dual interface is largely nonfunctional in all the MS Office applications
	DispParams.cArgs = 1;               // of which I'm familiar, specifically Word and Excel.  So this horrendous IDispatch access is forced upon us.  Its only redeeming
	DispParams.cNamedArgs = 1;               // feature is that it does work in spite of the fact that it is horribly awkward.
	VariantInit(&vResult);
	hr = pObject->Invoke(0x0000022e, IID_NULL, lcid, DISPATCH_PROPERTYPUT, &DispParams, &vResult, NULL, NULL);

	return hr;
}

HRESULT GetCellRange(IDispatch* pXLSheet, LCID& lcid, wchar_t* pszRange, VARIANT& pVArr)
{
	DISPPARAMS      NoArgs = { NULL,NULL,0,0 };           // Now this function here - 'GetCell()', is more general purpose than my 'GetXLCell()' above.  Use this
	IDispatch*      pXLRange = NULL;                      // one when you aren't sure what type of value is contained within a cell, or, stated even more precisely,
	VARIANT         vArgArray[1];                               // how Excel itself is internally handling the value contained within a cell.  For example, if you type
	VARIANT         vResult;                                    // 123456 in a cell, Excel the miserable but intelligent beaste will recognize that as a number, and when
	DISPPARAMS      DispParams;                                 // the cell contents is returned in a VARIANT, the VARIANT::vt member will likely be VT_I4 or VT_R8.
	HRESULT         hr;                                         // However, if the user or creator of the Excel SpreadSheet's data explicitely typed the cell as a 'Text'

	VARIANT parm;
	parm.vt = VT_BSTR;
	parm.bstrVal = ::SysAllocString(pszRange);
																// cell, (Right Click On A Cell And Choose 'Format Cells') a VT_BSTR will be returned in the VARIANT.  So
	VariantInit(&vResult);                                      // in other words, you can't just look at the data in an Excel spreadsheet and determine what the data
	vArgArray[0].vt = VT_BSTR,                   // type of the data is.  You had better reread that last sentence and think long and hard about it.  But
		vArgArray[0].bstrVal = SysAllocString(pszRange);  // this function begins exactly like the one above, but when it gets the IDispatch* to the Excel Range,
	DispParams.rgvarg = vArgArray;                 // it simply makes the IDispatch::Invoke() call to obtain the contents of the cell, and returns that
	DispParams.rgdispidNamedArgs = 0;                         // contents to the caller in the [out] parameter VARIANT& pVt.  The caller then has the responsibility
	DispParams.cArgs = 1;                         // for determining what the VARIANT contains, e.g, a number, a string, a date, an interface pointer, etc.,
	DispParams.cNamedArgs = 0;                         // and for freeing that memory if necessary.
	hr = pXLSheet->Invoke(0xC5, IID_NULL, lcid, DISPATCH_PROPERTYGET, &DispParams, &vResult, NULL, NULL);
	if (FAILED(hr)) {
		VariantClear(&vResult);
		return E_FAIL;
	}
	//	return hr;
	pXLRange = vResult.pdispVal;
	

	//Member Get Value <6> () As Variant
	VariantClear(&vArgArray[0]);
	VariantClear(&pVArr);
	hr = pXLRange->Invoke(6, IID_NULL, lcid, DISPATCH_PROPERTYGET, &NoArgs, &pVArr, NULL, NULL);
	pXLRange->Release();

	return hr;
}
IDispatch* SelectWorkSheet(IDispatch* pXLWorksheets, LCID& lcid, wchar_t* pszSheet)
{
	VARIANT         vResult;
	HRESULT         hr;
	VARIANT         vArgArray[1];
	DISPPARAMS      DispParams;
	DISPID          dispidNamed;
	IDispatch*      pXLWorksheet = NULL;

	// Member Get Item <170> (In Index As Variant<0>) As IDispatch  >> Gets pXLWorksheet
	// [id(0x000000aa), propget, helpcontext(0x000100aa)] IDispatch* Item([in] VARIANT Index);
	VariantInit(&vResult);
	vArgArray[0].vt = VT_BSTR;
	vArgArray[0].bstrVal = SysAllocString(pszSheet);
	DispParams.rgvarg = vArgArray;
	DispParams.rgdispidNamedArgs = &dispidNamed;
	DispParams.cArgs = 1;
	DispParams.cNamedArgs = 0;
	hr = pXLWorksheets->Invoke(0xAA, IID_NULL, lcid, DISPATCH_PROPERTYGET, &DispParams, &vResult, NULL, NULL);
	if (FAILED(hr))
		return NULL;
	pXLWorksheet = vResult.pdispVal;
	SysFreeString(vArgArray[0].bstrVal);

	// Worksheet::Select()
	VariantInit(&vResult);
	VARIANT varReplace;
	varReplace.vt = VT_BOOL;
	varReplace.boolVal = VARIANT_TRUE;
	dispidNamed = 0;
	DispParams.rgvarg = &varReplace;
	DispParams.rgdispidNamedArgs = &dispidNamed;
	DispParams.cArgs = 1;
	DispParams.cNamedArgs = 1;
	hr = pXLWorksheet->Invoke(0xEB, IID_NULL, lcid, DISPATCH_METHOD, &DispParams, &vResult, NULL, NULL);

	return pXLWorksheet;
}


IDispatch* OpenXLWorkBook(IDispatch* pXLWorkbooks, LCID& lcid, wchar_t* pszWorkBookPath)
{
	VARIANT         vResult;
	VARIANT         vArgArray[1];
	DISPPARAMS      DispParams;
	DISPID          dispidNamed;
	HRESULT         hr;
	DISPID dispid;
	OLECHAR* szOpen = (OLECHAR*)L"Open";

	hr = pXLWorkbooks->GetIDsOfNames(IID_NULL, &szOpen, 1, GetUserDefaultLCID(), &dispid);
	if (SUCCEEDED(hr))
	{
		VariantInit(&vResult);         // Call Workbooks::Open() - 682  >> Gets pXLWorkbook
		vArgArray[0].vt = VT_BSTR;
		vArgArray[0].bstrVal = SysAllocString(pszWorkBookPath);
		DispParams.rgvarg = vArgArray;
		DispParams.rgdispidNamedArgs = &dispidNamed;
		DispParams.cArgs = 1;
		DispParams.cNamedArgs = 0;
		hr = pXLWorkbooks->Invoke(dispid, IID_NULL, lcid, DISPATCH_METHOD, &DispParams, &vResult, NULL, NULL);
		//hr = pXLWorkbooks->Invoke(682, IID_NULL, lcid, DISPATCH_METHOD, &DispParams, &vResult, NULL, NULL);
		SysFreeString(vArgArray[0].bstrVal);
		if (FAILED(hr))
			return NULL;
		else
			return vResult.pdispVal;
	}
	return NULL;
}

IDispatch* GetDispatchObject(IDispatch* pCallerObject, DISPID dispid, WORD wFlags, LCID lcid)
{
	DISPPARAMS   NoArgs = { NULL,NULL,0,0 };
	VARIANT      vResult;
	HRESULT      hr;

	VariantInit(&vResult);
	hr = pCallerObject->Invoke(dispid, IID_NULL, lcid, wFlags, &NoArgs, &vResult, NULL, NULL);
	if (FAILED(hr))
		return NULL;
	else
		return vResult.pdispVal;
}


int xlsread(wchar_t* szWorkBookPath, wchar_t* szSheet, wchar_t* szRange, Matrix<double>& ret)
{
	const CLSID  CLSID_XLApplication = { 0x00024500,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
	const IID    IID_Application = { 0x000208D5,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
	IDispatch*   pXLApp = NULL;
	IDispatch*   pXLWorkbooks = NULL;
	IDispatch*   pXLWorkbook = NULL;
	IDispatch*   pXLWorksheets = NULL;
	IDispatch*   pXLWorksheet = NULL;
	DISPPARAMS   NoArgs = { NULL,NULL,0,0 };
	wchar_t      szCell[64];
	HRESULT      hr;
	LCID         lcid;
	VARIANT		 cellRange;
	cellRange.vt = VT_ARRAY | VT_VARIANT;
	SAFEARRAYBOUND sab[2];
	VARIANT vResult;
	sab[0].lLbound = 1; sab[0].cElements = 10;
	sab[1].lLbound = 1; sab[1].cElements = 1;
	cellRange.parray = SafeArrayCreate(VT_VARIANT, 2, sab);

	CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_XLApplication, NULL, CLSCTX_LOCAL_SERVER, IID_Application, (void**)&pXLApp);
	if (SUCCEEDED(hr))
	{

		lcid = GetUserDefaultLCID();
		//SetVisible(pXLApp, lcid);
		pXLWorkbooks = GetDispatchObject(pXLApp, 572, DISPATCH_PROPERTYGET, lcid);
		if (pXLWorkbooks)
		{
			pXLWorkbook = OpenXLWorkBook(pXLWorkbooks, lcid, szWorkBookPath);
			if (pXLWorkbook)
			{
				pXLWorksheets = GetDispatchObject(pXLWorkbook, 494, DISPATCH_PROPERTYGET, lcid);
				if (pXLWorksheets)
				{
					pXLWorksheet = SelectWorkSheet(pXLWorksheets, lcid, szSheet);
					if (pXLWorksheet)
					{
						GetCellRange(pXLWorksheet, lcid, szRange, cellRange);
						if (cellRange.vt < 0x2000) { // review this.
							PrintVariant(cellRange,ret);
						}
						else {

							PrintArray(cellRange,ret);
						}
						pXLWorksheet->Release();
					}
					pXLWorksheets->Release();
				}
				AutoWrap(DISPATCH_METHOD, NULL, pXLWorkbooks, L"Close", 0);
				pXLWorkbook->Release();
			}
			AutoWrap(DISPATCH_METHOD, NULL, pXLWorkbooks, L"Close", 0);
			pXLWorkbooks->Release();
		}

		// Quit the Excel application. (i.e. Application.Quit())
		//_putws(L"Quit the Excel application");

	//	getchar();
		pXLApp->Invoke(0x0000012e, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &NoArgs, NULL, NULL, NULL); // 
		//AutoWrap(DISPATCH_METHOD, NULL, pXLApp, L"Quit", 0);
		pXLApp->Release();

	}

	CoUninitialize();
//	std::cout << "" << std::endl;
	return 0;
}

int xlsreadLabels(wchar_t* szWorkBookPath, wchar_t* szSheet, wchar_t* szRange, std::vector<std::string>& ret)
{
	const CLSID  CLSID_XLApplication = { 0x00024500,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
	const IID    IID_Application = { 0x000208D5,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
	IDispatch*   pXLApp = NULL;
	IDispatch*   pXLWorkbooks = NULL;
	IDispatch*   pXLWorkbook = NULL;
	IDispatch*   pXLWorksheets = NULL;
	IDispatch*   pXLWorksheet = NULL;
	DISPPARAMS   NoArgs = { NULL,NULL,0,0 };
	wchar_t      szCell[64];
	HRESULT      hr;
	LCID         lcid;
	VARIANT		 cellRange;
	cellRange.vt = VT_ARRAY | VT_VARIANT;
	SAFEARRAYBOUND sab[2];
	sab[0].lLbound = 1; sab[0].cElements = 10;
	sab[1].lLbound = 1; sab[1].cElements = 1;
	cellRange.parray = SafeArrayCreate(VT_VARIANT, 2, sab);

	CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_XLApplication, NULL, CLSCTX_LOCAL_SERVER, IID_Application, (void**)&pXLApp);
	if (SUCCEEDED(hr))
	{

		lcid = GetUserDefaultLCID();
		//SetVisible(pXLApp, lcid);
		pXLWorkbooks = GetDispatchObject(pXLApp, 572, DISPATCH_PROPERTYGET, lcid);
		if (pXLWorkbooks)
		{
			pXLWorkbook = OpenXLWorkBook(pXLWorkbooks, lcid, szWorkBookPath);
			if (pXLWorkbook)
			{
				pXLWorksheets = GetDispatchObject(pXLWorkbook, 494, DISPATCH_PROPERTYGET, lcid);
				if (pXLWorksheets)
				{
					pXLWorksheet = SelectWorkSheet(pXLWorksheets, lcid, szSheet);
					if (pXLWorksheet)
					{
						GetCellRange(pXLWorksheet, lcid, szRange, cellRange);
						if (cellRange.vt < 0x2000) { // review this.
							PrintVariant(cellRange, ret);
						}
						else {

							PrintArray(cellRange, ret);
						}
						pXLWorksheet->Release();
					}
					pXLWorksheets->Release();
				}
				pXLWorkbook->Release();
			}
			pXLWorkbooks->Release();
		}
		//	getchar();
		pXLApp->Invoke(0x0000012e, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &NoArgs, NULL, NULL, NULL); // 
		pXLApp->Release();

	}
	CoUninitialize();

	return 0;
}
/*
int main()
{
	wchar_t      szWorkBookPath[] = L"K:\\work.interviews\\W.next.one\\excel\\import.from.excel\\set1_basic\\Book1.xls"; // << Fix This For Your Path!!!
	wchar_t      szSheet[] = L"Sheet1";
	wchar_t      szRange[] = L"A1:B5";
	xlsread(szWorkBookPath, szSheet, szRange);
	return 0;

}
*/