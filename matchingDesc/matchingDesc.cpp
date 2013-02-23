// Option for this program.
// ========================

// 1.)
// Uncomment this for use on Android.
// #define _USE_ON_ANDROID

// 2.)
// In file Matches.txt,define _JKDEBUG if you want to write sum of distance.
// Ex.	0008_4.jpg	120991	18	1.49365	0	18
//		0032_3.jpg	114101	9	1.74578	13	22
//
//		0008_4.jpg	= File name of flower template.
//		120991		= Sum of HSV distance.
//		18			= Rank by HSV color feature.
//		1.49365		= Sum of Surf distance.
//		0			= Rank by Surf shape feature.
//		18			= Rank by both color and shape features.
//		Each fields was separated by TAB.
#define _JKDEBUG
// ========================

// C++ header.
// ===========
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#ifndef _USE_ON_ANDROID
#include <ctime>
#endif

// OpenCV header.
// ==============
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\features2d\features2d.hpp>

// JNI header.
// ===========
#ifdef _USE_ON_ANDROID
#include "com_img_jk_beethelion_MatchingLib.h"
#endif

// User header.
// ============
#include "beeConfig.h"
#include "jkImageProcessingClass.h"
#ifndef _USE_ON_ANDROID
#include "..\batchRun\jkFunction.h"
#endif

using namespace std;
using namespace cv;

typedef vector<KeyPoint> vecKey;
typedef vector<DMatch> vecDMatch;

// Global variable.
// ================

bool bUseColorFeature = false;
int iColorFeature = CENTER;

bool bUseWeightColor = false;
float fWeightColor = 0.5F;

bool bUseShapeFeature = false;
int iShapeFeature = NUMTOPSMALL; // Keep only top 25 lowest distance.

bool bUseWeightShape = false;
float fWeightShape = 0.5F;
// ================

struct disStruct
{
	string strFNameOfPhoto;
	double distanceOfHSV;
	int rankOfHSV;
	float distanceOfSurf;
	int rankOfSurf;
	int distanceSumOfSurfAndColor;
};

struct outPutStruct
{
	string matchesPath;
	int numVec;
} outPut;

ofstream outFile; // Use together with function ShowResult.

// Function prototype.
// ===================
bool worseThan(const disStruct & r1,const disStruct & r2);
void WriteResult(const disStruct & rr);
bool sortByColor(const disStruct & r1,const disStruct & r2);
bool sortByShape(const disStruct & r1,const disStruct & r2);
void showHowToUse(const char *);

#ifdef _USE_ON_ANDROID
JNIEXPORT void JNICALL Java_com_img_jk_beethelion_MatchingLib_jkMatching
	(JNIEnv *env, jclass obj, jstring jStrRootProgram)
