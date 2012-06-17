// C++ header.
// ===========
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <sstream>

// User header.
// ============
#include "jkFunction.h"

using namespace std;

// Function prototype.
// ===================
void showHowToUse(const char *);
void showResult();

// Global variable.
// ================
bool bUseColorFeature = false;
int iColorFeature = 0;

bool bUseWeightColor = false;
float fWeightColor = 0.0F;

bool bUseShapeFeature = false;
int iShapeFeature = 0;

bool bUseWeightShape = false;
float fWeightShape = 0.0F;
// ================

void main(int argc, char *argv[])
{
	// Timer start.
	clock_t tmStart = clock();

	// Checking correctness of all arguments.
	// ======================================
	if(argc == 1)
	{
		showHowToUse(argv[0]);
	}

	if(argc > 5)
	{
		cout << "You gave exceeded number of arguments.\n";
		exit(1);
	}

	for(int i=1;i<argc;i++)
	{
		string strTemp = argv[i];
		bool bCorrectPara = false;

		if(strTemp.at(0) != '/')
			showHowToUse(argv[0]);

		if(strTemp.at(1) == 'c' && strTemp.at(2) == '=')
		{
			bUseColorFeature = true;
			iColorFeature = atoi(strTemp.substr(3).c_str());
			bCorrectPara = true;
		}

		if(strTemp.at(1) == 'w' && strTemp.at(2) == 'c' && strTemp.at(3) == '=')
		{
			bUseWeightColor = true;
			fWeightColor = (float)atof(strTemp.substr(4).c_str());
			bCorrectPara = true;
		}
		
		if(strTemp.at(1) == 's' && strTemp.at(2) == '=')
		{
			bUseShapeFeature = true;
			iShapeFeature = atoi(strTemp.substr(3).c_str());
			bCorrectPara = true;
		}

		if(strTemp.at(1) == 'w' && strTemp.at(2) == 's' && strTemp.at(3) == '=')
		{
			bUseWeightShape = true;
			fWeightShape = (float)atof(strTemp.substr(4).c_str());
			bCorrectPara = true;
		}

		if(!bCorrectPara)
			showHowToUse(argv[0]);
	}

	if((bUseColorFeature == false) && (bUseWeightColor == true))
	{
		cout << "If you want to specific weight color, you must specific length of radius as well.\n";
		exit(1);
	}
	if((bUseShapeFeature == false) && (bUseWeightShape == true))
	{
		cout << "If you want to specific weight shape, you must specific number of vectors as well.\n";
		exit(1);
	}

	if(XOR(bUseWeightColor,bUseWeightShape))
	{
		if((fWeightColor + fWeightShape) != 1.0) // May be you can write 1 instead of 1.0 .
		{
			cout << "Your weight color and shape don't sum to 1.0 .\n";
			exit(1);
		}
	}
	// ======================================

	// At this state, all parameter were ok.
	// showResult();
	string strDirTestImageDB = "testImage\\"; // The folder of test image.
	string strFNameTestImageDB = strDirTestImageDB + "testFlowerList.txt"; // List of test image and the answer.
	string strOneLine; // Indexer.
	ifstream inFile;
	int count = 0; // Number of image files.

	// Find number of photos.
	// ======================
	inFile.open(strFNameTestImageDB.c_str());
	if(!inFile.is_open()) cout << "Can't open file " << strFNameTestImageDB << endl;
	while(getline(inFile,strOneLine))
	{
		count++;
	}
	inFile.close();
	inFile.clear(); // This must be call clear() before it will be made second call
					// otherwise the command will finished immediately.

	cout << "Number of test photo = " << count << endl << endl;

	// Prepare description.
	// ====================
	// extractDesc
	string cmdLine = "extractDesc";
	stringstream ss;

	if(bUseColorFeature)
	{
		ss << iColorFeature;
		cmdLine += " /c=" + ss.str();
		ss.str(""); // Clear stringstream.
	}
	if(bUseShapeFeature)
	{
		ss << iShapeFeature;
		cmdLine += " /s=" + ss.str();
		ss.str(""); // Clear stringstream.
	}
	
	int retExtractDesc = -1, retMatchingDesc = -1, retShowRank = -1;
	//cout << cmdLine << endl << endl;
	retExtractDesc = system(cmdLine.c_str());
	if(retExtractDesc != 0)
		cout << "There is error in extractDesc\n";

	// Process each test image.
	// ========================
	inFile.open(strFNameTestImageDB.c_str());
	if(!inFile.is_open()) cout << "Can't open file " << strFNameTestImageDB << endl;
	string::size_type idx; // A position of '\t'.
	string strFNameTestImage;
	int iAnsOfTestImage = 0;
	count = 0; // Reset count to 0.

	while(getline(inFile,strOneLine))
	{
		idx = strOneLine.find('\t');
		if(idx == string::npos)
			cout << strOneLine << " doesn't have TAB\n";
		strFNameTestImage = strOneLine.substr(0,idx);
		iAnsOfTestImage = atoi(strOneLine.substr(idx+1).c_str());
		// matchingDesc
		cmdLine = "matchingDesc";

		if(bUseColorFeature)
		{
			ss << iColorFeature;
			cmdLine += " /c=" + ss.str();
			ss.str(""); // Clear stringstream.
		}
		if(bUseWeightColor)
		{
			ss << fWeightColor;
			cmdLine += " /wc=" + ss.str();
			ss.str(""); // Clear stringstream.
		}
		if(bUseShapeFeature)
		{
			ss << iShapeFeature;
			cmdLine += " /s=" + ss.str();
			ss.str(""); // Clear stringstream.
		}
		if(bUseWeightShape)
		{
			ss << fWeightShape;
			cmdLine += " /ws=" + ss.str();
			ss.str(""); // Clear stringstream.
		}
		cmdLine += " " + strFNameTestImage;

		/*cout << cmdLine << endl;
		cout << iAnsOfTestImage << endl;*/
		retMatchingDesc = system(cmdLine.c_str());
		if(retMatchingDesc != 0)
			cout << "There is error in matchingDesc " << strFNameTestImage << endl;

		retMatchingDesc = -1; // Reset ret.
		//count++;
	}
	inFile.close();

	// Calculate accuracy.
	// ===================
	retShowRank = system("showRank");
	if(retShowRank != 0)
			cout << "There is error in showRank " << endl;
	
	// Timer stop.
	clock_t tmStop = clock();
	cout << endl << "Total using time = " << float(tmStop - tmStart)/CLOCKS_PER_SEC
		<< " sec" << endl;
	/*cout << "Press enter to exit.";
	getchar();*/
	soundOf7_11Door();
}

