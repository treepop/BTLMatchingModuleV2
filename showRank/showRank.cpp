// C++ header.
// ===========
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>

using namespace std;

int main()
{
	// Timer start.
	clock_t tmStart = clock();
	
	string strInputDistanceFile = "Matches.txt";
	string strOutputRankFile = "Rank.txt";
	string strBaseName;
	string strOneLine;
	ifstream inFile;
	ofstream outFile;
	vector<string> rank;
	string::size_type idx; // A position of '_'.
	vector<string>::iterator pos;
	bool bFound;
	int iLineNumber = 0;

	inFile.open(strInputDistanceFile.c_str());
	if(!inFile.is_open())
	{
		cout << "Can't open file " << strInputDistanceFile << endl;
		exit(EXIT_FAILURE);
	}
	while(getline(inFile,strOneLine))
	{
		idx = strOneLine.find('_');
		strBaseName = strOneLine.substr(0,idx);

		bFound = false;
		for(pos = rank.begin(); pos != rank.end(); pos++)
		{
			if(*pos == strBaseName)
				bFound = true;
		}
		if(!bFound)
			rank.push_back(strBaseName);
	}
	inFile.close();

	outFile.open(strOutputRankFile.c_str());
	if(!outFile.is_open())
	{
		cout << "Can't open file " << strOutputRankFile << endl;
		exit(EXIT_FAILURE);
	}
	for(pos = rank.begin(); pos != rank.end(); pos++)
	{
		iLineNumber++;
		outFile << iLineNumber << '\t' << *pos << endl;
	}
	outFile.close();

	// Timer stop.
	clock_t tmStop = clock();
	cout << endl << "Total using time = " << (tmStop - tmStart)/CLOCKS_PER_SEC
		<< " sec" << endl << "Press enter to exit.";

	//getchar();
	return 0;
}