#else
int main(int argc, char *argv[])
#endif
{
#ifdef _USE_ON_ANDROID
	// Convert JAVA string to UTF-8.
	const char *pStr = env->GetStringUTFChars(jStrRootProgram,NULL);
	string strRootProgram(pStr);
	env->ReleaseStringUTFChars(jStrRootProgram,pStr);
#else
	// Checking correctness of all arguments.
	// ======================================
	if(argc == 1)
	{
		showHowToUse(argv[0]);
	}

	if(argc > 6) // matchingDesc.exe /c=300 /wc=.5 /s=7 /ws=.5 unknownFlower.jpg
	{			 //         0          1       2     3     4          5
		cout << "Argument exceed 5 parameter.\n";
		exit(EXIT_FAILURE);
	}

	string strRootProgram;
	/*if(argc == 2)
	{
		strRootProgram = argv[1];
		strRootProgram += "/";
	}*/

	for(int i=1;i<argc-1;i++) // It does not process the last argument.
	{
		string strTemp = argv[i];

		if(strTemp.at(0) != '/')
			showHowToUse(argv[0]);

		if(strTemp.at(1) == 'c' && strTemp.at(2) == '=')
		{
			bUseColorFeature = true;
			iColorFeature = atoi(strTemp.substr(3).c_str());
		}

		if(strTemp.at(1) == 'w' && strTemp.at(2) == 'c' && strTemp.at(3) == '=')
		{
			bUseWeightColor = true;
			fWeightColor = (float)atof(strTemp.substr(4).c_str());
		}
		
		if(strTemp.at(1) == 's' && strTemp.at(2) == '=')
		{
			bUseShapeFeature = true;
			iShapeFeature = atoi(strTemp.substr(3).c_str());
		}

		if(strTemp.at(1) == 'w' && strTemp.at(2) == 's' && strTemp.at(3) == '=')
		{
			bUseWeightShape = true;
			fWeightShape = (float)atof(strTemp.substr(4).c_str());
		}
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

	if(bUseWeightColor && bUseWeightShape)
	{
		if((fWeightColor + fWeightShape) != 1.0) // May be you can write 1 instead of 1.0 .
		{
			cout << "Your weight color and shape don't sum to 1.0 .\n";
			exit(1);
		}
	}
	// ======================================

	// Timer start.
	clock_t tmStart = clock();
#endif

	// In android edit these strings.
	// You can write either flowerPicDB\\ or flowerPicDB/ if it ran on windows.
	// But you must write flowerPicDB/ if you ran on Android.
	// Otherwise it cause of an error.
	string strDirFlowerDB = strRootProgram + "flowerPicDB/";
	string strDirDescriptionDB = strRootProgram + "descriptionDB/";
	string strFNameFlowerDB = strDirFlowerDB + "files.txt";
	string strFNameFlower;
	string strFNameDesc;
#ifdef _USE_ON_ANDROID
	string strFNameUnknownFlower = strRootProgram + "unknownFlower.jpg";
#else
	string strDirTestImageDB = "testImage\\"; // The folder of test image.
	string strFNameUnknownFlower = strRootProgram + strDirTestImageDB + argv[argc-1];
	cout << endl << argv[argc-1] << endl;
#endif
	string strFNameDescTemp;
	// ------------------------------

	ifstream inFile;
	int count = 0;
	string::size_type idx;

	Mat imgUnknownFlower;
	imgUnknownFlower = imread(strFNameUnknownFlower);

	// Extract SURF feature of unknown flower.
	// =======================================
	vecKey keypointOfUnknownFlower;
	Mat descriptorOfUnknowFlower;
	SurfFeatureDetector surf(2500.);
	SurfDescriptorExtractor surfDesc;
	surf.detect(imgUnknownFlower,keypointOfUnknownFlower);
	surfDesc.compute(imgUnknownFlower,keypointOfUnknownFlower,descriptorOfUnknowFlower);

	// For debug.
	outPut.numVec = descriptorOfUnknowFlower.rows;

	// Extract color feature of unknown flower.
	// ========================================
	HistogramHSV hsvObj;
	cv::Mat imgTempROIOfUnknownFlower = getCenterOfImage(imgUnknownFlower,iColorFeature);
	MatND hueHistogramOfUnknownFlower = hsvObj.getHueHistogram(imgTempROIOfUnknownFlower);
	MatND saturationHistogramOfUnknownFlower = hsvObj.getSaturationHistogram(
												imgTempROIOfUnknownFlower);
	MatND valueHistogramOfUnknownFlower = hsvObj.getValueHistogram(
											imgTempROIOfUnknownFlower);

#ifdef _USE_ON_ANDROID
	// Get the class.
	jclass class_MatchingLib = env->FindClass("com/img/jk/beethelion/MatchingLib");

	// Get the field id.
	jfieldID id_numVec = env->GetStaticFieldID(class_MatchingLib,"numVec","I");

	// Set the data value to the field.
	env->SetStaticIntField(class_MatchingLib,id_numVec,outPut.numVec);
#endif

	// Find number of photos.
	// ======================
	inFile.open(strFNameFlowerDB.c_str());
#ifndef _USE_ON_ANDROID
	if(!inFile.is_open())
	{
		cout << "Can't open file " << strFNameFlowerDB << endl;
		exit(EXIT_FAILURE);
	}
#endif
	while(inFile >> strFNameFlower)
	{
		count++;
	}
	inFile.close();
	inFile.clear(); // If not clear() before it will make second otherwise command finished
					// immediately.
#ifndef _USE_ON_ANDROID
	// cout << "Number of flower photo = " << count << endl << endl;
#endif

	// Read all description from DB.
	// =============================
	// Array of shape feature.
	Mat *descriptorDB = new Mat[count];
	// Array of colour feature.
	MatND *hueHistogram = new MatND[count];
	MatND *saturationHistogram = new MatND[count];
	MatND *valueHistogram = new MatND[count];

	BruteForceMatcher<L2<float> > matcher;
	vecDMatch *resultOfMatchDB = new vecDMatch[count]; // Similarity between unknown and 
													   // each flowers in DB.
	ImageComparator imgCompObj;
	vector<disStruct> similarity; // The less distance the more similarity.
	disStruct temp;
	temp.strFNameOfPhoto = "";
	temp.distanceOfHSV = 0;
	temp.rankOfHSV = 0;
	temp.distanceOfSurf = 0;
	temp.rankOfSurf = 0;
	temp.distanceSumOfSurfAndColor = 0;
	FileStorage inDescFileSurf,inDescFileH,inDescFileS,inDescFileV;
	count = 0;
	inFile.open(strFNameFlowerDB.c_str());
#ifndef _USE_ON_ANDROID
	if(!inFile.is_open())
	{
		cout << "Can't open file " << strFNameFlowerDB << endl;
		exit(EXIT_FAILURE);
	}
#endif
	while(inFile >> strFNameFlower)
	{
		// Read SURF description from DB.
		// Find each name of yml files.
		strFNameDesc = strFNameFlower;
		idx = strFNameDesc.find('.');
		strFNameDesc = strFNameDesc.substr(0,idx);

		// Read shape feature from DB.
		// ===========================
		strFNameDescTemp = strFNameDesc + "Surf" + ".yml";
#ifndef _USE_ON_ANDROID
		// cout << "Read  " << strFNameDescTemp << endl;
#endif
		inDescFileSurf.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::READ);
#ifndef _USE_ON_ANDROID
		if(!inDescFileSurf.isOpened())
		{
			cout << "Can't open file " << strFNameDescTemp << endl;
			exit(EXIT_FAILURE);
		}
#endif
#ifdef _USE_ON_ANDROID
		inDescFileSurf["descriptionSurfOfPic"] >> descriptorDB[count];
#else
		if(bUseShapeFeature)
			inDescFileSurf["descriptionSurfOfPic"] >> descriptorDB[count];
#endif
		inDescFileSurf.release();

		// Read color feature (hue) from DB.
		// =================================
		strFNameDescTemp = strFNameDesc + "H" + ".yml";
#ifndef _USE_ON_ANDROID
		// cout << "Read  " << strFNameDescTemp << endl;
#endif
		inDescFileH.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::READ);
#ifndef _USE_ON_ANDROID
		if(!inDescFileH.isOpened())
		{
			cout << "Can't open file " << strFNameDescTemp << endl;
			exit(EXIT_FAILURE);
		}
#endif
#ifdef _USE_ON_ANDROID
		inDescFileH["descriptionHOfPic"] >> hueHistogram[count];
#else
		if(bUseColorFeature)
			inDescFileH["descriptionHOfPic"] >> hueHistogram[count];
#endif
		inDescFileH.release();

		// Read color feature (saturation) from DB.
		// ========================================
		strFNameDescTemp = strFNameDesc + "S" + ".yml";
#ifndef _USE_ON_ANDROID
		// cout << "Read  " << strFNameDescTemp << endl;
#endif
		inDescFileS.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::READ);
