#pragma once
#include "../SupportedGame.h"

class Claw : public SupportedGame
{
public:
	Claw(PIDStudio* app, const char* name, const char* iniKey, const char* exeName)
		: SupportedGame(app, name, iniKey, exeName) {}

	void initializeLibrary(std::shared_ptr<AssetLibrary::TreeNode> root) override;
};
