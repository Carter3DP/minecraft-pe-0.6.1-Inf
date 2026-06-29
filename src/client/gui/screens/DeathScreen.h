#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__DeathScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__DeathScreen_H__

#include "../Screen.h"
class Button;

class DeathScreen: public Screen
{
public:
	DeathScreen();

	virtual ~DeathScreen();

	void init() override;

	void setupPositions() override;

	void tick() override;
	void render(int xm, int ym, float a) override;

	void buttonClicked(Button* button) override;

private:
	Button* bRespawn;
	Button* bTitle;
	bool _hasChosen;
	int _tick;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__DeathScreen_H__*/