#ifndef _USE_ON_ANDROID
		if(!inDescFileS.isOpened())
		{
			cout << "Can't open file " << strFNameDescTemp << endl;
			exit(EXIT_FAILURE);
		}
#endif
#ifdef _USE_ON_ANDROID
		inDescFileS["descriptionSOfPic"] >> saturationHistogram[count];
#else
		if(bUseColorFeature)
			inDescFileS["descriptionSOfPic"] >> saturationHistogram[count];
#endif
		inDescFileS.release();

		// Read color feature (value) from DB.
		// ===================================
		strFNameDescTemp = strFNameDesc + "V" + ".yml";
#ifndef _USE_ON_ANDROID
		// cout << "Read  " << strFNameDescTemp << endl;
#endif
		inDescFileV.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::READ);
#ifndef _USE_ON_ANDROID
		if(!inDescFileV.isOpened())
		{
			cout << "Can't open file " << strFNameDescTemp << endl;
			exit(EXIT_FAILURE);
		}
#endif
#ifdef _USE_ON_ANDROID
		inDescFileV["descriptionVOfPic"] >> valueHistogram[count];
#else
		if(bUseColorFeature)
			inDescFileV["descriptionVOfPic"] >> valueHistogram[count];
#endif
		inDescFileV.release();
		
		// Matching section.
		// =================
		// Matching each description point of unknown flower with each photo in DB.

		temp.strFNameOfPhoto = strFNameFlower;

		// Calculate similarity from shape features.
		// =========================================
