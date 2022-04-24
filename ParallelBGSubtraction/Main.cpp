#pragma once
#include "ImgProcessing.h"
#include "FileManager.h"

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;
#define MASK 294
#define THRESHOLD 40
int main()
{
	ImgProcessing imgProc;
	FileManager fManager;
	System::String^ imagePath;
	int ImageWidth = 4, ImageHeight = 4;
	int imageCount = 0;
	int* imagesData; // 2D matrix to hold all the image vectors
	std::string* fileList = fManager.getFiles("..//Data//Input", &imageCount);
	imagesData = new int[320 * 240 * imageCount];

	// Load each image in the Input directory as a 1D Vector and save it in a 2D matrix
	for (int i = 0; i < imageCount; i++)
	{
		// Seperate imageCount with count of processors with scatter/bcast
		imagePath = marshal_as<System::String^>(fileList[i]);
		int* input = imgProc.inputImage(&ImageWidth, &ImageHeight, imagePath);
		//put all imgs array into input1d array such tath input1d[i] is the i-th img
		for (int j = 0; j < ImageHeight * ImageWidth; j++)
		{
			imagesData[i * ImageHeight * ImageWidth + j] = input[j];
		}
		delete[]input;
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
		{
			sum += imagesData[i * ImageHeight * ImageWidth + p];
		}
		BG_Pixels[p] = sum / imageCount;
		FG_Pixels[p] = (std::abs(BG_Pixels[p] - imagesData[(MASK) * (ImageHeight * ImageWidth) + p]) > THRESHOLD ? 255 : 0);

	}

	// Calculate the foreground mask pixels
	// Create foreground mask and background images
	imgProc.createImage(FG_Pixels, ImageWidth, ImageHeight, 0);
	imgProc.createImage(BG_Pixels, ImageWidth, ImageHeight, 1);
	delete[] imagesData;
	system("pause");
	return 0;

}