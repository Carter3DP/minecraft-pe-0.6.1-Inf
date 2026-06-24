#ifndef LOG_H__
#define LOG_H__

#ifdef __cplusplus
	#include <cstdio>
	#include <cstdarg>
#else
	#include <stdio.h>
	#include <stdarg.h>
#endif

#define __LOG_PUBLISH(...) do { __VA_ARGS__; } while(0)

// Runtime logging levels:
// 0 = Infinite-world debug traces added during this investigation.
// 1 = Existing project logging (LOGI/LOGW/LOGE).
// 2 = Silent for controlled logging paths.
extern int g_mcpeLogLevel;

static inline void MCPE_LOG_PRINTF(const char* fmt, ...)
{
	if (g_mcpeLogLevel != 1)
		return;

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	fflush(stdout);
	va_end(args);
}

#ifdef ANDROID
	#include <android/log.h>
	#ifdef ANDROID_PUBLISH
		#define LOGV(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
		#define LOGI(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
		#define LOGW(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
		#define LOGE(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
	#else
		// @todo @fix; Obiously the tag shouldn't be hardcoded in here..
		#define LOGV(...) ((void)__android_log_print( ANDROID_LOG_VERBOSE, "MinecraftPE", __VA_ARGS__ ))
		#define LOGI(...) ((void)__android_log_print( ANDROID_LOG_INFO,  "MinecraftPE", __VA_ARGS__ ))
		#define LOGW(...) ((void)__android_log_print( ANDROID_LOG_WARN,  "MinecraftPE", __VA_ARGS__ ))
		#define LOGE(...) ((void)__android_log_print( ANDROID_LOG_ERROR, "MinecraftPE", __VA_ARGS__ ))
		#define printf LOGI
	#endif
#else
#ifdef PUBLISH
    #define LOGV(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
    #define LOGI(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
    #define LOGW(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
    #define LOGE(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
#else
	#define LOGV(...) MCPE_LOG_PRINTF(__VA_ARGS__)
	#define LOGI(...) MCPE_LOG_PRINTF(__VA_ARGS__)
	#define LOGW(...) MCPE_LOG_PRINTF(__VA_ARGS__)
	#define LOGE(...) MCPE_LOG_PRINTF(__VA_ARGS__)
#endif
#endif

#ifdef _DEBUG
	#define LOGVV LOGV
#else
	#define LOGVV(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
#endif

static inline void MCPE_CRASH_TRACE(const char* fmt, ...)
{
	if (g_mcpeLogLevel != 0)
		return;

	va_list args;
	va_start(args, fmt);

	va_list fileArgs;
	va_copy(fileArgs, args);

	vprintf(fmt, args);
	fflush(stdout);

	FILE* file = fopen("mcpe64-crash-trace.log", "ab");
	if (file)
	{
		vfprintf(file, fmt, fileArgs);
		fflush(file);
		fclose(file);
	}

	va_end(fileArgs);
	va_end(args);
}

#endif /*LOG_H__*/
