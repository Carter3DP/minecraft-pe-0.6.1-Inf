#include "OptionsScreen.h"

#include "StartMenuScreen.h"
#include "UsernameScreen.h"
#include "DialogDefinitions.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "CreditsScreen.h"

#include "../components/ImageButton.h"
#include "../components/OptionsGroup.h"
#include "../Gui.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"

namespace {
const int CATEGORY_SCROLL_STEP = 24;
const int CATEGORY_SCROLL_DRAG_SLOP = 4;
}

OptionsScreen::OptionsScreen()
	: btnClose(NULL),
	bHeader(NULL),
	btnCredits(NULL),
	selectedCategory(0),
	categoryScrollOffset(0),
	maxCategoryScrollOffset(0),
	categoryDragging(false),
	categoryDragged(false),
	categoryDragStartY(0),
	categoryDragStartScrollOffset(0) {
}

OptionsScreen::~OptionsScreen() {
	if (btnClose != NULL) {
		delete btnClose;
		btnClose = NULL;
	}

	if (bHeader != NULL) {
		delete bHeader;
		bHeader = NULL;
	}

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		if (*it != NULL) {
			delete* it;
			*it = NULL;
		}
	}

	for (std::vector<OptionsGroup*>::iterator it = optionPanes.begin(); it != optionPanes.end(); ++it) {
		if (*it != NULL) {
			delete* it;
			*it = NULL;
		}
	}

	categoryButtons.clear();
}

void OptionsScreen::init() {
	bHeader = new Touch::THeader(0, "Options");

	btnClose = new ImageButton(1, "");

	ImageDef def;
	def.name = "gui/touchgui.png";
	def.width = 34;
	def.height = 26;

	def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
	btnClose->setImageDef(def, true);

	categoryButtons.push_back(new Touch::TButton(2, "General"));
	categoryButtons.push_back(new Touch::TButton(3, "Game"));
	categoryButtons.push_back(new Touch::TButton(4, "Controls"));
	categoryButtons.push_back(new Touch::TButton(5, "Graphics"));
	categoryButtons.push_back(new Touch::TButton(6, "Tweaks"));

	btnCredits = new Touch::TButton(11, "Credits");
	categoryButtons.push_back(btnCredits);

	buttons.push_back(bHeader);
	buttons.push_back(btnClose);

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		buttons.push_back(*it);
		tabButtons.push_back(*it);
	}

	generateOptionScreens();
	// start with first category selected
	selectCategory(0);
}

void OptionsScreen::setupPositions() {
	int buttonHeight = btnClose->height;

	btnClose->x = (minecraft->SafeZone.right * minecraft->gui.InvGuiScale) - btnClose->width;
	btnClose->y = 0;

	int offsetNum = 1;

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {

		(*it)->x = minecraft->SafeZone.left * minecraft->gui.InvGuiScale;
		(*it)->y = offsetNum * buttonHeight;
		(*it)->selected = false;

		offsetNum++;
	}

	bHeader->x = 0;
	bHeader->y = 0;
	bHeader->width = width - btnClose->width;
	bHeader->height = btnClose->height;

	updateCategoryScrollBounds();

	for (std::vector<OptionsGroup*>::iterator it = optionPanes.begin(); it != optionPanes.end(); ++it) {

		if (categoryButtons.size() > 0 && categoryButtons[0] != NULL) {

			(*it)->x = (minecraft->SafeZone.left * minecraft->gui.InvGuiScale) + categoryButtons[0]->width;
			(*it)->y = bHeader->height;
			(*it)->width = width - (*it)->x;
			(*it)->setViewportHeight(height - bHeader->height);

			(*it)->setupPositions();
		}
	}

	// don't override user selection on resize
}


