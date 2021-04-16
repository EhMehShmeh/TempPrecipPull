// pickLatLong.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <array>

namespace fs = std::filesystem;

std::ifstream currentFile;
std::ofstream resultsFile;
std::string path = R"raw(data\)raw";

std::string tminArray[372]{ "null" };
std::string tmaxArray[372]{ "null" };
std::string prcpArray[372]{ "null" };

enum {T_MIN, T_MAX, PRCP};

std::string getLineAt(std::string pathToFile, int lineNum);
int findLineNumAtString(std::string pathToFile, std::string searchTerm);
void clearArrays();
void extractData(std::string pathToFile, int lineNum, int targetArray);
bool within_limits(std::string pathToFile);

std::string get_stem(const fs::path& p) { return (p.stem().string()); }

std::string getLineAt(std::string pathToFile, int lineNum)
{
	std::ifstream fooFile;
	fooFile.open(pathToFile);

	for (int i = 0; i < lineNum - 1; i++) {
		fooFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	std::string result;

	std::getline(fooFile, result);

	return result;
}

int findLineNumAtString(std::string pathToFile, std::string searchTerm)
{
	std::ifstream fooFile;
	fooFile.open(pathToFile);
	std::stringstream currentLine;
	std::string lineString;
	std::string currentToken;
	int lineCount = 0;

	while (!fooFile.eof()) {

		lineCount++;

		std::getline(fooFile, lineString);

		currentLine << lineString;

		while (!currentLine.eof()) {
			currentLine >> currentToken;

			if (currentToken == searchTerm) {
				return lineCount;
			}

			//std::cout << currentToken << "\n";
		}

		currentLine.clear();
	}

	return -1;
}

void clearArrays() {
	for (int i = 0; i < 372; i++) {
		tminArray[i] = "null";
		tmaxArray[i] = "null";
		prcpArray[i] = "null";
	}
}

void extractData(std::string pathToFile, int lineNum, int targetArray)
{
	std::cout << "Extracting data\n";
	std::ifstream fooFile;
	fooFile.open(pathToFile);
	std::string lineString;
	std::stringstream currentLine;
	std::string currentToken;
	std::array<std::string, 15> filterList = { "dly-tmin-normal", "dly-tmax-normal", "mtd-prcp-normal", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
	int arrayCount = 0;
	int addedElementCount = 0;
	std::string dataArray[372];
	bool everythingOK = true;

	for (int i = 0; i < 12; i++) {

		addedElementCount = 0;
		everythingOK = true;
		lineString = getLineAt(pathToFile, lineNum + i);
		currentLine << lineString;

		//std::cout << lineString;

		while (everythingOK) {

			currentLine >> currentToken;

			for (int x = 0; x < filterList.size(); x++) {
				if (filterList[x] == currentToken) {
					goto END_FILTER;
				}
			}

			dataArray[arrayCount] = currentToken;
			arrayCount++;
			addedElementCount++;
			//std::cout << arrayCount << "   " << currentToken <<"\n";

			END_FILTER:{}

			if (addedElementCount == 31) everythingOK = false;
		}

		currentLine.clear();
		currentToken.clear();
	}

	if (targetArray == T_MIN) {
		for (int i = 0; i < 372; i++) {
			tminArray[i] = dataArray[i];
		}
		std::cout << "finished T_MIN\n";
	}
	if (targetArray == T_MAX) {
		for (int i = 0; i < 372; i++) {
			tmaxArray[i] = dataArray[i];
		}
		std::cout << "finished T_MAX\n";
	}
	if (targetArray == PRCP) {
		for (int i = 0; i < 372; i++) {
			prcpArray[i] = dataArray[i];
		}
		std::cout << "finished PRCP\n";
	}
}

bool within_limits(std::string pathToFile)
{
	std::ifstream fooFile;
	fooFile.open(pathToFile);

	for (int i = 0; i < 2; i++) {
		fooFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	fooFile.ignore(10);

	std::string latString;
	std::string lonString;
	double latitude;
	double longitude;

	std::getline(fooFile, latString);

	fooFile.ignore(11);

	std::getline(fooFile, lonString);

	//std::cout << latString << " " << lonString << '\n';

	latitude = std::stod(latString);
	longitude = std::stod(lonString);

	if (latitude > 23 && latitude < 46) {
		if (longitude > -123 && longitude < -77) {
			return true;
		}
	}

	return false;

}

int main()
{
	std::string csvExtension = ".csv";

	for (const auto& entry : fs::directory_iterator(path))
	{
		std::string entryString{ entry.path().u8string() }; // <<< u8string() is a literal godsend
		std::string outputPath;
		std::string stationName = getLineAt(entryString, 1).substr(14);
		std::string stationID = getLineAt(entryString, 2).substr(15);
		std::string latitude = getLineAt(entryString, 3).substr(10);
		std::string longitude = getLineAt(entryString, 4).substr(11);
		std::string elevation = getLineAt(entryString, 5).substr(11, 5);
		int tmaxPosition = findLineNumAtString(entryString, "dly-tmax-normal");
		int tminPosition = findLineNumAtString(entryString, "dly-tmin-normal");
		int precipPosition = findLineNumAtString(entryString, "mtd-prcp-normal");

		if (tmaxPosition != -1 && precipPosition != -1)
		{
			outputPath = R"raw(results\both\)raw";
			resultsFile.open(outputPath + get_stem(entry.path()) + csvExtension);
			extractData(entryString, tminPosition, T_MIN);
			extractData(entryString, tmaxPosition, T_MAX);
			extractData(entryString, precipPosition, PRCP);
		}
		else if (tmaxPosition != -1)
		{
			outputPath = R"raw(results\onlytemp\)raw";
			resultsFile.open(outputPath + get_stem(entry.path()) + csvExtension);
			extractData(entryString, tminPosition, T_MIN);
			extractData(entryString, tmaxPosition, T_MAX);
		}
		else if (precipPosition != -1)
		{
			outputPath = R"raw(results\onlyprecip\)raw";
			resultsFile.open(outputPath + get_stem(entry.path()) + csvExtension);
			extractData(entryString, precipPosition, PRCP);
		}

		resultsFile << "station_name,gchn_daily_id,latitude,longitude,elevation(m),month,day,temp_min,temp_max,precipitation_mtd" << "\n";

		int month = 1;
		int day = 1;

		for (int i = 0; i < 372; i++) {
			resultsFile << stationName << "," << stationID << "," << latitude
				<< "," << longitude << "," << elevation << "," << month << "," << day << "," << tminArray[i] << "," << tmaxArray[i] <<
				"," << prcpArray[i] << "\n";

			day++;

			if (day == 32) {
				month++;
				day = 1;
			}
		}

		resultsFile.close();
		clearArrays();
		std::cout << "converted: " << entry.path().stem().u8string() << "\n";

	}

	/*
	while (!done)
	{

	}
	*/

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

