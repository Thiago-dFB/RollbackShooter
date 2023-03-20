//std
#include <string>
#include <stack>
#include <vector> 
//GGPO
#include <ggponet.h>
//Raylib
#include <raylib.h>

#include "Math.hpp"

using playerid = std::uint8_t;

const int PREGAME_COUNTDOWN = 180;
const int ROUNDTIMER_COUNTDOWN = 120 * 60;
const int ROUNDEND_COUNTDOWN = 300;

enum PState
{
	Standby,
	Default,
	Charging,
	Dashing,
	Hitstop
};

enum RoundPhase
{
	Countdown,
	Play,
	End
};

enum AttackInput
{
	None=0,
	Shot=1,
	AltShot=2,
	Dash=3
};

enum MoveInput
{
	BackLeft=1,
	Back=2,
	BackRight=3,
	Left=4,
	Neutral=5,
	Right=6,
	ForLeft=7,
	Forward=8,
	ForRight=9
};

struct PlayerInput
{
	AttackInput atk;
	MoveInput mov;
	//horizontal mouse movement (already in radians)
	num8_24 mouse;
};

struct InputData
{
	PlayerInput p1Input;
	PlayerInput p2Input;
};

struct Player
{
	playerid id;
	std::stack<PState> pushdown;
	//xform
	Vec2 pos;
	Vec2 vel;
	Vec2 dir;
	//shots
	int ammo;
	int chargeCount;
	//dash
	int stamina;
	Vec2 perfectPos;
	Vec2 dashDir;
	int dashCount;
	//other countdowns
	int hitstopCount;
	int stunCount;
};

struct Projectile
{
	Vec2 pos;
	Vec2 vel;
	playerid owner;
};

struct Config
{
	int ammoMax;
	int shotCost;
	int altShotCost;
	int staminaMax;
	int dashCost;
	num8_24 playerRadius;
	num8_24 projRadius;
	num8_24 comboRadius;
	num8_24 arenaRadius;
	num8_24 spawnRadius;
	num8_24 projSpeed;
	num8_24 projCounterMultiply;
	num8_24 playerWalkSpeed;
	num8_24 playerWalkAccel;
	num8_24 playerWalkFric;
	num8_24 playerDashSpeed;
	int dashDuration;
	int dashPerfect;
	int chargeDuration;
	int hitstopDuration;
	int playerHealth;
	int roundsToWin;
};

struct GameState
{
	long frame;
	int roundCountdown;
	RoundPhase phase;
	
	Player p1;
	std::int8_t health1;
	std::int8_t rounds1;

	Player p2;
	std::int8_t health2;
	std::int8_t rounds2;
	
	std::vector<Projectile> projs;
};

Player respawn(Player player, Config cfg)
{
	switch (player.id)
	{
	case 1:
		player.pos = v2::scalarMult(v2::left(), cfg.spawnRadius);
		player.dir = v2::right();
		break;
	case 2:
		player.pos = v2::scalarMult(v2::right(), cfg.spawnRadius);
		player.dir = v2::left();
		break;
	}
	player.vel = v2::zero();
	player.ammo = cfg.ammoMax;
	player.chargeCount = 0;
	player.stamina = cfg.staminaMax;
	player.perfectPos = v2::zero();
	player.dashDir = player.dir;
	player.dashCount = 0;
	player.hitstopCount = 0;
	player.stunCount = 0;

	while (!player.pushdown.empty())
	{
		player.pushdown.pop();
	}
	player.pushdown.push(PState::Default);

	return player;
}

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

Player simPlayer(Player player, Config cfg, PlayerInput input)
{
	if ((input.atk == AttackInput::Shot && player.ammo < cfg.shotCost) ||
		(input.atk == AttackInput::AltShot && player.ammo < cfg.altShotCost) ||
		(input.atk == AttackInput::Dash && player.stamina < cfg.dashCost) )
	{
		input.atk = AttackInput::None;
	}

	switch (player.pushdown.top())
	{
	case PState::Standby:
		input.atk = AttackInput::None;
		input.mov = MoveInput::Neutral;
		input.mouse = num8_24{ 0 };
		//no break, fall through to default
	case PState::Default:
		//MOUSE MOVEMENT
		player.dir = v2::rotate(player.dir, input.mouse);

		//NORMAL WALKING MOVEMENT - ACCELERATION
		num8_24 speed = v2::length(player.vel);
		Vec2 impulse = v2::scalarMult(player.dir, cfg.playerWalkAccel);
		num8_24 quarter_pi = speed.pi() / 4;
		switch (input.mov)
		{
		case MoveInput::Neutral:
			if (speed < cfg.playerWalkFric)
			{
				player.vel = v2::zero();
			}
			else
			{
				//every opposite vector gets normalized to friction
				impulse = v2::scalarMult(player.vel, num8_24{-1});
			}
			break;
		case MoveInput::ForLeft:
			impulse = v2::rotate(impulse, quarter_pi);
			break;
		case MoveInput::Left:
			impulse = v2::rotate(impulse, speed.half_pi());
			break;
		case MoveInput::BackLeft:
			impulse = v2::rotate(impulse, speed.pi() - quarter_pi);
			break;
		case MoveInput::Back:
			impulse = v2::scalarMult(impulse, num8_24{ -1 });
			break;
		case MoveInput::BackRight:
			impulse = v2::rotate(impulse, speed.pi() + quarter_pi);
			break;
		case MoveInput::Right:
			impulse = v2::rotate(impulse, -speed.half_pi());
			break;
		case MoveInput::ForRight:
			impulse = v2::rotate(impulse, -quarter_pi);
			break;
		}
		//if player is backpedaling, turn directly opposite force into friction
		if (v2::dot(player.vel, impulse) < num8_24{ 0 })
		{
			impulse = v2::rejection(impulse, player.vel);
			impulse = v2::add(impulse, v2::normalizeMult(player.vel, -cfg.playerWalkFric));
		}
		player.vel = v2::add(player.vel, impulse);
		if (v2::length(player.vel) > cfg.playerWalkSpeed)
		{
			player.vel = v2::normalizeMult(player.vel, cfg.playerWalkSpeed);
		}
		
		//NORMAL WALKING MOVEMENT - DISPLACEMENT AND CORRECTION
		player.pos = v2::add(player.pos, player.vel);
		if (v2::length(player.pos) > cfg.arenaRadius)
		{
			player.pos = v2::normalizeMult(player.pos, cfg.arenaRadius);
			player.vel = v2::rejection(player.vel, player.pos);
		}
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