void showHowToUse(const char *ptrCharFNameOfProgram)
{
	string strFullName = ptrCharFNameOfProgram;
	string strFNameOnly;
	string::size_type idxSlash; // A position of '\'.
	string::size_type idxDot; // A position of '.'.

	idxSlash = strFullName.find_last_of('\\');
	idxDot = strFullName.find_last_of('.');
	strFNameOnly = strFullName.substr(idxSlash+1,idxDot -1 -idxSlash);

	cout << "Example of using.\n";
	cout << "=================\n";
	cout << strFNameOnly << " /c=100 /s=5\n";
	cout << strFNameOnly << " /c=300 /wc=.5 /s=7 /ws=.5\n";
	cout << endl;
	cout << "/c  = Length of radius that used in extracting color feature.\n";
	cout << "/wc = Weight of color feature.\n";
	cout << "/s  = Number of vectors of shape feature.\n";
	cout << "/ws = Weight of shape feature.\n";
	exit(1);
}

void showResult()
{
	cout << "Arguments.\n";
	cout << "==========\n\n";
	cout << "bUseColorFeature = " << bUseColorFeature << endl;
	cout << "c = " << iColorFeature << endl;
	cout << "bUseWeightColor = " << bUseWeightColor << endl;
	cout << "wc = " << fWeightColor << endl;
	cout << "bUseShapeFeature = " << bUseShapeFeature << endl;
	cout << "s = " << iShapeFeature << endl;
	cout << "bUseWeightShape = " << bUseWeightShape << endl;
	cout << "ws = " << fWeightShape << endl;
}