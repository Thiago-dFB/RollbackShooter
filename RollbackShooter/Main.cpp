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
	GameState state;
	state.frame = 0;
	state.roundCountdown = PREGAME_COUNTDOWN;
	state.phase = RoundPhase::Countdown;

	state.p1.id = 1;
	respawnPlayer(&(state.p1), cfg);
	state.health1 = cfg.playerHealth;
	state.rounds1 = 0;

	state.p2.id = 2;
	respawnPlayer(&(state.p2), cfg);
	state.health2 = cfg.playerHealth;
	state.rounds2 = 0;

	return state;
}

void altShot(GameState* state, Config cfg, Vec2 origin, Vec2 direction, playerid owner)
{
	Player* opposition;
	if (owner == 1)	opposition = &(state->p2); else opposition = &(state->p1);
	//PARRY
	if (opposition->pushdown.top() == PState::Dashing &&
		opposition->dashCount < cfg.dashPerfect &&
		v2::closest(origin, direction, opposition->perfectPos) < cfg.playerRadius)
	{
		//right back at ya
		origin = v2::add(v2::projection(v2::sub(opposition->perfectPos, origin), direction), origin);
		direction = v2::scalarMult(direction, num_det{ -1 });
		owner = opposition->id;
		opposition->pushdown.push(PState::Hitstop);
		opposition->hitstopCount = cfg.midHitstop;
		if (owner == 1)	opposition = &(state->p2); else opposition = &(state->p1);
	}
	//DIRECT HIT OR GRAZE
	num_det dist = v2::closest(origin, direction, opposition->pos);
	if (dist < cfg.playerRadius)
	{
		damagePlayer(opposition, cfg, origin, 2);
		if (owner == 1)	(state->health2)--; else (state->health1)--;
	}
	else if (dist < cfg.grazeRadius)
	{
		opposition->ammo = cfg.ammoMax;
		opposition->stamina = cfg.staminaMax;
	}
	//COMBO
	auto it = state->projs.begin();
	while (it != state->projs.end())
	{
		if (v2::closest(origin, direction, it->pos) < cfg.projRadius)
		{
			if (v2::length(v2::sub(it->pos, state->p1.pos)) < (cfg.comboRadius + cfg.playerRadius))
			{
				damagePlayer(&(state->p1), cfg, it->pos, 3);
				(state->health1)--;
			}
			if (v2::length(v2::sub(it->pos, state->p2.pos)) < (cfg.comboRadius + cfg.playerRadius))
			{
				damagePlayer(&(state->p2), cfg, it->pos, 3);
				(state->health2)--;
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
		return v2::rotate(front, quarter_pi);
	case MoveInput::Left:
		return v2::rotate(front, num.half_pi());
	case MoveInput::BackLeft:
		return v2::rotate(front, num.pi() - quarter_pi);
	case MoveInput::Back:
		return v2::scalarMult(front, num_det{ -1 });
	case MoveInput::BackRight:
		return v2::rotate(front, num.pi() + quarter_pi);
	case MoveInput::Right:
		return v2::rotate(front, -num.half_pi());
	case MoveInput::ForRight:
		return v2::rotate(front, -quarter_pi);
	default:
		return front;
	}
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
				respawnPlayer(&(state.p1), cfg);
				state.health1 = cfg.playerHealth;
				respawnPlayer(&(state.p2), cfg);
				state.health2 = cfg.playerHealth;
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
	case RoundPhase::Play:
		//PLAYER 1
		movePlayer(&(state.p1), cfg, input.p1Input);
		switch (state.p1.pushdown.top())
		{
		case PState::Dashing:
			state.p1.dashCount++;
			if (state.p1.dashCount >= cfg.dashDuration)
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
			if (v2::length(it->pos) > cfg.arenaRadius)
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
						state.p2.dashCount < cfg.dashPerfect &&
						v2::length(v2::sub(it->pos, state.p2.perfectPos)) < (cfg.playerRadius + cfg.projRadius))
					{
						//ayo a parry just happened, send that projectile back
						it->owner = 2;
						num_det newSpeed = v2::length(it->vel) * cfg.projCounterMultiply;
						it->vel = v2::normalizeMult(v2::sub(state.p1.pos, it->pos), newSpeed);
						state.p2.pushdown.push(PState::Hitstop);
						state.p2.hitstopCount = cfg.weakHitstop;
					}
					else if (v2::length(v2::sub(it->pos, state.p2.pos)) < (cfg.playerRadius + cfg.projRadius))
					{
						damagePlayer(&(state.p2), cfg, it->pos, 1);
						state.health2--;
						state.projs.erase(it);
						erased = true;
					}
					break;
				case 2:
					if (state.p1.pushdown.top() == PState::Dashing &&
						state.p1.dashCount < cfg.dashPerfect &&
						v2::length(v2::sub(it->pos, state.p1.perfectPos)) < (cfg.playerRadius + cfg.projRadius))
					{
						//ayo a parry just happened, send that projectile back
						it->owner = 1;
						num_det newSpeed = v2::length(it->vel) * cfg.projCounterMultiply;
						it->vel = v2::normalizeMult(v2::sub(state.p2.pos, it->pos), newSpeed);
						state.p1.pushdown.push(PState::Hitstop);
						state.p1.hitstopCount = cfg.weakHitstop;
					}
					else if (v2::length(v2::sub(it->pos, state.p1.pos)) < (cfg.playerRadius + cfg.projRadius))
					{
						damagePlayer(&(state.p1), cfg, it->pos, 1);
						state.health1--;
						state.projs.erase(it);
						erased = true;
					}
					break;
				}
			}
			if (!erased) ++it;
		}
		//DASHING
		bool directColl = v2::length(v2::sub(state.p1.pos, state.p2.pos)) < (cfg.playerRadius + cfg.playerRadius);
		//BOTH DASHING IN THIS FRAME
		if (state.p1.pushdown.top() == PState::Dashing && state.p2.pushdown.top() == PState::Dashing)
		{
			//P2 PERFECT EVADES
			if (state.p2.dashCount < cfg.dashPerfect && (v2::length(v2::sub(state.p1.pos, state.p2.perfectPos))) < (cfg.playerRadius + cfg.playerRadius))
			{
				damagePlayer(&(state.p1), cfg, state.p2.perfectPos, 2);
				state.health1--;
				state.p2.pushdown.push(PState::Hitstop);
				state.p2.hitstopCount = cfg.midHitstop;
			}
			//P1 PERFECT EVADES
			else if (state.p1.dashCount < cfg.dashPerfect && (v2::length(v2::sub(state.p2.pos, state.p1.perfectPos))) < (cfg.playerRadius + cfg.playerRadius))
			{
				damagePlayer(&(state.p2), cfg, state.p1.perfectPos, 2);
				state.health2--;
				state.p1.pushdown.push(PState::Hitstop);
				state.p1.hitstopCount = cfg.midHitstop;
			}
			else if (directColl)
			{
				//DIRECT HIT, MOST RECENT DASH LOSES
				int dashDiff = state.p1.dashCount - state.p2.dashCount;
				if (dashDiff > 0)
				{
					damagePlayer(&(state.p2), cfg, state.p1.pos, 2);
					state.health2--;
					state.p1.pushdown.push(PState::Hitstop);
					state.p1.hitstopCount = cfg.midHitstop;
				}
				else if (dashDiff < 0)
				{
					damagePlayer(&(state.p1), cfg, state.p2.pos, 2);
					state.health1--;
					state.p2.pushdown.push(PState::Hitstop);
					state.p2.hitstopCount = cfg.midHitstop;
				}
				else
				{
					damagePlayer(&(state.p1), cfg, state.p2.pos, 2);
					state.health1--;
					damagePlayer(&(state.p2), cfg, state.p1.pos, 2);
					state.health2--;
				}
			}
		}
		//P1 HITS P2
		else if (directColl && state.p1.pushdown.top() == PState::Dashing)
		{
			damagePlayer(&(state.p2), cfg, state.p1.pos, 2);
			state.health2--;
			state.p1.pushdown.push(PState::Hitstop);
			state.p1.hitstopCount = cfg.midHitstop;
		}
		//P2 HITS P1
		else if (directColl && state.p2.pushdown.top() == PState::Dashing)
		{
			damagePlayer(&(state.p1), cfg, state.p2.pos, 2);
			state.health1--;
			state.p2.pushdown.push(PState::Hitstop);
			state.p2.hitstopCount = cfg.midHitstop;
		}

		//ALT SHOT
		if (state.p1.pushdown.top() == PState::Charging && state.p1.chargeCount >= cfg.chargeDuration)
		{
			state.p1.pushdown.pop();
			altShot(&state, cfg, state.p1.pos, state.p1.dir, 1);
		}
		if (state.p2.pushdown.top() == PState::Charging && state.p2.chargeCount >= cfg.chargeDuration)
		{
			state.p2.pushdown.pop();
			altShot(&state, cfg, state.p2.pos, state.p2.dir, 2);
		}

		//PLAYER 1 ATTACKS
		if (state.p1.pushdown.top() == PState::Default)
		{
			switch (input.p1Input.atk)
			{
			case AttackInput::Dash:
				if (state.p1.stamina < cfg.dashCost) break;
				state.p1.dashCount = 0;
				state.p1.dashVel = pickDashDir(v2::scalarMult(state.p1.dir, cfg.playerDashSpeed), input.p1Input.mov);
				state.p1.pushdown.push(PState::Dashing);
				state.p1.stamina = state.p1.stamina - cfg.dashCost;
				break;
			case AttackInput::Shot:
				if (state.p1.ammo < cfg.shotCost) break;
				state.projs.push_back({ state.p1.pos, v2::scalarMult(state.p1.dir, cfg.projSpeed), 1 });
				state.p1.ammo = state.p1.ammo - cfg.shotCost;
				break;
			case AttackInput::AltShot:
				if (state.p1.ammo < cfg.altShotCost) break;
				state.p1.chargeCount = 0;
				state.p1.pushdown.push(PState::Charging);
				state.p1.ammo = state.p1.ammo - cfg.altShotCost;
				break;
			}
		}
		//WEAVE A DASH INTO ANOTHER
		else if (state.p1.pushdown.top() == PState::Dashing &&
			input.p1Input.atk == AttackInput::Dash &&
			state.p1.stamina >= cfg.dashCost)
		{
			state.p1.dashCount = 0;
			state.p1.dashVel = pickDashDir(v2::scalarMult(state.p1.dir, cfg.playerDashSpeed), input.p1Input.mov);
			state.p1.stamina = state.p1.stamina - cfg.dashCost;
		}

		//PLAYER 2 ATTACKS
		if (state.p2.pushdown.top() == PState::Default)
		{
			switch (input.p2Input.atk)
			{
			case AttackInput::Dash:
				if (state.p2.stamina < cfg.dashCost) break;
				state.p2.dashCount = 0;
				state.p2.dashVel = pickDashDir(v2::scalarMult(state.p2.dir, cfg.playerDashSpeed), input.p2Input.mov);
				state.p2.pushdown.push(PState::Dashing);
				state.p2.stamina = state.p2.stamina - cfg.dashCost;
				break;
			case AttackInput::Shot:
				if (state.p2.ammo < cfg.shotCost) break;
				state.projs.push_back({ state.p2.pos, v2::scalarMult(state.p2.dir, cfg.projSpeed), 1 });
				state.p2.ammo = state.p2.ammo - cfg.shotCost;
				break;
			case AttackInput::AltShot:
				if (state.p2.ammo < cfg.altShotCost) break;
				state.p2.chargeCount = 0;
				state.p2.pushdown.push(PState::Charging);
				state.p2.ammo = state.p2.ammo - cfg.altShotCost;
				break;
			}
		}
		//WEAVE A DASH INTO ANOTHER
		else if (state.p2.pushdown.top() == PState::Dashing &&
			input.p2Input.atk == AttackInput::Dash &&
			state.p2.stamina >= cfg.dashCost)
		{
			state.p2.dashCount = 0;
			state.p2.dashVel = pickDashDir(v2::scalarMult(state.p2.dir, cfg.playerDashSpeed), input.p2Input.mov);
			state.p2.stamina = state.p2.stamina - cfg.dashCost;
		}

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