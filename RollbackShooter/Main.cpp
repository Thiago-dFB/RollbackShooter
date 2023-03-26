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

enum MenuItem
{
	Dummy=0,
	ConnectP1=1,
	ConnectP2=2
};

int main(int argc, char* argv[])
{
	bool homeScreen = true;
	POV replayPOV = Spectator;
	MenuItem selected = Dummy;
	
	auto launchOpt = toml::parse_file("RBST_launch.toml");

	InitWindow(screenWidth, screenHeight, "RBST");
	SetTargetFPS(60);
	SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
	DisableCursor();

	std::string remoteAddress = launchOpt["Network"]["remoteAddress"].value_or("127.0.0.1");
	unsigned short localPort = launchOpt["Network"]["localPort"].value_or(8001);

	Camera3D replayCam = initialCamera();
	Vec2 replayLazyCam = v2::zero();
	GameState replayState = initialState(&cfg);
	std::ostringstream replayOSS;

	//home screen
	while (!WindowShouldClose())
	{
		//menu controls
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
		{
			remoteAddress = std::string(GetClipboardText());
		}
		if (IsKeyPressed(KEY_RIGHT))
		{
			selected = static_cast<MenuItem>((selected + 1) % 3);
		}
		else if (IsKeyPressed(KEY_LEFT))
		{
			selected = static_cast<MenuItem>((selected + 2) % 3);
		}

		if (homeScreen)
		{
			if (IsKeyPressed(KEY_F1))
				replayPOV = Player1;
			else if (IsKeyPressed(KEY_F2))
				replayPOV = Player2;
			else if (IsKeyPressed(KEY_F3))
				replayPOV = Spectator;
		}

		if (IsKeyPressed(KEY_ENTER)) {
			switch (selected)
			{
			case Dummy:
				DummyMain();
				break;
			case ConnectP1:
				NetworkedMain(remoteAddress, localPort, 1);
				break;
			case ConnectP2:
				NetworkedMain(remoteAddress, localPort, 2);
				break;
			}
		}
		else
		{
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
			present(replayPOV, &replayState, &cfg, &replayCam, &replayLazyCam, &replayOSS);
		}
	}
	
	CloseWindow();
}