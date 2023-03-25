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

const int screenWidth = 800;
const int screenHeight = 600;

enum LaunchMode
{
	Home=0,
	Replay=1,
	Dummy=2,
	Connect=3
};

enum POV
{
	Spectator=0,
	Player1=1,
	Player2=2
};

inline Vector3 fromDetVec2(Vec2 vec, float height=0.0f)
{
	return Vector3{ static_cast<float>(vec.x), height, static_cast<float>(vec.y) };
}

inline float fromDetNum(num_det num)
{
	return static_cast<float>(num);
}

void setCamera(Camera3D* cam, Vec2* lazyCam, const GameState* state, POV pov)
{
	num_det camBack{ -5 };
	float camHeight = 4;
	num_det tgtFront{ 6 };
	float tgtHeight = 0;
	num_det laziness{ 0.2 };
	float specHeight = 6;

	switch (pov)
	{
	case Player1:
		cam->position = fromDetVec2(v2::add(state->p1.pos, v2::scalarMult(state->p1.dir, camBack)), camHeight);
		cam->target = fromDetVec2(v2::add(state->p1.pos, v2::scalarMult(state->p1.dir, tgtFront)), tgtHeight);
		break;
	case Player2:
		cam->position = fromDetVec2(v2::add(state->p2.pos, v2::scalarMult(state->p2.dir, camBack)), camHeight);
		cam->target = fromDetVec2(v2::add(state->p2.pos, v2::scalarMult(state->p2.dir, tgtFront)), tgtHeight);
		break;
	case Spectator:
		Vec2 midDist = v2::scalarDiv(v2::sub(state->p1.pos, state->p2.pos), num_det{2});
		Vec2 actionCenter = v2::add(state->p2.pos, midDist);
		Vec2 standBack = v2::rotate(v2::scalarMult(midDist, num_det{1.5}), -camBack.half_pi());
		*lazyCam = v2::lerp(*lazyCam, v2::add(actionCenter, standBack), laziness);
		cam->position = fromDetVec2(*lazyCam, specHeight);
		cam->target = fromDetVec2(actionCenter, tgtHeight);
		break;
	}
}

