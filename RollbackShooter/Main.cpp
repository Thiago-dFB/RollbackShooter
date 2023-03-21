//std
#include <string>
#include <stack>
#include <vector> 
//GGPO
#include <ggponet.h>
//Raylib
#include <raylib.h>

#include "Math.hpp"
#include "Config.hpp"
#include "Player.hpp"

const int PREGAME_COUNTDOWN = 180;
const int ROUNDTIMER_COUNTDOWN = 120 * 60;
const int ROUNDEND_COUNTDOWN = 300;

enum RoundPhase
{
	Countdown,
	Play,
	End
};

struct InputData
{
	PlayerInput p1Input;
	PlayerInput p2Input;
};


struct Projectile
{
	Vec2 pos = v2::zero();
	Vec2 vel = v2::zero();
	playerid owner = 0;
};

struct GameState
{
	long frame = 0;
	int16 roundCountdown = 0;
	RoundPhase phase = Countdown;
	
	Player p1;
	int8 health1 = 0;
	int8 rounds1 = 0;

	Player p2;
	int8 health2 = 0;
	int8 rounds2 = 0;
	
	std::vector<Projectile> projs;
};

GameState initialState(Config cfg)
{
	GameState gs;
	gs.frame = 0;
	gs.roundCountdown = PREGAME_COUNTDOWN;
	gs.phase = RoundPhase::Countdown;

	gs.p1.id = 1;
	gs.p1 = respawn(gs.p1, cfg);
	gs.health1 = cfg.playerHealth;
	gs.rounds1 = 0;

	gs.p2.id = 2;
	gs.p2 = respawn(gs.p2, cfg);
	gs.health2 = cfg.playerHealth;
	gs.rounds2 = 0;

	return gs;
}

GameState simulate(GameState state, Config cfg, InputData input)
{
	state.frame++;
	state.roundCountdown--;
	switch (state.phase)
	{
	case RoundPhase::Countdown:
		if (state.roundCountdown <= 0)
		{
			state.roundCountdown = ROUNDTIMER_COUNTDOWN;
			state.phase = RoundPhase::Play;
		}
		break;
	case RoundPhase::End:
		if (state.roundCountdown <= 0)
		{
			if (state.rounds1 < cfg.roundsToWin && state.rounds2 < cfg.roundsToWin)
			{
				state.roundCountdown = PREGAME_COUNTDOWN;
				state.phase = RoundPhase::Countdown;
				state.p1 = respawn(state.p1, cfg);
				state.health1 = cfg.playerHealth;
				state.p2 = respawn(state.p2, cfg);
				state.health2 = cfg.playerHealth;
				state.projs.erase(state.projs.begin(), state.projs.end());
			}
			else
			{
				//TODO SIGNAL THAT GAME ENDED
			}
			break;
		}
		// will simulate at half speed
		else if (state.roundCountdown % 2 == 0)
		{
			break;
		}
	case RoundPhase::Play:

		break;
	}
	return state;
}

int main(int argc, char* argv[])
{
	const int screenWidth = 800;
	const int screenHeight = 600;

	InitWindow(screenWidth, screenHeight, "Game");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		//process input

		//simulation

		//presentation

		BeginDrawing();

		ClearBackground(RAYWHITE);

		int currentFps = GetFPS();
		std::string topLeftText = "FPS: " + std::to_string(currentFps);
		DrawText(topLeftText.c_str(), 5, 5, 20, GRAY);

		EndDrawing();
	}
}