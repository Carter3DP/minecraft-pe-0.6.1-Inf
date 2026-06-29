#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__

#include "../Screen.h"
#include <string>

class ConsoleScreen: public Screen
{
    typedef Screen super;
public:
    ConsoleScreen();
    virtual ~ConsoleScreen() {}

    void init() override;
    void render(int xm, int ym, float a) override;
    void tick() override;

    virtual bool renderGameBehind() override { return true; }
    virtual bool isInGameScreen() override { return true; }
    virtual bool isPauseScreen() override { return false; }

    virtual void keyPressed(int eventKey) override;
    virtual void charPressed(char inputChar) override;
    virtual bool handleBackEvent(bool isDown) override;

private:
    void execute();
    std::string processCommand(const std::string& cmd);

    std::string _input;
    int         _cursorBlink; // tick counter for cursor blink
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__*/
