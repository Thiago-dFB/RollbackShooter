#ifndef RBST_INPUT_HPP
#define RBST_INPUT_HPP

#include <istream>
#include <ostream>

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

struct InputData
{
	PlayerInput p1Input;
	PlayerInput p2Input;
};

struct InputBindings
{
	num_det sensitivity{ 0 };
	int forward = 0;
	int back = 0;
	int left = 0;
	int right = 0;
	int shotKey = 0;
	int shotBtn = 6;
	int altShotKey = 0;
	int altShotBtn = 6;
	int dashKey = 0;
	int dashBtn = 6;
	int replayP1Key = 0;
	int replayP2Key = 0;
	int replaySpecKey = 0;
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

	bind.replayP1Key = file["Replay"]["p1Key"].value_or(0);
	bind.replayP2Key = file["Replay"]["p2Key"].value_or(0);
	bind.replaySpecKey = file["Replay"]["specKey"].value_or(0);

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

PlayerInput predictInput(PlayerInput prevInput)
{
	prevInput.atk = None;
	return prevInput; //simple as that lul
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

void putInput(std::ostream* os, PlayerInput input)
{
	char* buffer = new char[5];
	
	//0mmmmaa
	buffer[0] = (static_cast<char>(input.mov) << 2) | static_cast<char>(input.atk);

	//never let me cook again
	int32_t mask = 0xff;
	int32_t raw = input.mouse.raw_value();
	buffer[4] = raw & mask;
	raw = raw >> 8;
	buffer[3] = raw & mask;
	raw = raw >> 8;
	buffer[2] = raw & mask;
	raw = raw >> 8;
	buffer[1] = raw & mask;

	os->write(buffer, 5);
}

PlayerInput getInput(std::istream* is)
{
	PlayerInput input;
	
	char* buffer = new char[5];
	is->read(buffer, 5);

	//0mmmmaa
	char movAtk = buffer[0];
	char movMask = 0b0111100;
	char atkMask = 0b0000011;
	input.atk = static_cast<AttackInput>(movAtk & atkMask);
	input.mov = static_cast<MoveInput>((movAtk & movMask) >> 2);

	//seriously, never let me cook again
	int32_t raw = 0;
	raw = raw | buffer[1];
	raw = raw << 8;
	raw = raw | buffer[2];
	raw = raw << 8;
	raw = raw | buffer[3];
	raw = raw << 8;
	raw = raw | buffer[4];
	input.mouse.from_raw_value(raw);

	return input;
}

#endif