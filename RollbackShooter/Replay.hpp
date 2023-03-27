#ifndef RBST_REPLAY_HPP
#define RBST_REPLAY_HPP

//std
#include <vector>
#include <ostream>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <filesystem>
namespace fs = std::filesystem;

#include "Input.hpp"

struct ReplayWriter
{
	long confirmFrame;
	std::vector<InputData> inputBuffer;
	std::ofstream fileStream;
};

void openReplayFile(ReplayWriter* replay)
{
	replay->confirmFrame = 0;
	replay->inputBuffer.clear();

	struct tm currDate;
	time_t currTime;
	time(&currTime);
	localtime_s(&currDate, &currTime);
	std::ostringstream fileNameOSS("");
	fileNameOSS << currDate.tm_mday << "-" << currDate.tm_mon + 1 << "-" << currDate.tm_year + 1900 << "_";
	fileNameOSS << currDate.tm_hour << "-" << currDate.tm_min << "-" << currDate.tm_sec;
	fileNameOSS << ".rbst";

	const fs::path demos{ "demos" };
	fs::create_directory(demos);
	replay->fileStream.open(demos/(fileNameOSS.str().c_str()), std::fstream::out | std::fstream::binary);
}

void overwriteReplayInput(ReplayWriter* replay, InputData input, long frame)
{
	int index = static_cast<int>(frame - replay->confirmFrame);
	replay->inputBuffer[index] = input;
}

void writeReplayInput(ReplayWriter* replay, InputData input, long frame)
{
	int index = static_cast<int>(frame - replay->confirmFrame);
	assert(replay->inputBuffer.size() == index);
	replay->inputBuffer.push_back(input);
}

void consumeReplayInput(ReplayWriter* replay, long confFrame)
{
	while (replay->confirmFrame < confFrame)
	{
		InputData input = replay->inputBuffer.front();
		PlayerInputZip zips[2] = { 0 };
		zips[0] = zipInput(input.p1Input);
		zips[1] = zipInput(input.p2Input);
		replay->fileStream.write((char*)zips, sizeof(PlayerInputZip)*2);
		replay->inputBuffer.erase(replay->inputBuffer.begin());
		replay->confirmFrame++;
	}
}

void closeReplayFile(ReplayWriter* replay)
{
	replay->fileStream.close();
	replay->confirmFrame = 0;
	replay->inputBuffer.clear();
}

struct ReplayReader
{
	std::ifstream fileStream;
	int fileSize;
};

void openReplayFile(ReplayReader* replay, const char* fileName)
{
	replay->fileStream.open(fileName, std::fstream::in | std::fstream::binary);
	replay->fileStream.seekg(0, replay->fileStream.end);
	replay->fileSize = replay->fileStream.tellg();
	replay->fileStream.seekg(0, replay->fileStream.beg);
}

bool replayFileEnd(ReplayReader* replay)
{
	return replay->fileStream.tellg() == replay->fileSize;
}

InputData readReplayFile(ReplayReader* replay)
{
	PlayerInputZip zips[2];
	replay->fileStream.read((char*)zips, sizeof(PlayerInputZip) * 2);
	PlayerInput p1 = unzipInput(zips[0]);
	PlayerInput p2 = unzipInput(zips[1]);
	return InputData{ p1,p2 };
}

void closeReplayFile(ReplayReader* replay)
{
	replay->fileStream.close();
}

#endif