#include "AppPlatform_iOS.h"
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

#import "../project/iosproj/minecraftpe/minecraftpeViewController.h"
#import "../project/iosproj/minecraftpe/dialogs/BaseDialogController.h"
#import "../project/iosproj/minecraftpe/PVRTexture.h"
#import "client/gui/screens/DialogDefinitions.h"

#import "terrain_565.h"

typedef unsigned int PVRTuint32;

struct PVR_Texture_Header
{
	PVRTuint32 dwHeaderSize;			/*!< size of the structure */
	PVRTuint32 dwHeight;				/*!< height of surface to be created */
	PVRTuint32 dwWidth;				/*!< width of input surface */
	PVRTuint32 dwMipMapCount;			/*!< number of mip-map levels requested */
	PVRTuint32 dwpfFlags;				/*!< pixel format flags */
	PVRTuint32 dwTextureDataSize;		/*!< Total size in bytes */
	PVRTuint32 dwBitCount;			/*!< number of bits per pixel  */
	PVRTuint32 dwRBitMask;			/*!< mask for red bit */
	PVRTuint32 dwGBitMask;			/*!< mask for green bits */
	PVRTuint32 dwBBitMask;			/*!< mask for blue bits */
	PVRTuint32 dwAlphaBitMask;		/*!< mask for alpha channel */
	PVRTuint32 dwPVR;					/*!< magic number identifying pvr file */
	PVRTuint32 dwNumSurfs;			/*!< the number of surfaces present in the pvr */
} ;

void AppPlatform_iOS::showDialog(int dialogId) {
    if (dialogId == DialogDefinitions::DIALOG_NEW_CHAT_MESSAGE) {
        [_viewController showChatKeyboardInput];
        super::showKeyboard();
    }
    if (dialogId == DialogDefinitions::DIALOG_CREATE_NEW_WORLD) {
        [_viewController showDialog_CreateWorld];
    }
    if (dialogId == DialogDefinitions::DIALOG_MAINMENU_OPTIONS) {
        [_viewController showDialog_MainMenuOptions];
    }
    if (dialogId == DialogDefinitions::DIALOG_RENAME_MP_WORLD) {
        [_viewController showDialog_RenameMPWorld];
    }
    if (dialogId == DialogDefinitions::DIALOG_DEMO_FEATURE_DISABLED) {
        UIAlertView *a = [[UIAlertView alloc]
            initWithTitle:@"" 
            message:@"Feature not enabled for this demo" 
            delegate:nil 
            cancelButtonTitle:@"OK"
            otherButtonTitles:nil];
        [a show];
        [a release];
    }
}

