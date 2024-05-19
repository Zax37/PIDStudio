#pragma once
#include "AssetLibrary.h"

class PIDStudio;

int getNumSuffix(const std::string& s);

class SupportedGame
{
public:
	SupportedGame(PIDStudio* app, const char* name, const char* iniKey, const char* exeName)
		: app(app), name(name), iniKey(iniKey), exeName(exeName) {};

	virtual void initializeLibrary(const std::shared_ptr<AssetLibrary::TreeNode>& root);

	[[nodiscard]] const char* getName() const { return name; }
	[[nodiscard]] const char* getIniKey() const { return iniKey; }
	[[nodiscard]] const char* getExeName() const { return exeName; }

	static std::string getNames(const std::vector<std::shared_ptr<SupportedGame>>& games);

protected:
	PIDStudio* app;
	const char* name;
	const char* iniKey;
	const char* exeName;
};
