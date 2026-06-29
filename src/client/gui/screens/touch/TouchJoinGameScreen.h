#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchJoinGameScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchJoinGameScreen_H__

#include "../../Screen.h"
#include "../../components/Button.h"
#include "../../components/RolledSelectionListV.h"
#include "../../../Minecraft.h"
#include "../../../../platform/input/Multitouch.h"
#include "../../../../network/RakNetInstance.h"

namespace Touch {

class JoinGameScreen;

class AvailableGamesList : public RolledSelectionListV
{
	int startSelected;
	int selectedItem;
	ServerList copiedServerList;

	friend class JoinGameScreen;

public:
	AvailableGamesList(Minecraft* _minecraft, int _width, int _height)
	:	RolledSelectionListV(_minecraft, _width, _height, 0, _width, 24, _height, 34),
		selectedItem(-1),
		startSelected(-1)
	{
	}

protected:

	virtual int getNumberOfItems() override { return (int)copiedServerList.size(); }

	virtual void selectCancel() override;
	virtual void selectStart(int item) override;
	virtual void selectItem(int item, bool doubleClick) override;

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
	TButton bBack;
	TButton bJoinByIp;
	THeader bHeader;
	AvailableGamesList* gamesList;
};

};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchJoinGameScreen_H__*/
