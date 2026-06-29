#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchStartMenuScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchStartMenuScreen_H__

#include "../../Screen.h"
#include "../../components/LargeImageButton.h"
#include "../../components/ImageButton.h"
#include "../../components/TextBox.h"

namespace Touch {

class StartMenuScreen: public Screen
{
public:
	StartMenuScreen();
	virtual ~StartMenuScreen();
	
	void init() override;
	void setupPositions() override;

	void render(int xm, int ym, float a) override;

	void buttonClicked(Button* button) override;
	virtual void mouseClicked(int x, int y, int buttonNum) override;
	bool handleBackEvent(bool isDown) override;
	bool isInGameScreen() override;
private:
	
	LargeImageButton bHost;
	LargeImageButton bJoin;
	LargeImageButton bOptions;
	ImageButton bQuit; // X close icon

	std::string copyright;
	int copyrightPosX;

	std::string version;
	int versionPosX;

	std::string username;
};
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchStartMenuScreen_H__*/
