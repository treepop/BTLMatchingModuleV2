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

// User header.
// ============
#include "..\matchingDesc\beeConfig.h"
#include "..\matchingDesc\jkImageProcessingClass.h"

using namespace std;
using namespace cv;

typedef vector<KeyPoint> vecKey;

// Function prototype.
// ===================
void showHowToUse(const char *);

int main(int argc, char *argv[])
{
	// Timer start.
	clock_t tmStart = clock();

	// Argument variable.
	// ==================
	bool bUseColorFeature = false;
	int iColorFeature = 0;

	bool bUseShapeFeature = false;
	int iShapeFeature = 0;
	// ==================

	// Checking correctness of all arguments.
	// ======================================
	if(argc == 1)
	{
		showHowToUse(argv[0]);
	}

	if(argc > 3)
	{
		cout << "You gave exceeded number of arguments.\n";
		exit(1);
	}

	for(int i=1;i<argc;i++)
	{
		string strTemp = argv[i];

		if(strTemp.at(0) != '/')
			showHowToUse(argv[0]);

		if(strTemp.at(1) == 'c' && strTemp.at(2) == '=')
		{
			bUseColorFeature = true;
			iColorFeature = atoi(strTemp.substr(3).c_str());
		}

		if(strTemp.at(1) == 's' && strTemp.at(2) == '=')
		{
			bUseShapeFeature = true;
			iShapeFeature = atoi(strTemp.substr(3).c_str());
		}
	}
	// ======================================
	
	string strDirFlowerDB = "flowerPicDB\\"; // The folder of flower images.
	string strDirDescriptionDB = "descriptionDB\\"; // The folder of features output.
	string strFNameFlowerDB = strDirFlowerDB + "files.txt"; // List of flowers' name.
	string strFNameFlower; // Indexer.
	string strFNameDesc; // Indexer.
	string strFNameDescTemp;
	ifstream inFile;
	int count = 0; // Number of image files.
	string::size_type idx; // A position of '.'.

	SurfFeatureDetector surf(2500.);
	SurfDescriptorExtractor surfDesc;
	HistogramHSV hsvObj;

	// Find number of photos.
	// ======================
	inFile.open(strFNameFlowerDB.c_str());
	if(!inFile.is_open()) cout << "Can't open file " << strFNameFlowerDB << endl;
	while(inFile >> strFNameFlower)
	{
		count++;
	}
	inFile.close();
	inFile.clear(); // This must be call clear() before it will be made second call
					// otherwise the command will finished immediately.

	cout << "Number of flower photo = " << count << endl << endl;

	// Extract features of all photos in the DB.
	// =========================================
	// Array of image in DB.
	Mat *imgFlowerDB = new Mat[count];
	// Array of shape feature.
	vecKey *keypointDB;
	Mat *descriptorDB;
	if(bUseShapeFeature)
	{
		keypointDB = new vecKey[count];
		descriptorDB = new Mat[count];
	}
	// Array of colour feature.
	MatND *hueHistogram;
	MatND *saturationHistogram;
	MatND *valueHistogram;
	if(bUseColorFeature)
	{
		hueHistogram = new MatND[count];
		saturationHistogram = new MatND[count];
		valueHistogram = new MatND[count];
	}
	// File pointer of output file.
	FileStorage outDescFileSurf,outDescFileH,outDescFileS,outDescFileV;
	count = 0; // Reset count to 0.

	inFile.open(strFNameFlowerDB.c_str());
	if(!inFile.is_open()) cout << "Can't open file " << strFNameFlowerDB << endl;
	while(inFile >> strFNameFlower)
	{
		cout << "Read  " << strFNameFlower << endl;
		// Read all flower photos to array.
		// ================================
		imgFlowerDB[count] = imread(strDirFlowerDB + strFNameFlower);
		// Extract shape feature.
		// ======================
		if(bUseShapeFeature)
		{
			surf.detect(imgFlowerDB[count],keypointDB[count]);
			surfDesc.compute(imgFlowerDB[count],keypointDB[count],descriptorDB[count]);
		}
		// Extract colour feature.
		// =======================
		if(bUseColorFeature)
		{
			cv::Mat imgTempROI = getCenterOfImage(imgFlowerDB[count],iColorFeature);
			hueHistogram[count] = hsvObj.getHueHistogram(imgTempROI);
			saturationHistogram[count] = hsvObj.getSaturationHistogram(imgTempROI);
			valueHistogram[count] = hsvObj.getValueHistogram(imgTempROI);
		}

		// Write all description.
		// ======================
		strFNameDesc = strFNameFlower;
		/* position = strFNameDesc.find("jpg",0);
		strFNameDesc.replace(position,3,"yml"); */
		idx = strFNameDesc.find('.');
		strFNameDesc = strFNameDesc.substr(0,idx);

		// Write shape feature.
		// --------------------
		if(bUseShapeFeature)
		{
			strFNameDescTemp = strFNameDesc + "Surf" + ".yml";
			cout << "Write " << strFNameDescTemp << endl;
			outDescFileSurf.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::WRITE);
			if(!outDescFileSurf.isOpened()) cout << "Can't open file " << strFNameDescTemp
				<< endl;
			outDescFileSurf << "descriptionSurfOfPic" << descriptorDB[count];
			cout << "Number of vector = " << descriptorDB[count].rows << endl;
			outDescFileSurf.release();
		}

		// Write colour feature.
		// ---------------------
		if(bUseColorFeature)
		{
			strFNameDescTemp = strFNameDesc + "H" + ".yml";
			cout << "Write " << strFNameDescTemp << endl;
			outDescFileH.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::WRITE);
			if(!outDescFileH.isOpened()) cout << "Can't open file " << strFNameDescTemp
				<< endl;
			outDescFileH << "descriptionHOfPic" << hueHistogram[count];
			outDescFileH.release();

			strFNameDescTemp = strFNameDesc + "S" + ".yml";
			cout << "Write " << strFNameDescTemp << endl;
			outDescFileS.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::WRITE);
			if(!outDescFileS.isOpened()) cout << "Can't open file " << strFNameDescTemp
				<< endl;
			outDescFileS << "descriptionSOfPic" << saturationHistogram[count];
			outDescFileS.release();

			strFNameDescTemp = strFNameDesc + "V" + ".yml";
			cout << "Write " << strFNameDescTemp << endl;
			outDescFileV.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::WRITE);
			if(!outDescFileV.isOpened()) cout << "Can't open file " << strFNameDescTemp
				<< endl;
			outDescFileV << "descriptionVOfPic" << valueHistogram[count];
			outDescFileV.release();
		}

		count++;
	}
	inFile.close();

	// Timer stop.
	clock_t tmStop = clock();
	cout << endl << "Time for extractDesc = " << float(tmStop - tmStart)/CLOCKS_PER_SEC
		<< " sec" << endl;
	/*cout << "Press enter to exit.";
	getchar();*/

	return 0;
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
	cout << strFNameOnly << " /c=100\n";
	cout << strFNameOnly << " /s=5\n";
	cout << strFNameOnly << " /c=100 /s=5\n";
	cout << endl;
	cout << "/c  = Length of radius that used in extracting color feature.\n";
	cout << "/s  = Number of vectors of shape feature.\n";
	exit(1);
}