TextureData AppPlatform_iOS::loadTexture(const std::string& filename_, bool textureFolder)
{
    TextureData out;
    out.memoryHandledExternally = false;

    std::string filename = filename_;
    size_t dotp = filename.rfind(".");
    size_t slashp = filename.rfind("/");
    if (dotp != std::string::npos || slashp != std::string::npos) {
        if (slashp == std::string::npos) slashp = -1;
        filename = filename.substr(slashp+1, dotp-(slashp+1));
    }

//    if (filename == "terrain" || filename_[dotp+2] == 'v') { // @fix
//        //NSString *path = [[NSBundle mainBundle] pathForResource:[[NSString alloc] initWithUTF8String:filename.c_str()] ofType:@"pvr4"];
//        
//        //FILE* fp = fopen([path UTF8String], "rb");
//        int fp = 1;
//        if (fp) {
//            PVR_Texture_Header header;
//            header = *((PVR_Texture_Header*)terrain_565);
//            //fread(&header, 1, sizeof(PVR_Texture_Header), fp);
//            int numBytes = header.dwTextureDataSize;
//            //out.data = new unsigned char[numBytes];
//            out.data = (unsigned char*)&terrain_565[header.dwHeaderSize];
//            out.memoryHandledExternally = true;
//            out.numBytes = numBytes;
//            out.transparent = (header.dwAlphaBitMask != 0);
//            //fread(out.data, 1, numBytes, fp);
//            out.w = header.dwWidth;
//            out.h = header.dwHeight;
//            LOGI("Size of file: %d (%d, %d) - %x,%x,%x,%x\n", out.numBytes, out.w, out.h,
//                 header.dwRBitMask,
//                 header.dwGBitMask,
//                 header.dwBBitMask,
//                 header.dwAlphaBitMask);
//            out.format = TEXF_UNCOMPRESSED_565;// TEXF_COMPRESSED_PVRTC_5551;
//            //fclose(fp);
//        }
//        
////        PVRTexture* tex = [PVRTexture pvrTextureWithContentsOfFile:path];
////        //NSLog(@"path: %@, tex: %p, name: %d\n", path, tex, [tex name]);
////        out.identifier = [tex name];
////        out.w = [tex width];
////        out.h = [tex height];
////        GLuint texId;
////        //PVRTTextureLoadFromPVR([path UTF8String], &texId);
////        out.identifier = texId;
//        return out;
//    }
    
    NSString *p = [[NSString alloc] initWithUTF8String:filename.c_str()];
    NSString *path =  [[NSBundle mainBundle] pathForResource:p ofType:@"png"];
    [p release];
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    UIImage *image = [[UIImage alloc] initWithData:texData];

    if (image != nil) {
        // Get Image size
        out.w = CGImageGetWidth(image.CGImage);
        out.h = CGImageGetHeight(image.CGImage);
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        // Allocate memory for image
        out.data = new unsigned char[4 * out.w * out.h];
        CGContextRef imgcontext = CGBitmapContextCreate( out.data, out.w, out.h, 8, 4 * out.w, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
        CGColorSpaceRelease( colorSpace );
        CGContextClearRect( imgcontext, CGRectMake( 0, 0, out.w, out.h ) );
        CGContextTranslateCTM( imgcontext, 0, 0);//height - height );
        CGContextDrawImage( imgcontext, CGRectMake( 0, 0, out.w, out.h ), image.CGImage );
        CGContextRelease(imgcontext);
    } else {
        LOGI("Couldn't find file: %s\n", filename.c_str());

        if ("this is idiotic but temporary") {
            out.w = 16;
            out.h = 16;
            bool isTerrain = (filename.find("terrain") != std::string::npos);
            int numPixels = out.w * out.h;
            out.data = new unsigned char[4 * numPixels];
            if (isTerrain) {
                for (int i = 0; i < numPixels; ++i) {
                    unsigned int color = 0xff000000 | ((rand() & 0xff) << 16) | (rand() & 0xffff);
                    *((int*)(&out.data[4*i])) = color;
                }
            } else {
                unsigned int color = 0xff000000 | ((rand() & 0xff) << 16) | (rand() & 0xffff);
                for (int i = 0; i < numPixels; ++i) {
                    *((int*)(&out.data[4*i])) = color;
                }
            }
        }
    }
    [image release];
    [texData release];
    
    return out;
}

BinaryBlob AppPlatform_iOS::readAssetFile(const std::string& filename_) {
    std::string filename = filename_;
    size_t dotp = filename.rfind(".");
    size_t slashp = filename.rfind("/");
    std::string ext;
    if (dotp != std::string::npos || slashp != std::string::npos) {
        // Get file extension
        if (dotp != std::string::npos) {
            ext = filename.substr(dotp+1);
        }
        if (slashp == std::string::npos) slashp = -1;
        filename = filename.substr(slashp+1, dotp-(slashp+1));
    }
    NSString *rext = [NSString stringWithUTF8String:ext.c_str()];
    NSString *p = [[NSString alloc] initWithUTF8String:filename.c_str()];
    NSString *path =  [[NSBundle mainBundle] pathForResource:p ofType:rext];
    [p release];
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data)
        return BinaryBlob();

    unsigned int numBytes = [data length];
    unsigned char* bytes = new unsigned char[numBytes];
    memcpy(bytes, [data bytes], numBytes);
    return BinaryBlob(bytes, numBytes);
}

std::string AppPlatform_iOS::getDateString(int s) {

    NSDate* date = [NSDate dateWithTimeIntervalSince1970:s];
    
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    //dateFormatter.dateFormat = @"yyyy-MM-dd HH:mm";

    [dateFormatter setDateStyle:NSDateFormatterMediumStyle];
    [dateFormatter setTimeStyle:NSDateFormatterShortStyle];
    
    //NSTimeZone *gmt = [NSTimeZone timeZoneWithAbbreviation:@"GMT"];
    //[dateFormatter setTimeZone:gmt];
    NSString *timeStamp = [dateFormatter stringFromDate:date];
    [dateFormatter release];

    return std::string( [timeStamp UTF8String] );
}

