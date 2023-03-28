//std
#include <string>
//Raylib
#include <raylib.h>
//TOML++
#include <toml++/toml.h>

#include "Math.hpp"
#include "GameState.hpp"
#include "Input.hpp"
#include "Config.hpp"
#include "Player.hpp"
#include "Presentation.hpp"
#include "GGPOController.hpp"

int main(int argc, char* argv[])
{
	InitWindow(screenWidth, screenHeight, "RBST");
	SetTargetFPS(60);
	SetWindowState(FLAG_WINDOW_ALWAYS_RUN);

	HomeInfo home;

	bool cleanMode = false;
	
	auto launchOpt = toml::parse_file("RBST_launch.toml");
	home.remoteAddress = launchOpt["Network"]["remoteAddress"].value_or("127.0.0.1");
	unsigned short port = launchOpt["Network"]["port"].value_or(8001);

	POV replayPOV = Spectator;
	Camera3D replayCam = initialCamera();
	GameState replayState = initialState(&cfg);
	ReplayReader replayR;
	std::ostringstream replayOSS;

	std::string replayFile(launchOpt["Replay"]["replayFile"].value_or("demo.rbst"));
	openReplayFile(&replayR, replayFile.c_str());

	home.bgTarget = LoadRenderTexture(screenWidth, screenHeight);
	home.bgShader = LoadShader(0, "shader/bg.fs");
	Sprites sprs = LoadSprites();

	//home screen
	while (!WindowShouldClose())
	{
		home.freshUpdate = std::max(0.0f, home.freshUpdate - (1.0f / 60.0f));
		
		//show/hide background replay
		if (IsKeyReleased(KEY_F4))
		{
			home.homeScreen = !home.homeScreen;
			if (home.homeScreen) replayPOV = Spectator;
		}

		if (!home.homeScreen)
		{
			//REPLAY CONTROLS
			if (IsKeyPressed(KEY_F1))
				replayPOV = Player1;
			else if (IsKeyPressed(KEY_F2))
				replayPOV = Player2;
			else if (IsKeyPressed(KEY_F3))
				replayPOV = Spectator;
			else if (IsKeyPressed(KEY_C))
				cleanMode = !cleanMode;
		}
		else
		{
			//MENU CONTROLS
			if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
			{
				home.remoteAddress = std::string(GetClipboardText());
				home.freshUpdate = 1;
			}
			else if (IsKeyPressed(KEY_F1))
			{
				DisableCursor();
				NetworkedMain(&sprs, home.remoteAddress, port, 1);
				//back from match
				EnableCursor();
			}
			else if (IsKeyPressed(KEY_F2))
			{
				DisableCursor();
				NetworkedMain(&sprs, home.remoteAddress, port, 2);
				//back from match
				EnableCursor();
			}
		}
		if (!replayR.fileStream.is_open())
			replayOSS << "DEMO FILE NOT FOUND, YOU'VE DOOMED US ALL AAAAAAAAAAAAAAAAAAAAAA" << std::endl;
		if (replayFileEnd(&replayR))
		{
			closeReplayFile(&replayR);
			//repeat
			replayState = initialState(&cfg);
			openReplayFile(&replayR, replayFile.c_str());
		}
		InputData input = readReplayFile(&replayR);
		//simulation
		replayState = simulate(replayState, &cfg, input);
		//secondary simulation
		int currentFps = GetFPS();
		replayOSS.str("");
		replayOSS << "FPS: " << currentFps << std::endl;
		if (!home.homeScreen)
		{
			replayOSS << "Press F1 and F2 for player POVs," << std::endl;
			replayOSS << "F3 for spectator POV," << std::endl;
			replayOSS << "C for (clean? camera? cinematic?) mode," << std::endl;
			replayOSS << "or F4 to go back to menu." << std::endl;
		}
		if (cleanMode) replayOSS.str("");
		//presentation
		presentMenu(replayPOV, &replayState, &cfg, &replayCam, &sprs, &replayOSS, &home);
	}
	UnloadRenderTexture(home.bgTarget);
	UnloadShader(home.bgShader);
	UnloadSprites(sprs);
	closeReplayFile(&replayR);
	CloseWindow();
}