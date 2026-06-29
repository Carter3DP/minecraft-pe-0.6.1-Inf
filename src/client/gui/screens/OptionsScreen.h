#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../components/OptionsGroup.h"

class ImageButton;
class OptionsPane;

class OptionsScreen: public Screen
{
	typedef Screen super;

	void init() override;
	void generateOptionScreens();

public:
	OptionsScreen();
	~OptionsScreen();

	void setupPositions() override;
	void buttonClicked(Button* button) override;
	void render(int xm, int ym, float a) override;
	void removed() override;
	void selectCategory(int index);

	virtual void mouseClicked(int x, int y, int buttonNum) override;
	virtual void mouseReleased(int x, int y, int buttonNum) override;
	virtual void mouseEvent() override;
	virtual void keyPressed(int eventKey) override;
	virtual void charPressed(char inputChar) override;
	virtual void mouseWheel(int dx, int dy, int xm, int ym) override;
	
	virtual void tick() override;

private:
	void updateCategoryScrollBounds();
	void scrollCategoriesBy(int amount);
	bool handleCategoryDrag(int x, int y);
	void offsetCategoryButtons(int dy);
	bool isInsideCategoryViewport(int x, int y) const;

	Touch::THeader* bHeader;
	ImageButton* btnClose;

	Touch::TButton* btnCredits;

	std::vector<Touch::TButton*> categoryButtons;
	std::vector<OptionsGroup*> optionPanes;

	OptionsGroup* currentOptionsGroup;

	int selectedCategory;
	int categoryScrollOffset;
	int maxCategoryScrollOffset;
	bool categoryDragging;
	bool categoryDragged;
	int categoryDragStartY;
	int categoryDragStartScrollOffset;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__*/
