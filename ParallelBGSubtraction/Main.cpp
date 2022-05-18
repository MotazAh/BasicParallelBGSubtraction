#pragma once
#include "ImgProcessing.h"
#include "FileManager.h"
#include <mpi.h>

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#define MASK 294
using namespace std;
using namespace msclr::interop;

int main()
{
	ImgProcessing imgProc; // Holds functions for loading and creating images
	FileManager fManager; // Holds function for 
	System::String^ imagePath;

	imagePath = marshal_as<System::String^>("..//Data//input\\in000" + std::to_string(MASK) + ".jpg");
	int ImageWidth = 4, ImageHeight = 4;
	int* maskImg = imgProc.inputImage(&ImageWidth, &ImageHeight, imagePath);

	// Initialise MPI environment
	MPI_Init(NULL, NULL);

	// Get processor count
	int worldSize;
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	// Get processors ranks
	int worldRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

	
	int start_s, stop_s, TotalTime = 0; // Stopwatch for testing

	int imageCount = 0;
	int threshold = 50; // Controls what pixels to show in FG mask
	int** imagesData = NULL; // 2D matrix to hold all the image vectors
	int* pathLength = NULL; // Holds the length of each string for each path

	std::string* fileList;

	start_s = clock(); // Get starting time of stopwatch

	// Processors get path list, count, length
	fileList = fManager.getFiles("..//Data//Input", &imageCount, pathLength);

	// Ends the program and tells user that there is too few images or too many processors
	if (imageCount < worldSize)
	{
		if (worldRank == 0)
			std::cout << "Too many processors/few images, increase images in input folder or decrease processors" << std::endl;
		MPI_Finalize();
		return 0;
	}

	int localImageCount;
	int* imagesPerProcess = new int[worldSize];

	// Set inital images per process counts to 0
	for (int i = 0; i < worldSize; i++)
	{
		imagesPerProcess[i] = 0;
	}

	// Distribute images count in array
	if (worldRank == 0)
	{
		for (int i = 0; i < imageCount; i++)
		{
			imagesPerProcess[i % (worldSize)]++;
		}
		for (int c = 0; c < worldSize; c++)
			std::cout << "Processor " << c << " has " << imagesPerProcess[c] << " images" << std::endl;
	}
	
	// Scatter the image count array so each processor knows their image count
	MPI_Scatter(imagesPerProcess, 1, MPI_INT, &localImageCount, 1, MPI_INT, 0, MPI_COMM_WORLD);

	imagesData = new int* [localImageCount];

	// Get image data with each processor to 2d array
	for (int i = worldRank * (localImageCount); i < (worldRank + 1) * (localImageCount); i++)
	{
		imagePath = marshal_as<System::String^>(fileList[i]);
		imagesData[i % (localImageCount)] = imgProc.inputImage(&ImageWidth, &ImageHeight, imagePath);
	}

	float* localBG_Pixels = new float[ImageHeight * ImageWidth];
	float* BG_Pixels = new float[ImageHeight * ImageWidth];
	float* FG_Pixels = new float[ImageHeight * ImageWidth];
	float* localFG_Pixels = new float[(ImageHeight * ImageWidth) / worldSize];
	
	// Calculate local bg pixels mean
	for (int p = 0; p < ImageHeight * ImageWidth; p++)
	{
		int sum = 0;
		for (int i = 0; i < localImageCount; i++)
			sum += imagesData[i][p];
		localBG_Pixels[p] = sum / imageCount;
	}

	// Collect and sum the local means to get final mean
	MPI_Reduce(localBG_Pixels, BG_Pixels, ImageHeight * ImageWidth, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
	
	// Create background image
	if (worldRank == 0)
		imgProc.createImage(BG_Pixels, ImageWidth, ImageHeight, 0);

	float* finalLocalBG = new float[(ImageHeight * ImageWidth) / worldSize];
	int* InputImageData = new int[ImageHeight * ImageWidth];
	
	// Get data for input image
	InputImageData = maskImg;

	// Scatter the bg pixels to each processor
	MPI_Scatter(BG_Pixels, (ImageHeight * ImageWidth) / worldSize, MPI_FLOAT, finalLocalBG, (ImageHeight * ImageWidth) / worldSize, MPI_FLOAT, 0, MPI_COMM_WORLD);

	// Calculate the local foreground mask pixels
	for (int p = 0; p < (ImageHeight * ImageWidth) / worldSize; p++)
	{
		localFG_Pixels[p] = (std::abs(finalLocalBG[p] - InputImageData[(((ImageHeight * ImageWidth) / worldSize) * worldRank ) + p]) > threshold ? 255 : 0);
	}
	
	// Gather all local FG pixels into FG_Pixels in root 
	MPI_Gather(localFG_Pixels, (ImageHeight * ImageWidth) / worldSize, MPI_INT, FG_Pixels, (ImageHeight * ImageWidth) / worldSize, MPI_FLOAT, 0, MPI_COMM_WORLD);

	// Create foreground maks using FG_Pixels at root
	if (worldRank == 0)
		imgProc.createImage(FG_Pixels, ImageWidth, ImageHeight, 1);
	
	stop_s = clock(); // Get end time of stopwatch
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	cout << "time: " << TotalTime << endl;
	MPI_Finalize();
	return 0;
}