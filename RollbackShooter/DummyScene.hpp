#ifndef RBST_DUMMYSCENE_HPP
#define RBST_DUMMYSCENE_HPP

//std
#include <string>
#include <ostream>
//Raylib
#include <raylib.h>

#include "Input.hpp"
#include "Config.hpp"
#include "Player.hpp"
#include "Presentation.hpp"

void DummyMain(const Sprites* sprs)
{
	Camera3D cam = initialCamera();
	GameState state = initialState(&cfg);
	std::ostringstream gameInfoOSS;
	double semaphoreIdleTime = 0;

	bool leave = false;

	while (!leave && !WindowShouldClose() && !endCondition(&state, &cfg))
	{
		//process input
		PlayerInput p1input = processInput(&inputBind);
		PlayerInput p2Dummy{ None, Neutral, num_det {0} };
		InputData input{ p1input,p2Dummy };
		if (IsKeyPressed(KEY_F10))
		{
			leave = true;
		}

		//simulation
		state = simulate(state, &cfg, input);

		//secondary simulation (stateful)
		// probably not doing it here lol

		//secondary simulation (stateless)
		int currentFps = GetFPS();
		gameInfoOSS.str("");
		gameInfoOSS << "FPS: " << currentFps << std::endl;
		gameInfoOSS << "Semaphore idle time: " << semaphoreIdleTime * 1000 << " ms" << std::endl;
		gameInfoOSS << "P1 HP: " << state.health1 << "; ";
		gameInfoOSS << "P2 HP: " << state.health2 << std::endl;
		gameInfoOSS << "Round Phase: " << std::to_string(state.phase) << "; ";
		gameInfoOSS << "Round Countdown : " << (state.roundCountdown / 60) << "." << (state.roundCountdown % 60) << std::endl;

		//presentation
		semaphoreIdleTime = present(Player1, &state, &cfg, &cam, sprs, &gameInfoOSS);
	}
}

#endif
