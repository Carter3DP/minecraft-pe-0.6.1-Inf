#include "RendererBackend.h"

RendererBackend* createGLESRendererBackend();

static RendererBackend* gRendererBackend = 0;

RendererBackend& rendererBackend()
{
	if (!gRendererBackend) {
		gRendererBackend = createGLESRendererBackend();
		gRendererBackend->initialize();
	}
	return *gRendererBackend;
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
