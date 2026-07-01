#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__JoinGameScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__JoinGameScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../components/ScrolledSelectionList.h"
#include "../../Minecraft.h"
#include "../../../network/RakNetInstance.h"
#include "../../../raknet/GetTime.h"


class JoinGameScreen;

class AvailableGamesList : public ScrolledSelectionList
{
	int selectedItem;
	ServerList copiedServerList;

	friend class JoinGameScreen;

public:

	AvailableGamesList(Minecraft* _minecraft, int _width, int _height)
	:	ScrolledSelectionList(_minecraft, _width, _height, 24, _height - 30, 28)
	,	selectedItem(-1)
	{
	}

protected:

	virtual int getNumberOfItems() override { return (int)copiedServerList.size(); }

	virtual void selectItem(int item, bool doubleClick) override { selectedItem = item; }
	virtual bool isSelectedItem(int item) override { return item == selectedItem; }

	virtual void renderBackground() override {}
	virtual void renderItem(int i, int x, int y, int h, Tesselator& t) override;
};

class JoinGameScreen: public Screen
{
public:
	JoinGameScreen();
	virtual ~JoinGameScreen();

	void init() override;
	void setupPositions() override;

	virtual bool handleBackEvent(bool isDown) override;

	virtual bool isIndexValid(int index);

	virtual void tick() override;

	void render(int xm, int ym, float a) override;

	void buttonClicked(Button* button) override;

	bool isInGameScreen() override;
private:
	Button bJoin;
	Button bBack;
	AvailableGamesList* gamesList;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__JoinGameScreen_H__*/