std::string AppPlatform_iOS::getAppleHardwareMachine() {
    size_t size = 0;

    if (sysctlbyname("hw.machine", NULL, &size, NULL, 0) != 0 || size == 0) {
        return "";
    }

    std::string machine(size, '\0');

    if (sysctlbyname("hw.machine", &machine[0], &size, NULL, 0) != 0) {
        return "";
    }

    if (!machine.empty() && machine[machine.size() - 1] == '\0') {
        machine.resize(machine.size() - 1);
    }

    return machine;
}

int AppPlatform_iOS::getScreenWidth()  {
    CGRect screen = [[UIScreen mainScreen] bounds];
    return (int)(MAX(screen.size.width, screen.size.height) * _viewController->viewScale);
}

int AppPlatform_iOS::getScreenHeight() { 
    CGRect screen = [[UIScreen mainScreen] bounds];
    return (int)(MIN(screen.size.width, screen.size.height) * _viewController->viewScale);
    
}

bool AppPlatform_iOS::machineIsOneOf(const std::string& machine, const char* const* list, int count) {
    for (int i = 0; i < count; ++i) {
        if (machine == list[i]) {
            return true;
        }
    }
    return false;
}

float AppPlatform_iOS::applePixelsPerMillimeterForMachine(const std::string& machine) {
    /*
        Return real physical display pixels per millimeter.

        These are already px/mm values.
        Do NOT multiply by UIScreen.scale or viewScale.
    */

    // Simulator fallback if SIMULATOR_MODEL_IDENTIFIER was unavailable.
    if (machine == "arm64" || machine == "x86_64" || machine == "i386") {
        BOOL isIpad = (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad);
        return isIpad ? 10.3937007874f : 18.1102362205f; // iPad 264ppi, iPhone 460ppi
    }

    // ---- iPhone 163 PPI -> 6.4173228346 px/mm ----
    static const char* const iphone_163[] = {
        "iPhone1,1",
        "iPhone1,2",
        "iPhone2,1"
    };
    if (machineIsOneOf(machine, iphone_163, sizeof(iphone_163) / sizeof(iphone_163[0]))) {
        return 6.4173228346f;
    }

    // ---- iPhone 326 PPI -> 12.8346456693 px/mm ----
    static const char* const iphone_326[] = {
        "iPhone3,1", "iPhone3,2", "iPhone3,3",
        "iPhone4,1",

        "iPhone5,1", "iPhone5,2",
        "iPhone5,3", "iPhone5,4",

        "iPhone6,1", "iPhone6,2",

        "iPhone7,2",

        "iPhone8,1",
        "iPhone8,4",

        "iPhone9,1", "iPhone9,3",

        "iPhone10,1", "iPhone10,4",

        "iPhone11,8",

        "iPhone12,1",
        "iPhone12,8",

        "iPhone14,6"
    };
    if (machineIsOneOf(machine, iphone_326, sizeof(iphone_326) / sizeof(iphone_326[0]))) {
        return 12.8346456693f;
    }

    // ---- iPhone 401 PPI -> 15.7874015748 px/mm ----
    static const char* const iphone_401[] = {
        "iPhone7,1",
        "iPhone8,2",
        "iPhone9,2", "iPhone9,4",
        "iPhone10,2", "iPhone10,5"
    };
    if (machineIsOneOf(machine, iphone_401, sizeof(iphone_401) / sizeof(iphone_401[0]))) {
        return 15.7874015748f;
    }

    // ---- iPhone 458 PPI -> 18.0314960630 px/mm ----
    static const char* const iphone_458[] = {
        "iPhone10,3", "iPhone10,6",

        "iPhone11,2",
        "iPhone11,4", "iPhone11,6",

        "iPhone12,3",
        "iPhone12,5",

        "iPhone13,4",

        "iPhone14,3",
        "iPhone14,8"
    };
    if (machineIsOneOf(machine, iphone_458, sizeof(iphone_458) / sizeof(iphone_458[0]))) {
        return 18.0314960630f;
    }

    // ---- iPhone 460 PPI -> 18.1102362205 px/mm ----
    static const char* const iphone_460[] = {
        "iPhone13,2",
        "iPhone13,3",

        "iPhone14,2",
        "iPhone14,5",
        "iPhone14,7",

        "iPhone15,2",
        "iPhone15,3",
        "iPhone15,4",
        "iPhone15,5",

        "iPhone16,1",
        "iPhone16,2",

        "iPhone17,1",
        "iPhone17,2",
        "iPhone17,3",
        "iPhone17,4",
        "iPhone17,5",

        "iPhone18,1",
        "iPhone18,2",
        "iPhone18,3",
        "iPhone18,4",
        "iPhone18,5"
    };
    if (machineIsOneOf(machine, iphone_460, sizeof(iphone_460) / sizeof(iphone_460[0]))) {
        return 18.1102362205f;
    }

    // ---- iPhone 476 PPI -> 18.7401574803 px/mm ----
    static const char* const iphone_476[] = {
        "iPhone13,1",
        "iPhone14,4"
    };
    if (machineIsOneOf(machine, iphone_476, sizeof(iphone_476) / sizeof(iphone_476[0]))) {
        return 18.7401574803f;
    }

    // ---- iPad 132 PPI -> 5.1968503937 px/mm ----
    static const char* const ipad_132[] = {
        "iPad1,1",
        "iPad2,1", "iPad2,2", "iPad2,3", "iPad2,4"
    };
    if (machineIsOneOf(machine, ipad_132, sizeof(ipad_132) / sizeof(ipad_132[0]))) {
        return 5.1968503937f;
    }

    // ---- iPad 163 PPI -> 6.4173228346 px/mm ----
    static const char* const ipad_163[] = {
        "iPad2,5", "iPad2,6", "iPad2,7"
    };
    if (machineIsOneOf(machine, ipad_163, sizeof(ipad_163) / sizeof(ipad_163[0]))) {
        return 6.4173228346f;
    }

    // ---- iPad 264 PPI -> 10.3937007874 px/mm ----
    static const char* const ipad_264[] = {
        "iPad3,1", "iPad3,2", "iPad3,3",
        "iPad3,4", "iPad3,5", "iPad3,6",

        "iPad4,1", "iPad4,2", "iPad4,3",

        "iPad5,3", "iPad5,4",

        "iPad6,3", "iPad6,4",
        "iPad6,7", "iPad6,8",
        "iPad6,11", "iPad6,12",

        "iPad7,1", "iPad7,2",
        "iPad7,3", "iPad7,4",
        "iPad7,5", "iPad7,6",
        "iPad7,11", "iPad7,12",

        "iPad8,1", "iPad8,2", "iPad8,3", "iPad8,4",
        "iPad8,5", "iPad8,6", "iPad8,7", "iPad8,8",
        "iPad8,9", "iPad8,10",
        "iPad8,11", "iPad8,12",

        "iPad11,3", "iPad11,4",
        "iPad11,6", "iPad11,7",

        "iPad12,1", "iPad12,2",

        "iPad13,1", "iPad13,2",
        "iPad13,4", "iPad13,5", "iPad13,6", "iPad13,7",
        "iPad13,8", "iPad13,9", "iPad13,10", "iPad13,11",
        "iPad13,16", "iPad13,17",
        "iPad13,18", "iPad13,19",

        "iPad14,3", "iPad14,4",
        "iPad14,5", "iPad14,6",
        "iPad14,8", "iPad14,9",
        "iPad14,10", "iPad14,11",

        "iPad15,3", "iPad15,4",
        "iPad15,5", "iPad15,6",
        "iPad15,7", "iPad15,8",

        "iPad16,3", "iPad16,4",
        "iPad16,5", "iPad16,6",
        "iPad16,8", "iPad16,9",
        "iPad16,10", "iPad16,11",

        "iPad17,1", "iPad17,2",
        "iPad17,3", "iPad17,4"
    };
    if (machineIsOneOf(machine, ipad_264, sizeof(ipad_264) / sizeof(ipad_264[0]))) {
        return 10.3937007874f;
    }

    // ---- iPad mini Retina / modern mini 326 PPI -> 12.8346456693 px/mm ----
    static const char* const ipad_326[] = {
        "iPad4,4", "iPad4,5", "iPad4,6",
        "iPad4,7", "iPad4,8", "iPad4,9",
        "iPad5,1", "iPad5,2",
        "iPad11,1", "iPad11,2",
        "iPad14,1", "iPad14,2",
        "iPad16,1", "iPad16,2"
    };
    if (machineIsOneOf(machine, ipad_326, sizeof(ipad_326) / sizeof(ipad_326[0]))) {
        return 12.8346456693f;
    }

    // Unknown future device fallbacks.
    if (machine.rfind("iPhone", 0) == 0) {
        return 18.1102362205f; // assume modern 460 PPI iPhone
    }

    if (machine.rfind("iPad", 0) == 0) {
        return 10.3937007874f; // assume normal 264 PPI iPad
    }

    // Generic fallback.
    BOOL isIpad = (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad);
    return isIpad ? 10.3937007874f : 18.1102362205f;
}

