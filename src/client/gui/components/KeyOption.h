#pragma once
#include "Button.h"
#include <client/Options.h>

class KeyOption : public Touch::TButton {
public:
    KeyOption(Minecraft* minecraft, OptionId optId);

    virtual void mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) override;
    virtual void released(int mx, int my) override {}
    virtual void keyPressed(Minecraft* minecraft, int key) override;
protected:
    bool m_captureMode;
};
