
# SABR. This study illustrates how SABR, alpha beta rho nu, is solved using NelderMead 

+ This C++ app does not require any external libraries.  
+ It is built with Visual C++ 2017 Community Edition. x64.  Only debug mode has been tested.

+ It has two SABR related computations in function: SABR_BlackVol_InitialAlpha
	+ 'Obloj2008'  Method in Obloj 2008
	+ 'hagan2002'  Original Hagan's method in Hagan et. al. 2002
+ This SABR model does an initial fit of beta rho nu using method of a local calibration algorithm.  

# Original VB code by Changwei Xiong. 
+ http://www.cs.utah.edu/~cxiong/

+ His Excel spreadshet is titled "Swaption_Volatility_SABR_Calibration.xls" See his worksheet titled "Copyright and Disclaimer"  
+ This C++ console app reads in data from his spreadsheet.  Then duplicates computations found on his worksheet 'SABR(Implied Alpha)'
+ This C++ console app has output that can be compared to Changwei Xiong's spreadsheet output on worksheet 'SABR(Implied Alpha)'

# Issues
	+ This code needs some cleanup. It was meant as a quick study.
	+ Excel s/h was tested with Excel 2007 on Windows 10 Business
	+ Two addins are used
		+ Solver
			+ If problems, google, how delete this Addin and then reinstall.
		+ EDate for 2007 is available via Analysis Tool Pack Add for Excel 2007,
	+ Decimal accuracy between Excel and C++ is off...not sure why.
	+ This app does not yet promise to always close excel s/h.  This has to be reviewed.
		+ If run of this app is interrupted, an excel s/h will not correctly closed.  Have to add some code.
	
# To run: 
+ Swaption_Volatility_SABR_Calibration.xls MUST BE CLOSED.
+ Verify that this line is in 'Command Arguments" passed to app via VS properties, in debug mode.
$(ProjectDir)Swaption_Volatility_SABR_Calibration.xls
	
	
======
# Below is a partial out put from this C++ application
=======
+ Fetching data from spreadsheet.... I added some formatting in Readme.md

+ =================Excel Input Local Calibration for NelderMead: Beta, Rho Nu ===========

                 |  Beta                 |  Rho                     | Nu
                 |---------------------- | ------------------------ | ---------------------
                 |  0.56000000000000005  |   -0.01000000000000000   |   0.34599999999999997
				 
+ =================Excel Input Fixed Rates ===========

| Fixed Rates
| -----------
| 0.035257595 
| 0.041738169
| 0.042418148

+ ================= Excel Input MktVol ( SABR Implied Alpha Worksheet) ===========

                  |  -200               |  -100					
                  |-------------------- | ------------------------ 
                  | 0.27410000000000001 |  0.21270000000000000    
                  | 0.20929999999999999 |  0.16990000000000000
                  | 0.18420000000000000 |  0.14940000000000001
                  | 0.18140000000000001 |  0.14760000000000001 
				  
================= Excel Input K ===========

                  |  -200.00000000000000000 |  -100.00000000000000000  
				
================= Excel Input StrikeRates_K ( Data Worksheet) ===========

                  |  -200               |  -100					
                  |-------------------- | ------------------------ 
                  | 0.01525759459242056 |  0.02525759459242056   
                  | 0.02173816907673853 |  0.03173816907673853 
                  | 0.02241814840263004 |  0.03241814840263003  
                  | 0.01925608289916845 |  0.02925608289916845 
				  
================= Excel Input ATMmktVol ( Data Worksheet) ===========

                  |  1 yr               |  2 yr					
                  |-------------------- | ------------------------ 
                  | 0.21500000000000000 |  0.21900000000000000 
                  | 0.21600000000000000 |  0.21299999999999999 
                  | 0.20899999999999999 |  0.20499999999999999  
                  | 0.20000000000000001 |  0.19600000000000001 
				  
================= SABR Model Output: NelderMead Iterations to Converge ========== =
=================       for each row of SABR parameters  ===========

| Loops per row for NelderMean to converge
| -----------------------------------------
| Row 0 Count 93
| Row 1 Count 115
| Row 2 Count 104
| Row 3 Count 124
================= SABR Model Output: Beta Rho Nu (via NelderMead's RunFunction/MyFunction) ===========

                  |  beta                |  rho		              |	Nu		
                  |--------------------  | ---------------------- | ----------------------
                  | 0.56321204711218043  |   -0.01319411793721832 |     0.34591921732199021
                  | 0.56305676412278327  |   -0.02290658753951835 |     0.30525247364584918
                  | 0.59977120409842088  |   -0.15816151757678021 |     0.25428353284912453
                  | 0.62201488699163354  |   -0.17437854011038034 |     0.21155171237474185
				  
================= SABR Model Output: Alpha (via NelderMead's RunFunction/MyFunction)  ===========

| alpha
| -------------
| 0.041294239
| 0.035201727
| 0.033891166
| 0.035117295
================= SABR Model Output: SABR volatilities  (via NelderMead's RunFunction/MyFunction)  ===========
                  0.27407554051058425     0.21280701292774060     
                  0.20927971575906210     0.16996182736375970    
                  0.18418457013227427     0.14946216745150520   
                  0.18138201330104567     0.14768012006332795 






				