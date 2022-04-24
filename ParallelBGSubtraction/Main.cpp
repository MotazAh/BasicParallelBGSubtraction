#pragma once
#include "ImgProcessing.h"
#include "FileManager.h"
#include <mpi.h>

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;

int main()
{
	// Initialise MPI environment
	MPI_Init(NULL, NULL);

	// Get processor count
	int worldSize;
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	// Get processors ranks
	int worldRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

	ImgProcessing imgProc;
	FileManager fManager;
	System::String^ imagePath;

	int start_s, stop_s, TotalTime = 0; // Stopwatch for testing
	int ImageWidth = 4, ImageHeight = 4;
	int imageCount = 0;
	int threshold = 40; // Controls what pixels to show in FG mask
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

	imagesData = new int* [imageCount / worldSize];

	// Get image data with each processor to 2d array
	for (int i = worldRank * (imageCount / worldSize); i < (worldRank + 1) * (imageCount / worldSize); i++)
	{
		imagePath = marshal_as<System::String^>(fileList[i]);
		imagesData[i % (imageCount / worldSize)] = imgProc.inputImage(&ImageWidth, &ImageHeight, imagePath);
	}

	float* localBG_Pixels = new float[ImageHeight * ImageWidth];
	float* BG_Pixels = new float[ImageHeight * ImageWidth];
	float* FG_Pixels = new float[ImageHeight * ImageWidth];
	float* localFG_Pixels = new float[(ImageHeight * ImageWidth) / worldSize];
	
	// Calculate local bg pixels mean
	for (int p = 0; p < ImageHeight * ImageWidth; p++)
	{
		int sum = 0;
		for (int i = 0; i < imageCount / worldSize; i++)
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
	
	// Get data for input image (last image)
	InputImageData = imagesData[(imageCount / worldSize) - 1];

	// Broadcast image data from last processor as it is the last image
	MPI_Bcast(InputImageData, ImageHeight * ImageWidth, MPI_INT, worldSize - 1, MPI_COMM_WORLD);

	// Scatter the bg pixels to each processor
	MPI_Scatter(BG_Pixels, (ImageHeight * ImageWidth) / worldSize, MPI_FLOAT, finalLocalBG, (ImageHeight * ImageWidth) / worldSize, MPI_FLOAT, 0, MPI_COMM_WORLD);

	// Calculate the local foreground mask pixels
	for (int p = 0; p < (ImageHeight * ImageWidth) / worldSize; p++)
	{
		localFG_Pixels[p] = (std::abs(finalLocalBG[p] - InputImageData[(((ImageHeight * ImageWidth) / worldSize) * worldRank ) + p]) > threshold ? 255 : 0);
	}
	
	MPI_Gather(localFG_Pixels, (ImageHeight * ImageWidth) / worldSize, MPI_INT, FG_Pixels, (ImageHeight * ImageWidth) / worldSize, MPI_FLOAT, 0, MPI_COMM_WORLD);

	if (worldRank == 0)
		imgProc.createImage(FG_Pixels, ImageWidth, ImageHeight, 1);

	stop_s = clock(); // Get end time of stopwatch
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	cout << "time: " << TotalTime << endl;

	/*
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
	*/
	//system("pause");
	MPI_Finalize();
	return 0;

}
/*
	int ImageWidth = 4, ImageHeight = 4;

	int start_s, stop_s, TotalTime = 0;

	System::String^ imagePath;
	std::string img;
	img = "..//Data//Input//test.png";

	imagePath = marshal_as<System::String^>(img);
	int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);


	start_s = clock();
	createImage(imageData, ImageWidth, ImageHeight, 0);
	stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	cout << "time: " << TotalTime << endl;

	free(imageData);
	return 0;
*/