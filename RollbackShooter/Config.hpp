#ifndef RBST_CONFIG_HPP
#define RBST_CONFIG_HPP

#include <istream>

//TOML++
#include <toml++/toml.h>
//fpm
#include <fpm/ios.hpp>

#include "Math.hpp"

struct Config
{
	//GameRules
	int16 playerHealth = 0;
	int16 roundsToWin = 0;
	//PlayerResources
	int16 ammoMax = 0;
	int16 shotCost = 0;
	int16 altShotCost = 0;
	int16 staminaMax = 0;
	int16 dashCost = 0;
	//PlayerAction
	int16 dashDuration = 0;
	int16 dashPhase = 0;
	int16 dashPerfect = 0;
	int16 chargeDuration = 0;
	//PlayerSpeed
	num_det playerWalkSpeed{ 0 };
	num_det playerWalkAccel{ 0 };
	num_det playerWalkFric{ 0 };
	num_det playerDashSpeed{ 0 };
	//ProjSpeed
	num_det projSpeed{ 0 };
	num_det projCounterMultiply{ 0 };
	//Radius
	num_det playerRadius{ 0 };
	num_det grazeRadius{ 0 };
	num_det projRadius{ 0 };
	num_det comboRadius{ 0 };
	num_det arenaRadius{ 0 };
	num_det spawnRadius{ 0 };
	//HitStrength
	num_det weakForce{ 0 };
	int16 weakHitstop = 0;
	num_det midForce{ 0 };
	int16 midHitstop = 0;
	num_det strongForce{ 0 };
	int16 strongHitstop = 0;
};

Config readTOMLForCfg()
{
	Config cfg;
	auto file = toml::parse_file("RBST.toml");
	std::istringstream iss;
	num_det extract;
	num_det sixty{ 60 };
	num_det thirtySixHundred{ 3600 };

	cfg.playerHealth = file["GameConfig"]["GameRules"]["playerHealth"].value_or(5);
	cfg.roundsToWin = file["GameConfig"]["GameRules"]["roundsToWin"].value_or(2);

	cfg.ammoMax = file["GameConfig"]["PlayerResources"]["ammoMax"].value_or(120);
	cfg.shotCost = file["GameConfig"]["PlayerResources"]["shotCost"].value_or(60);
	cfg.altShotCost = file["GameConfig"]["PlayerResources"]["altShotCost"].value_or(120);
	cfg.staminaMax = file["GameConfig"]["PlayerResources"]["staminaMax"].value_or(150);
	cfg.dashCost = file["GameConfig"]["PlayerResources"]["dashCost"].value_or(50);

	cfg.dashPhase = file["GameConfig"]["PlayerActions"]["dashPhase"].value_or(10);
	cfg.dashPerfect = file["GameConfig"]["PlayerActions"]["dashPerfect"].value_or(15);
	cfg.dashDuration = file["GameConfig"]["PlayerActions"]["dashDuration"].value_or(60);
	cfg.chargeDuration = file["GameConfig"]["PlayerActions"]["chargeDuration"].value_or(60);

	iss.str(file["GameConfig"]["PlayerSpeed"]["playerWalkSpeed"].value_or("0.8"));
	iss >> extract;
	cfg.playerWalkSpeed = extract / sixty;
	iss.str(file["GameConfig"]["PlayerSpeed"]["playerWalkAccel"].value_or("0.4"));
	iss >> extract;
	cfg.playerWalkAccel = extract / thirtySixHundred;
	iss.str(file["GameConfig"]["PlayerSpeed"]["playerWalkFric"].value_or("0.4"));
	iss >> extract;
	cfg.playerWalkFric = extract / thirtySixHundred;
	iss.str(file["GameConfig"]["PlayerSpeed"]["playerDashSpeed"].value_or("1.2"));
	iss >> extract;
	cfg.playerDashSpeed = extract / sixty;

	iss.str(file["GameConfig"]["ProjSpeed"]["projSpeed"].value_or("0.6"));
	iss >> extract;
	cfg.playerWalkSpeed = extract / sixty;
	iss.str(file["GameConfig"]["ProjSpeed"]["projCounterMultiply"].value_or("1.2"));
	iss >> extract;
	cfg.playerWalkSpeed = extract;

	iss.str(file["GameConfig"]["Radius"]["playerRadius"].value_or("0.5"));
	iss >> extract;
	cfg.playerRadius = extract;
	iss.str(file["GameConfig"]["Radius"]["grazeRadius"].value_or("0.7"));
	iss >> extract;
	cfg.grazeRadius = extract;
	iss.str(file["GameConfig"]["Radius"]["projRadius"].value_or("0.3"));
	iss >> extract;
	cfg.projRadius = extract;
	iss.str(file["GameConfig"]["Radius"]["comboRadius"].value_or("3.0"));
	iss >> extract;
	cfg.comboRadius = extract;
	iss.str(file["GameConfig"]["Radius"]["arenaRadius"].value_or("12.0"));
	iss >> extract;
	cfg.arenaRadius = extract;
	iss.str(file["GameConfig"]["Radius"]["spawnRadius"].value_or("10.0"));
	iss >> extract;
	cfg.spawnRadius = extract;

	iss.str(file["GameConfig"]["HitStrength"]["weakForce"].value_or("1.0"));
	iss >> extract;
	cfg.weakForce = extract / sixty;
	cfg.weakHitstop = file["GameConfig"]["HitStrength"]["weakHitstop"].value_or(5);

	iss.str(file["GameConfig"]["HitStrength"]["midForce"].value_or("2.5"));
	iss >> extract;
	cfg.midForce = extract / sixty;
	cfg.midHitstop = file["GameConfig"]["HitStrength"]["midHitstop"].value_or(10);

	iss.str(file["GameConfig"]["HitStrength"]["strongForce"].value_or("4.0"));
	iss >> extract;
	cfg.strongForce = extract / sixty;
	cfg.strongHitstop = file["GameConfig"]["HitStrength"]["strongHitstop"].value_or(20);

	return cfg;
}

#endif