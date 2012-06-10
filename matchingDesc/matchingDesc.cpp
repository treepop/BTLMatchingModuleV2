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

using namespace std;
using namespace cv;

typedef vector<KeyPoint> vecKey;
typedef vector<DMatch> vecDMatch;

const int numTopSmall = NUMTOPSMALL; // Keep only top 25 lowest distance.

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

bool worseThan(const disStruct & r1,const disStruct & r2);
void ShowResult(const disStruct & rr);
bool sortByColor(const disStruct & r1,const disStruct & r2);
bool sortByShape(const disStruct & r1,const disStruct & r2);

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
	if(argc > 2)
	{
		cout << "Argument exceed 1 parameter.\n";
		exit(EXIT_FAILURE);
	}

	string strRootProgram;
	if(argc == 2)
	{
		strRootProgram = argv[1];
		strRootProgram += "/";
	}

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
	string strFNameUnknownFlower = strRootProgram + "unknownFlower.jpg";
	string strFNameDescTemp;
	// ------------------------------

	ifstream inFile;
	int count = 0;
	string::size_type idx;

	// Extract SURF feature of unknown flower.
	// =======================================
	Mat imgUnknownFlower;
	vecKey keypointOfUnknownFlower;
	Mat descriptorOfUnknowFlower;
	SurfFeatureDetector surf(2500.);
	SurfDescriptorExtractor surfDesc;
	imgUnknownFlower = imread(strFNameUnknownFlower);
	surf.detect(imgUnknownFlower,keypointOfUnknownFlower);
	surfDesc.compute(imgUnknownFlower,keypointOfUnknownFlower,descriptorOfUnknowFlower);

	// For debug.
	outPut.numVec = descriptorOfUnknowFlower.rows;

	// Extract color feature of unknown flower.
	// ========================================
	HistogramHSV hsvObj;
	cv::Mat imgTempROIOfUnknownFlower = getCenterOfImage(imgUnknownFlower,CENTER);
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
	cout << "Number of flower photo = " << count << endl << endl;
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
		cout << "Read  " << strFNameDescTemp << endl;
#endif
		inDescFileSurf.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::READ);
#ifndef _USE_ON_ANDROID
		if(!inDescFileSurf.isOpened())
		{
			cout << "Can't open file " << strFNameDescTemp << endl;
			exit(EXIT_FAILURE);
		}
#endif
		inDescFileSurf["descriptionSurfOfPic"] >> descriptorDB[count];
		inDescFileSurf.release();

		// Read color feature (hue) from DB.
		// =================================
		strFNameDescTemp = strFNameDesc + "H" + ".yml";
#ifndef _USE_ON_ANDROID
		cout << "Read  " << strFNameDescTemp << endl;
#endif
		inDescFileH.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::READ);
#ifndef _USE_ON_ANDROID
		if(!inDescFileH.isOpened())
		{
			cout << "Can't open file " << strFNameDescTemp << endl;
			exit(EXIT_FAILURE);
		}
#endif
		inDescFileH["descriptionHOfPic"] >> hueHistogram[count];
		inDescFileH.release();

		// Read color feature (saturation) from DB.
		// ========================================
		strFNameDescTemp = strFNameDesc + "S" + ".yml";
#ifndef _USE_ON_ANDROID
		cout << "Read  " << strFNameDescTemp << endl;
#endif
		inDescFileS.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::READ);
#ifndef _USE_ON_ANDROID
		if(!inDescFileS.isOpened())
		{
			cout << "Can't open file " << strFNameDescTemp << endl;
			exit(EXIT_FAILURE);
		}
#endif
		inDescFileS["descriptionSOfPic"] >> saturationHistogram[count];
		inDescFileS.release();

		// Read color feature (value) from DB.
		// ===================================
		strFNameDescTemp = strFNameDesc + "V" + ".yml";
#ifndef _USE_ON_ANDROID
		cout << "Read  " << strFNameDescTemp << endl;
#endif
		inDescFileV.open(strDirDescriptionDB + strFNameDescTemp,FileStorage::READ);
#ifndef _USE_ON_ANDROID
		if(!inDescFileV.isOpened())
		{
			cout << "Can't open file " << strFNameDescTemp << endl;
			exit(EXIT_FAILURE);
		}
#endif
		inDescFileV["descriptionVOfPic"] >> valueHistogram[count];
		inDescFileV.release();
		
		// Matching section.
		// =================
		// Matching each description point of unknown flower with each photo in DB.

		// Calculate similarity from shape features.
		// =========================================
		matcher.match(descriptorOfUnknowFlower,descriptorDB[count]
			,resultOfMatchDB[count]);
		// Sort and get top 25 lowest distance vector.
		// In 25 numbers don't sort but not have a problem.
		nth_element(resultOfMatchDB[count].begin()
			,resultOfMatchDB[count].begin()+numTopSmall-1
			,resultOfMatchDB[count].end());
		resultOfMatchDB[count].erase(resultOfMatchDB[count].begin()+numTopSmall
			,resultOfMatchDB[count].end());
		// Sum all 25 distance. if which photo has less number more similarity.
		float sumShape = 0;
		for(int i=0;i<numTopSmall;i++)
		{
			sumShape += resultOfMatchDB[count][i].distance;
		}
		temp.strFNameOfPhoto = strFNameFlower;
		temp.distanceOfSurf = sumShape;

		// Calculate similarity from color features.
		// =========================================
		imgCompObj.setRefHistogram(hueHistogram[count],
									saturationHistogram[count],
									valueHistogram[count]);
		temp.distanceOfHSV = imgCompObj.compareWithHist(hueHistogramOfUnknownFlower,
														saturationHistogramOfUnknownFlower,
														valueHistogramOfUnknownFlower);

		// Sum of shape and color feature.
		// ===============================
		/* temp.distanceSumOfSurfAndColor = static_cast<double>(temp.distanceOfSurf) +
											temp.distanceOfHSV; */

		similarity.push_back(temp);

		count++;
	}
	inFile.close();

	int choice = FEATURE;

	if(choice == 3)
	{
		int max = similarity.size();
		
		// Get order by color feature.
		sort(similarity.begin(),similarity.end(),sortByColor);
		for(int i=0;i<max;i++)
			similarity[i].rankOfHSV = i;
		// Get order by shape feature.
		sort(similarity.begin(),similarity.end(),sortByShape);
		for(int i=0;i<max;i++)
			similarity[i].rankOfSurf = i;
		for(int i=0;i<max;i++)
			similarity[i].distanceSumOfSurfAndColor = similarity[i].rankOfHSV
													+ similarity[i].rankOfSurf;
	}
	
	// Sort list lowest on the top indicate more similarity.
	sort(similarity.begin(),similarity.end(),worseThan);

	// Write similarity list to a file.
	strRootProgram += "Matches.txt";
	outFile.open(strRootProgram.c_str());
#ifndef _USE_ON_ANDROID
	if(!outFile.is_open())
	{
		cout << "Can't open file " << "Matches.txt" << endl;
		exit(EXIT_FAILURE);
	}
#endif
	for_each(similarity.begin(),similarity.end(),ShowResult);
	outFile.close();

#ifndef _USE_ON_ANDROID
	// Timer stop.
	clock_t tmStop = clock();
	cout << endl << "Total using time = " << float(tmStop - tmStart)/CLOCKS_PER_SEC
		<< " sec" << endl;
	cout << "Number of vector = " << outPut.numVec << endl;
	cout << "Pass enter to exit.";
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
	int choice = FEATURE;

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

void ShowResult(const disStruct & rr)
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