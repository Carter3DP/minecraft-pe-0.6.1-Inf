#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatScreen_H__

#include "../Screen.h"
#include <string>

class ChatScreen: public Screen
{
public:
	ChatScreen() {}
	virtual ~ChatScreen() {}

	void init();
	void removed();

	void render(int xm, int ym, float a);

	void buttonClicked(Button* button) {};

private:
#ifdef __APPLE__
	void keyPressed(int eventKey);
	void charPressed(char inputChar);
	std::string input;
#endif
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatScreen_H__*/