float AppPlatform_iOS::getPixelsPerMillimeter() {
    return applePixelsPerMillimeterForMachine(getAppleHardwareMachine());
}

bool AppPlatform_iOS::isTouchscreen()  { return true; }

void AppPlatform_iOS::vibrate(int ms) {
    // Note: In iOS 4, there's no way to set length of the vibration, so it's useless
    //AudioServicesPlaySystemSound (kSystemSoundID_Vibrate);
}

int AppPlatform_iOS::getUserInputStatus() {
    return [_viewController getUserInputStatus];
}

StringVector AppPlatform_iOS::getUserInput() {
    return [_viewController getUserInput];
}

StringVector AppPlatform_iOS::getOptionStrings() {
    //@options
    StringVector options;
    NSDictionary* d = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
    
    for( NSString *key in d )
    {
        if ([key hasPrefix:@"mp_"]
         || [key hasPrefix:@"gfx_"]
         || [key hasPrefix:@"ctrl_"]
         || [key hasPrefix:@"feedback_"]
         || [key hasPrefix:@"game_"] ) {
            id value = [d objectForKey: key];
            options.push_back([key UTF8String]);
            options.push_back([[value description] UTF8String]);
            //LOGI("Added strings: %s\n", options[options.size()-1].c_str());
        }
    }
    return options;
}

