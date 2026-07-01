#include "TouchJoinGameScreen.h"
#include "../StartMenuScreen.h"
#include "../ProgressScreen.h"
#include "../../Font.h"
#include "../../../Minecraft.h"
#include "../../../renderer/Textures.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/gles.h"

namespace Touch {

static const int SERVER_ICON_SIZE = 24;
static const int SERVER_ICON_TEXT_GAP = 6;

//
// Games list
//

void AvailableGamesList::selectStart( int item) {
	startSelected = item;
}

void AvailableGamesList::selectCancel() {
	startSelected = -1;
}

void AvailableGamesList::selectItem( int item, bool doubleClick ) {
	LOGI("selected an item! %d\n", item);
	selectedItem = item;
}

void AvailableGamesList::renderItem( int i, int x, int y, int h, Tesselator& t )
{
	if (startSelected == i && Multitouch::getFirstActivePointerIdEx() >= 0) {
		fill((int)x0, y, (int)x1, y+h, 0x809E684F);
	}

	//static int colors[2] = {0xffffb0, 0xcccc90};
	const PingedCompatibleServer& s = copiedServerList[i];
	unsigned int color  = s.isSpecial? 0x6090a0 : 0xffffb0;
	unsigned int color2 = 0xffffa0;//colors[i&1];

	int xx2 = (int)(minecraft->SafeZone.left * minecraft->gui.InvGuiScale) + 24;

	std::string icon = (!s.isClientHosted && s.hasIcon && !s.icon.empty()) ? s.icon : "gui/default_world.png";
	TextureId texId = (icon == "gui/default_world.png") ? minecraft->textures->loadTexture(icon) : minecraft->textures->loadTexture("%clamp%" + icon, false);
	if (!Textures::isTextureIdValid(texId) && icon != "gui/default_world.png")
		texId = minecraft->textures->loadTexture("gui/default_world.png");
	if (Textures::isTextureIdValid(texId)) {
		minecraft->textures->bind(texId);
		glColor4f2(1, 1, 1, 1);
		glEnable2(GL_BLEND);
		t.begin();
			t.vertexUV((float)xx2, (float)(y + 5 + SERVER_ICON_SIZE), blitOffset, 0, 1);
			t.vertexUV((float)(xx2 + SERVER_ICON_SIZE), (float)(y + 5 + SERVER_ICON_SIZE), blitOffset, 1, 1);
			t.vertexUV((float)(xx2 + SERVER_ICON_SIZE), (float)(y + 5), blitOffset, 1, 0);
			t.vertexUV((float)xx2, (float)(y + 5), blitOffset, 0, 0);
		t.draw();
	}
	int xx1 = xx2 + SERVER_ICON_SIZE + SERVER_ICON_TEXT_GAP;
	int textX = xx1;

	std::string ping = std::to_string(s.pingTime) + "ms";

	int xx3 = (int)x1 - minecraft->font->width(ping) - 24;
	if (s.isSpecial) {
		xx1 += 50;

		glEnable2(GL_TEXTURE_2D);
        glColor4f2(1,1,1,1);
        glEnable2(GL_BLEND);
		minecraft->textures->loadAndBindTexture("gui/badge/minecon140.png");
		blit(textX, y + 6, 0, 0, 37, 8, 140, 240);
	}

	drawString(minecraft->font, s.name.C_String(), xx1, y + 4 + 2, color);
	drawString(minecraft->font, s.address.ToString(false), textX, y + 18, color2);
	drawString(minecraft->font, ping, xx3, y + 18, color2);

	/*
	drawString(minecraft->font, copiedServerList[i].name.C_String(), (int)x0 + 24, y + 4, color);
	drawString(minecraft->font, copiedServerList[i].address.ToString(false), (int)x0 + 24, y + 18, color);
	*/
}


//
// Join Game screen
//
JoinGameScreen::JoinGameScreen()
:	bJoin(  2, "Join Game"),
	bBack(  3, "Back"),
	bJoinByIp(4, "Join By IP"),
	bHeader(0, ""),
	gamesList(NULL)
{
	bJoin.active = false;
	//gamesList->yInertia = 0.5f;
}

JoinGameScreen::~JoinGameScreen()
{
	delete gamesList;
}

void JoinGameScreen::init()
{
	//buttons.push_back(&bJoin);
	buttons.push_back(&bBack);
	buttons.push_back(&bJoinByIp);
	buttons.push_back(&bHeader);

	minecraft->raknetInstance->clearServerList();
	gamesList = new AvailableGamesList(minecraft, width, height);
	gamesList->copiedServerList = minecraft->raknetInstance->loadSavedServers();

#ifdef ANDROID
	//tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bBack);
	tabButtons.push_back(&bJoinByIp);
#endif
}

void JoinGameScreen::setupPositions() {
	//int yBase = height - 26;

	//#ifdef ANDROID
	bJoin.y     = 0;
	bBack.y     = 0;
	bJoinByIp.y = 0;
	bHeader.y   = 0;
	//#endif

	// Center buttons
	//bJoin.x = width / 2 - 4 - bJoin.w;
	bBack.x = 0;//width / 2 + 4;
	bJoinByIp.x = width - bJoinByIp.width;;
	bHeader.x = bJoinByIp.width;
	bHeader.width = width - (bBack.width + bJoinByIp.width);
}

void JoinGameScreen::buttonClicked(Button* button)
{
	if (button->id == bJoin.id)
	{
		if (isIndexValid(gamesList->selectedItem))
		{
			PingedCompatibleServer selectedServer = gamesList->copiedServerList[gamesList->selectedItem];
			minecraft->joinMultiplayer(selectedServer);
			{
				bJoin.active = false;
				bBack.active = false;
				minecraft->setScreen(new ProgressScreen());
			}
		}
		//minecraft->locateMultiplayer();
		//minecraft->setScreen(new JoinGameScreen());
	}
	if(button->id == bJoinByIp.id) {
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINBYIP);
	}

