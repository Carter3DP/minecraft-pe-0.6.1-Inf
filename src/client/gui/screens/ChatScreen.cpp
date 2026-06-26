#include "ChatScreen.h"
#include "DialogDefinitions.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/log.h"
#include "../../../platform/input/Keyboard.h"

void ChatScreen::init() {
#ifdef __APPLE__
	input = "";
	minecraft->platform()->showKeyboard();
#else
	minecraft->platform()->createUserInput(DialogDefinitions::DIALOG_NEW_CHAT_MESSAGE);
#endif
}

void ChatScreen::removed() {
	minecraft->platform()->hideKeyboard();
}

void ChatScreen::render(int xm, int ym, float a)
{
	int status = minecraft->platform()->getUserInputStatus();
#ifdef __APPLE__
	if (status == 0)
		minecraft->setScreen(NULL);
#else
	if (status > -1) {
		if (status == 1) {
			std::vector<std::string> v = minecraft->platform()->getUserInput();
			if (v.size() && v[0].length() > 0)
				minecraft->gui.addMessage(v[0]);
		}
		minecraft->setScreen(NULL);
	}
#endif
}

#ifdef __APPLE__
void ChatScreen::keyPressed(int eventKey)
{
	if (eventKey == Keyboard::KEY_RETURN) {
		if (!input.empty())
			minecraft->gui.addMessage(input);
		minecraft->setScreen(NULL);
		return;
	}

	if (eventKey == Keyboard::KEY_ESCAPE) {
		minecraft->setScreen(NULL);
		return;
	}

	if (eventKey == Keyboard::KEY_BACKSPACE && !input.empty())
		input.erase(input.size() - 1, 1);
}

void ChatScreen::charPressed(char inputChar)
{
	if (inputChar >= 32 && inputChar < 127 && input.size() < 256)
		input.push_back(inputChar);
}
#endif
