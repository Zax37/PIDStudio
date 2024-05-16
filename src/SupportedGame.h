#pragma once
#include "AssetLibrary.h"

class PIDStudio;

int getNumSuffix(const std::string& s);

class SupportedGame
{
public:
	SupportedGame(PIDStudio* app, const char* name, const char* iniKey, const char* exeName)
		: app(app), name(name), iniKey(iniKey), exeName(exeName) {};

	virtual void initializeLibrary(std::shared_ptr<AssetLibrary::TreeNode> root);

	const char* getName() const { return name; }
	const char* getIniKey() const { return iniKey; }
	const char* getExeName() const { return exeName; }

	static const std::string& getNames(std::vector<std::shared_ptr<SupportedGame>> games);

protected:
	PIDStudio* app;
	const char* name;
	const char* iniKey;
	const char* exeName;
};
