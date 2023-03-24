//std
#include <string>
//ETL
#include <etl/vector.h>
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
const size_t MAX_PROJECTILES = 32;

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
	int16 health1 = 0;
	int16 rounds1 = 0;
	bool p1DmgThisFrame = false;

	Player p2;
	int16 health2 = 0;
	int16 rounds2 = 0;
	bool p2DmgThisFrame = false;
	
	etl::vector<Projectile, MAX_PROJECTILES> projs;
};

GameState initialState(const Config* cfg)
{
	GameState state;
	state.frame = 0;
	state.roundCountdown = PREGAME_COUNTDOWN;
	state.phase = RoundPhase::Countdown;

	state.p1.id = 1;
	respawnPlayer(&(state.p1), cfg);
	state.health1 = cfg->playerHealth;
	state.rounds1 = 0;

	state.p2.id = 2;
	respawnPlayer(&(state.p2), cfg);
	state.health2 = cfg->playerHealth;
	state.rounds2 = 0;

	return state;
}

void regDamage(GameState* state, playerid damaged)
{
	if (damaged == 1)
	{
		(state->health1)--;
		state->p1DmgThisFrame = true;
	}
	else
	{
		(state->health2)--;
		state->p2DmgThisFrame = true;
	}
}

void altShot(GameState* state, const Config* cfg, Vec2 origin, Vec2 direction, playerid owner)
{
	Player* opposition;
	if (owner == 1)	opposition = &(state->p2); else opposition = &(state->p1);
	//PARRY
	if (opposition->pushdown.top() == PState::Dashing &&
		opposition->dashCount < cfg->dashPerfect &&
		v2::closest(origin, direction, opposition->perfectPos) < cfg->playerRadius)
	{
		//right back at ya
		origin = v2::add(v2::projection(v2::sub(opposition->perfectPos, origin), direction), origin);
		direction = v2::scalarMult(direction, num_det{ -1 });
		owner = opposition->id;
		opposition->pushdown.push(PState::Hitstop);
		opposition->hitstopCount = cfg->midHitstop;
		if (owner == 1)	opposition = &(state->p2); else opposition = &(state->p1);
	}
	//DIRECT HIT OR GRAZE
	num_det dist = v2::closest(origin, direction, opposition->pos);
	if (!opposition->stunned && dist < cfg->playerRadius)
	{
		damagePlayer(opposition, cfg, origin, 2);
		if (owner == 1)	regDamage(state, 2); else regDamage(state, 1);
	}
	else if (dist < cfg->grazeRadius)
	{
		opposition->ammo = cfg->ammoMax;
		opposition->stamina = cfg->staminaMax;
	}
	//COMBO
	auto it = state->projs.begin();
	while (it != state->projs.end())
	{
		if (v2::closest(origin, direction, it->pos) < cfg->projRadius)
		{
			if (!state->p1.stunned && v2::length(v2::sub(it->pos, state->p1.pos)) < (cfg->comboRadius + cfg->playerRadius))
			{
				damagePlayer(&(state->p1), cfg, it->pos, 3);
				regDamage(state, 1);
			}
			if (!state->p2.stunned && v2::length(v2::sub(it->pos, state->p2.pos)) < (cfg->comboRadius + cfg->playerRadius))
			{
				damagePlayer(&(state->p2), cfg, it->pos, 3);
				regDamage(state, 2);
			}
			state->projs.erase(it);
		}
		else
		{
			++it;
		}
	}
}

Vec2 pickDashDir(Vec2 front, MoveInput mov)
{
	const num_det num;
	const num_det quarter_pi = num.pi() / 4;
	switch (mov)
	{
	case MoveInput::ForLeft:
		return v2::rotate(front, -quarter_pi);
	case MoveInput::Left:
		return v2::rotate(front, -num.half_pi());
	case MoveInput::BackLeft:
		return v2::rotate(front, num.pi() + quarter_pi);
	case MoveInput::Back:
		return v2::scalarMult(front, num_det{ -1 });
	case MoveInput::BackRight:
		return v2::rotate(front, num.pi() - quarter_pi);
	case MoveInput::Right:
		return v2::rotate(front, num.half_pi());
	case MoveInput::ForRight:
		return v2::rotate(front, quarter_pi);
	default:
		return front;
	}
}

