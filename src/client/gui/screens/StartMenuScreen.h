#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../components/ImageButton.h"

class StartMenuScreen: public Screen
{
public:
	StartMenuScreen();
	virtual ~StartMenuScreen();

	void init() override;
	void setupPositions() override;

	void tick() override;
	void render(int xm, int ym, float a) override;

	void buttonClicked(Button* button) override;
	virtual void mouseClicked(int x, int y, int buttonNum) override;
	bool handleBackEvent(bool isDown) override;
	bool isInGameScreen() override;
private:

	Button* bHost;
	Button* bJoin;
	Button* bOptions;
	Button* bQuit; 

	std::string copyright;
	int copyrightPosX;

	std::string version;
	int versionPosX;

	std::string username;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__*/