void OptionsScreen::render(int xm, int ym, float a) {
	renderBackground();

	int xmm = xm * width / minecraft->width;
	int ymm = ym * height / minecraft->height - 1;

	if (currentOptionsGroup != NULL)
		currentOptionsGroup->render(minecraft, xmm, ymm);

	if (bHeader != NULL)
		((Button*)bHeader)->render(minecraft, xm, ym);
	if (btnClose != NULL)
		((Button*)btnClose)->render(minecraft, xm, ym);

	updateCategoryScrollBounds();
	int categoryX = minecraft->SafeZone.left * minecraft->gui.InvGuiScale;
	int categoryY = bHeader->height;
	int categoryWidth = categoryButtons.empty() ? 0 : categoryButtons[0]->width;
	int categoryHeight = height - categoryY;
	if (categoryHeight < 0)
		categoryHeight = 0;

	glEnable2(GL_SCISSOR_TEST);
	glScissor(
		Gui::GuiScale * categoryX,
		minecraft->height - Gui::GuiScale * (categoryY + categoryHeight),
		Gui::GuiScale * categoryWidth,
		Gui::GuiScale * categoryHeight
	);

	offsetCategoryButtons(-categoryScrollOffset);
	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it)
		(*it)->render(minecraft, xm, ym);
	offsetCategoryButtons(categoryScrollOffset);

	glDisable2(GL_SCISSOR_TEST);
}

void OptionsScreen::removed() {
}

void OptionsScreen::buttonClicked(Button* button) {
	if (button == btnClose) {
		minecraft->options.save();
		if (minecraft->screen != NULL) {
			minecraft->setScreen(NULL);
		} else {
			minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
		}
	}
	else if (button->id > 1 && button->id < 7) {
		int categoryButton = button->id - categoryButtons[0]->id;
		selectCategory(categoryButton);
	}
	else if (button == btnCredits) {
		minecraft->setScreen(new CreditsScreen());
	}
}

void OptionsScreen::selectCategory(int index) {
	int currentIndex = 0;

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {

		if (index == currentIndex)
			(*it)->selected = true;
		else
			(*it)->selected = false;

		currentIndex++;
	}

	if (index < (int)optionPanes.size())
		currentOptionsGroup = optionPanes[index];
}

void OptionsScreen::updateCategoryScrollBounds() {
	int contentBottom = bHeader ? bHeader->height : 0;
	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		if ((*it)->y + (*it)->height > contentBottom)
			contentBottom = (*it)->y + (*it)->height;
	}

	int viewportBottom = height;
	maxCategoryScrollOffset = contentBottom - viewportBottom;
	if (maxCategoryScrollOffset < 0)
		maxCategoryScrollOffset = 0;

	if (categoryScrollOffset < 0)
		categoryScrollOffset = 0;
	if (categoryScrollOffset > maxCategoryScrollOffset)
		categoryScrollOffset = maxCategoryScrollOffset;
}

void OptionsScreen::scrollCategoriesBy(int amount) {
	categoryScrollOffset += amount;
	updateCategoryScrollBounds();
}

void OptionsScreen::offsetCategoryButtons(int dy) {
	if (dy == 0)
		return;

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it)
		(*it)->y += dy;
}

bool OptionsScreen::isInsideCategoryViewport(int x, int y) const {
	if (categoryButtons.empty() || bHeader == NULL)
		return false;

	int categoryX = minecraft->SafeZone.left * minecraft->gui.InvGuiScale;
	int categoryY = bHeader->height;
	int categoryWidth = categoryButtons[0]->width;
	return x >= categoryX && y >= categoryY && x < categoryX + categoryWidth && y < height;
}

bool OptionsScreen::handleCategoryDrag(int x, int y) {
	if (!categoryDragging || !Mouse::isButtonDown(MouseAction::ACTION_LEFT))
		return false;

	int dy = y - categoryDragStartY;
	if (dy < -CATEGORY_SCROLL_DRAG_SLOP || dy > CATEGORY_SCROLL_DRAG_SLOP)
		categoryDragged = true;

	if (!categoryDragged)
		return false;

	categoryScrollOffset = categoryDragStartScrollOffset - dy;
	updateCategoryScrollBounds();
	return true;
}