GameState simulate(GameState state, const Config* cfg, InputData input)
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
			if (state.rounds1 < cfg->roundsToWin && state.rounds2 < cfg->roundsToWin)
			{
				state.roundCountdown = PREGAME_COUNTDOWN;
				state.phase = RoundPhase::Countdown;
				respawnPlayer(&(state.p1), cfg);
				state.health1 = cfg->playerHealth;
				respawnPlayer(&(state.p2), cfg);
				state.health2 = cfg->playerHealth;
				state.projs.clear();
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
		input.p1Input = PlayerInput{};
		input.p2Input = PlayerInput{};
	case RoundPhase::Play:
		state.p1DmgThisFrame = false;
		state.p2DmgThisFrame = false;

		//PLAYER 1
		movePlayer(&(state.p1), cfg, input.p1Input);
		state.p1.ammo = std::min(++state.p1.ammo, cfg->ammoMax);
		state.p1.stamina = std::min(++state.p1.stamina, cfg->staminaMax);
		switch (state.p1.pushdown.top())
		{
		case PState::Dashing:
			state.p1.dashCount++;
			if (state.p1.dashCount >= cfg->dashDuration)
			{
				state.p1.pushdown.pop();
			}
			break;
		case PState::Charging:
			state.p1.chargeCount++;
			break;
		case PState::Hitstop:
			state.p1.hitstopCount--;
			if (state.p1.hitstopCount <= 0)
			{
				state.p1.pushdown.pop();
			}
			break;
		}
		//PLAYER 2
		movePlayer(&(state.p2), cfg, input.p2Input);
		state.p2.ammo = std::min(++state.p2.ammo, cfg->ammoMax);
		state.p2.stamina = std::min(++state.p2.stamina, cfg->staminaMax);
		switch (state.p2.pushdown.top())
		{
		case PState::Dashing:
			state.p2.dashCount++;
			break;
		case PState::Charging:
			state.p2.chargeCount++;
			break;
		case PState::Hitstop:
			state.p2.hitstopCount--;
			if (state.p2.hitstopCount <= 0)
			{
				state.p2.pushdown.pop();
			}
			break;
		}
		//PROJECTILES - MOVE AND CHECK FOR COLLISION OR PARRY
		auto it = state.projs.begin();
		while (it != state.projs.end())
		{
			bool erased = false;
			it->pos = v2::add(it->pos, it->vel);
			if (v2::length(it->pos) > cfg->arenaRadius)
			{
				state.projs.erase(it);
				erased = true;
			}
			else
			{
				switch (it->owner)
				{
				case 1:
					if (state.p2.pushdown.top() == PState::Dashing &&
						state.p2.dashCount < cfg->dashPerfect &&
						v2::length(v2::sub(it->pos, state.p2.perfectPos)) < (cfg->playerRadius + cfg->projRadius))
					{
						//ayo a parry just happened, send that projectile back
						it->owner = 2;
						num_det newSpeed = v2::length(it->vel) * cfg->projCounterMultiply;
						it->vel = v2::normalizeMult(v2::sub(state.p1.pos, it->pos), newSpeed);
						state.p2.pushdown.push(PState::Hitstop);
						state.p2.hitstopCount = cfg->weakHitstop;
					}
					else if (!state.p2.stunned && v2::length(v2::sub(it->pos, state.p2.pos)) < (cfg->playerRadius + cfg->projRadius))
					{
						damagePlayer(&(state.p2), cfg, it->pos, 1);
						regDamage(&state, 2);
						state.projs.erase(it);
						erased = true;
					}
					break;
				case 2:
					if (state.p1.pushdown.top() == PState::Dashing &&
						state.p1.dashCount < cfg->dashPerfect &&
						v2::length(v2::sub(it->pos, state.p1.perfectPos)) < (cfg->playerRadius + cfg->projRadius))
					{
						//ayo a parry just happened, send that projectile back
						it->owner = 1;
						num_det newSpeed = v2::length(it->vel) * cfg->projCounterMultiply;
						it->vel = v2::normalizeMult(v2::sub(state.p2.pos, it->pos), newSpeed);
						state.p1.pushdown.push(PState::Hitstop);
						state.p1.hitstopCount = cfg->weakHitstop;
					}
					else if (!state.p1.stunned && v2::length(v2::sub(it->pos, state.p1.pos)) < (cfg->playerRadius + cfg->projRadius))
					{
						damagePlayer(&(state.p1), cfg, it->pos, 1);
						regDamage(&state, 1);
						state.projs.erase(it);
						erased = true;
					}
					break;
				}
			}
			if (!erased) ++it;
		}
		//DASHING
		bool directColl = v2::length(v2::sub(state.p1.pos, state.p2.pos)) < (cfg->playerRadius + cfg->playerRadius);
		//BOTH DASHING IN THIS FRAME
		if (state.p1.pushdown.top() == PState::Dashing && state.p2.pushdown.top() == PState::Dashing)
		{
			//P2 PERFECT EVADES
			if (state.p2.dashCount < cfg->dashPerfect && (v2::length(v2::sub(state.p1.pos, state.p2.perfectPos))) < (cfg->playerRadius + cfg->playerRadius))
			{
				damagePlayer(&(state.p1), cfg, state.p2.perfectPos, 2);
				regDamage(&state, 1);
				state.p2.pushdown.push(PState::Hitstop);
				state.p2.hitstopCount = cfg->midHitstop;
			}
			//P1 PERFECT EVADES
			else if (state.p1.dashCount < cfg->dashPerfect && (v2::length(v2::sub(state.p2.pos, state.p1.perfectPos))) < (cfg->playerRadius + cfg->playerRadius))
			{
				damagePlayer(&(state.p2), cfg, state.p1.perfectPos, 2);
				regDamage(&state, 2);
				state.p1.pushdown.push(PState::Hitstop);
				state.p1.hitstopCount = cfg->midHitstop;
			}
			else if (directColl)
			{
				//DIRECT HIT, MOST RECENT DASH LOSES
				int dashDiff = state.p1.dashCount - state.p2.dashCount;
				if (dashDiff > 0)
				{
					damagePlayer(&(state.p2), cfg, state.p1.pos, 2);
					regDamage(&state, 2);
					state.p1.pushdown.push(PState::Hitstop);
					state.p1.hitstopCount = cfg->midHitstop;
				}
				else if (dashDiff < 0)
				{
					damagePlayer(&(state.p1), cfg, state.p2.pos, 2);
					regDamage(&state, 1);
					state.p2.pushdown.push(PState::Hitstop);
					state.p2.hitstopCount = cfg->midHitstop;
				}
				else
				{
					damagePlayer(&(state.p1), cfg, state.p2.pos, 2);
					regDamage(&state, 1);
					damagePlayer(&(state.p2), cfg, state.p1.pos, 2);
					regDamage(&state, 2);
				}
			}
		}
		//P1 HITS P2
		else if (directColl && !state.p2.stunned && state.p1.pushdown.top() == PState::Dashing)
		{
			damagePlayer(&(state.p2), cfg, state.p1.pos, 2);
			regDamage(&state, 2);
			state.p1.pushdown.push(PState::Hitstop);
			state.p1.hitstopCount = cfg->midHitstop;
		}
		//P2 HITS P1
		else if (directColl && !state.p1.stunned && state.p2.pushdown.top() == PState::Dashing)
		{
			damagePlayer(&(state.p1), cfg, state.p2.pos, 2);
			regDamage(&state, 1);
			state.p2.pushdown.push(PState::Hitstop);
			state.p2.hitstopCount = cfg->midHitstop;
		}

		//ALT SHOT
		if (state.p1.pushdown.top() == PState::Charging && state.p1.chargeCount >= cfg->chargeDuration)
		{
			state.p1.pushdown.pop();
			altShot(&state, cfg, state.p1.pos, state.p1.dir, 1);
		}
		if (state.p2.pushdown.top() == PState::Charging && state.p2.chargeCount >= cfg->chargeDuration)
		{
			state.p2.pushdown.pop();
			altShot(&state, cfg, state.p2.pos, state.p2.dir, 2);
		}

		state.p1.stunned = state.p1.stunned || state.p1DmgThisFrame;
		state.p2.stunned = state.p2.stunned || state.p2DmgThisFrame;

		//PLAYER 1 ATTACKS
		if (state.p1.stunned)
		{
			//break out of stun at the cost of a dash
			if (input.p1Input.atk == AttackInput::Dash && !state.p1DmgThisFrame && state.p1.stamina >= cfg->dashCost)
			{
				state.p1.vel = v2::scalarMult(pickDashDir(state.p1.dir, input.p1Input.mov), cfg->playerDashSpeed);
				state.p1.stamina = state.p1.stamina - cfg->dashCost;
				state.p1.stunned = false;
			}
			//cancel your next move nevertheless
			input.p1Input.atk == AttackInput::None;
		}
		if (state.p1.pushdown.top() == PState::Default)
		{
			switch (input.p1Input.atk)
			{
			case AttackInput::Dash:
				if (state.p1.stamina < cfg->dashCost) break;
				state.p1.dashCount = 0;
				state.p1.dashVel = v2::scalarMult(pickDashDir(state.p1.dir, input.p1Input.mov), cfg->playerDashSpeed);
				state.p1.perfectPos = state.p1.pos;
				state.p1.pushdown.push(PState::Dashing);
				state.p1.stamina = state.p1.stamina - cfg->dashCost;
				break;
			case AttackInput::Shot:
				if (state.p1.ammo < cfg->shotCost) break;
				state.projs.push_back({ state.p1.pos, v2::scalarMult(state.p1.dir, cfg->projSpeed), 1 });
				state.p1.ammo = state.p1.ammo - cfg->shotCost;
				break;
			case AttackInput::AltShot:
				if (state.p1.ammo < cfg->altShotCost) break;
				state.p1.chargeCount = 0;
				state.p1.pushdown.push(PState::Charging);
				state.p1.ammo = state.p1.ammo - cfg->altShotCost;
				break;
			}
		}
		//WEAVE A DASH INTO ANOTHER
		else if (state.p1.pushdown.top() == PState::Dashing &&
			input.p1Input.atk == AttackInput::Dash &&
			state.p1.stamina >= cfg->dashCost)
		{
			state.p1.dashCount = 0;
			state.p1.dashVel = v2::scalarMult(pickDashDir(state.p1.dir, input.p1Input.mov), cfg->playerDashSpeed);
			state.p1.perfectPos = state.p1.pos;
			state.p1.stamina = state.p1.stamina - cfg->dashCost;
		}

		//PLAYER 2 ATTACKS
		if (state.p2.stunned)
		{
			//break out of stun at the cost of a dash
			if (input.p2Input.atk == AttackInput::Dash && !state.p2DmgThisFrame && state.p2.stamina >= cfg->dashCost)
			{
				state.p2.vel = v2::scalarMult(pickDashDir(state.p1.dir, input.p1Input.mov), cfg->playerDashSpeed);
				state.p2.stamina = state.p2.stamina - cfg->dashCost;
				state.p2.stunned = false;
			}
			//cancel your next move nevertheless
			input.p2Input.atk == AttackInput::None;
		}
		if (state.p2.pushdown.top() == PState::Default)
		{
			switch (input.p2Input.atk)
			{
			case AttackInput::Dash:
				if (state.p2.stamina < cfg->dashCost) break;
				state.p2.dashCount = 0;
				state.p2.dashVel = v2::scalarMult(pickDashDir(state.p1.dir, input.p1Input.mov), cfg->playerDashSpeed);
				state.p2.perfectPos = state.p2.pos;
				state.p2.pushdown.push(PState::Dashing);
				state.p2.stamina = state.p2.stamina - cfg->dashCost;
				break;
			case AttackInput::Shot:
				if (state.p2.ammo < cfg->shotCost) break;
				state.projs.push_back({ state.p2.pos, v2::scalarMult(state.p2.dir, cfg->projSpeed), 1 });
				state.p2.ammo = state.p2.ammo - cfg->shotCost;
				break;
			case AttackInput::AltShot:
				if (state.p2.ammo < cfg->altShotCost) break;
				state.p2.chargeCount = 0;
				state.p2.pushdown.push(PState::Charging);
				state.p2.ammo = state.p2.ammo - cfg->altShotCost;
				break;
			}
		}
		//WEAVE A DASH INTO ANOTHER
		else if (state.p2.pushdown.top() == PState::Dashing &&
			input.p2Input.atk == AttackInput::Dash &&
			state.p2.stamina >= cfg->dashCost)
		{
			state.p2.dashCount = 0;
			state.p2.dashVel = v2::scalarMult(pickDashDir(state.p1.dir, input.p1Input.mov), cfg->playerDashSpeed);
			state.p2.perfectPos = state.p2.pos;
			state.p2.stamina = state.p2.stamina - cfg->dashCost;
		}

		//ROUND END
		if (state.phase == RoundPhase::Play && (state.roundCountdown <= 0 || state.health1 <= 0 || state.health2 <= 0))
		{
			if (state.health1 > state.health2)
			{
				state.rounds1++;
			}
			else if (state.health2 > state.health1)
			{
				state.rounds2++;
			}
			state.phase = RoundPhase::End;
			state.roundCountdown = ROUNDEND_COUNTDOWN;
		}

		break;
	}
	return state;
}