#ifdef _USE_ON_ANDROID
		matcher.match(descriptorOfUnknowFlower,descriptorDB[count]
			,resultOfMatchDB[count]);
		// Sort and get top 25 lowest distance vector.
		// In 25 numbers don't sort but not have a problem.
		nth_element(resultOfMatchDB[count].begin()
			,resultOfMatchDB[count].begin()+iShapeFeature-1
			,resultOfMatchDB[count].end());
		resultOfMatchDB[count].erase(resultOfMatchDB[count].begin()+iShapeFeature
			,resultOfMatchDB[count].end());
		// Sum all 25 distance. if which photo has less number more similarity.
		float sumShape = 0;
		for(int i=0;i<iShapeFeature;i++)
		{
			sumShape += resultOfMatchDB[count][i].distance;
		}
		temp.distanceOfSurf = sumShape;
#else
		if(bUseShapeFeature)
		{
			matcher.match(descriptorOfUnknowFlower,descriptorDB[count]
				,resultOfMatchDB[count]);
			// Sort and get top 25 lowest distance vector.
			// In 25 numbers don't sort but not have a problem.
			nth_element(resultOfMatchDB[count].begin()
				,resultOfMatchDB[count].begin()+iShapeFeature-1
				,resultOfMatchDB[count].end());
			resultOfMatchDB[count].erase(resultOfMatchDB[count].begin()+iShapeFeature
				,resultOfMatchDB[count].end());
			// Sum all 25 distance. if which photo has less number more similarity.
			float sumShape = 0;
			for(int i=0;i<iShapeFeature;i++)
			{
				sumShape += resultOfMatchDB[count][i].distance;
			}
			temp.distanceOfSurf = sumShape;
		}
#endif

		// Calculate similarity from color features.
		// =========================================
#ifdef _USE_ON_ANDROID
		imgCompObj.setRefHistogram(hueHistogram[count],
									saturationHistogram[count],
									valueHistogram[count]);
		temp.distanceOfHSV = imgCompObj.compareWithHist(hueHistogramOfUnknownFlower,
														saturationHistogramOfUnknownFlower,
														valueHistogramOfUnknownFlower);
