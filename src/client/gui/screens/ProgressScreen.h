#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ProgressScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ProgressScreen_H__

#include "../Screen.h"

class ProgressScreen: public Screen
{
public:
	ProgressScreen();

	void render(int xm, int ym, float a) override;
	bool isInGameScreen() override;

	virtual void keyPressed(int eventKey) override {}

	void tick() override;
private:
	int ticks;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ProgressScreen_H__*/
