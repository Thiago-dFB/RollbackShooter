#ifndef RBST_CONFIG_HPP
#define RBST_CONFIG_HPP

#include "Math.hpp"

struct Config
{
	std::int16_t ammoMax;
	std::int16_t shotCost;
	std::int16_t altShotCost;
	std::int16_t staminaMax;
	std::int16_t dashCost;
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
	std::int16_t dashDuration;
	std::int16_t dashPerfect;
	std::int16_t chargeDuration;
	std::int16_t hitstopDuration;
	std::int16_t playerHealth;
	std::int16_t roundsToWin;
};

#endif