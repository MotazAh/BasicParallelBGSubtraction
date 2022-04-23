#pragma once
#include "ImgProcessing.h"
#include "FileManager.h"

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;

int main()
{
	ImgProcessing imgProc;
	FileManager fManager;
	System::String^ imagePath;
	int ImageWidth = 4, ImageHeight = 4;
	int imageCount = 0;
	int threshold = 40; // Controls what pixels to show in FG mask
	int** imagesData; // 2D matrix to hold all the image vectors

	std::string* fileList = fManager.getFiles("..//Data//Input", &imageCount);
	imagesData = new int* [imageCount];

	// Load each image in the Input directory as a 1D Vector and save it in a 2D matrix
	for (int i = 0; i < imageCount; i++)
	{
		// Seperate imageCount with count of processors with scatter/bcast
		imagePath = marshal_as<System::String^>(fileList[i]);
		imagesData[i] = imgProc.inputImage(&ImageWidth, &ImageHeight, imagePath);
		std::cout << fileList[i] << std::endl;
	}

	int* BG_Pixels = new int[ImageHeight * ImageWidth];
	int* FG_Pixels = new int[ImageHeight * ImageWidth];

	// Calculate background pixels with mean
	for (int p = 0; p < ImageHeight * ImageWidth; p++)
	{
		// Convert to 1D array for MPI
		int sum = 0;
		for (int i = 0; i < imageCount; i++)
			sum += imagesData[i][p];
		BG_Pixels[p] = sum / imageCount;
	}

	// Calculate the foreground mask pixels
	for (int p = 0; p < ImageHeight * ImageWidth; p++)
	{
		// Joing with above loop
		FG_Pixels[p] = (std::abs(BG_Pixels[p] - imagesData[imageCount - 1][p]) > threshold ? 255 : 0);
	}
	// Create foreground mask and background images
	imgProc.createImage(FG_Pixels, ImageWidth, ImageHeight, 0);
	imgProc.createImage(BG_Pixels, ImageWidth, ImageHeight, 1);
	
	for (int i = 0; i < imageCount; i++)
	{
		free(imagesData[i]);
	}
	
	//system("pause");
	return 0;

}