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

	void init();
	void generateOptionScreens();

public:
	OptionsScreen();
	~OptionsScreen();

	void setupPositions();
	void buttonClicked(Button* button);
	void render(int xm, int ym, float a);
	void removed();
	void selectCategory(int index);

	virtual void mouseClicked(int x, int y, int buttonNum);
	virtual void mouseReleased(int x, int y, int buttonNum);
	virtual void mouseEvent();
	virtual void keyPressed(int eventKey);
	virtual void charPressed(char inputChar);
	virtual void mouseWheel(int dx, int dy, int xm, int ym);
	
	virtual void tick();

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
