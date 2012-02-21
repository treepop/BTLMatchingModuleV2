// OpenCV header.
// ==============
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\imgproc\imgproc.hpp> // For histogram func.

class HistogramHSV
/* input : color image.
output : 3D HSV histogram
		 1D H histogram
		 1D S histogram
		 1D V histogram
		 Image of H histogram
		 Image of S histogram
		 Image of V histogram
*/
{
	private:
		int histSize[3];
		float hranges[3][2];
		const float* ranges[3];
		int channels[3];

	public:
		HistogramHSV()
		{
			histSize[0] = 181;
			histSize[1] = 256;
			histSize[2] = 256;
			
			hranges[0][0] = 0.0;
			hranges[0][1] = 180.0;
			hranges[1][0] = 0.0;
			hranges[1][1] = 255.0;
			hranges[2][0] = 0.0;
			hranges[2][1] = 255.0;

			ranges[0] = hranges[0];
			ranges[1] = hranges[1];
			ranges[2] = hranges[2];

			channels[0] = 0;
			channels[1] = 1;
			channels[2] = 2;
		}

		cv::MatND getHSVHistogram(const cv::Mat &image,int minSaturation=0)
		{
			cv::MatND hist;
			cv::Mat hsv;
			cv::cvtColor(image,hsv,CV_BGR2HSV);
			cv::Mat mask;
			if(minSaturation > 0)
			{
				std::vector<cv::Mat> v;
				cv::split(hsv,v);
				cv::threshold(v[1],mask,minSaturation,255,cv::THRESH_BINARY);
			}

			cv::calcHist(&hsv,
				1,
				channels,
				mask,
				hist,
				3,
				histSize,
				ranges);
			return hist;
		}

		cv::MatND getHueHistogram(const cv::Mat &image,int minSaturation=0)
		{
			cv::MatND hist;
			cv::Mat hsv;
			cv::cvtColor(image,hsv,CV_BGR2HSV);
			cv::Mat mask;
			if(minSaturation > 0)
			{
				std::vector<cv::Mat> v;
				cv::split(hsv,v);
				cv::threshold(v[1],mask,minSaturation,255,cv::THRESH_BINARY);
			}

			channels[0] = 0;
			
			cv::calcHist(&hsv,
				1,
				channels,
				mask,
				hist,
				1,
				&histSize[0],
				&ranges[0]);
			return hist;
		}

		cv::MatND getSaturationHistogram(const cv::Mat &image,int minSaturation=0)
		{
			cv::MatND hist;
			cv::Mat hsv;
			cv::cvtColor(image,hsv,CV_BGR2HSV);
			cv::Mat mask;
			if(minSaturation > 0)
			{
				std::vector<cv::Mat> v;
				cv::split(hsv,v);
				cv::threshold(v[1],mask,minSaturation,255,cv::THRESH_BINARY);
			}

			channels[0] = 1;
			
			cv::calcHist(&hsv,
				1,
				channels,
				mask,
				hist,
				1,
				&histSize[1],
				&ranges[1]);
			return hist;
		}

		cv::MatND getValueHistogram(const cv::Mat &image,int minSaturation=0)
		{
			cv::MatND hist;
			cv::Mat hsv;
			cv::cvtColor(image,hsv,CV_BGR2HSV);
			cv::Mat mask;
			if(minSaturation > 0)
			{
				std::vector<cv::Mat> v;
				cv::split(hsv,v);
				cv::threshold(v[1],mask,minSaturation,255,cv::THRESH_BINARY);
			}

			channels[0] = 2;
			
			cv::calcHist(&hsv,
				1,
				channels,
				mask,
				hist,
				1,
				&histSize[2],
				&ranges[2]);
			return hist;
		}

		cv::Mat getHueHistogramImage(const cv::Mat &image)
		{
			cv::MatND hist = getHueHistogram(image);
			double maxVal = 0;
			double minVal = 0;
			cv::minMaxLoc(hist,&minVal,&maxVal,0,0);
			cv::Mat histImg(histSize[0],histSize[0],CV_8U,cv::Scalar(255));
			int hpt = static_cast<int>(0.9*histSize[0]);
			for(int h=0; h<histSize[0]; h++)
			{
				float binVal = hist.at<float>(h);
				int intensity = static_cast<int>(binVal*hpt/maxVal);
				cv::line(histImg,cv::Point(h,histSize[0]),
								cv::Point(h,histSize[0] - intensity),
								cv::Scalar::all(0));
			}
			return histImg;
		}

		cv::Mat getSaturationHistogramImage(const cv::Mat &image)
		{
			cv::MatND hist = getSaturationHistogram(image);
			double maxVal = 0;
			double minVal = 0;
			cv::minMaxLoc(hist,&minVal,&maxVal,0,0);
			cv::Mat histImg(histSize[1],histSize[1],CV_8U,cv::Scalar(255));
			int hpt = static_cast<int>(0.9*histSize[1]);
			for(int h=0; h<histSize[1]; h++)
			{
				float binVal = hist.at<float>(h);
				int intensity = static_cast<int>(binVal*hpt/maxVal);
				cv::line(histImg,cv::Point(h,histSize[1]),
								cv::Point(h,histSize[1] - intensity),
								cv::Scalar::all(0));
			}
			return histImg;
		}

		cv::Mat getValueHistogramImage(const cv::Mat &image)
		{
			cv::MatND hist = getValueHistogram(image);
			double maxVal = 0;
			double minVal = 0;
			cv::minMaxLoc(hist,&minVal,&maxVal,0,0);
			cv::Mat histImg(histSize[2],histSize[2],CV_8U,cv::Scalar(255));
			int hpt = static_cast<int>(0.9*histSize[2]);
			for(int h=0; h<histSize[2]; h++)
			{
				float binVal = hist.at<float>(h);
				int intensity = static_cast<int>(binVal*hpt/maxVal);
				cv::line(histImg,cv::Point(h,histSize[2]),
								cv::Point(h,histSize[2] - intensity),
								cv::Scalar::all(0));
			}
			return histImg;
		}
};