bool AppPlatform_iOS::isSuperFast() {
    const char* s = (const char*)glGetString(GL_RENDERER);
    if (!s) return false;
    
    return (strstr(s, "SGX") != NULL) && (strstr(s, "543") != NULL);
}

bool AppPlatform_iOS::isNetworkEnabled(bool onlyWifiAllowed) {
    return true;
    /*
    Reachability *reachability = [Reachability reachabilityForInternetConnection];
    [reachability startNotifier];
    
    NetworkStatus status = [reachability currentReachabilityStatus];
    bool success = (status == ReachableViaWiFiNetwork);
    if (!onlyWifiAllowed && !success)
        success = (status == ReachableViaWWAN);

    [reachability stopNotifier];
    return success;
    */
}

void AppPlatform_iOS::showKeyboard() {
    [_viewController showKeyboard];
	super::showKeyboard();
}

void AppPlatform_iOS::hideKeyboard() {
    [_viewController hideKeyboard];
	super::hideKeyboard();
}

// this was originally void but i changed it to bool because void cant return values
bool AppPlatform_iOS::isPowerVR() {
	const char* s = (const char*)glGetString(GL_RENDERER);
	if (!s) return false;
	return strstr(s, "SGX") != NULL;
}

//ios specific safe zone code
ScreenSafeBounds AppPlatform_iOS::getSafeZone() {
    ScreenSafeBounds bounds;
    bounds.left = 0;
    bounds.right = getScreenWidth();
    bounds.top = 0;
    bounds.bottom = getScreenHeight();

    UIView* view = [_viewController view];
    if (!view || ![view respondsToSelector:@selector(safeAreaInsets)])
        return bounds;

    UIEdgeInsets insets = view.safeAreaInsets;
    bounds.left = (int)(insets.left * _viewController->viewScale);
    bounds.right = getScreenWidth() - (int)(insets.right * _viewController->viewScale);
    bounds.top = (int)(insets.top * _viewController->viewScale);
    bounds.bottom = getScreenHeight() - (int)(insets.bottom * _viewController->viewScale);
    return bounds;
}