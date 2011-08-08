// C++ header.
// ===========
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>

// OpenCV header.
// ==============
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\features2d\features2d.hpp>

using namespace std;
using namespace cv;

typedef vector<KeyPoint> vecKey;

int main()
{
	// Timer start.
	clock_t tmStart = clock();
	
	string strDirFlowerDB = "flowerPicDB\\";
	string strDirDescriptionDB = "descriptionDB\\";
	string strFNameFlowerDB = strDirFlowerDB + "files.txt";
	string strFNameFlower;
	string strFNameDesc;
	ifstream inFile;
	int count = 0;
	int position = 0;

	SurfFeatureDetector surf(2500.);
	SurfDescriptorExtractor surfDesc;

	// Find number of photos.
	// ======================
	inFile.open(strFNameFlowerDB.c_str());
	if(!inFile.is_open()) cout << "Can't open file " << strFNameFlowerDB;
	while(inFile >> strFNameFlower)
	{
		count++;
	}
	inFile.close();
	inFile.clear(); // If not clear() before it will make second while command finished
					// immediately.
	cout << "Number of flower photo = " << count << endl << endl;

	// Extract SURF feature of all photos in the DB.
	// =============================================
	Mat *imgFlowerDB = new Mat[count];
	vecKey *keypointDB = new vecKey[count];
	Mat *descriptorDB = new Mat[count];
	FileStorage outDescFile;
	count = 0;

	inFile.open(strFNameFlowerDB.c_str());
	if(!inFile.is_open()) cout << "Can't open file " << strFNameFlowerDB;
	while(inFile >> strFNameFlower)
	{
		cout << "Read  " << strFNameFlower << endl;
		imgFlowerDB[count] = imread(strDirFlowerDB + strFNameFlower);
		surf.detect(imgFlowerDB[count],keypointDB[count]);
		surfDesc.compute(imgFlowerDB[count],keypointDB[count],descriptorDB[count]);

		// Write description.
		// Name each file name.
		strFNameDesc = strFNameFlower;
		position = strFNameDesc.find("jpg",0);
		strFNameDesc.replace(position,3,"yml");

		cout << "Write " << strFNameDesc << endl;
		outDescFile.open(strDirDescriptionDB + strFNameDesc,FileStorage::WRITE);
		if(!outDescFile.isOpened()) cout << "Can't open file " << strFNameDesc;
		outDescFile << "descriptionOfPic" << descriptorDB[count];
		outDescFile.release();

		count++;
	}
	inFile.close();

	// Timer stop.
	clock_t tmStop = clock();
	cout << endl << "Total using time = " << (tmStop - tmStart)/CLOCKS_PER_SEC
		<< " sec" << endl << "Pass enter to exit.";

	getchar();
	return 0;
}