cv::Mat getCenterOfImage(const cv::Mat &image,int iRadius)
/* Function get ROI of image.
input : image and radius
output : a rectangle segment of the image that has a number of pixels specific by radius.
Minimum of iRadius is 1.
*/
{
	cv::Mat imageROI = image(cv::Rect(image.cols/2 - iRadius - 1
									,image.rows/2 - iRadius - 1
									,1 + (iRadius-1)*2
									,1 + (iRadius-1)*2));
	return imageROI;
}

class ImageComparator
{
	private:
		cv::MatND refH;
		cv::MatND refS;
		cv::MatND refV;

		cv::MatND inputH;
		cv::MatND inputS;
		cv::MatND inputV;

		HistogramHSV hsvObj;

	public:
		void setReferenceImage(const cv::Mat &image)
		{
			refH = hsvObj.getHueHistogram(image);
			refS = hsvObj.getSaturationHistogram(image);
			refV = hsvObj.getValueHistogram(image);
		}

		void setRefHistogram(const cv::MatND &hHist,
								const cv::MatND &sHist,
								const cv::MatND &vHist)
		{
			refH = hHist;
			refS = sHist;
			refV = vHist;
		}

		double compareWithImage(const cv::Mat &image)
		{
			double sum = 0;

			inputH = hsvObj.getHueHistogram(image);
			inputS = hsvObj.getSaturationHistogram(image);
			inputV = hsvObj.getValueHistogram(image);
			sum += cv::compareHist(refH,inputH,CV_COMP_CHISQR);
			sum += cv::compareHist(refS,inputS,CV_COMP_CHISQR);
			sum += cv::compareHist(refV,inputV,CV_COMP_CHISQR);

			return sum;
		}

		double compareWithHist(const cv::MatND &hHist,
								const cv::MatND &sHist,
								const cv::MatND &vHist)
		{
			double sum = 0;

			sum += cv::compareHist(refH,hHist,CV_COMP_CHISQR);
			sum += cv::compareHist(refS,sHist,CV_COMP_CHISQR);
			sum += cv::compareHist(refV,vHist,CV_COMP_CHISQR);

			return sum;
		}

		void clearMem()
		{
			refH.release();
			refS.release();
			refV.release();

			inputH.release();
			inputS.release();
			inputV.release();
		}
};