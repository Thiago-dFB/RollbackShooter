#ifndef RBST_PRESENTATION_HPP
#define RBST_PRESENTATION_HPP

//Raylib
#include <raylib.h>
//-----
#include "Math.hpp"
#include "GameState.hpp"

const int screenWidth = 1280;
const int screenHeight = 720;
const int centerX = screenWidth / 2;
const int centerY = screenHeight / 2;

struct Sprites
{
	Texture2D logo;
	Texture2D chars;
	Texture2D charsFlipped;
	Texture2D projs;
	Texture2D hearts;
	Shader billShader;
	struct {
		Model plane;
		Texture2D texture;
	} radius;
	struct {
		struct {
			Model path;
			Texture2D texture;
			Shader shader;
			int scroll;
		} charge;
		struct {
			Model path;
			Texture2D texture;
		} hitscanRed;
		struct {
			Model path;
			Texture2D texture;
		} hitscanBlue;
	} path;
};


static Mesh GenMeshPath()
{
	Mesh mesh = { 0 };
	mesh.triangleCount = 2;
	mesh.vertexCount = 4;
	mesh.vertices  = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
	mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
	mesh.normals   = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
	mesh.indices   = (unsigned short*)MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short));

	mesh.vertices[0] = 0;    mesh.vertices[1]  = 0; mesh.vertices[2]  = -0.5;
	mesh.vertices[3] = 0;    mesh.vertices[4]  = 0; mesh.vertices[5]  = 0.5;
	mesh.vertices[6] = 1000; mesh.vertices[7]  = 0; mesh.vertices[8]  = -0.5;
	mesh.vertices[9] = 1000; mesh.vertices[10] = 0; mesh.vertices[11] = 0.5;
	mesh.normals[0] = 0; mesh.normals[1]  = 1; mesh.normals[2]  = 0;
	mesh.normals[3] = 0; mesh.normals[4]  = 1; mesh.normals[5]  = 0;
	mesh.normals[6] = 0; mesh.normals[7]  = 1; mesh.normals[8]  = 0;
	mesh.normals[9] = 0; mesh.normals[10] = 1; mesh.normals[11] = 0;
	mesh.texcoords[0] = 0; mesh.texcoords[1] = 0;
	mesh.texcoords[2] = 0; mesh.texcoords[3] = 1;
	mesh.texcoords[4] = 1; mesh.texcoords[5] = 0;
	mesh.texcoords[6] = 1; mesh.texcoords[7] = 1;

	mesh.indices[0] = 0; mesh.indices[1] = 1; mesh.indices[2] = 3;
	mesh.indices[3] = 0; mesh.indices[4] = 3; mesh.indices[5] = 2;

	UploadMesh(&mesh, false);

	return mesh;
}

Sprites LoadSprites()
{
	Sprites sprs;
	sprs.logo = LoadTexture("sprite/logo.png");
	Image atlas = LoadImage("sprite/chars.png");
	sprs.chars = LoadTextureFromImage(atlas);
	ImageFlipHorizontal(&atlas);
	sprs.charsFlipped = LoadTextureFromImage(atlas);
	UnloadImage(atlas);
	sprs.projs = LoadTexture("sprite/projs.png");
	sprs.hearts = LoadTexture("sprite/hearts.png");
	sprs.billShader = LoadShader(0, "shader/bill.fs");

	sprs.radius.texture = LoadTexture("sprite/radius.png");
	sprs.radius.plane = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
	sprs.radius.plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = sprs.radius.texture;
	sprs.radius.plane.materials[0].shader = sprs.billShader;

	sprs.path.charge.shader = LoadShader(0,"shader/path.fs");
	sprs.path.charge.scroll = GetShaderLocation(sprs.path.charge.shader, "scroll");
	sprs.path.charge.texture = LoadTexture("sprite/charge.png");
	sprs.path.charge.path = LoadModelFromMesh(GenMeshPath());
	sprs.path.charge.path.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = sprs.path.charge.texture;
	sprs.path.charge.path.materials[0].shader = sprs.path.charge.shader;
	sprs.path.hitscanRed.texture = LoadTexture("sprite/hitscanRed.png");
	sprs.path.hitscanRed.path = LoadModelFromMesh(GenMeshPath());
	sprs.path.hitscanRed.path.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = sprs.path.hitscanRed.texture;
	sprs.path.hitscanBlue.texture = LoadTexture("sprite/hitscanBlue.png");
	sprs.path.hitscanBlue.path = LoadModelFromMesh(GenMeshPath());
	sprs.path.hitscanBlue.path.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = sprs.path.hitscanBlue.texture;

	return sprs;
}

