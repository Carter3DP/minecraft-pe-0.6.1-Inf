#ifndef NET_MINECRAFT_CLIENT_RENDERER__RendererBackend_H__
#define NET_MINECRAFT_CLIENT_RENDERER__RendererBackend_H__

#include "gles.h"
#include "TextureData.h"

typedef GLuint TextureId;

enum RendererBackendType
{
	RENDERER_BACKEND_GLES
};

enum RendererVertexFormat
{
	RENDERER_VERTEX_FORMAT_V,
	RENDERER_VERTEX_FORMAT_VC,
	RENDERER_VERTEX_FORMAT_VN,
	RENDERER_VERTEX_FORMAT_VCN,
	RENDERER_VERTEX_FORMAT_VT,
	RENDERER_VERTEX_FORMAT_VTC,
	RENDERER_VERTEX_FORMAT_VTN,
	RENDERER_VERTEX_FORMAT_VTCN
};

struct TextureUploadOptions
{
	TextureUploadOptions()
		: mipmap(false), blur(false), clamp(false)
	{}

	bool mipmap;
	bool blur;
	bool clamp;
};

class RendererBackend
{
public:
	virtual ~RendererBackend() {}

	virtual RendererBackendType type() const = 0;
	virtual bool initialize(void* nativeLayer = 0) = 0;
	virtual void shutdown() = 0;

	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
	virtual void present() = 0;

	virtual void createVertexBuffers(GLsizei count, GLuint* buffers) = 0;
	virtual void deleteVertexBuffer(GLuint buffer) = 0;
	virtual void updateVertexBuffer(GLuint buffer, const void* data, GLsizei bytes, bool dynamic) = 0;
	virtual void drawVertexBuffer(GLuint buffer, int vertices, int vertexSize, RendererVertexFormat format, unsigned int mode) = 0;

	virtual TextureId createTexture() = 0;
	virtual void deleteTexture(TextureId texture) = 0;
	virtual void bindTexture(TextureId texture) = 0;
	virtual void uploadTexture(TextureId texture, const TextureData& data, const TextureUploadOptions& options) = 0;
	virtual void updateTextureRegion(TextureId texture, int x, int y, int width, int height, const void* data) = 0;

	virtual void setProjectionMatrix(const float* matrix16) = 0;
	virtual void setModelViewMatrix(const float* matrix16) = 0;
	virtual void setViewport(int x, int y, int width, int height) = 0;
	virtual void setBlendEnabled(bool enabled) = 0;
	virtual void setDepthTestEnabled(bool enabled) = 0;
	virtual void setCullFaceEnabled(bool enabled) = 0;
};

RendererBackend& rendererBackend();
void rendererUseGLESBackend();

#endif
