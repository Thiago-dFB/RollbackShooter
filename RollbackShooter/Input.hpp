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
};

InputBindings readTOMLForBind()
{
	IsKeyPressed(KEY_A);
	
	InputBindings inputBind;
	auto file = toml::parse_file("RBST_controls.toml");
	std::istringstream iss;

	iss.str(file["Mouse"]["sensitivity"].value_or("1"));
	iss >> inputBind.sensitivity;
	
	inputBind.forward = file["Direction"]["forward"].value_or(0);
	inputBind.back = file["Direction"]["back"].value_or(0);
	inputBind.left = file["Direction"]["left"].value_or(0);
	inputBind.right = file["Direction"]["right"].value_or(0);

	inputBind.shotKey = file["Attack"]["fireKey"].value_or(0);
	inputBind.shotBtn = file["Attack"]["fireBtn"].value_or(6);
	inputBind.altShotKey = file["Attack"]["altFireKey"].value_or(0);
	inputBind.altShotBtn = file["Attack"]["altFireBtn"].value_or(6);
	inputBind.dashKey = file["Attack"]["dashKey"].value_or(0);
	inputBind.dashBtn = file["Attack"]["dashBtn"].value_or(6);

	return inputBind;
}

const InputBindings inputBind = readTOMLForBind();

//INPUT PROCESSING AND CONVERSION

PlayerInput processInput(const InputBindings* inputBind)
{
	if (IsWindowFocused())
	{
		PlayerInput input;

		input.mouse = inputBind->sensitivity * num_det{ DEG2RAD * GetMouseDelta().x };

		int8 direction = 5;
		if (IsKeyDown(inputBind->forward)) direction = direction + 3;
		if (IsKeyDown(inputBind->back)) direction = direction - 3;
		if (IsKeyDown(inputBind->left)) --direction;
		if (IsKeyDown(inputBind->right)) ++direction;
		input.mov = static_cast<MoveInput>(direction);

		if (IsMouseButtonPressed(inputBind->shotBtn) || IsKeyPressed(inputBind->shotKey)) input.atk = Shot;
		if (IsMouseButtonPressed(inputBind->altShotBtn) || IsKeyPressed(inputBind->altShotKey)) input.atk = AltShot;
		if (IsKeyPressed(inputBind->dashKey) || IsMouseButtonPressed(inputBind->dashBtn)) input.atk = Dash;

		return input;
	}
	else
	{
		return PlayerInput{ None, Neutral, num_det{0} };
	}
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