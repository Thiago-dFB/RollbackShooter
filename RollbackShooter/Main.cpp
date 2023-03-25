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

enum LaunchMode
{
	Home=0,
	Replay=1,
	Dummy=2,
	Connect=3
};

int main(int argc, char* argv[])
{
	LaunchMode launch = Home;
	//TODO if I ever implement scene change this goes to a function just so it can leave scope naturally
	auto launchOpt = toml::parse_file("RBST_launch.toml");
	std::string mode = launchOpt["launchMode"].value_or("home");
	if (mode.compare("replay") == 0)
		launch = Replay;
	else if (mode.compare("dummy") == 0)
		launch = Dummy;
	else if (mode.compare("connect") == 0)
		launch = Connect;

	InitWindow(screenWidth, screenHeight, "RBST");
	SetTargetFPS(60);
	SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
	DisableCursor();

	if (launch == Connect)
	{
		std::string remoteAddress = launchOpt["Network"]["remoteAddress"].value_or("127.0.0.1");
		unsigned short localPort = launchOpt["Network"]["localPort"].value_or(8001);
		playerid localPlayer = launchOpt["Network"]["localPlayer"].value_or(1);
		NetworkedMain(remoteAddress, localPort, localPlayer);
	}
	else
	{
		Camera3D cam = initialCamera();
		Vec2 lazyCam = v2::zero();
		POV replayPOV = Spectator;
		GameState state = initialState(&cfg);
		std::ostringstream gameInfoOSS;
		double semaphoreIdleTime = 0;

		while (!WindowShouldClose() && !endCondition(&state, &cfg))
		{
			//process input
			PlayerInput p1input = processInput(&inputBind);
			PlayerInput p2Dummy{ None, Neutral, num_det {0} };
			InputData input{ p1input,p2Dummy };

			if (launch == Replay)
			{
				if (IsKeyPressed(inputBind.replayP1Key))
					replayPOV = Player1;
				else if (IsKeyPressed(inputBind.replayP2Key))
					replayPOV = Player2;
				else if (IsKeyPressed(inputBind.replaySpecKey))
					replayPOV = Spectator;
			}

			//simulation
			state = simulate(state, &cfg, input);

			//secondary simulation (stateful)
			// probably not doing it here lol

			//secondary simulation (stateless)
			POV pov;
			if (launch == Dummy)
				pov = Player1;
			else
				pov = replayPOV;

			int currentFps = GetFPS();
			gameInfoOSS.str("");
			gameInfoOSS << "FPS: " << currentFps << std::endl;
			gameInfoOSS << "Semaphore idle time: " << semaphoreIdleTime * 1000 << " ms" << std::endl;
			gameInfoOSS << "P1 HP: " << state.health1 << "; ";
			gameInfoOSS << "P2 HP: " << state.health2 << std::endl;
			gameInfoOSS << "Round Phase: " << std::to_string(state.phase) << "; ";
			gameInfoOSS << "Round Countdown : " << (state.roundCountdown / 60) << "." << (state.roundCountdown % 60) << std::endl;

			//presentation
			semaphoreIdleTime = present(pov, &state, &cfg, &cam, &lazyCam, &gameInfoOSS);
		}
	}
	
	CloseWindow();
}