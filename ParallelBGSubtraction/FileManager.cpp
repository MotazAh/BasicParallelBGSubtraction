#include "FileManager.h"

// Returns string array of file names in a given directory
std::string* FileManager::getFiles(std::string directoryPath, int* fileCount, int* pathLength)
{
	int count = 0;
	std::string* files;

	// Get number of files to iterate
	for (auto const& file : std::filesystem::directory_iterator(directoryPath))
	{
		count++;
	}
	*fileCount = count;
	pathLength = new int[count];
	files = new std::string[count];
	count = 0;

	// Add each file path to files pointer (array)
	for (auto const& file : std::filesystem::directory_iterator(directoryPath))
	{
		files[count] = file.path().u8string();
		pathLength[count] = files[count].length();
		count++;
	}
	return files;
}