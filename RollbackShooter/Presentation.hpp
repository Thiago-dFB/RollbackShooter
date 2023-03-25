#ifndef RBST_PRESENTATION_HPP
#define RBST_PRESENTATION_HPP

//std
#include <ostream>
//Raylib
#include <raylib.h>

#include "Math.hpp"
#include "GameState.hpp"

const int screenWidth = 800;
const int screenHeight = 600;

enum POV
{
	Spectator = 0,
	Player1 = 1,
	Player2 = 2
};

Camera3D initialCamera()
{
	Camera3D cam = { 0 };
	cam.position = Vector3{ 0.0f, 0.0f, 0.0f };
	cam.target = Vector3{ 0.0f, 0.0f, 0.0f };
	cam.up = Vector3{ 0.0f, 1.0f, 0.0f };
	cam.fovy = 70.0f;
	cam.projection = CAMERA_PERSPECTIVE;
	return cam;
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
		Vec2 midDist = v2::scalarDiv(v2::sub(state->p1.pos, state->p2.pos), num_det{ 2 });
		Vec2 actionCenter = v2::add(state->p2.pos, midDist);
		Vec2 standBack = v2::rotate(v2::scalarMult(midDist, num_det{ 1.5 }), -camBack.half_pi());
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

void present(POV pov, const GameState* state, const Config* cfg, Camera3D* cam, Vec2* lazyCam, double frameStart, double* frameNearEnd)
{
	setCamera(cam, lazyCam, state, pov);
	
	BeginDrawing();

	ClearBackground(RAYWHITE);

	BeginMode3D(*cam);
	{
		DrawCylinderWires(
			fromDetVec2(state->p1.pos),
			fromDetNum(cfg->playerRadius),
			fromDetNum(cfg->playerRadius),
			1.0f, 10, RED);
		if (state->p1.pushdown.top() == PState::Charging)
		{
			DrawRay(Ray{ fromDetVec2(state->p1.pos), fromDetVec2(state->p1.dir) }, RED);
		}
		DrawCylinderWires(
			fromDetVec2(state->p2.pos),
			fromDetNum(cfg->playerRadius),
			fromDetNum(cfg->playerRadius),
			1.0f, 10, BLUE);
		if (state->p2.pushdown.top() == PState::Charging)
		{
			DrawRay(Ray{ fromDetVec2(state->p2.pos), fromDetVec2(state->p2.dir) }, BLUE);
		}
		for (auto it = state->projs.begin(); it != state->projs.end(); ++it)
		{
			switch (it->owner)
			{
			case 1:
				DrawCylinderWires(
					fromDetVec2(it->pos),
					fromDetNum(cfg->projRadius),
					fromDetNum(cfg->projRadius),
					.5f, 10, RED);
				break;
			case 2:
				DrawCylinderWires(
					fromDetVec2(it->pos),
					fromDetNum(cfg->projRadius),
					fromDetNum(cfg->projRadius),
					.5f, 10, BLUE);
				break;
			}
		}
		DrawCylinderWires(
			Vector3{ 0.0f, 0.0f, 0.0f },
			fromDetNum(cfg->arenaRadius + cfg->playerRadius),
			fromDetNum(cfg->arenaRadius),
			-.5f, 50, BLACK);
		DrawGrid(10, static_cast<float>(cfg->arenaRadius) / 10.0f);
	}
	EndMode3D();

	if (pov == Player1)
		drawBars(&state->p1, cfg);
	else if (pov == Player2)
		drawBars(&state->p2, cfg);

	*frameNearEnd = GetTime();

	int currentFps = GetFPS();
	std::ostringstream gameInfoOSS;
	gameInfoOSS << "FPS: " << currentFps << std::endl;
	gameInfoOSS << "Frame start to near end: " << (*frameNearEnd - frameStart) * 1000 << std::endl;
	gameInfoOSS << "P1 HP: " << state->health1 << "; ";
	gameInfoOSS << "P2 HP: " << state->health2 << std::endl;
	gameInfoOSS << "Round Phase: " << std::to_string(state->phase) << "; Round Countdown: " << (state->roundCountdown / 60) << "." << (state->roundCountdown % 60) << std::endl;
	DrawText(gameInfoOSS.str().c_str(), 5, 5, 20, GRAY);

	//I figure this is also the timing semaphore
	EndDrawing();
}

#endif // !RBST_PRESENTATION_HPP