void OptionsScreen::generateOptionScreens() {
	// how the fuck it works

	optionPanes.push_back(new OptionsGroup("options.group.general"));
	optionPanes.push_back(new OptionsGroup("options.group.game"));
	optionPanes.push_back(new OptionsGroup("options.group.controls"));
	optionPanes.push_back(new OptionsGroup("options.group.graphics"));
	optionPanes.push_back(new OptionsGroup("options.group.tweaks"));

	// General Pane
	optionPanes[0]->addOptionItem(OPTIONS_USERNAME, minecraft)
		.addOptionItem(OPTIONS_SENSITIVITY, minecraft);

	// Game Pane
	optionPanes[1]->addOptionItem(OPTIONS_DIFFICULTY, minecraft)
		.addOptionItem(OPTIONS_SERVER_VISIBLE, minecraft)
		.addOptionItem(OPTIONS_THIRD_PERSON_VIEW, minecraft)
		.addOptionItem(OPTIONS_WINDOW_SCALE, minecraft)
		.addOptionItem(OPTIONS_GUI_SCALE, minecraft)
		.addOptionItem(OPTIONS_SENSITIVITY, minecraft)
		.addOptionItem(OPTIONS_MUSIC_VOLUME, minecraft)
		.addOptionItem(OPTIONS_SOUND_VOLUME, minecraft)
		.addOptionItem(OPTIONS_SMOOTH_CAMERA, minecraft)
		.addOptionItem(OPTIONS_DESTROY_VIBRATION, minecraft)
		.addOptionItem(OPTIONS_IS_LEFT_HANDED, minecraft);

	// // Controls Pane
	optionPanes[2]->addOptionItem(OPTIONS_INVERT_Y_MOUSE, minecraft)
		.addOptionItem(OPTIONS_USE_TOUCHSCREEN, minecraft)
		.addOptionItem(OPTIONS_AUTOJUMP, minecraft)	
		.addOptionItem(OPTIONS_BLOCK_OUTLINE, minecraft)
		.addOptionItem(OPTIONS_IS_JOY_TOUCH_AREA, minecraft);

	for (int i = OPTIONS_KEY_FORWARD; i <= OPTIONS_KEY_USE; i++) {
		optionPanes[2]->addOptionItem((OptionId)i, minecraft);
	}

	// // Graphics Pane
	optionPanes[3]->addOptionItem(OPTIONS_FANCY_GRAPHICS, minecraft)
		// .addOptionItem(&Option::VIEW_BOBBING, minecraft)
		// .addOptionItem(&Option::AMBIENT_OCCLUSION, minecraft)
		// .addOptionItem(&Option::ANAGLYPH, minecraft)
		.addOptionItem(OPTIONS_LIMIT_FRAMERATE, minecraft)
		.addOptionItem(OPTIONS_VSYNC, minecraft)
		.addOptionItem(OPTIONS_VIEW_DISTANCE, minecraft)
		.addOptionItem(OPTIONS_RENDER_DEBUG, minecraft)
		.addOptionItem(OPTIONS_SHOW_POS, minecraft)
		.addOptionItem(OPTIONS_ANAGLYPH_3D, minecraft)
		.addOptionItem(OPTIONS_VIEW_BOBBING, minecraft)
		.addOptionItem(OPTIONS_AMBIENT_OCCLUSION, minecraft)
		.addOptionItem(OPTIONS_NORMAL_LIGHTING, minecraft)
		.addOptionItem(OPTIONS_BEAUTIFUL_SKY, minecraft)
		.addOptionItem(OPTIONS_VIGNETTE, minecraft);

	optionPanes[4]->addOptionItem(OPTIONS_ALLOW_SPRINT, minecraft)
		.addOptionItem(OPTIONS_BAR_ON_TOP, minecraft)
		.addOptionItem(OPTIONS_MENU_STYLE, minecraft)
		.addOptionItem(OPTIONS_RPI_CURSOR, minecraft)
		.addOptionItem(OPTIONS_FOLIAGE_TINT, minecraft)
		.addOptionItem(OPTIONS_TINTED_SIDE, minecraft)
		.addOptionItem(OPTIONS_JAVA_HUD, minecraft)
		.addOptionItem(OPTIONS_FOG_TYPE, minecraft)
		.addOptionItem(OPTIONS_BETA_SKY, minecraft)
		.addOptionItem(OPTIONS_RESTORED_ANIMS, minecraft)
		.addOptionItem(OPTIONS_DEBUG_STYLE, minecraft)
		.addOptionItem(OPTIONS_LOG_LEVEL, minecraft);
		
}

