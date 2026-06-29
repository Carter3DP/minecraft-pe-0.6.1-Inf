#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ArmorScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ArmorScreen_H__

#include "BaseContainerScreen.h"

#include "../components/InventoryPane.h"
#include "../components/Button.h"

class Font;
class CItem;
class Textures;
class NinePatchLayer;
class Tesselator;

class ArmorScreen:	public Screen,
					public Touch::IInventoryPaneCallback
{
	typedef Screen super;
	typedef std::vector<CItem*> ItemList;

	static const int NUM_ARMORBUTTONS = 4;
public:
    ArmorScreen();
	~ArmorScreen();

    void init() override;
	void setupPositions() override;

	void tick() override;
	void render(int xm, int ym, float a) override;
	bool renderGameBehind() override;
	void buttonClicked(Button* button) override;

	// IInventoryPaneCallback
	bool addItem(const Touch::InventoryPane* pane, int itemId) override;
	bool isAllowed( int slot ) override;
	std::vector<const ItemInstance*> getItems( const Touch::InventoryPane* forPane ) override;
private:
	void renderPlayer(float xo, float yo);

	void setupInventoryPane();
	void updateItems();

	void drawSlotItemAt(Tesselator& t, int slot, const ItemInstance* item, int x, int y);
	ItemInstance moveOver(const ItemInstance* item, int maxCount);
	void takeAndClearSlot( int slot );
	void handleRenderPane(Touch::InventoryPane* pane, Tesselator& t, int xm, int ym, float a);
	bool canMoveToSlot(int slot, const ItemInstance* item);
	ItemList _items;

	std::string currentItemDesc;
	ItemInstance burnResult;
	float descWidth;
	ImageButton btnClose;

	BlankButton btnArmor0;
	BlankButton btnArmor1;
	BlankButton btnArmor2;
	BlankButton btnArmor3;
	BlankButton* armorButtons[4];

	Touch::THeader bHeader;

	Touch::InventoryPane* inventoryPane;
	IntRectangle inventoryPaneRect;
	IntRectangle guiPlayerBgRect;
	
	std::vector<const ItemInstance*> armorItems;
	bool doRecreatePane;

	// GUI elements such as 9-Patches
	NinePatchLayer* guiBackground;
	NinePatchLayer* guiSlot;
	NinePatchLayer* guiPaneFrame;
	NinePatchLayer* guiPlayerBg;
	Player* player;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ArmorScreen_H__*/
