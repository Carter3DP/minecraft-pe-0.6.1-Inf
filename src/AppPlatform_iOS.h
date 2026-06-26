#ifndef APPPLATFORM_IOS_H__
#define APPPLATFORM_IOS_H__

#include "AppPlatform.h"
#include "client/renderer/gles.h"
#include "platform/log.h"
#include <sys/sysctl.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>

@class minecraftpeViewController;

class AppPlatform_iOS: public AppPlatform
{
	typedef AppPlatform super;
public:
    AppPlatform_iOS(minecraftpeViewController* vc) {
        _viewController = vc;
        srand(time(0));

        LOGI("ViewController in AppPlatform: %p\n", _viewController);
    }

    void setBasePath(const std::string& bp) { _basePath = bp; }
    
    void saveScreenshot(const std::string& filename, int glWidth, int glHeight) {
        //@todo
    }

    __inline unsigned int rgbToBgr(unsigned int p) {
        return (p & 0xff00ff00) | ((p >> 16) & 0xff) | ((p << 16) & 0xff0000);
    }

    static std::string getAppleHardwareMachine() {
        size_t size = 0;
    
        if (sysctlbyname("hw.machine", nullptr, &size, nullptr, 0) != 0 || size == 0) {
            return "";
        }
    
        std::string machine(size, '\0');
    
        if (sysctlbyname("hw.machine", &machine[0], &size, nullptr, 0) != 0) {
            return "";
        }
    
        // sysctl returns a null-terminated string.
        if (!machine.empty() && machine.back() == '\0') {
            machine.pop_back();
        }
    
        return machine;
    }

	virtual void showDialog(int dialogId);
	virtual int getUserInputStatus();
	virtual StringVector getUserInput();
    
    TextureData loadTexture(const std::string& filename_, bool textureFolder);

	virtual BinaryBlob readAssetFile(const std::string& filename);
    
    std::string getDateString(int s);

	virtual int getScreenWidth();
	virtual int getScreenHeight();
    virtual float getPixelsPerMillimeter();
    static float machineIsOneOf(const std::string& machine, const char* const* list, int count);
    static float applePixelsPerMillimeterForMachine(const std::string& machine);
    
	virtual bool isTouchscreen();
    virtual void vibrate(int milliSeconds);
    
	virtual bool isNetworkEnabled(bool onlyWifiAllowed);
    
	virtual StringVector getOptionStrings();

    virtual bool isPowerVR();
    virtual bool isSuperFast();
    virtual void showKeyboard();
    virtual void hideKeyboard();

    virtual ScreenSafeBounds getSafeZone();
private:
    
    std::string _basePath;
    minecraftpeViewController* _viewController;
};

#endif /*APPPLATFORM_IOS_H__*/