void OptionsScreen::mouseClicked(int x, int y, int buttonNum) {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->mouseClicked(minecraft, x, y, buttonNum);

	bool allowCategories = isInsideCategoryViewport(x, y);
	if (buttonNum == MouseAction::ACTION_LEFT && allowCategories) {
		categoryDragging = true;
		categoryDragged = false;
		categoryDragStartY = y;
		categoryDragStartScrollOffset = categoryScrollOffset;
	}

	std::vector<bool> activeStates;
	activeStates.reserve(categoryButtons.size());
	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		activeStates.push_back((*it)->active);
		if (!allowCategories)
			(*it)->active = false;
	}
	offsetCategoryButtons(-categoryScrollOffset);
	super::mouseClicked(x, y, buttonNum);
	offsetCategoryButtons(categoryScrollOffset);
	for (unsigned int i = 0; i < categoryButtons.size(); ++i) {
		categoryButtons[i]->active = activeStates[i];
		if (!allowCategories)
			categoryButtons[i]->released(x, y);
	}
}

void OptionsScreen::mouseReleased(int x, int y, int buttonNum) {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->mouseReleased(minecraft, x, y, buttonNum);

	bool allowCategories = isInsideCategoryViewport(x, y) && !categoryDragged;
	categoryDragging = false;

	std::vector<bool> activeStates;
	activeStates.reserve(categoryButtons.size());
	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		activeStates.push_back((*it)->active);
		if (!allowCategories)
			(*it)->active = false;
	}
	offsetCategoryButtons(-categoryScrollOffset);
	super::mouseReleased(x, y, buttonNum);
	offsetCategoryButtons(categoryScrollOffset);
	for (unsigned int i = 0; i < categoryButtons.size(); ++i)
		categoryButtons[i]->active = activeStates[i];
}

void OptionsScreen::mouseEvent() {
	const MouseAction& e = Mouse::getEvent();
	if (e.action == MouseAction::ACTION_MOVE) {
		int xm = e.x * width / minecraft->width;
		int ym = e.y * height / minecraft->height - 1;
		if (handleCategoryDrag(xm, ym))
			return;
	}

	if (currentOptionsGroup != NULL && e.action == MouseAction::ACTION_MOVE) {
		int xm = e.x * width / minecraft->width;
		int ym = e.y * height / minecraft->height - 1;
		if (currentOptionsGroup->mouseDragged(minecraft, xm, ym))
			return;
	}

	super::mouseEvent();
}

void OptionsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
	(void)dx;

	if (dy == 0)
		return;

	if (isInsideCategoryViewport(xm, ym))
		scrollCategoriesBy(dy > 0 ? -CATEGORY_SCROLL_STEP : CATEGORY_SCROLL_STEP);
	else if (currentOptionsGroup != NULL)
		currentOptionsGroup->scrollBy(dy > 0 ? -24 : 24);
}

void OptionsScreen::keyPressed(int eventKey) {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->keyPressed(minecraft, eventKey);
	if (eventKey == Keyboard::KEY_ESCAPE) 
		minecraft->options.save();

	super::keyPressed(eventKey);
}

void OptionsScreen::charPressed(char inputChar) {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->charPressed(minecraft, inputChar);

	super::keyPressed(inputChar);
}

void OptionsScreen::tick() {

	if (currentOptionsGroup != NULL)
		currentOptionsGroup->tick(minecraft);

	super::tick();
}
