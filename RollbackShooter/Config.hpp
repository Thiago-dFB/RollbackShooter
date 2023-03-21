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
	num_det playerRadius;
	num_det projRadius;
	num_det comboRadius;
	num_det arenaRadius;
	num_det spawnRadius;
	num_det projSpeed;
	num_det projCounterMultiply;
	num_det playerWalkSpeed;
	num_det playerWalkAccel;
	num_det playerWalkFric;
	num_det playerDashSpeed;
	std::int16_t dashDuration;
	std::int16_t dashPerfect;
	std::int16_t chargeDuration;
	std::int16_t hitstopDuration;
	std::int16_t playerHealth;
	std::int16_t roundsToWin;
};

#endif