#include "RendererBackend.h"

RendererBackend* createGLESRendererBackend();

#if defined(__APPLE__) && defined(USE_METAL_RENDERER)
RendererBackend* createMetalRendererBackend();
#endif

static RendererBackend* gRendererBackend = 0;

RendererBackend& rendererBackend()
{
	if (!gRendererBackend) {
		gRendererBackend = createGLESRendererBackend();
		gRendererBackend->initialize();
	}
	return *gRendererBackend;
}

bool rendererUseMetalBackend(void* nativeLayer)
{
#if defined(__APPLE__) && defined(USE_METAL_RENDERER)
	RendererBackend* backend = createMetalRendererBackend();
	if (!backend || !backend->initialize(nativeLayer)) {
		delete backend;
		return false;
	}
	if (gRendererBackend) {
		gRendererBackend->shutdown();
		delete gRendererBackend;
	}
	gRendererBackend = backend;
	return true;
#else
	(void)nativeLayer;
	return false;
#endif
}

void rendererUseGLESBackend()
{
	RendererBackend* backend = createGLESRendererBackend();
	backend->initialize();
	if (gRendererBackend) {
		gRendererBackend->shutdown();
		delete gRendererBackend;
	}
	gRendererBackend = backend;
}
