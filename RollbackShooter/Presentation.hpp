#ifndef RBST_PRESENTATION_HPP
#define RBST_PRESENTATION_HPP

//std
#include <ostream>
//Raylib
#include <raylib.h>

#include "Math.hpp"
#include "GameState.hpp"

const int screenWidth = 1280;
const int screenHeight = 720;
const int centerX = screenWidth / 2;
const int centerY = screenHeight / 2;

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
	num_det camBack{ -8 };
	float camHeight = 7;
	num_det tgtFront{ 3 };
	float tgtHeight = 0;
	num_det laziness{ 0.2 };
	float specHeight = 6;
	num_det standBackMult{ 1.5 };

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
		Vec2 standBack = v2::rotate(v2::scalarMult(midDist, standBackMult), -camBack.half_pi());
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

void gameScene(POV pov, const GameState * state, const Config * cfg, const Camera3D * cam)
{
	BeginMode3D(*cam);
	{
		//draw players (and their alt shot direction if they're charging)
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
		//draw projectiles
		bool showCombos = false;
		int framesToAltShot = 0;
		switch (pov)
		{
		case Player1:
			showCombos = state->p1.ammo >= cfg->altShotCost;
			framesToAltShot = state->p1.pushdown.top() == Charging ? (cfg->chargeDuration - state->p1.chargeCount) : cfg->chargeDuration;
			break;
		case Player2:
			showCombos = state->p2.ammo >= cfg->altShotCost;
			framesToAltShot = state->p2.pushdown.top() == Charging ? (cfg->chargeDuration - state->p2.chargeCount) : cfg->chargeDuration;
			break;
		}
		for (auto it = state->projs.begin(); it != state->projs.end(); ++it)
		{
			Vec2 futurePos = v2::add(it->pos, v2::scalarMult(it->vel, num_det{ framesToAltShot }));
			bool withinReach = v2::length(futurePos) < cfg->arenaRadius;
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
			if (withinReach)
			{
				DrawCylinderWires(
					fromDetVec2(futurePos),
					fromDetNum(cfg->projRadius),
					fromDetNum(cfg->projRadius),
					.5f, 10, PURPLE);
				if (showCombos)
				{
					DrawCylinderWires(
						fromDetVec2(futurePos),
						fromDetNum(cfg->comboRadius),
						fromDetNum(cfg->comboRadius),
						.1f, 10, PURPLE);
				}
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
}

//MATCH PRESENTATION

//returns semaphore idle time
double present(POV pov, const GameState* state, const Config* cfg, Camera3D* cam, std::ostringstream* gameInfoOSS)
{
	setCamera(cam, NULL, state, pov);
	
	BeginDrawing();

	ClearBackground(RAYWHITE);

	gameScene(pov, state, cfg, cam);

	//crosshair
	DrawLineEx(
		Vector2{ centerX - 20 , centerY - 100 },
		Vector2{ centerX - 5, centerY - 100 },
		5.f, RED);
	DrawLineEx(
		Vector2{ centerX + 5 , centerY - 100 },
		Vector2{ centerX + 20, centerY - 100 },
		5.f, RED);
	DrawLineEx(
		Vector2{ centerX, centerY - 120 },
		Vector2{ centerX, centerY - 105 },
		5.f, RED);
	DrawLineEx(
		Vector2{ centerX, centerY - 95 },
		Vector2{ centerX, centerY - 80 },
		5.f, RED);
	DrawLineEx(
		Vector2{ centerX, centerY - 20 },
		Vector2{ centerX, centerY + 20 },
		5.f, RED);

	if (pov == Player1)
		drawBars(&state->p1, cfg);
	else if (pov == Player2)
		drawBars(&state->p2, cfg);

	DrawText(gameInfoOSS->str().c_str(), 5, 5, 20, GRAY);

	//I figure this is also the timing semaphore
	double beforeSemaphore = GetTime();
	EndDrawing();
	return GetTime() - beforeSemaphore;
}

//HOME SCREEN PRESENTATION

enum MenuItem
{
	Dummy = 0,
	ConnectP1 = 1,
	ConnectP2 = 2
};

struct HomeInfo
{
	MenuItem selected = Dummy;
	Vec2 lazyCam = v2::zero();
	bool homeScreen = true;
	std::string remoteAddress = "";
	float freshUpdate = 0;
};

struct BGInfo
{
	RenderTexture2D target;
	Shader shader;
};

void presentMenu(POV pov, const GameState* state, const Config* cfg, Camera3D* cam, std::ostringstream* gameInfoOSS, HomeInfo* home, BGInfo* bg)
{
	setCamera(cam, &home->lazyCam, state, pov);

	BeginDrawing();

	ClearBackground(RAYWHITE);

	BeginTextureMode(bg->target);
	ClearBackground(RAYWHITE);
	gameScene(pov, state, cfg, cam);
	EndTextureMode();

	if (home->homeScreen)
	{
		BeginShaderMode(bg->shader);
		// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
		DrawTextureRec(
			bg->target.texture,
			Rectangle{ 0, 0, (float)bg->target.texture.width, (float)-bg->target.texture.height },
			Vector2 { 0, 0 }, WHITE);
		EndShaderMode();
		
		//TODO replace with grain noise texture
		//DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 0,0,0,64 });
		
		//TODO game logo

		//MENU
		float xCenter = screenWidth / 2;
		float btnTop = screenHeight - 60;
		float btnWidth = 240;
		float btnHeight = 40;
		float btnDist = 30;

		float dummyX = xCenter - 3 * btnDist - 2 * btnWidth;
		float p1X = xCenter - btnDist - btnWidth;
		float p2X = xCenter + btnDist;
		float addressX = xCenter + 3 * btnDist + btnWidth;
		Rectangle dummyBtn = Rectangle{dummyX, btnTop, btnWidth, btnHeight };
		Rectangle p1Btn = Rectangle{ p1X, btnTop, btnWidth, btnHeight };
		Rectangle p2Btn = Rectangle{ p2X, btnTop, btnWidth, btnHeight };
		Rectangle addressField = Rectangle{ addressX, btnTop, btnWidth, btnHeight };

		Color dummyColor = home->selected == Dummy ? RED : GRAY;
		Color p1Color = home->selected == Player1 ? RED : GRAY;
		Color p2Color = home->selected == Player2 ? RED : GRAY;
		Color addressColor = Color{
			static_cast<unsigned char>(130 + home->freshUpdate * (0-130)),
			static_cast<unsigned char>(130 + home->freshUpdate * (158-130)),
			static_cast<unsigned char>(130 + home->freshUpdate * (47-130)),
			255
		};

		DrawRectangleRec(dummyBtn, LIGHTGRAY);
		DrawRectangleLinesEx(dummyBtn, 4, dummyColor);
		DrawText("Start Dummy Match", dummyBtn.x + 10, dummyBtn.y + 10, 20, BLACK);
		DrawRectangleRec(p1Btn, LIGHTGRAY);
		DrawRectangleLinesEx(p1Btn, 4, p1Color);
		DrawText("Connect as Player 1", p1Btn.x + 10, p1Btn.y + 10, 20, BLACK);
		DrawRectangleRec(p2Btn, LIGHTGRAY);
		DrawRectangleLinesEx(p2Btn, 4, p2Color);
		DrawText("Connect as Player 2", p2Btn.x + 10, p2Btn.y + 10, 20, BLACK);
		DrawRectangleRec(addressField, WHITE);
		DrawRectangleLinesEx(addressField, 4, addressColor);
		DrawText(home->remoteAddress.c_str(), addressField.x + 10, addressField.y + 10, 20, BLACK);
		DrawText("Remote IP Address\nCtrl+V to paste it here", addressField.x, addressField.y - 50, 20, BLACK);
	}
	else
	{
		DrawTextureRec(
			bg->target.texture,
			Rectangle{ 0, 0, (float)bg->target.texture.width, (float)-bg->target.texture.height },
			Vector2{ 0, 0 }, WHITE);
	}

	DrawText(gameInfoOSS->str().c_str(), 5, 5, 20, GRAY);

	EndDrawing();
}

#endif // !RBST_PRESENTATION_HPP