inline Vector3 fromDetVec2(Vec2 vec, float height=0.0f)
{
	return Vector3{ static_cast<float>(vec.x), height, static_cast<float>(vec.y) };
}

inline float fromDetNum(num_det num)
{
	return static_cast<float>(num);
}

int main(int argc, char* argv[])
{
	const int screenWidth = 800;
	const int screenHeight = 600;

	InitWindow(screenWidth, screenHeight, "Game");
	SetTargetFPS(60);
	SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
	DisableCursor();

	Camera3D cam = { 0 };
	cam.position = Vector3{ 0.0f, 15.0f, 15.0f };  // Camera position
	cam.target = Vector3{ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	cam.up = Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	cam.fovy = 70.0f;                                // Camera field-of-view Y
	cam.projection = CAMERA_PERSPECTIVE;             // Camera mode type

	num_det camBack{ -5 };
	float camHeight = 4;
	num_det tgtFront{ 8 };
	float tgtHeight = 0;

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

		//simulation
		double before = GetTime();
		state = simulate(state, &cfg, input);
		double after = GetTime();
		worstFrame = std::max(worstFrame, after - before);

		//secondary simulation (holds its own state)
		// probably not doing it here lol

		//secondary simulation (doesn't hold its own state)
		cam.position = fromDetVec2(v2::add(state.p1.pos, v2::scalarMult(state.p1.dir, camBack)), camHeight);
		cam.target = fromDetVec2(v2::add(state.p1.pos, v2::scalarMult(state.p1.dir, tgtFront)), tgtHeight);

		//presentation

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(cam);
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
				Vector3{0.0f, 0.0f, 0.0f},
				fromDetNum(cfg.arenaRadius),
				fromDetNum(cfg.arenaRadius),
				.1f, 10, BLACK);
			DrawGrid(10, static_cast<float>(cfg.arenaRadius)/10.0f);
		EndMode3D();

		DrawRectangle((screenWidth / 2) - 150, screenHeight - 60, (300 * state.p1.ammo) / cfg.ammoMax, 20, GREEN);
		for (int i = 1; i <= (cfg.ammoMax / cfg.shotCost); i++)
		{
			int lineX = (screenWidth / 2) - 150 + (300 * i * cfg.shotCost) / cfg.ammoMax;
			DrawLine(lineX, screenHeight - 65, lineX, screenHeight - 25, BLACK);
		}
		DrawRectangle((screenWidth / 2) - 150, screenHeight - 30, (300 * state.p1.stamina) / cfg.staminaMax, 20, YELLOW);
		for (int i = 1; i <= (cfg.staminaMax / cfg.dashCost); i++)
		{
			int lineX = (screenWidth / 2) - 150 + (300 * i * cfg.dashCost) / cfg.staminaMax;
			DrawLine(lineX, screenHeight - 35, lineX, screenHeight - 5, BLACK);
		}

		int currentFps = GetFPS();
		gameInfoOSS.str("");
		gameInfoOSS << "FPS: " << currentFps << std::endl;
		gameInfoOSS << "Simulation cost: " << (after - before) * 1000 << " ms" << std::endl;
		gameInfoOSS << "(Worst frame yet: " << worstFrame * 1000 << " ms)" << std::endl;
		gameInfoOSS << "P1 HP: " << state.health1 << "; ";
		gameInfoOSS << "P2 HP: " << state.health2 << std::endl;
		gameInfoOSS << "Round Phase: " << std::to_string(state.phase) << "; Round Countdown: " << (state.roundCountdown / 60) << "." << (state.roundCountdown % 60) << std::endl;
		DrawText(gameInfoOSS.str().c_str(), 5, 5, 20, GRAY);

		EndDrawing();
	}

	CloseWindow();
}