#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__InBedScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__InBedScreen_H__

#include "../Screen.h"
class Button;

class InBedScreen: public Screen
{
public:
	InBedScreen();

	virtual ~InBedScreen();

	void init() override;

	void setupPositions() override;

	void render(int xm, int ym, float a) override;

	void buttonClicked(Button* button) override;

private:
	Button* bWakeUp;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__InBedScreen_H__*/
