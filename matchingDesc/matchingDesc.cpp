// C++ header.
// ===========
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>

// OpenCV header.
// ==============
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\features2d\features2d.hpp>

using namespace std;
using namespace cv;

typedef vector<KeyPoint> vecKey;
typedef vector<DMatch> vecDMatch;

const int numTopSmall = 25; // Keep only top 25 lowest distance.

struct disStruct
{
	string strFNameOfPhoto;
	float distance;
};

ofstream outFile; // Use together with function ShowResult.

bool worseThan(const disStruct & r1,const disStruct & r2);
void ShowResult(const disStruct & rr);

int main()
{
	// Timer start.
	clock_t tmStart = clock();
	
	string strDirFlowerDB = "flowerPicDB\\";
	string strDirDescriptionDB = "descriptionDB\\";
	string strFNameFlowerDB = strDirFlowerDB + "files.txt";
	string strFNameFlower;
	string strFNameDesc;
	string strFNameUnknownFlower = "unknownFlower.jpg";
	ifstream inFile;
	int count = 0;
	int position = 0;

	// Extract SURF feature of unknown flower.
	// =======================================
	Mat imgUnknownFlower;
	vecKey keypointOfUnknownFlower;
	SurfFeatureDetector surf(2500.);
	SurfDescriptorExtractor surfDesc;
	Mat descriptorOfUnknowFlower;
	imgUnknownFlower = imread(strFNameUnknownFlower);
	surf.detect(imgUnknownFlower,keypointOfUnknownFlower);
	surfDesc.compute(imgUnknownFlower,keypointOfUnknownFlower,descriptorOfUnknowFlower);

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

	// Read SURF description from DB.
	// ==============================
	Mat *imgFlowerDB = new Mat[count];
	vecKey *keypointDB = new vecKey[count];
	Mat *descriptorDB = new Mat[count];

	BruteForceMatcher<L2<float>> matcher;
	vecDMatch *resultOfMatchDB = new vecDMatch[count]; // Similarity between unknown and 
													   // each in DB.
	vector<disStruct> similarity; // The less distance the more similarity.
	disStruct temp;
	FileStorage inDescFile;
	count = 0;
	inFile.open(strFNameFlowerDB.c_str());
	if(!inFile.is_open()) cout << "Can't open file " << strFNameFlowerDB;
	while(inFile >> strFNameFlower)
	{
		// Read SURF description from DB.
		// Find each name of yml files.
		strFNameDesc = strFNameFlower;
		position = strFNameDesc.find("jpg",0);
		strFNameDesc.replace(position,3,"yml");
		cout << "Read  " << strFNameDesc << endl;
		inDescFile.open(strDirDescriptionDB + strFNameDesc,FileStorage::READ);
		if(!inDescFile.isOpened()) cout << "Can't open file " << strFNameDesc;
		inDescFile["descriptionOfPic"] >> descriptorDB[count];
		inDescFile.release();
		
		// Matching each description point of unknown flower with each photo in DB.
		matcher.match(descriptorOfUnknowFlower,descriptorDB[count]
			,resultOfMatchDB[count]);
		// Sort and get top 25 lowest distance vector.
		nth_element(resultOfMatchDB[count].begin()
			,resultOfMatchDB[count].begin()+numTopSmall-1
			,resultOfMatchDB[count].end());
		resultOfMatchDB[count].erase(resultOfMatchDB[count].begin()+numTopSmall
			,resultOfMatchDB[count].end());
		// Sum all 25 distance. if which photo has less number more similarity.
		float sum = 0;
		for(int i=0;i<numTopSmall;i++)
		{
			sum += resultOfMatchDB[count][i].distance;
		}
		temp.strFNameOfPhoto = strFNameFlower;
		temp.distance = sum;
		similarity.push_back(temp);
		count++;
	}
	inFile.close();

	// Sort list lowest on the top indicate more similarity.
	sort(similarity.begin(),similarity.end(),worseThan);

	// Write similarity list to a file.
	outFile.open("Matches.txt");
	if(!outFile.is_open()) cout << "Can't open file " << "Matches.txt";
	for_each(similarity.begin(),similarity.end(),ShowResult);
	outFile.close();

	// Timer stop.
	clock_t tmStop = clock();
	cout << endl << "Total using time = " << (tmStop - tmStart)/CLOCKS_PER_SEC
		<< " sec" << endl << "Pass enter to exit.";

	getchar();
	return 0;
}

bool worseThan(const disStruct & r1,const disStruct & r2)
{
	if(r1.distance < r2.distance)
		return true;
	else
		return false;
}

void ShowResult(const disStruct & rr)
{
	outFile << rr.strFNameOfPhoto << "\t" << rr.distance << endl;
}