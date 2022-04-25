#pragma once
#include <iostream>
#include <filesystem>

ref class FileManager
{
public:
	// Returns string array of file names in a given directory
	std::string* getFiles(std::string directoryPath, int* fileCount, int* pathLength);
};