	if (button->id == bBack.id)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
}

bool JoinGameScreen::handleBackEvent(bool isDown)
{
	if (!isDown)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
	return true;
}


bool JoinGameScreen::isIndexValid( int index )
{
	return gamesList && index >= 0 && index < gamesList->getNumberOfItems();
}

void JoinGameScreen::tick()
{
	if (isIndexValid(gamesList->selectedItem)) {
		buttonClicked(&bJoin);
		return;
	}

	//gamesList->tick();

	const ServerList& orgServerList = minecraft->raknetInstance->getServerList();
	ServerList serverList;
	for (unsigned int i = 0; i < orgServerList.size(); ++i)
		if (orgServerList[i].name.GetLength() > 0)
			serverList.push_back(orgServerList[i]);

	bool shouldSaveServerList = false;
	for (unsigned int i = 0; i < serverList.size(); ++i) {
		bool found = false;
		for (unsigned int j = 0; j < gamesList->copiedServerList.size(); ++j) {
			if (gamesList->copiedServerList[j].address == serverList[i].address) {
				if (gamesList->copiedServerList[j].name.StrCmp(serverList[i].name) != 0 ||
					gamesList->copiedServerList[j].icon != serverList[i].icon ||
					gamesList->copiedServerList[j].hasIcon != serverList[i].hasIcon ||
					gamesList->copiedServerList[j].isClientHosted != serverList[i].isClientHosted)
					shouldSaveServerList = true;
				serverList[i].isSaved = gamesList->copiedServerList[j].isSaved || serverList[i].isSaved;
				gamesList->copiedServerList[j] = serverList[i];
				found = true;
				break;
			}
		}
		if (!found)
		{
			gamesList->copiedServerList.push_back(serverList[i]);
			shouldSaveServerList = true;
		}
	}

	if (shouldSaveServerList)
		minecraft->raknetInstance->saveServerList(gamesList->copiedServerList);
	bJoin.active = isIndexValid(gamesList->selectedItem);
}

void JoinGameScreen::render( int xm, int ym, float a )
{
	bool hasNetwork = minecraft->platform()->isNetworkEnabled(true);
#ifdef WIN32
	hasNetwork = hasNetwork && !GetAsyncKeyState(VK_TAB);
#endif

	renderBackground();
	if (hasNetwork) gamesList->render(xm, ym, a);
	else gamesList->renderDirtBackground();
	Screen::render(xm, ym, a);

	const int baseX = bHeader.x + bHeader.width / 2;

	if (hasNetwork) {
		std::string s = "Scanning for WiFi Games...";
		drawCenteredString(minecraft->font, s, baseX, 8, 0xffffffff);

		const int textWidth = minecraft->font->width(s);
		const int spinnerX = baseX + textWidth / 2 + 6;

		static const char* spinnerTexts[] = {"-", "\\", "|", "/"};
		int n = ((int)(5.5f * getTimeS()) % 4);
		drawCenteredString(minecraft->font, spinnerTexts[n], spinnerX, 8, 0xffffffff);
	} else {
		drawCenteredString(minecraft->font, "WiFi is disabled", baseX, 8, 0xffffffff);
	}
}

bool JoinGameScreen::isInGameScreen() { return false; }

};
