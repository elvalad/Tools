#pragma once

class Config
{
public:
	int  alphaColor; // 0 - 100
	int  alphaLabel; // 0 - 100
	bool overlayDisplay;
	bool savePointCloud;
	// 1 - not registered; 2 - hardware registration; 
	// 3 - software registration(parameters from file)
	// 4 - software registration(parameters from flash)
	int  registrationType; 
	bool useDistortCoef;
};