void UnloadSprites(Sprites sprs)
{
	UnloadTexture(sprs.logo);
	UnloadTexture(sprs.chars);
	UnloadTexture(sprs.charsFlipped);
	UnloadTexture(sprs.projs);
	UnloadTexture(sprs.hearts);
	UnloadShader(sprs.billShader);
	UnloadTexture(sprs.radius.texture);
	UnloadModel(sprs.radius.plane);
	UnloadShader(sprs.path.charge.shader);
	UnloadTexture(sprs.path.charge.texture);
	UnloadModel(sprs.path.charge.path);
	UnloadTexture(sprs.path.hitscanRed.texture);
	UnloadModel(sprs.path.hitscanRed.path);
	UnloadTexture(sprs.path.hitscanBlue.texture);
	UnloadModel(sprs.path.hitscanBlue.path);
}

//state: 0=default, 1=dashing, 2=stunned
Rectangle CharAtlas(playerid player, bool isPov, bool flipped, int state)
{
	int x = state * 32;
	int y = (player - 1) * 32;
	if (isPov) y = y + 64;
	if (flipped) x = 64 - x;
	return Rectangle{
		static_cast<float>(x),
		static_cast<float>(y),
		32,32};
}

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
	cam.fovy = 60.0f;
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

void gameScene(POV pov, const GameState* state, const Config* cfg, const Camera3D* cam, const Sprites* sprs)
{
	BeginMode3D(*cam);
	{
		//draw players (and their alt shot direction if they're charging)
		BeginShaderMode(sprs->billShader);

		Vec2 camAngle {
			num_det{ cam->target.x - cam->position.x },
			num_det{ cam->target.z - cam->position.z }
		};
		Vec2 camRight = v2::normalize(v2::rotate(camAngle, camAngle.x.half_pi()));

		float p1Shake = 0, p2Shake = 0;
		int p1State = 0, p2State = 0;
		bool p1Mirror = false, p2Mirror = false;
		if (state->p1.stunned)
		{
			p1State = 2;
			p1Shake = state->p1.hitstopCount * GetRandomValue(-10, 10) / 300.0f;
			p1Mirror = v2::dot(camRight, state->p1.vel) < num_det{ 0 };
		}
		else if (state->p1.pushdown.top() == Dashing)
		{
			p1State = 1;
			p1Mirror = v2::dot(camRight, state->p1.dashVel) < num_det{ 0 };
		}
		else p1Mirror = (!pov == Player1) && v2::dot(camRight, state->p1.dir) < num_det{ 0 };

		if (state->p2.stunned)
		{
			p2State = 2;
			p2Shake = state->p2.hitstopCount * GetRandomValue(-10, 10) / 300.0f;
			p2Mirror = v2::dot(camRight, state->p2.vel) < num_det{ 0 };
		}
		else if (state->p2.pushdown.top() == Dashing)
		{
			p2State = 1;
			p2Mirror = v2::dot(camRight, state->p2.dashVel) < num_det{ 0 };
		}
		else p2Mirror = (!pov == Player2) && v2::dot(camRight, state->p2.dir) < num_det{ 0 };

		//player sprites
		DrawBillboardPro(*cam,
			(p1Mirror ? sprs->charsFlipped : sprs->chars),
			CharAtlas(1, (pov == Player1), p1Mirror, p1State),
			fromDetVec2WithShake(state->p1.pos, camRight, fromDetNum(cfg->playerRadius) * 1.5, p1Shake),
			Vector3{ 0,1,0 },
			Vector2{ fromDetNum(cfg->playerRadius) * 3, fromDetNum(cfg->playerRadius) * 3 },
			Vector2{ 0.0f, 0.0f },
			0.0f, WHITE);
		DrawBillboardPro(*cam,
			(p2Mirror ? sprs->charsFlipped : sprs->chars),
			CharAtlas(2, (pov == Player2), p2Mirror, p2State),
			fromDetVec2WithShake(state->p2.pos, camRight, fromDetNum(cfg->playerRadius) * 1.5, p2Shake),
			Vector3{ 0,1,0 },
			Vector2{ fromDetNum(cfg->playerRadius) * 3, fromDetNum(cfg->playerRadius) * 3 },
			Vector2{ 0.0f, 0.0f },
			0.0f, WHITE);
		//player radius
		DrawModel(
			sprs->radius.plane,
			fromDetVec2(state->p1.pos, .01f),
			fromDetNum(cfg->playerRadius) * 2,
			WHITE);
		DrawModel(
			sprs->radius.plane,
			fromDetVec2(state->p2.pos, .01f),
			fromDetNum(cfg->playerRadius) * 2,
			WHITE);

		//draw charge paths
		if (state->p1.pushdown.top() == PState::Charging)
		{
			float angle = RAD2DEG * angleFromDetVec2(state->p1.dir);
			float divert = 1.0f - (static_cast<float>(state->p1.chargeCount) / static_cast<float>(cfg->chargeDuration));
			float scroll = (divert * divert);
			SetShaderValue(sprs->path.charge.shader, sprs->path.charge.scroll, &scroll, SHADER_UNIFORM_FLOAT);
			DrawModelEx(sprs->path.charge.path,
				fromDetVec2(state->p1.pos, .01f),
				Vector3{ 0,-1,0 },
				angle,
				Vector3{ 1,1,.25 },
				WHITE);
			DrawModelEx(sprs->path.charge.path,
				fromDetVec2(state->p1.pos, .005f),
				Vector3{ 0,-1,0 },
				angle + (divert * 30),
				Vector3{ 1,1,.1 },
				WHITE);
			DrawModelEx(sprs->path.charge.path,
				fromDetVec2(state->p1.pos, .005f),
				Vector3{ 0,-1,0 },
				angle - (divert * 30),
				Vector3{ 1,1,.1 },
				WHITE);
		}
		if (state->p2.pushdown.top() == PState::Charging)
		{
			float angle = RAD2DEG * angleFromDetVec2(state->p2.dir);
			float divert = 1.0f - (static_cast<float>(state->p2.chargeCount) / static_cast<float>(cfg->chargeDuration));
			float scroll = (divert * divert);
			SetShaderValue(sprs->path.charge.shader, sprs->path.charge.scroll, &scroll, SHADER_UNIFORM_FLOAT);
			DrawModelEx(sprs->path.charge.path,
				fromDetVec2(state->p2.pos, .01f),
				Vector3{ 0,-1,0 },
				angle,
				Vector3{ 1,1,.25 },
				WHITE);
			DrawModelEx(sprs->path.charge.path,
				fromDetVec2(state->p2.pos, .005f),
				Vector3{ 0,-1,0 },
				angle + (divert * 30),
				Vector3{ 1,1,.1 },
				WHITE);
			DrawModelEx(sprs->path.charge.path,
				fromDetVec2(state->p2.pos, .005f),
				Vector3{ 0,-1,0 },
				angle - (divert * 30),
				Vector3{ 1,1,.1 },
				WHITE);
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
			bool withinReach = pov != Spectator && v2::length(futurePos) < cfg->arenaRadius;
			//draw projectile
			switch (it->owner)
			{
			case 1:
				DrawBillboardPro(*cam,
					sprs->projs,
					Rectangle{ 0,0,32,32 }, //source rect
					fromDetVec2(it->pos, 1.0f), //world pos
					Vector3{ 0.0f,1.0f,0.0f }, //up vector
					Vector2{ fromDetNum(cfg->projRadius) * 4, fromDetNum(cfg->projRadius) * 4 }, //size (proj size is defined by circle with half dimensions of sprite)
					Vector2{ 0.0f, 0.0f }, //anchor for rotation and scaling
					12*it->lifetime, //rotation (degrees per frame)
					WHITE);
				break;
			case 2:
				DrawBillboardPro(*cam,
					sprs->projs,
					Rectangle{ 32,0,32,32 }, //source rect
					fromDetVec2(it->pos, 1.0f), //world pos
					Vector3{ 0.0f,1.0f,0.0f }, //up vector
					Vector2{ fromDetNum(cfg->projRadius) * 4, fromDetNum(cfg->projRadius) * 4 }, //size (proj size is defined by circle with half dimensions of sprite)
					Vector2{ 0.0f, 0.0f }, //anchor for rotation and scaling
					12*it->lifetime, //rotation (degrees per frame)
					WHITE);
				break;
			}
			//draw future position and combo radius
			if (withinReach)
			{
				DrawBillboardPro(*cam,
					sprs->projs,
					Rectangle{ 64,0,32,32 }, //source rect
					fromDetVec2(futurePos, 1.0f), //world pos
					Vector3{ 0.0f,1.0f,0.0f }, //up vector
					Vector2{ fromDetNum(cfg->projRadius) * 2, fromDetNum(cfg->projRadius) * 2 }, //size
					Vector2{ 0.0f, 0.0f }, //anchor for rotation and scaling
					0.0f,
					WHITE);
				if (showCombos)
				{
					DrawCylinderWires(
						fromDetVec2(futurePos),
						fromDetNum(cfg->comboRadius),
						fromDetNum(cfg->comboRadius),
						.1f, 10, PURPLE);
				}
			}
			//draw ground radius
			DrawModel(
				sprs->radius.plane,
				fromDetVec2(it->pos, .01f),
				fromDetNum(cfg->projRadius) * 2,
				WHITE);
		}
		EndShaderMode();
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
double present(POV pov, const GameState* state, const Config* cfg, Camera3D* cam, const Sprites* sprs, std::ostringstream* gameInfoOSS)
{
	setCamera(cam, NULL, state, pov);
	
	BeginDrawing();

	ClearBackground(RAYWHITE);

	gameScene(pov, state, cfg, cam, sprs);

	float size = 6;
	for (int i = 0; i < state->health1; i++)
	{
		DrawTexturePro(sprs->hearts,
			Rectangle{ 0,0,8,8 },
			Rectangle{
				5 + 8 * size * i,
				5,
				8 * size,
				8 * size
			},
			Vector2{ 0,0 }, 0.0f, WHITE);
	}
	for (int i = 0; i < state->health2; i++)
	{
		DrawTexturePro(sprs->hearts,
			Rectangle{ 8,0,8,8 },
			Rectangle{
				screenWidth - 5 - 8 * size * (i+1),
				5,
				8 * size,
				8 * size
			},
			Vector2{ 0,0 }, 0.0f, WHITE);
	}
	for (int i = 0; i < cfg->roundsToWin; i++)
	{
		DrawTexturePro(sprs->hearts,
			(i + 1 <= state->rounds1 ? Rectangle{ 24,0,8,8 } : Rectangle{ 16,0,8,8 }),
			Rectangle{
				5 + 8 * size * i,
				5 + 8 * size,
				8 * size,
				8 * size
			},
			Vector2{ 0,0 }, 0.0f, WHITE);
		DrawTexturePro(sprs->hearts,
			(i + 1 <= state->rounds2 ? Rectangle{ 24,0,8,8 } : Rectangle{ 16,0,8,8 }),
			Rectangle{
				screenWidth - 5 - 8 * size * (i + 1),
				5 + 8 * size,
				8 * size,
				8 * size
			},
			Vector2{ 0,0 }, 0.0f, WHITE);
	}
	//crosshair
	DrawLineEx(
		Vector2{ centerX - 20 , centerY - 150 },
		Vector2{ centerX - 5, centerY - 150 },
		3.f, RED);
	DrawLineEx(
		Vector2{ centerX + 5 , centerY - 150 },
		Vector2{ centerX + 20, centerY - 150 },
		3.f, RED);
	DrawLineEx(
		Vector2{ centerX, centerY - 170 },
		Vector2{ centerX, centerY - 155 },
		3.f, RED);
	DrawLineEx(
		Vector2{ centerX, centerY - 145 },
		Vector2{ centerX, centerY - 130 },
		3.f, RED);
	DrawLineEx(
		Vector2{ centerX, centerY - 70 },
		Vector2{ centerX, centerY - 20 },
		3.f, RED);

	if (pov == Player1)
		drawBars(&state->p1, cfg);
	else if (pov == Player2)
		drawBars(&state->p2, cfg);

	switch (state->phase)
	{
	case Countdown:
		DrawText(std::to_string(static_cast<int>(ceil(state->roundCountdown / 60.0))).c_str(),
			screenWidth / 2 - 50, screenHeight / 2 - 100, 150, BLACK);
		break;
	case Play:
		DrawText(std::to_string(state->roundCountdown / 60).c_str(),
			screenWidth / 2 - 40, 5, 60, BLACK);
		break;
	case End:
		if (state->health1 > state->health2)
			DrawText("P1 WIN!", screenWidth / 2 - 200, screenHeight / 2 - 100, 150, RED);
		else if (state->health1 < state->health2)
			DrawText("P2 WIN!", screenWidth / 2 - 200, screenHeight / 2 - 100, 150, BLUE);
		else
			DrawText("It's a tie.", screenWidth / 2 - 240, screenHeight / 2 - 100, 150, BLACK);
		break;
	}

	DrawText(gameInfoOSS->str().c_str(), 5, 5 + 16 * size, 20, GRAY);

	//I figure this is also the timing semaphore
	double beforeSemaphore = GetTime();
	EndDrawing();
	return GetTime() - beforeSemaphore;
}

//HOME SCREEN PRESENTATION

struct HomeInfo
{
	Vec2 lazyCam = v2::zero();
	bool homeScreen = true;
	std::string remoteAddress = "";
	float freshUpdate = 0;
	RenderTexture2D bgTarget;
	Shader bgShader;
};

void presentMenu(POV pov, const GameState* state, const Config* cfg, Camera3D* cam, const Sprites* sprs, std::ostringstream* gameInfoOSS, HomeInfo* home)
{
	setCamera(cam, &home->lazyCam, state, pov);

	BeginDrawing();

	ClearBackground(RAYWHITE);

	BeginTextureMode(home->bgTarget);
	ClearBackground(RAYWHITE);
	gameScene(pov, state, cfg, cam, sprs);
	EndTextureMode();

	if (home->homeScreen)
	{
		BeginShaderMode(home->bgShader);
		// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
		DrawTextureRec(
			home->bgTarget.texture,
			Rectangle{ 0, 0, (float)home->bgTarget.texture.width, (float)-home->bgTarget.texture.height },
			Vector2 { 0, 0 }, WHITE);
		EndShaderMode();
		
		//LOGO
		int logoSize = 12;
		Vector2 logoPos = {
			screenWidth / 2 - (logoSize * sprs->logo.width / 2),
			screenHeight / 2 - (logoSize * sprs->logo.height / 2)
		};
		DrawTextureEx(
			sprs->logo,
			logoPos,
			0.0f,
			logoSize,
			WHITE);

		//MENU
		float xCenter = screenWidth / 2;
		float btnTop = screenHeight - 60;
		float btnWidth = 240;
		float btnHeight = 40;

		float p1X      = xCenter - 120 - 320;
		float p2X      = xCenter - 120;
		float addressX = xCenter - 120 + 320;
		Rectangle p1Btn = Rectangle{ p1X, btnTop, btnWidth, btnHeight };
		Rectangle p2Btn = Rectangle{ p2X, btnTop, btnWidth, btnHeight };
		Rectangle addressField = Rectangle{ addressX, btnTop, btnWidth, btnHeight };

		Color addressColor = Color{
			static_cast<unsigned char>(130 + home->freshUpdate * (0-130)),
			static_cast<unsigned char>(130 + home->freshUpdate * (158-130)),
			static_cast<unsigned char>(130 + home->freshUpdate * (47-130)),
			255
		};

		DrawRectangleRec(p1Btn, LIGHTGRAY);
		DrawRectangleLinesEx(p1Btn, 4, RED);
		DrawText("Connect as Player 1", p1Btn.x + 10, p1Btn.y + 10, 20, BLACK);
		DrawText("F1", p1Btn.x, p1Btn.y - 20, 20, BLACK);

		DrawRectangleRec(p2Btn, LIGHTGRAY);
		DrawRectangleLinesEx(p2Btn, 4, BLUE);
		DrawText("Connect as Player 2", p2Btn.x + 10, p2Btn.y + 10, 20, BLACK);
		DrawText("F2", p2Btn.x, p2Btn.y - 20, 20, BLACK);

		DrawRectangleRec(addressField, WHITE);
		DrawRectangleLinesEx(addressField, 4, addressColor);
		DrawText(home->remoteAddress.c_str(), addressField.x + 10, addressField.y + 10, 20, BLACK);
		DrawText("Remote IP Address\nCtrl+V to paste it here", addressField.x, addressField.y - 50, 20, BLACK);


		DrawText("Press F4 to show background demo", 880, 5, 20, BLACK);
	}
	else
	{
		DrawTextureRec(
			home->bgTarget.texture,
			Rectangle{ 0, 0, (float)home->bgTarget.texture.width, (float)-home->bgTarget.texture.height },
			Vector2{ 0, 0 }, WHITE);
	}

	DrawText(gameInfoOSS->str().c_str(), 5, 5, 20, BLACK);

	EndDrawing();
}

#endif // !RBST_PRESENTATION_HPP
