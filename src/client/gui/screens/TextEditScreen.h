#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__TextEditScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__TextEditScreen_H__

//package net.minecraft.client.gui;

#include "../Screen.h"
#include <string>
#include "../components/ImageButton.h"
class SignTileEntity;
class Button;
class TextEditScreen: public Screen
{
    typedef Screen super;
public:
	TextEditScreen(SignTileEntity* signEntity);
	~TextEditScreen();
    void init() override;
	void tick() override;
	bool handleBackEvent(bool isDown) override;
	void render(int xm, int ym, float a) override;
	virtual void lostFocus() override;
	virtual void keyPressed(int eventKey) override;
	virtual void charPressed(char inputChar) override;
	void setupPositions() override;
	void buttonClicked(Button* button) override;
protected:
	bool isShowingKeyboard;
	SignTileEntity* sign;
	int frame;
	int line;
private:
	ImageButton btnClose;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__TextEditScreen_H__*/
