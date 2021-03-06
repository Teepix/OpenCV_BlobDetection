#include "stdafx.h"//header for Visual Studio
#include <opencv2/core/core.hpp>//header for OpenCV core
#include <opencv2/highgui/highgui.hpp>//header for OpenCV UI
#include <iostream>//header for c++ IO
#include <opencv2/imgproc/imgproc.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include<iostream>
#include<conio.h>

#include "Blob.h"
using namespace std;
#define SHOW_STEPS            

// global variables ///////////////////////////////////////////////////////////////////////////////
const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

// function prototypes ////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs);
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex);
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs);
double distanceBetweenPoints(cv::Point point1, cv::Point point2);
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);
void drawAndShowContours(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName);
bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount);
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy);
void drawCarCountOnImage(int &carCount, cv::Mat &imgFrame2Copy);
void drawCarAlertOnImage(int &alert, cv::Mat &imgFrame2Copy);
void drawAlertMessage(int &alert, cv::Mat &imgFrame2Copy);
void drawFrameCount(int frame, cv::Mat &imgFrame2Copy);
void drawStatsCar(int debit, int frame,int carCount, cv::Mat &imgFrame2Copy);
//void drawAverageCar(int debit, int random, cv::Mat &imgFrame2Copy);

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {

	cv::VideoCapture capVideo;

	cv::Mat imgFrame1;
	cv::Mat imgFrame2;

	std::vector<Blob> blobs;

	cv::Point crossingLine[2];
	cv::Point crossingLine1[2];
	cv::Point crossingLine2[2];
	cv::Point crossingLine3[2];
	cv::Point crossingLine4[2];

	int alert = 0;
	int carCount = 0;
	int j = 0;
	int i = 0;
	int frame = 0;
	int voiture = 0;
	int lastCarCount = 0;
	int debit = 0;
	//int average = 0;

	capVideo.open("CarsDrivingUnderBridge.mp4");

	if (!capVideo.isOpened()) {
		std::cout << "error reading video file" << std::endl << std::endl;
		_getch();
		return(0);
	}

	if (capVideo.get(CV_CAP_PROP_FRAME_COUNT) < 2) {
		std::cout << "error: video file must have at least two frames";
		_getch();
		return(0);
	}

	capVideo.read(imgFrame1);
	capVideo.read(imgFrame2);

	int intHorizontalLinePosition = (int)std::round((double)imgFrame1.rows * 0.35);
	//////////////////////////////////dessin lignes
	crossingLine[0].x = 350;
	crossingLine[0].y = intHorizontalLinePosition;

	crossingLine[1].x = 500;//imgFrame1.cols - 1
	crossingLine[1].y = intHorizontalLinePosition;

	crossingLine1[0].x = 500;
	crossingLine1[0].y = intHorizontalLinePosition;

	crossingLine1[1].x = 625;//imgFrame1.cols - 1
	crossingLine1[1].y = intHorizontalLinePosition;

	crossingLine2[0].x = 625;
	crossingLine2[0].y = intHorizontalLinePosition;

	crossingLine2[1].x = 750;//imgFrame1.cols - 1
	crossingLine2[1].y = intHorizontalLinePosition;

	crossingLine3[0].x = 750;
	crossingLine3[0].y = intHorizontalLinePosition;

	crossingLine3[1].x = 880;//imgFrame1.cols - 1
	crossingLine3[1].y = intHorizontalLinePosition;

	crossingLine4[0].x = 880;
	crossingLine4[0].y = intHorizontalLinePosition;

	crossingLine4[1].x = 1000;//imgFrame1.cols - 1
	crossingLine4[1].y = intHorizontalLinePosition;



	///////////////////////
	char chCheckForEscKey = 0;

	bool blnFirstFrame = true;

	int frameCount = 2;

	while (capVideo.isOpened() && chCheckForEscKey != 27) {

		std::vector<Blob> currentFrameBlobs;
		frame++;
		cv::Mat imgFrame1Copy = imgFrame1.clone();
		cv::Mat imgFrame2Copy = imgFrame2.clone();

		cv::Mat imgDifference;
		cv::Mat imgThresh;

		cv::cvtColor(imgFrame1Copy, imgFrame1Copy, CV_BGR2GRAY);
		cv::cvtColor(imgFrame2Copy, imgFrame2Copy, CV_BGR2GRAY);

		cv::GaussianBlur(imgFrame1Copy, imgFrame1Copy, cv::Size(5, 5), 0);
		cv::GaussianBlur(imgFrame2Copy, imgFrame2Copy, cv::Size(5, 5), 0);

		cv::absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);

		cv::threshold(imgDifference, imgThresh, 30, 255.0, CV_THRESH_BINARY);

		cv::imshow("imgThresh", imgThresh);

		cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
		cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
		cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
		cv::Mat structuringElement15x15 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));

		for (unsigned int i = 0; i < 2; i++) {
			cv::dilate(imgThresh, imgThresh, structuringElement5x5);
			cv::dilate(imgThresh, imgThresh, structuringElement5x5);
			cv::erode(imgThresh, imgThresh, structuringElement5x5);
		}

		cv::Mat imgThreshCopy = imgThresh.clone();

		std::vector<std::vector<cv::Point> > contours;

		cv::findContours(imgThreshCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		drawAndShowContours(imgThresh.size(), contours, "imgContours");

		std::vector<std::vector<cv::Point> > convexHulls(contours.size());

		for (unsigned int i = 0; i < contours.size(); i++) {
			cv::convexHull(contours[i], convexHulls[i]);
		}

		drawAndShowContours(imgThresh.size(), convexHulls, "imgConvexHulls");

		for (auto &convexHull : convexHulls) {
			Blob possibleBlob(convexHull);

			if (possibleBlob.currentBoundingRect.area() > 400 &&
				possibleBlob.dblCurrentAspectRatio > 0.2 &&
				possibleBlob.dblCurrentAspectRatio < 4.0 &&
				possibleBlob.currentBoundingRect.width > 30 &&
				possibleBlob.currentBoundingRect.height > 30 &&
				possibleBlob.dblCurrentDiagonalSize > 60.0 &&
				(cv::contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.50) {
				currentFrameBlobs.push_back(possibleBlob);
			}
		}

		drawAndShowContours(imgThresh.size(), currentFrameBlobs, "imgCurrentFrameBlobs");

		if (blnFirstFrame == true) {
			for (auto &currentFrameBlob : currentFrameBlobs) {
				blobs.push_back(currentFrameBlob);
			}
		}
		else {
			matchCurrentFrameBlobsToExistingBlobs(blobs, currentFrameBlobs);
		}

		drawAndShowContours(imgThresh.size(), blobs, "imgBlobs");

		imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above

		drawBlobInfoOnImage(blobs, imgFrame2Copy);

		//////////////////////////franchir
		cv::rectangle(imgFrame2Copy, cv::Point(0, 0), cv::Point(400, 200), SCALAR_BLACK, CV_FILLED, CV_AA, 0);

		bool blnAtLeastOneBlobCrossedTheLine0 = false;
		bool blnAtLeastOneBlobCrossedTheLine1 = false;
		bool blnAtLeastOneBlobCrossedTheLine2 = false;
		bool blnAtLeastOneBlobCrossedTheLine3 = false;
		bool blnAtLeastOneBlobCrossedTheLine4 = false;

		for (auto blob : blobs) {

			if (blob.blnStillBeingTracked == true && blob.centerPositions.size() >= 2) {
				int prevFrameIndex = (int)blob.centerPositions.size() - 2;
				int currFrameIndex = (int)blob.centerPositions.size() - 1;
				///////////////////////////////4if
				if (blob.centerPositions[prevFrameIndex].y > intHorizontalLinePosition && blob.centerPositions[currFrameIndex].y <= intHorizontalLinePosition && blob.centerPositions[prevFrameIndex].x < 500) {
					carCount++;
					blnAtLeastOneBlobCrossedTheLine0 = true;
				}
				if (blob.centerPositions[prevFrameIndex].y > intHorizontalLinePosition && blob.centerPositions[currFrameIndex].y <= intHorizontalLinePosition && blob.centerPositions[prevFrameIndex].x < 625 && blob.centerPositions[prevFrameIndex].x>500) {
					carCount++;
					blnAtLeastOneBlobCrossedTheLine1 = true;
				}
				if (blob.centerPositions[prevFrameIndex].y > intHorizontalLinePosition && blob.centerPositions[currFrameIndex].y <= intHorizontalLinePosition && blob.centerPositions[prevFrameIndex].x < 750 && blob.centerPositions[prevFrameIndex].x>625) {
					carCount++;
					blnAtLeastOneBlobCrossedTheLine2 = true;
				}
				if (blob.centerPositions[prevFrameIndex].y > intHorizontalLinePosition && blob.centerPositions[currFrameIndex].y <= intHorizontalLinePosition && blob.centerPositions[prevFrameIndex].x < 880 && blob.centerPositions[prevFrameIndex].x>750) {
					carCount++;
					blnAtLeastOneBlobCrossedTheLine3 = true;
				}
				if (blob.centerPositions[prevFrameIndex].y > intHorizontalLinePosition && blob.centerPositions[currFrameIndex].y <= intHorizontalLinePosition && blob.centerPositions[prevFrameIndex].x < 1000 && blob.centerPositions[prevFrameIndex].x>880) {
					carCount++;
					alert++;
					blnAtLeastOneBlobCrossedTheLine4 = true;
				}
				if (alert > 0)
				{
					drawAlertMessage(alert, imgFrame2Copy);
				}
			}

		}

		cv::line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_GREEN, 2);
		cv::line(imgFrame2Copy, crossingLine1[0], crossingLine1[1], SCALAR_YELLOW, 2);
		cv::line(imgFrame2Copy, crossingLine2[0], crossingLine2[1], SCALAR_GREEN, 2);
		cv::line(imgFrame2Copy, crossingLine3[0], crossingLine3[1], SCALAR_YELLOW, 2);
		cv::line(imgFrame2Copy, crossingLine4[0], crossingLine4[1], SCALAR_RED, 2);


		drawCarCountOnImage(carCount, imgFrame2Copy);
//		drawCarAlertOnImage(alert, imgFrame2Copy);
		drawFrameCount(frame, imgFrame2Copy);
		drawStatsCar(debit, frame, carCount, imgFrame2Copy);
		//drawAverageCar(debit, imgFrame2Copy);
		if (frame % 60 == 0)
		{
			debit = carCount - lastCarCount;
			lastCarCount = carCount;
		}

		cv::imshow("imgFrame2Copy", imgFrame2Copy);

		//cv::waitKey(0);                 // image par image


		currentFrameBlobs.clear();

		imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is

		if ((capVideo.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVideo.get(CV_CAP_PROP_FRAME_COUNT)) {
			capVideo.read(imgFrame2);
		}
		else {
			std::cout << "end of video\n";
			break;
		}

		blnFirstFrame = false;
		frameCount++;
		chCheckForEscKey = cv::waitKey(1);
		j++;
	}

	if (chCheckForEscKey != 27) {
		cv::waitKey(0);
	}


	return(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs) {

	for (auto &existingBlob : existingBlobs) {

		existingBlob.blnCurrentMatchFoundOrNewBlob = false;

		existingBlob.predictNextPosition();
	}

	for (auto &currentFrameBlob : currentFrameBlobs) {

		int intIndexOfLeastDistance = 0;
		double dblLeastDistance = 100000.0;

		for (unsigned int i = 0; i < existingBlobs.size(); i++) {

			if (existingBlobs[i].blnStillBeingTracked == true) {

				double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);

				if (dblDistance < dblLeastDistance) {
					dblLeastDistance = dblDistance;
					intIndexOfLeastDistance = i;
				}
			}
		}

		if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 0.5) {
			addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance);
		}
		else {
			addNewBlob(currentFrameBlob, existingBlobs);
		}

	}

	for (auto &existingBlob : existingBlobs) {

		if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) {
			existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;
		}

		if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 5) {
			existingBlob.blnStillBeingTracked = false;
		}

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex) {

	existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
	existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;

	existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());

	existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
	existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

	existingBlobs[intIndex].blnStillBeingTracked = true;
	existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) {

	currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;

	existingBlobs.push_back(currentFrameBlob);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double distanceBetweenPoints(cv::Point point1, cv::Point point2) {

	int intX = abs(point1.x - point2.x);
	int intY = abs(point1.y - point2.y);

	return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) {
	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

	cv::imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName) {

	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	std::vector<std::vector<cv::Point> > contours;

	for (auto &blob : blobs) {
		if (blob.blnStillBeingTracked == true) {
			contours.push_back(blob.currentContour);
		}
	}

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

	cv::imshow(strImageName, image);
}


void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy) {

	for (unsigned int i = 0; i < blobs.size(); i++) {

		if (blobs[i].blnStillBeingTracked == true) {
			cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);

			int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
			double dblFontScale = blobs[i].dblCurrentDiagonalSize / 60.0;
			int intFontThickness = (int)std::round(dblFontScale * 1.0);

			cv::putText(imgFrame2Copy, std::to_string(i), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawCarCountOnImage(int &carCount, cv::Mat &imgFrame2Copy) {

	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 1000000.0;
	int intFontThickness = (int)std::round(dblFontScale * 2);

	cv::Size textSize = cv::getTextSize(std::to_string(carCount), intFontFace, dblFontScale, intFontThickness, 0);

	cv::Point ptTextBottomLeftPosition;

	ptTextBottomLeftPosition.x = (int)((double)textSize.width * 1.25);
	ptTextBottomLeftPosition.y = (int)((double)textSize.height * 2);

	cv::putText(imgFrame2Copy, "Vehicules : " + std::to_string(carCount) + " V", ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

}

void drawCarAlertOnImage(int &alert, cv::Mat &imgFrame2Copy) {


	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;
	int intFontThickness = (int)std::round(dblFontScale * 1.5);

	cv::Size textSize = cv::getTextSize(std::to_string(alert), intFontFace, dblFontScale, intFontThickness, 0);


	cv::Point ptTextBottomLeftPosition;

	ptTextBottomLeftPosition.y = (int)((double)textSize.height * 1.25);

	cv::putText(imgFrame2Copy, std::to_string(alert), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_RED, intFontThickness);

}

void drawAlertMessage(int &alert, cv::Mat &img2Frame2Copy) {

	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (img2Frame2Copy.rows * img2Frame2Copy.cols) / 300000.0;
	int intFontThickness = (int)std::round(dblFontScale * 1.5);
	cv::Size textSize = cv::getTextSize(std::to_string(alert), intFontFace, dblFontScale, intFontThickness, 0);

	double dblFontScale1 = (img2Frame2Copy.rows * img2Frame2Copy.cols) / 1000000.0;
	int intFontThickness1 = (int)std::round(dblFontScale * 0.5);
	cv::Size textSize1 = cv::getTextSize(std::to_string(alert), intFontFace, dblFontScale, intFontThickness, 0);

	cv::Point ptTextBottomLeftPosition;
	cv::Point ptTextCenterPosition;


	ptTextBottomLeftPosition.y = (img2Frame2Copy.rows - 1) - (int)((double)textSize.height * 1.25);
	ptTextBottomLeftPosition.x = (img2Frame2Copy.cols - 1) / 2.5;

	ptTextCenterPosition.x = (int)((double)textSize1.width * 0.35);
	ptTextCenterPosition.y = (int)((double)textSize1.height * 2.35);
//	ptTextCenterPosition.y = (img2Frame2Copy.rows - 1) - (int)((double)textSize.height * 1.25);
//	ptTextCenterPosition.x = ((img2Frame2Copy.cols - 1) / 1.65);

	cv::putText(img2Frame2Copy, "Alert !", ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_RED, intFontThickness);
	//cv::putText(img2Frame2Copy, std::to_string(alert), ptTextCenterPosition, intFontFace, dblFontScale, SCALAR_RED, intFontThickness);
	cv::putText(img2Frame2Copy, "Alert : " + std::to_string(alert), ptTextCenterPosition, intFontFace, dblFontScale1, SCALAR_RED, intFontThickness1);
}

void drawFrameCount(int frame, cv::Mat &imgFrame2Copy) {

	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 2000000.0;
	int intFontThickness = (int)std::round(dblFontScale * 1);

	cv::Size textSize = cv::getTextSize(std::to_string(frame), intFontFace, dblFontScale, intFontThickness, 0);

	cv::Point ptTextBottomLeftPosition;

	ptTextBottomLeftPosition.x = (int)((double)textSize.width);
	ptTextBottomLeftPosition.y = (imgFrame2Copy.rows - 1) - (int)((double)textSize.height);
	cv::putText(imgFrame2Copy, std::to_string(frame), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_RED, intFontThickness);

}

void drawStatsCar(int debit, int frame, int carCount, cv::Mat &imgFrame2Copy) {
	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 1000000.0;
	int intFontThickness = (int)std::round(dblFontScale * 2);
	cv::Size textSize = cv::getTextSize(std::to_string(debit), intFontFace, dblFontScale, intFontThickness, 0);

	cv::Point ptTextBottomLeftPosition;
	cv::Point ptTextPosition;

	ptTextBottomLeftPosition.x = (int)((double)textSize.width * 1.25);
	ptTextBottomLeftPosition.y = (int)((double)textSize.height * 4);

	ptTextPosition.x = (int)((double)textSize.width * 1.25);
	ptTextPosition.y = (int)((double)textSize.height * 6);

	cv::putText(imgFrame2Copy, "Debit : " + std::to_string(((debit / 2) * 60)) + " V/min", ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

	if (((frame / 30)) != 0)
	{
		cv::putText(imgFrame2Copy, "Average : " + std::to_string((carCount/((frame / 30))*3600)) + " V/h", ptTextPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);
	}
	else
	{
		cv::putText(imgFrame2Copy, "Average : 0 V/h", ptTextPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

	}
	


}