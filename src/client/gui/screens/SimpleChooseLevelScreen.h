#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__

#include "ChooseLevelScreen.h"
#include "../components/TextBox.h"
#include "../components/Button.h"    // for Touch::THeader
class Button;
class ImageButton;

class SimpleChooseLevelScreen: public ChooseLevelScreen
{
public:
	SimpleChooseLevelScreen(const std::string& levelName);

	virtual ~SimpleChooseLevelScreen();

	void init() override;
	void setupPositions() override;
	void tick() override;

	void render(int xm, int ym, float a) override;

	void buttonClicked(Button* button) override;
	bool handleBackEvent(bool isDown) override;
	virtual void keyPressed(int eventKey) override;
	virtual void mouseClicked(int x, int y, int buttonNum) override;

private:
	Touch::THeader* bHeader;
	Button* bGamemode;
	Button* bCheats;
	ImageButton* bBack;
	Button* bCreate;
	bool hasChosen;

	std::string levelName;
	int gamemode;
	bool cheatsEnabled;

	TextBox tLevelName;
	TextBox tSeed;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__*/
