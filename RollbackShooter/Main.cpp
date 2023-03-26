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
#include "DummyScene.hpp"

int main(int argc, char* argv[])
{
	InitWindow(screenWidth, screenHeight, "RBST");
	SetTargetFPS(60);
	SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
	DisableCursor();

	HomeInfo home;
	
	auto launchOpt = toml::parse_file("RBST_launch.toml");
	home.remoteAddress = launchOpt["Network"]["remoteAddress"].value_or("127.0.0.1");
	unsigned short localPort = launchOpt["Network"]["localPort"].value_or(8001);

	POV replayPOV = Spectator;
	Camera3D replayCam = initialCamera();
	GameState replayState = initialState(&cfg);
	std::ostringstream replayOSS;

	BGInfo bg;
	bg.target = LoadRenderTexture(screenWidth, screenHeight);
	bg.shader = LoadShader(0, "shader/gauss.fs");

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
		}
		else
		{
			//MENU CONTROLS
			if (IsKeyPressed(KEY_RIGHT))
			{
				home.selected = static_cast<MenuItem>((home.selected + 1) % 3);
			}
			else if (IsKeyPressed(KEY_LEFT))
			{
				home.selected = static_cast<MenuItem>((home.selected + 2) % 3);
			}
			if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
			{
				home.remoteAddress = std::string(GetClipboardText());
				home.freshUpdate = 1;
			}

			if (IsKeyPressed(KEY_ENTER)) {
				switch (home.selected)
				{
				case Dummy:
					DummyMain();
					//back from match
					replayState = initialState(&cfg);
					home.lazyCam = v2::zero();
					break;
				case ConnectP1:
					NetworkedMain(home.remoteAddress, localPort, 1);
					//back from match
					replayState = initialState(&cfg);
					home.lazyCam = v2::zero();
					break;
				case ConnectP2:
					NetworkedMain(home.remoteAddress, localPort, 2);
					//back from match
					replayState = initialState(&cfg);
					home.lazyCam = v2::zero();
					break;
				}
			}
		}

		//TODO replay input
		PlayerInput p1Input{ None, Neutral, num_det {0} };
		PlayerInput p2Input{ None, Neutral, num_det {0} };
		InputData input{ p1Input, p2Input };
		//simulation
		replayState = simulate(replayState, &cfg, input);
		//secondary simulation
		int currentFps = GetFPS();
		replayOSS.str("");
		replayOSS << "FPS: " << currentFps << std::endl;
		//presentation
		presentMenu(replayPOV, &replayState, &cfg, &replayCam, &replayOSS, &home, &bg);
	}
	
	CloseWindow();
}