void drawBars(const Player* player, const Config* cfg)
{
	DrawRectangle((screenWidth / 2) - 150, screenHeight - 60, (300 * player->ammo) / cfg->ammoMax, 20, GREEN);
	for (int i = 1; i <= (cfg->ammoMax / cfg->shotCost); i++)
	{
		int lineX = (screenWidth / 2) - 150 + (300 * i * cfg->shotCost) / cfg->ammoMax;
		DrawLine(lineX, screenHeight - 65, lineX, screenHeight - 25, BLACK);
	}
	DrawRectangle((screenWidth / 2) - 150, screenHeight - 30, (300 * player->stamina) / cfg->staminaMax, 20, YELLOW);
	for (int i = 1; i <= (cfg->staminaMax / cfg->dashCost); i++)
	{
		int lineX = (screenWidth / 2) - 150 + (300 * i * cfg->dashCost) / cfg->staminaMax;
		DrawLine(lineX, screenHeight - 35, lineX, screenHeight - 5, BLACK);
	}
}

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

	Camera3D cam = { 0 };
	cam.position = Vector3{ 0.0f, 15.0f, 15.0f };
	cam.target = Vector3{ 0.0f, 0.0f, 0.0f };
	cam.up = Vector3{ 0.0f, 1.0f, 0.0f };
	cam.fovy = 70.0f;
	cam.projection = CAMERA_PERSPECTIVE;
	Vec2 lazyCam = v2::zero();
	POV replayPOV = Spectator;

	std::ostringstream gameInfoOSS;
	double worstFrame = 0;

	const InputBindings bind = readTOMLForBind();
	const Config cfg = readTOMLForCfg();
	GameState state = initialState(&cfg);

	while (!WindowShouldClose())
	{
		//process input
		PlayerInput p1input = processInput(&bind);
		PlayerInput p2Dummy{ None, Neutral, num_det {0} };
		InputData input{ p1input,p2Dummy };

		if (launch == Replay)
		{
			if (IsKeyPressed(bind.replayP1Key))
				replayPOV = Player1;
			else if (IsKeyPressed(bind.replayP2Key))
				replayPOV = Player2;
			else if (IsKeyPressed(bind.replaySpecKey))
				replayPOV = Spectator;
		}

		//simulation
		double before = GetTime();
		state = simulate(state, &cfg, input);
		double after = GetTime();
		worstFrame = std::max(worstFrame, after - before);

		//secondary simulation (stateful)
		// probably not doing it here lol

		//secondary simulation (stateless)
		if (launch == Dummy)
			setCamera(&cam, &lazyCam, &state, Player1);
		else if (false) //TODO make networked player 2 this POV
			setCamera(&cam, &lazyCam, &state, Player2);
		else
			setCamera(&cam, &lazyCam, &state, replayPOV);

		//presentation

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(cam);
		{
			DrawCylinderWires(
				fromDetVec2(state.p1.pos),
				fromDetNum(cfg.playerRadius),
				fromDetNum(cfg.playerRadius),
				1.0f, 10, RED);
			if (state.p1.pushdown.top() == PState::Charging)
			{
				DrawRay(Ray{ fromDetVec2(state.p1.pos), fromDetVec2(state.p1.dir) }, RED);
			}
			DrawCylinderWires(
				fromDetVec2(state.p2.pos),
				fromDetNum(cfg.playerRadius),
				fromDetNum(cfg.playerRadius),
				1.0f, 10, BLUE);
			if (state.p2.pushdown.top() == PState::Charging)
			{
				DrawRay(Ray{ fromDetVec2(state.p2.pos), fromDetVec2(state.p2.dir) }, BLUE);
			}
			for (auto it = state.projs.begin(); it != state.projs.end(); ++it)
			{
				switch (it->owner)
				{
				case 1:
					DrawCylinderWires(
						fromDetVec2(it->pos),
						fromDetNum(cfg.projRadius),
						fromDetNum(cfg.projRadius),
						.5f, 10, RED);
					break;
				case 2:
					DrawCylinderWires(
						fromDetVec2(it->pos),
						fromDetNum(cfg.projRadius),
						fromDetNum(cfg.projRadius),
						.5f, 10, BLUE);
					break;
				}
			}
			DrawCylinderWires(
				Vector3{ 0.0f, 0.0f, 0.0f },
				fromDetNum(cfg.arenaRadius + cfg.playerRadius),
				fromDetNum(cfg.arenaRadius),
				-.5f, 50, BLACK);
			DrawGrid(10, static_cast<float>(cfg.arenaRadius) / 10.0f);
		}
		EndMode3D();

		if (launch == Dummy)
			drawBars(&state.p1, &cfg);
		else if (false) //TODO make networked player 2 this POV
			drawBars(&state.p2, &cfg);

		int currentFps = GetFPS();
		gameInfoOSS.str("");
		gameInfoOSS << "FPS: " << currentFps << std::endl;
		gameInfoOSS << "Sim cost: " << (after - before) * 1000 << " ms" << std::endl;
		gameInfoOSS << "Worst frame: " << worstFrame * 1000 << " ms" << std::endl;
		gameInfoOSS << "P1 HP: " << state.health1 << "; ";
		gameInfoOSS << "P2 HP: " << state.health2 << std::endl;
		gameInfoOSS << "Round Phase: " << std::to_string(state.phase) << "; Round Countdown: " << (state.roundCountdown / 60) << "." << (state.roundCountdown % 60) << std::endl;
		DrawText(gameInfoOSS.str().c_str(), 5, 5, 20, GRAY);

		EndDrawing();
	}

	CloseWindow();
}