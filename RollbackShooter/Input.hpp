#ifndef RBST_INPUT_HPP
#define RBST_INPUT_HPP

#include <istream>

//TOML++
#include <toml++/toml.h>
//Raylib
#include <raylib.h>

#include "Math.hpp"

enum AttackInput
{
	None = 0,
	Shot = 1,
	AltShot = 2,
	Dash = 3
};

enum MoveInput
{
	BackLeft = 1,
	Back = 2,
	BackRight = 3,
	Left = 4,
	Neutral = 5,
	Right = 6,
	ForLeft = 7,
	Forward = 8,
	ForRight = 9
};

struct PlayerInput
{
	AttackInput atk = None;
	MoveInput mov = Neutral;
	//horizontal mouse movement (already in radians)
	num_det mouse{ 0 };
};

struct PlayerInputZipped
{
	int8 movatk;
	int32_t mouseRaw;
};

struct InputBindings
{
	num_det sensitivity{ 0 };
	int forward;
	int back;
	int left;
	int right;
	int shotKey;
	int shotBtn;
	int altShotKey;
	int altShotBtn;
	int dashKey;
	int dashBtn;
};

InputBindings readTOMLForBind()
{
	IsKeyPressed(KEY_A);
	
	InputBindings bind;
	auto file = toml::parse_file("RBST_controls.toml");
	std::istringstream iss;

	iss.str(file["Mouse"]["sensitivity"].value_or("1"));
	iss >> bind.sensitivity;
	
	bind.forward = file["Direction"]["forward"].value_or(0);
	bind.back = file["Direction"]["back"].value_or(0);
	bind.left = file["Direction"]["left"].value_or(0);
	bind.right = file["Direction"]["right"].value_or(0);

	bind.shotKey = file["Attack"]["fireKey"].value_or(0);
	bind.shotBtn = file["Attack"]["fireBtn"].value_or(6);
	bind.altShotKey = file["Attack"]["altFireKey"].value_or(0);
	bind.altShotBtn = file["Attack"]["altFireBtn"].value_or(6);
	bind.dashKey = file["Attack"]["dashKey"].value_or(0);
	bind.dashBtn = file["Attack"]["dashBtn"].value_or(6);

	return bind;
}

PlayerInput processInput(const InputBindings* bind)
{
	PlayerInput input;
	
	//TODO flip this value if mouselook is inverted
	input.mouse = IsWindowFocused() ? bind->sensitivity * num_det{ DEG2RAD * GetMouseDelta().x } : num_det{0};

	int8 direction = 5;
	if (IsKeyDown(bind->forward)) direction = direction + 3;
	if (IsKeyDown(bind->back)) direction = direction - 3;
	if (IsKeyDown(bind->left)) --direction;
	if (IsKeyDown(bind->right)) ++direction;
	input.mov = static_cast<MoveInput>(direction);

	if (IsMouseButtonPressed(bind->shotBtn) || IsKeyPressed(bind->shotKey)) input.atk = Shot;
	if (IsMouseButtonPressed(bind->altShotBtn) || IsKeyPressed(bind->altShotKey)) input.atk = AltShot;
	if (IsKeyPressed(bind->dashKey) || IsMouseButtonPressed(bind->dashBtn)) input.atk = Dash;

	return input;
}

std::string inputToString(PlayerInput input)
{
	std::ostringstream oss;
	oss << "a" << static_cast<int>(input.atk) << "m" << static_cast<int>(input.mov) << "l" << input.mouse.raw_value() << ";";
	return oss.str();
}

PlayerInput stringToInput(std::string str)
{
	PlayerInput input;
	//0123456789...
	//a*m*l****;
	input.atk = static_cast<AttackInput>(std::stoi(str.substr(1, 1)));
	input.mov = static_cast<MoveInput>(std::stoi(str.substr(3, 1)));
	size_t delim = str.find(";");
	input.mouse.from_raw_value(std::stoi(str.substr(5, delim - 5)));
	return input;
}

#endif