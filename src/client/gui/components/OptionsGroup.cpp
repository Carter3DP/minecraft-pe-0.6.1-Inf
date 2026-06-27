#include "OptionsGroup.h"
#include "../../Minecraft.h"
#include "ImageButton.h"
#include "OptionsItem.h"
#include "Slider.h"
#include "../../../locale/I18n.h"
#include "TextOption.h"
#include "KeyOption.h"
#include "../Gui.h"
#include "../../../platform/input/Mouse.h"

namespace {
const int OPTIONS_SCROLL_PADDING = 6;
const int OPTIONS_SCROLL_DRAG_SLOP = 4;
}

OptionsGroup::OptionsGroup( std::string labelID )
: viewportHeight(0),
  contentBottom(0),
  scrollOffset(0),
  maxScrollOffset(0),
  dragging(false),
  dragged(false),
  dragStartY(0),
  dragStartScrollOffset(0) {
	label = I18n::get(labelID);
}

void OptionsGroup::setupPositions() {
	// First we write the header and then we add the items
	int curY = y + 18;
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->width = width - 5;
		
		(*it)->y = curY;
		(*it)->x = x + 10;
		(*it)->setupPositions();
		curY += (*it)->height + 3;
	}
	contentBottom = curY;
	height = viewportHeight > 0 ? viewportHeight : curY - y;
	updateScrollBounds();
}

void OptionsGroup::render( Minecraft* minecraft, int xm, int ym ) {
	float padX = 10.0f;
	float padY = 5.0f;
	
	minecraft->font->draw(label, (float)x + padX, (float)y + padY, 0xffffffff, false);

	updateScrollBounds();
	int clipY = y + 18;
	int clipHeight = height - 18;
	if (clipHeight < 0)
		clipHeight = 0;

	glEnable2(GL_SCISSOR_TEST);
	glScissor(
		Gui::GuiScale * x,
		minecraft->height - Gui::GuiScale * (clipY + clipHeight),
		Gui::GuiScale * width,
		Gui::GuiScale * clipHeight
	);

	offsetChildren(-scrollOffset);
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->render(minecraft, xm, ym);
		glEnable2(GL_SCISSOR_TEST);
		glScissor(
			Gui::GuiScale * x,
			minecraft->height - Gui::GuiScale * (clipY + clipHeight),
			Gui::GuiScale * width,
			Gui::GuiScale * clipHeight
		);
	}
	offsetChildren(scrollOffset);

	glDisable2(GL_SCISSOR_TEST);
}

void OptionsGroup::tick( Minecraft* minecraft ) {
	super::tick(minecraft);
}

void OptionsGroup::mouseClicked( Minecraft* minecraft, int x, int y, int buttonNum ) {
	if (buttonNum == MouseAction::ACTION_LEFT && isInsideViewport(x, y)) {
		dragging = true;
		dragged = false;
		dragStartY = y;
		dragStartScrollOffset = scrollOffset;
	}

	super::mouseClicked(minecraft, x, y + scrollOffset, buttonNum);
}

void OptionsGroup::mouseReleased( Minecraft* minecraft, int x, int y, int buttonNum ) {
	dragging = false;
	super::mouseReleased(minecraft, x, y + scrollOffset, buttonNum);
}

bool OptionsGroup::mouseDragged( Minecraft* minecraft, int x, int y ) {
	(void)minecraft;
	(void)x;

	if (!dragging || !Mouse::isButtonDown(MouseAction::ACTION_LEFT))
		return false;

	int dy = y - dragStartY;
	if (dy < -OPTIONS_SCROLL_DRAG_SLOP || dy > OPTIONS_SCROLL_DRAG_SLOP)
		dragged = true;

	if (!dragged)
		return false;

	scrollOffset = dragStartScrollOffset - dy;
	updateScrollBounds();
	return true;
}

void OptionsGroup::setViewportHeight( int h ) {
	viewportHeight = h;
	height = h;
	updateScrollBounds();
}

void OptionsGroup::scrollBy( int amount ) {
	scrollOffset += amount;
	updateScrollBounds();
}

void OptionsGroup::updateScrollBounds() {
	int visibleBottom = y + height;
	maxScrollOffset = contentBottom - visibleBottom + OPTIONS_SCROLL_PADDING;
	if (maxScrollOffset < 0)
		maxScrollOffset = 0;

	if (scrollOffset < 0)
		scrollOffset = 0;
	if (scrollOffset > maxScrollOffset)
		scrollOffset = maxScrollOffset;
}

void OptionsGroup::offsetChildren( int dy ) {
	if (dy == 0)
		return;

	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		OptionsItem* item = dynamic_cast<OptionsItem*>(*it);
		if (item != NULL)
			item->translate(dy);
		else
			(*it)->y += dy;
	}
}

bool OptionsGroup::isInsideViewport( int px, int py ) const {
	return px >= x && py >= y + 18 && px < x + width && py < y + height;
}

OptionsGroup& OptionsGroup::addOptionItem(OptionId optId, Minecraft* minecraft ) {
	auto option = minecraft->options.getOpt(optId);

	if (option == nullptr) return *this;

	// TODO: do a options key class to check it faster via dynamic_cast
	if (option->getStringId().find("options.key") != std::string::npos) createKey(optId, minecraft);
	else if (dynamic_cast<OptionBool*>(option)) createToggle(optId, minecraft);
	else if (dynamic_cast<OptionFloat*>(option)) createProgressSlider(optId, minecraft);
	else if (dynamic_cast<OptionInt*>(option)) createStepSlider(optId, minecraft);
	else if (dynamic_cast<OptionString*>(option)) createTextbox(optId, minecraft);

	return *this;
}

// TODO: wrap this copypaste shit into templates

void OptionsGroup::createToggle(OptionId optId, Minecraft* minecraft ) {
	ImageDef def;

	def.setSrc(IntRectangle(160, 206, 39, 20));
	def.name = "gui/touchgui.png";
	def.width = 39 * 0.7f;
	def.height = 20 * 0.7f;
	
	OptionButton* element = new OptionButton(optId);
	element->setImageDef(def, true);
	element->updateImage(&minecraft->options);
	
	std::string itemLabel = I18n::get(minecraft->options.getOpt(optId)->getStringId());
	
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	
	addChild(item);
	setupPositions();
}

void OptionsGroup::createProgressSlider(OptionId optId, Minecraft* minecraft ) {
	Slider* element = new SliderFloat(minecraft, optId);
	element->width = 100;
	element->height = 20;

	std::string itemLabel = I18n::get(minecraft->options.getOpt(optId)->getStringId());
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	addChild(item);
	setupPositions();
}

void OptionsGroup::createStepSlider(OptionId optId, Minecraft* minecraft ) {
	Slider* element = new SliderInt(minecraft, optId);
	element->width = 100;
	element->height = 20;
	std::string itemLabel = I18n::get(minecraft->options.getOpt(optId)->getStringId());
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	addChild(item);
	setupPositions();
}

void OptionsGroup::createTextbox(OptionId optId, Minecraft* minecraft) {
	TextBox* element = new TextOption(minecraft, optId);
	element->width = 100;
	element->height = 20;

	std::string itemLabel = I18n::get(minecraft->options.getOpt(optId)->getStringId());
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	addChild(item);
	setupPositions();
}

void OptionsGroup::createKey(OptionId optId, Minecraft* minecraft) {
	KeyOption* element = new KeyOption(minecraft, optId);
	element->width = 50;
	element->height = 20;

	std::string itemLabel = I18n::get(minecraft->options.getOpt(optId)->getStringId());
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	addChild(item);
	setupPositions();
}