#else
		if(bUseColorFeature)
		{
			imgCompObj.setRefHistogram(hueHistogram[count],
										saturationHistogram[count],
										valueHistogram[count]);
			temp.distanceOfHSV = imgCompObj.compareWithHist(hueHistogramOfUnknownFlower,
															saturationHistogramOfUnknownFlower,
															valueHistogramOfUnknownFlower);
		}
#endif

		// Sum of shape and color feature.
		// ===============================
		/* temp.distanceSumOfSurfAndColor = static_cast<double>(temp.distanceOfSurf) +
											temp.distanceOfHSV; */
		// Because distanceOfSurf is float but distanceOfHSV is double.

		similarity.push_back(temp);

		count++;
	}
	inFile.close();

#ifdef _USE_ON_ANDROID
	int choice = FEATURE;
#else
	int choice = 0;
	if((bUseColorFeature == true) && (bUseShapeFeature == false))
		choice = 1;
	if((bUseColorFeature == false) && (bUseShapeFeature == true))
		choice = 2;
	if((bUseColorFeature == true) && (bUseShapeFeature == true))
		choice = 3;
#endif

	if(choice == 3)
	{
		int max = similarity.size();
		
		// Get order by color feature.
		sort(similarity.begin(),similarity.end(),sortByColor);
		for(int i=0;i<max;i++)
			similarity[i].rankOfHSV = ((i/10)+1)*100;
		// Get order by shape feature.
		sort(similarity.begin(),similarity.end(),sortByShape);
		for(int i=0;i<max;i++)
			similarity[i].rankOfSurf = i+1;
		for(int i=0;i<max;i++)
			/* Old equation
			similarity[i].distanceSumOfSurfAndColor = 
				similarity[i].rankOfHSV * (int)(fWeightShape * 100.0F)
				+ similarity[i].rankOfSurf * (int)(fWeightColor * 100.0F); */

			similarity[i].distanceSumOfSurfAndColor = 
				similarity[i].rankOfHSV + similarity[i].rankOfSurf;
	}
	
	// Sort list which lowest on the top indicate more similarity.
	sort(similarity.begin(),similarity.end(),worseThan);

	// Write similarity list to a file.
#ifdef _USE_ON_ANDROID
	strRootProgram += "Matches.txt";
	outFile.open(strRootProgram.c_str());
#else
	string::size_type idxDot; // A position of '.'.
	idxDot = strFNameUnknownFlower.find_last_of('.');
	string strFNameMatch = strFNameUnknownFlower.substr(0,idxDot) + "M" + ".txt";
	outFile.open(strFNameMatch.c_str());
	if(!outFile.is_open())
	{
		cout << "Can't open file " << "Matches.txt" << endl;
		exit(EXIT_FAILURE);
	}
#endif
	for_each(similarity.begin(),similarity.end(),WriteResult);
	outFile.close();

#ifndef _USE_ON_ANDROID
	cout << "Number of vector = " << outPut.numVec << endl;
	// Timer stop.
	clock_t tmStop = clock();
	cout << "MatchingDesc time for this file = " << float(tmStop - tmStart)/CLOCKS_PER_SEC
		<< " sec" << endl;
	// cout << "Press enter to exit.";
#endif

	// Clear memory for memory leak correction.
	// ========================================
	imgUnknownFlower.release();
	keypointOfUnknownFlower.clear();
	descriptorOfUnknowFlower.release();

	imgTempROIOfUnknownFlower.release();
	hueHistogramOfUnknownFlower.release();
	saturationHistogramOfUnknownFlower.release();
	valueHistogramOfUnknownFlower.release();

	if(descriptorDB != NULL)
	{
		delete [] descriptorDB;
		descriptorDB = NULL;
	}
	if(hueHistogram != NULL)
	{
		delete [] hueHistogram;
		hueHistogram = NULL;
	}
	if(saturationHistogram != NULL)
	{
		delete [] saturationHistogram;
		saturationHistogram = NULL;
	}
	if(valueHistogram != NULL)
	{
		delete [] valueHistogram;
		valueHistogram = NULL;
	}
	matcher.clear();
	if(resultOfMatchDB != NULL)
	{
		delete [] resultOfMatchDB;
		resultOfMatchDB = NULL;
	}
	imgCompObj.clearMem();
	similarity.clear();
	// ========================================

