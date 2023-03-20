//std
#include <string>
#include <stack>
#include <vector> 
//GGPO
#include <ggponet.h>
//Raylib
#include <raylib.h>
//fpm
#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

using num8_24 = fpm::fixed_8_24;
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

struct Vec2
{
	num8_24 x;
	num8_24 y;
};

namespace v2
{
	inline Vec2 zero()
	{
		return Vec2{ num8_24{ 0 }, num8_24{ 0 } };
	}

	inline Vec2 up()
	{
		return Vec2{ num8_24{ 0 }, num8_24{ 1 } };
	}

	inline Vec2 down()
	{
		return Vec2{ num8_24{ 0 }, num8_24{ -1 } };
	}

	inline Vec2 left()
	{
		return Vec2{ num8_24{ -1 }, num8_24{ 0 } };
	}

	inline Vec2 right()
	{
		return Vec2{ num8_24{ 1 }, num8_24{ 0 } };
	}
	
	inline Vec2 add(Vec2 a, Vec2 b)
	{
		return Vec2{ a.x + b.x, a.y + b.y };
	}

	inline Vec2 sub(Vec2 a, Vec2 b)
	{
		return Vec2{ a.x - b.x, a.y - b.y };
	}

	inline num8_24 dot(Vec2 a, Vec2 b)
	{
		return (a.x * b.x) + (a.y * b.y);
	}

	inline Vec2 scalar(Vec2 v, num8_24 n)
	{
		return Vec2{ v.x * n, v.y * n };
	}

	inline num8_24 length(Vec2 v)
	{
		return fpm::sqrt((v.x * v.x) + (v.y * v.y));
	}

	//angle in radians, please
	//positive rotates counter-clockwise
	Vec2 rotate(Vec2 vec, num8_24 angle)
	{
		num8_24 cos = fpm::cos(angle);
		num8_24 sin = fpm::sin(angle);
		num8_24 x = (vec.x * cos) - (vec.y * sin);
		num8_24 y = (vec.x * sin) + (vec.y * cos);
		return Vec2{ x, y };
	}

	// distance between closest point in a ray and the center of something
	// won't check radius because them multiple occasions can account for things such as grazing
	num8_24 closest(Vec2 origin, Vec2 vector, Vec2 center)
	{
		Vec2 o2c = sub(center, origin);
		num8_24 relative = dot(o2c, vector);
		if (relative < num8_24{ 0 })
		{
			// dunno how to define the highest value this type holds so this goes lmao
			return num8_24{ 127 };
		}
		Vec2 projection = scalar(vector, relative);
		Vec2 closest = sub(o2c, projection);
		return length(closest);
	}
}

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
	int ammoValue;
	int ammoAmount;
	int ammoMax;
	int staminaValue;
	int staminaAmount;
	int staminaMax;
	num8_24 playerRadius;
	num8_24 projRadius;
	num8_24 comboRadius;
	num8_24 arenaRadius;
	num8_24 spawnRadius;
	num8_24 projSpeed;
	num8_24 projCounterMultiply;
	num8_24 playerWalkSpeed;
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
		player.pos = v2::scalar(v2::left(), cfg.spawnRadius);
		player.dir = v2::right();
		break;
	case 2:
		player.pos = v2::scalar(v2::right(), cfg.spawnRadius);
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

// TODO try if I can just assign shit instead of doing this silliness
void copyPlayer(Player* to, Player* from)
{
	to->id = from->id;
	to->pushdown = from->pushdown;
	to->pos = from->pos;
	to->vel = from->vel;
	to->dir = from->dir;
	to->ammo = from->ammo;
	to->chargeCount = from->chargeCount;
	to->stamina = from->stamina;
	to->perfectPos = from->perfectPos;
	to->dashDir = from->dashDir;
	to->dashCount = from->dashCount;
	to->hitstopCount = from->hitstopCount;
	to->stunCount = from->stunCount;
}

void copyGameState(GameState* to, GameState* from)
{
	to->frame = from->frame;
	copyPlayer(&(to->p1), &(from->p1));
	copyPlayer(&(to->p2), &(from->p2));
	to->projs = from->projs;
	to->health1 = from->health1;
	to->health2 = from->health2;
	to->rounds1 = from->rounds1;
	to->rounds2 = from->rounds2;
	to->roundCountdown = from->roundCountdown;
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