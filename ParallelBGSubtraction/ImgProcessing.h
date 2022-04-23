#pragma once
#include <iostream>
#include <math.h>
#include <filesystem>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
ref class ImgProcessing
{
public:
	void createImage(int* image, int width, int height, int index);
	int* inputImage(int* w, int* h, System::String^ imagePath); //put the size of image in w & h
};

