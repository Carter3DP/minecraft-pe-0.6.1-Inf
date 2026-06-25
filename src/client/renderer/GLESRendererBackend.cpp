#include "RendererBackend.h"

namespace
{
class GLESRendererBackend : public RendererBackend
{
public:
	RendererBackendType type() const { return RENDERER_BACKEND_GLES; }

	bool initialize(void* nativeLayer)
	{
		(void)nativeLayer;
		return true;
	}

	void shutdown() {}
	void beginFrame() {}
	void endFrame() {}
	void present() {}

	void createVertexBuffers(GLsizei count, GLuint* buffers)
	{
		anGenBuffers(count, buffers);
	}

	void deleteVertexBuffer(GLuint buffer)
	{
		(void)buffer;
	}

	void updateVertexBuffer(GLuint buffer, const void* data, GLsizei bytes, bool dynamic)
	{
		glBindBuffer2(GL_ARRAY_BUFFER, buffer);
		glBufferData2(GL_ARRAY_BUFFER, bytes, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	}

	void drawVertexBuffer(GLuint buffer, int vertices, int vertexSize, RendererVertexFormat format, unsigned int mode)
	{
		const bool hasTexture = format == RENDERER_VERTEX_FORMAT_VT ||
			format == RENDERER_VERTEX_FORMAT_VTC ||
			format == RENDERER_VERTEX_FORMAT_VTN ||
			format == RENDERER_VERTEX_FORMAT_VTCN;
		const bool hasColor = format == RENDERER_VERTEX_FORMAT_VC ||
			format == RENDERER_VERTEX_FORMAT_VCN ||
			format == RENDERER_VERTEX_FORMAT_VTC ||
			format == RENDERER_VERTEX_FORMAT_VTCN;
		const bool hasNormal = format == RENDERER_VERTEX_FORMAT_VN ||
			format == RENDERER_VERTEX_FORMAT_VCN ||
			format == RENDERER_VERTEX_FORMAT_VTN ||
			format == RENDERER_VERTEX_FORMAT_VTCN;

		glBindBuffer2(GL_ARRAY_BUFFER, buffer);

		glVertexPointer2(3, GL_FLOAT, vertexSize, 0);
		glEnableClientState2(GL_VERTEX_ARRAY);

		if (hasTexture) {
			glTexCoordPointer2(2, GL_FLOAT, vertexSize, (GLvoid*) (3 * 4));
			glEnableClientState2(GL_TEXTURE_COORD_ARRAY);
		}
		if (hasColor) {
			glColorPointer2(4, GL_UNSIGNED_BYTE, vertexSize, (GLvoid*) (5 * 4));
			glEnableClientState2(GL_COLOR_ARRAY);
		}
		if (hasNormal) {
			glNormalPointer2(GL_FLOAT, vertexSize, (GLvoid*) (6 * 4));
			glEnableClientState2(GL_NORMAL_ARRAY);
		}

		glDrawArrays2(mode == GL_QUADS ? GL_TRIANGLES : mode, 0, vertices);

		glDisableClientState2(GL_VERTEX_ARRAY);
		if (hasTexture) glDisableClientState2(GL_TEXTURE_COORD_ARRAY);
		if (hasColor) glDisableClientState2(GL_COLOR_ARRAY);
		if (hasNormal) glDisableClientState2(GL_NORMAL_ARRAY);
	}

	TextureId createTexture()
	{
		TextureId texture;
		glGenTextures(1, &texture);
		return texture;
	}

	void deleteTexture(TextureId texture)
	{
		glDeleteTextures(1, &texture);
	}

	void bindTexture(TextureId texture)
	{
		glBindTexture2(GL_TEXTURE_2D, texture);
	}

	void uploadTexture(TextureId texture, const TextureData& data, const TextureUploadOptions& options)
	{
		bindTexture(texture);

		if (options.mipmap) {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		if (options.blur) {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		if (options.clamp) {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		} else {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		switch (data.format)
		{
			case TEXF_COMPRESSED_PVRTC_4444:
			case TEXF_COMPRESSED_PVRTC_565:
			case TEXF_COMPRESSED_PVRTC_5551:
			{
#if defined(__APPLE__)
				int fmt = data.transparent ? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, fmt, data.w, data.h, 0, data.numBytes, data.data);
#endif
				break;
			}
			default:
			{
				const GLint mode = data.transparent ? GL_RGBA : GL_RGB;
				if (data.format == TEXF_UNCOMPRESSED_565) {
					glTexImage2D2(GL_TEXTURE_2D, 0, mode, data.w, data.h, 0, mode, GL_UNSIGNED_SHORT_5_6_5, data.data);
				} else if (data.format == TEXF_UNCOMPRESSED_4444) {
					glTexImage2D2(GL_TEXTURE_2D, 0, mode, data.w, data.h, 0, mode, GL_UNSIGNED_SHORT_4_4_4_4, data.data);
				} else if (data.format == TEXF_UNCOMPRESSED_5551) {
					glTexImage2D2(GL_TEXTURE_2D, 0, mode, data.w, data.h, 0, mode, GL_UNSIGNED_SHORT_5_5_5_1, data.data);
				} else {
					glTexImage2D2(GL_TEXTURE_2D, 0, mode, data.w, data.h, 0, mode, GL_UNSIGNED_BYTE, data.data);
				}
				break;
			}
		}
	}

	void updateTextureRegion(TextureId texture, int x, int y, int width, int height, const void* data)
	{
		bindTexture(texture);
		glTexSubImage2D2(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	void setProjectionMatrix(const float* matrix16) { (void)matrix16; }
	void setModelViewMatrix(const float* matrix16) { (void)matrix16; }
	void setViewport(int x, int y, int width, int height)
	{
		glViewport(x, y, width, height);
	}

	void setBlendEnabled(bool enabled)
	{
		if (enabled) glEnable2(GL_BLEND);
		else glDisable2(GL_BLEND);
	}

	void setDepthTestEnabled(bool enabled)
	{
		if (enabled) glEnable2(GL_DEPTH_TEST);
		else glDisable2(GL_DEPTH_TEST);
	}

	void setCullFaceEnabled(bool enabled)
	{
		if (enabled) glEnable2(GL_CULL_FACE);
		else glDisable2(GL_CULL_FACE);
	}
};
}

RendererBackend* createGLESRendererBackend()
{
	return new GLESRendererBackend();
}