#ifdef _USE_ON_ANDROID
	outPut.matchesPath = strRootProgram;
	
	// Get the field id.
	jfieldID id_matchesPath = env->GetStaticFieldID(class_MatchingLib,"matchesPath","Ljava/lang/String;");

	// Set the data value to the field.
	jstring jStr = env->NewStringUTF(outPut.matchesPath.c_str());
	env->SetStaticObjectField(class_MatchingLib,id_matchesPath,jStr);
#else
	// getchar();
	return 0;
#endif
}

bool worseThan(const disStruct & r1,const disStruct & r2)
{
#ifdef _USE_ON_ANDROID
	int choice = FEATURE;
#else
	int choice = 0;
	if((bUseColorFeature == true) && (bUseShapeFeature == false))
		choice = 1;
	if((bUseColorFeature == false) && (bUseShapeFeature == true))
		choice = 2;
	if((bUseColorFeature == true) && (bUseShapeFeature == true))
		choice = 3;
#endif

	// Sort by color feature.
	if(choice == 1)
	{
		if(r1.distanceOfHSV < r2.distanceOfHSV)
			return true;
		else
			return false;
	}

	// Sort by shape feature.
	if(choice == 2)
	{
		if(r1.distanceOfSurf < r2.distanceOfSurf)
			return true;
		else
			return false;
	}

	// Sort by both color and shape feature.
	if(choice == 3)
	{
		if(r1.distanceSumOfSurfAndColor < r2.distanceSumOfSurfAndColor)
			return true;
		else
			return false;
	}
}

bool sortByColor(const disStruct & r1,const disStruct & r2)
// Sort by color feature.
{
	if(r1.distanceOfHSV < r2.distanceOfHSV)
		return true;
	else
		return false;
}

bool sortByShape(const disStruct & r1,const disStruct & r2)
// Sort by shape feature.
{
	if(r1.distanceOfSurf < r2.distanceOfSurf)
		return true;
	else
		return false;
}

void WriteResult(const disStruct & rr)
{
#ifdef _JKDEBUG
	outFile << rr.strFNameOfPhoto << "\t" << rr.distanceOfHSV << "\t" << rr.rankOfHSV
									<< "\t" << rr.distanceOfSurf << "\t" << rr.rankOfSurf
									<< "\t" << rr.distanceSumOfSurfAndColor
									<< endl;
#else
	outFile << rr.strFNameOfPhoto << endl;
#endif
}

/*
#ifdef _USE_ON_ANDROID
JNIEXPORT void JNICALL Java_com_img_jk_beethelion_MatchingLib_jkMatching
  (JNIEnv *env, jclass obj, jstring jStrRootProgram)
{
	return jkMatching(jStrRootProgram);
}
#endif
*/

#ifndef _USE_ON_ANDROID
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
	cout << strFNameOnly << " /c=99 unknownFlower0099.jpg\n";
	cout << strFNameOnly << " /c=100 /s=5 unknownFlower0099.jpg\n";
	cout << strFNameOnly << " /c=300 /wc=.5 /s=7 /ws=.5 unknownFlower0099.jpg\n";
	cout << endl;
	cout << "/c  = Length of radius that used in extracting color feature.\n";
	cout << "/wc = Weight of color feature.\n";
	cout << "/s  = Number of vectors of shape feature.\n";
	cout << "/ws = Weight of shape feature.\n";
	cout << "unknownFlower0099.jpg = File in testImage folder.\n";
	exit(1);
}
#endif