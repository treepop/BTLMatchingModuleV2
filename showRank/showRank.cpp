// C++ header.
// ===========
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>

// User header.
// ============
#include "..\matchingDesc\beeConfig.h"

using namespace std;

int main()
{
	// Timer start.
	clock_t tmStart = clock();
	
	//string strInputDistanceFile = "Matches.txt";
	string strDirTestImageDB = "testImage\\"; // The folder of test image.
	string strFNameTestImageDB = strDirTestImageDB + "testFlowerList.txt"; // List of test image and the answer.
	string strOutputRankFile = "Rank.txt";
	string strBaseName;
	string strOneLine;
	ifstream inFile;
	ifstream inMFile;
	ofstream outFile;
	vector<string> rank;
	string::size_type idxUnderScroll; // A position of '_'.
	vector<string>::iterator pos;
	bool bFound;
	int count = 0; // Number of image files.

	// Find number of photos.
	// ======================
	inFile.open(strFNameTestImageDB.c_str());
	if(!inFile.is_open())
	{
		cout << "Can't open file " << strFNameTestImageDB << endl;
		exit(EXIT_FAILURE);
	}
	while(getline(inFile,strOneLine))
	{
		count++;
	}
	inFile.close();
	inFile.clear(); // This must be call clear() before it will be made second call
					// otherwise the command will finished immediately.

	// cout << "Number of test photo = " << count << endl << endl;

	int *answer = new int[count];

	// Process each test image.
	// ========================
	inFile.open(strFNameTestImageDB.c_str());
	if(!inFile.is_open())
	{
		cout << "Can't open file " << strFNameTestImageDB << endl;
		exit(EXIT_FAILURE);
	}
	string::size_type idxTab; // A position of '\t'.
	string strFNameTestImage;
	count = 0; // Reset count to 0.
	int iCorrect = 0,iTotal = 0;

	while(getline(inFile,strOneLine))
	{
		idxTab = strOneLine.find('\t');
		if(idxTab == string::npos)
		{
			cout << strOneLine << " doesn't have TAB\n";
			exit(EXIT_FAILURE);
		}
		strFNameTestImage = strOneLine.substr(0,idxTab);
		answer[count] = atoi(strOneLine.substr(idxTab+1).c_str());

		string::size_type idxDot; // A position of '.'.
		idxDot = strFNameTestImage.find_last_of('.');
		string strFNameMatch = strDirTestImageDB + strFNameTestImage.substr(0,idxDot) + "M" + ".txt";
		inMFile.open(strFNameMatch.c_str());
		if(!inFile.is_open())
		{
			cout << "Can't open file " << strFNameMatch << endl;
			exit(EXIT_FAILURE);
		}
		while(getline(inMFile,strOneLine))
		{
			idxUnderScroll = strOneLine.find('_');
			strBaseName = strOneLine.substr(0,idxUnderScroll);

			bFound = false;
			for(pos = rank.begin(); pos != rank.end(); pos++)
			{
				if(*pos == strBaseName)
					bFound = true;
			}
			if(!bFound)
				rank.push_back(strBaseName);
		}
		inMFile.close();
		inMFile.clear();

		for(int i=0;i<TOPN;i++)
		{
			if(answer[count] == atoi(rank[i].c_str()))
				iCorrect++;
		}
		iTotal++;
		count++;
	}
	inFile.close();
	
	float fAccuracy = (float)iCorrect/(float)iTotal*100.0F;

	outFile.open(strOutputRankFile.c_str());
	if(!outFile.is_open())
	{
		cout << "Can't open file " << strOutputRankFile << endl;
		exit(EXIT_FAILURE);
	}
	outFile << fAccuracy;
	outFile.close();
	cout << endl << fAccuracy << " %";

	// Timer stop.
	clock_t tmStop = clock();
	cout << endl << "Time for showRank = " << (tmStop - tmStart)/CLOCKS_PER_SEC
		<< " sec" << endl;
	// cout << "Press enter to exit.";

	//getchar();
	return 0;
}