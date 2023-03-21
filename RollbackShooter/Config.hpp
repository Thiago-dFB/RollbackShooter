#ifndef RBST_CONFIG_HPP
#define RBST_CONFIG_HPP

#include "Math.hpp"

struct Config
{
	int16 ammoMax;
	int16 shotCost;
	int16 altShotCost;
	int16 staminaMax;
	int16 dashCost;
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
	int16 dashDuration;
	int16 dashPhase;
	int16 dashPerfect;
	int16 chargeDuration;
	int16 hitstopDuration;
	int16 playerHealth;
	int16 roundsToWin;
};

#endif