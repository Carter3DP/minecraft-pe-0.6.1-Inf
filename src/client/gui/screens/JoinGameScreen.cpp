#include "JoinGameScreen.h"
#include "StartMenuScreen.h"
#include "ProgressScreen.h"
#include "../Font.h"
#include "../../renderer/Textures.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/gles.h"
#include "../../../network/RakNetInstance.h"

static const int JOIN_LIST_EXTRA_WIDTH = 30;
static const int SERVER_ICON_SIZE = 24;
static const int SERVER_ICON_TEXT_GAP = 6;

JoinGameScreen::JoinGameScreen()
:	bJoin(  2, "Join Game"),
	bBack(  3, "Back"),
	gamesList(NULL)
{
	bJoin.active = false;
	//gamesList->yInertia = 0.5f;
}

JoinGameScreen::~JoinGameScreen()
{
	delete gamesList;
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
	const RakNet::TimeMS staleServerTimeoutMs = 10000;
	const RakNet::TimeMS now = RakNet::GetTimeMS();
	const ServerList& orgServerList = minecraft->raknetInstance->getServerList();
	ServerList serverList;
	for (unsigned int i = 0; i < orgServerList.size(); ++i)
		if (orgServerList[i].name.GetLength() > 0)
			serverList.push_back(orgServerList[i]);

	bool shouldSaveServerList = false;
	for (unsigned int i = 0; i < serverList.size(); ++i)
	{
		bool found = false;
		for (unsigned int j = 0; j < gamesList->copiedServerList.size(); ++j)
		{
			if (gamesList->copiedServerList[j].address == serverList[i].address)
			{
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
		if (!found) {
			gamesList->copiedServerList.push_back(serverList[i]);
			shouldSaveServerList = true;
		}
	}

	for (int i = (int)gamesList->copiedServerList.size() - 1; i >= 0; --i)
	{
		if (!gamesList->copiedServerList[i].isSaved && now - gamesList->copiedServerList[i].lastSeenTime > staleServerTimeoutMs)
		{
			gamesList->copiedServerList.erase(gamesList->copiedServerList.begin() + i);
			shouldSaveServerList = true;
			if (gamesList->selectedItem == i)
				gamesList->selectItem(-1, false);
			else if (gamesList->selectedItem > i)
				gamesList->selectItem(gamesList->selectedItem - 1, false);
		}
	}

	if (shouldSaveServerList)
		minecraft->raknetInstance->saveServerList(gamesList->copiedServerList);
	bJoin.active = isIndexValid(gamesList->selectedItem);
}

void JoinGameScreen::init()
{
	buttons.push_back(&bJoin);
	buttons.push_back(&bBack);

	minecraft->raknetInstance->clearServerList();
	gamesList = new AvailableGamesList(minecraft, width, height);
	gamesList->copiedServerList = minecraft->raknetInstance->loadSavedServers();

#ifdef ANDROID
	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bBack);
#endif
}

void JoinGameScreen::setupPositions() {
	int yBase = height - 26;

	//#ifdef ANDROID
	bJoin.y =	yBase;
	bBack.y =   yBase;

	bBack.width = bJoin.width = 120;
	//#endif

	// Center buttons
	bJoin.x = width / 2 - 4 - bJoin.width;
	bBack.x = width / 2 + 4;
}

void JoinGameScreen::render( int xm, int ym, float a )
{
	bool hasNetwork = minecraft->platform()->isNetworkEnabled(true);
#ifdef WIN32
	hasNetwork = hasNetwork && !GetAsyncKeyState(VK_TAB);
#endif

	renderBackground();
	if (hasNetwork) gamesList->render(xm, ym, a);
	Screen::render(xm, ym, a);

	if (hasNetwork) {
#ifdef RPI
		std::string s = "Scanning for Local Network Games...";
#else
		std::string s = "Scanning for WiFi Games...";
#endif
		drawCenteredString(minecraft->font, s, width / 2, 8, 0xffffffff);

		const int textWidth = minecraft->font->width(s);
		const int spinnerX = width/2 + textWidth / 2 + 6;

		static const char* spinnerTexts[] = {"-", "\\", "|", "/"};
		int n = ((int)(5.5f * getTimeS()) % 4);
		drawCenteredString(minecraft->font, spinnerTexts[n], spinnerX, 8, 0xffffffff);
	} else {
		std::string s = "WiFi is disabled";
		const int yy = height / 2 - 8;
		drawCenteredString(minecraft->font, s, width / 2, yy, 0xffffffff);
	}
}

bool JoinGameScreen::isInGameScreen() { return false; }

int AvailableGamesList::getRowLeft() const
{
	return width / 2 - (92 + 16 + 2) - JOIN_LIST_EXTRA_WIDTH / 2;
}

int AvailableGamesList::getRowRight() const
{
	return width / 2 + (92 + 16 + 2) + JOIN_LIST_EXTRA_WIDTH / 2;
}

void AvailableGamesList::renderItem(int i, int x, int y, int h, Tesselator& t) {
		const PingedCompatibleServer& s = copiedServerList[i];
		unsigned int color = s.isSpecial? 0xff00b0 : 0xffffa0;
		std::string ping = std::to_string(s.pingTime) + "ms";
		int xx3 = getRowRight() - minecraft->font->width(ping) - 3;
		const int textX = x + SERVER_ICON_SIZE + SERVER_ICON_TEXT_GAP;
		std::string icon = (!s.isClientHosted && s.hasIcon && !s.icon.empty()) ? s.icon : "gui/default_world.png";
		TextureId texId = (icon == "gui/default_world.png") ? minecraft->textures->loadTexture(icon) : minecraft->textures->loadTexture("%clamp%" + icon, false);
		if (!Textures::isTextureIdValid(texId) && icon != "gui/default_world.png")
			texId = minecraft->textures->loadTexture("gui/default_world.png");
		if (Textures::isTextureIdValid(texId)) {
			minecraft->textures->bind(texId);
			glColor4f2(1, 1, 1, 1);
			glEnable2(GL_BLEND);
			t.begin();
				t.vertexUV((float)x, (float)(y + SERVER_ICON_SIZE), blitOffset, 0, 1);
				t.vertexUV((float)(x + SERVER_ICON_SIZE), (float)(y + SERVER_ICON_SIZE), blitOffset, 1, 1);
				t.vertexUV((float)(x + SERVER_ICON_SIZE), (float)y, blitOffset, 1, 0);
				t.vertexUV((float)x, (float)y, blitOffset, 0, 0);
			t.draw();
		}
		drawString(minecraft->font, s.name.C_String(), textX, y + 2, color);
		drawString(minecraft->font, s.address.ToString(false), textX, y + 16, 0xffffa0);
		drawString(minecraft->font, ping, xx3, y + 16, color);
	}
