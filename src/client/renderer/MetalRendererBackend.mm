#if defined(__APPLE__) && defined(USE_METAL_RENDERER)

#include "RendererBackend.h"
#include "VertecDecl.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <TargetConditionals.h>

#include <algorithm>
#include <map>
#include <vector>
#include <cstring>

namespace
{
static const char* MetalShaderSource =
	"#include <metal_stdlib>\n"
	"using namespace metal;\n"
	"struct VertexIn { float3 position [[attribute(0)]]; float2 uv [[attribute(1)]]; float4 color [[attribute(2)]]; float3 normal [[attribute(3)]]; };\n"
	"struct Uniforms { float4x4 projection; float4x4 modelView; };\n"
	"struct VertexOut { float4 position [[position]]; float2 uv; float4 color; };\n"
	"vertex VertexOut mcpe_vertex(VertexIn in [[stage_in]], constant Uniforms& u [[buffer(1)]]) {\n"
	"    VertexOut out;\n"
	"    out.position = u.projection * u.modelView * float4(in.position, 1.0);\n"
	"    out.uv = in.uv;\n"
	"    out.color = in.color;\n"
	"    return out;\n"
	"}\n"
	"fragment float4 mcpe_fragment(VertexOut in [[stage_in]], texture2d<float> tex [[texture(0)]], sampler s [[sampler(0)]]) {\n"
	"    return tex.sample(s, in.uv) * in.color;\n"
	"}\n";

struct MatrixUniforms
{
	float projection[16];
	float modelView[16];
};

class MetalRendererBackend : public RendererBackend
{
public:
	MetalRendererBackend()
		: device(nil), queue(nil), layer(nil), commandBuffer(nil), encoder(nil),
		  pipeline(nil), samplerLinear(nil), samplerNearest(nil), whiteTexture(nil), currentTexture(0),
		  nextBufferId(1), nextTextureId(1), viewportWidth(0), viewportHeight(0)
	{
		identity(uniforms.projection);
		identity(uniforms.modelView);
		blendEnabled = true;
		depthTestEnabled = true;
		cullFaceEnabled = false;
	}

	RendererBackendType type() const { return RENDERER_BACKEND_METAL; }

	bool initialize(void* nativeLayer)
	{
		device = MTLCreateSystemDefaultDevice();
		if (!device)
			return false;

		queue = [device newCommandQueue];
		if (!queue)
			return false;

		if (nativeLayer) {
			layer = (CAMetalLayer*)nativeLayer;
			layer.device = device;
			layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		}

		NSError* error = nil;
		id<MTLLibrary> library = [device newLibraryWithSource:[NSString stringWithUTF8String:MetalShaderSource] options:nil error:&error];
		if (!library)
			return false;

		MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];
		vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
		vertexDescriptor.attributes[0].offset = 0;
		vertexDescriptor.attributes[0].bufferIndex = 0;
		vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
		vertexDescriptor.attributes[1].offset = 3 * sizeof(float);
		vertexDescriptor.attributes[1].bufferIndex = 0;
		vertexDescriptor.attributes[2].format = MTLVertexFormatUChar4Normalized;
		vertexDescriptor.attributes[2].offset = 5 * sizeof(float);
		vertexDescriptor.attributes[2].bufferIndex = 0;
		vertexDescriptor.attributes[3].format = MTLVertexFormatFloat3;
		vertexDescriptor.attributes[3].offset = 6 * sizeof(float);
		vertexDescriptor.attributes[3].bufferIndex = 0;
		vertexDescriptor.layouts[0].stride = sizeof(VertexDeclPTCN);

		MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];
		descriptor.vertexFunction = [library newFunctionWithName:@"mcpe_vertex"];
		descriptor.fragmentFunction = [library newFunctionWithName:@"mcpe_fragment"];
		descriptor.vertexDescriptor = vertexDescriptor;
		descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
		descriptor.colorAttachments[0].blendingEnabled = YES;
		descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
		descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
		descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
		descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
		pipeline = [device newRenderPipelineStateWithDescriptor:descriptor error:&error];
		if (!pipeline)
			return false;

		samplerLinear = createSampler(true, false);
		samplerNearest = createSampler(false, false);
		createWhiteTexture();
		return true;
	}

	void shutdown()
	{
		buffers.clear();
		textures.clear();
		currentTexture = 0;
		encoder = nil;
		commandBuffer = nil;
	}

	void beginFrame()
	{
		commandBuffer = [queue commandBuffer];
	}

	void endFrame()
	{
		if (encoder) {
			[encoder endEncoding];
			encoder = nil;
		}
	}

	void present()
	{
		if (!commandBuffer)
			return;
		[commandBuffer commit];
		commandBuffer = nil;
	}

	void createVertexBuffers(GLsizei count, GLuint* outBuffers)
	{
		for (GLsizei i = 0; i < count; ++i)
			outBuffers[i] = nextBufferId++;
	}

	void deleteVertexBuffer(GLuint buffer)
	{
		buffers.erase(buffer);
	}

	void updateVertexBuffer(GLuint buffer, const void* data, GLsizei bytes, bool dynamic)
	{
		(void)dynamic;
		if (!data || bytes <= 0)
			return;
		id<MTLBuffer> metalBuffer = [device newBufferWithBytes:data length:bytes options:MTLResourceStorageModeShared];
		buffers[buffer] = metalBuffer;
	}

	void drawVertexBuffer(GLuint buffer, int vertices, int vertexSize, RendererVertexFormat format, unsigned int mode)
	{
		(void)vertexSize;
		ensureEncoder();
		if (!encoder)
			return;

		std::map<GLuint, id<MTLBuffer> >::iterator it = buffers.find(buffer);
		if (it == buffers.end())
			return;

		[encoder setRenderPipelineState:pipeline];
		[encoder setCullMode:cullFaceEnabled ? MTLCullModeBack : MTLCullModeNone];
		[encoder setVertexBuffer:it->second offset:0 atIndex:0];
		[encoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:1];

		const bool hasTexture = format == RENDERER_VERTEX_FORMAT_VT ||
			format == RENDERER_VERTEX_FORMAT_VTC ||
			format == RENDERER_VERTEX_FORMAT_VTN ||
			format == RENDERER_VERTEX_FORMAT_VTCN;
		if (hasTexture) {
			std::map<TextureId, id<MTLTexture> >::iterator texIt = textures.find(currentTexture);
			[encoder setFragmentTexture:(texIt != textures.end() ? texIt->second : whiteTexture) atIndex:0];
		} else {
			[encoder setFragmentTexture:whiteTexture atIndex:0];
		}
		[encoder setFragmentSamplerState:samplerNearest atIndex:0];

		MTLPrimitiveType primitive = MTLPrimitiveTypeTriangle;
		if (mode == GL_LINES)
			primitive = MTLPrimitiveTypeLine;
		else if (mode == GL_TRIANGLE_STRIP)
			primitive = MTLPrimitiveTypeTriangleStrip;
		else if (mode == GL_TRIANGLE_FAN)
			primitive = MTLPrimitiveTypeTriangle;

		[encoder drawPrimitives:primitive vertexStart:0 vertexCount:vertices];
	}

	TextureId createTexture()
	{
		return nextTextureId++;
	}

	void deleteTexture(TextureId texture)
	{
		textures.erase(texture);
	}

	void bindTexture(TextureId texture)
	{
		currentTexture = texture;
	}

	void uploadTexture(TextureId texture, const TextureData& data, const TextureUploadOptions& options)
	{
		if (!data.data || data.w <= 0 || data.h <= 0)
			return;

		std::vector<unsigned char> converted;
		const void* uploadData = data.data;
		NSUInteger bytesPerRow = 0;
		MTLPixelFormat pixelFormat = metalPixelFormat(data);
		if (!prepareTextureUpload(data, converted, uploadData, bytesPerRow, pixelFormat))
			return;

		MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat width:data.w height:data.h mipmapped:options.mipmap];
		descriptor.usage = MTLTextureUsageShaderRead;
		id<MTLTexture> metalTexture = [device newTextureWithDescriptor:descriptor];
		if (!metalTexture)
			return;

		MTLRegion region = MTLRegionMake2D(0, 0, data.w, data.h);
		[metalTexture replaceRegion:region mipmapLevel:0 withBytes:uploadData bytesPerRow:bytesPerRow];
		textures[texture] = metalTexture;

		if (options.blur || options.clamp) {
			if (options.blur)
				samplerLinear = createSampler(true, options.clamp);
			else
				samplerNearest = createSampler(false, options.clamp);
		}
	}

	void updateTextureRegion(TextureId texture, int x, int y, int width, int height, const void* data)
	{
		std::map<TextureId, id<MTLTexture> >::iterator it = textures.find(texture);
		if (it == textures.end() || !data)
			return;

		MTLRegion region = MTLRegionMake2D(x, y, width, height);
		[it->second replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:width * 4];
	}

	void setProjectionMatrix(const float* matrix16)
	{
		if (matrix16) std::memcpy(uniforms.projection, matrix16, sizeof(uniforms.projection));
	}

	void setModelViewMatrix(const float* matrix16)
	{
		if (matrix16) std::memcpy(uniforms.modelView, matrix16, sizeof(uniforms.modelView));
	}

	void setViewport(int x, int y, int width, int height)
	{
		(void)x;
		(void)y;
		viewportWidth = width;
		viewportHeight = height;
		if (layer)
			layer.drawableSize = CGSizeMake(width, height);
	}

	void setBlendEnabled(bool enabled)
	{
		blendEnabled = enabled;
	}

	void setDepthTestEnabled(bool enabled)
	{
		depthTestEnabled = enabled;
	}

	void setCullFaceEnabled(bool enabled)
	{
		cullFaceEnabled = enabled;
		if (encoder)
			[encoder setCullMode:enabled ? MTLCullModeBack : MTLCullModeNone];
	}

private:
	void ensureEncoder()
	{
		if (encoder)
			return;
		if (!commandBuffer)
			beginFrame();
		if (!layer)
			return;

		id<CAMetalDrawable> drawable = [layer nextDrawable];
		if (!drawable)
			return;

		MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
		pass.colorAttachments[0].texture = drawable.texture;
		pass.colorAttachments[0].loadAction = MTLLoadActionLoad;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		encoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
		[commandBuffer presentDrawable:drawable];

		if (viewportWidth > 0 && viewportHeight > 0) {
			MTLViewport viewport = { 0.0, 0.0, (double)viewportWidth, (double)viewportHeight, 0.0, 1.0 };
			[encoder setViewport:viewport];
		}
	}

	id<MTLSamplerState> createSampler(bool linear, bool clamp)
	{
		MTLSamplerDescriptor* descriptor = [[MTLSamplerDescriptor alloc] init];
		descriptor.minFilter = linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
		descriptor.magFilter = linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
		descriptor.sAddressMode = clamp ? MTLSamplerAddressModeClampToEdge : MTLSamplerAddressModeRepeat;
		descriptor.tAddressMode = clamp ? MTLSamplerAddressModeClampToEdge : MTLSamplerAddressModeRepeat;
		return [device newSamplerStateWithDescriptor:descriptor];
	}

	static void identity(float* out)
	{
		for (int i = 0; i < 16; ++i)
			out[i] = 0.0f;
		out[0] = out[5] = out[10] = out[15] = 1.0f;
	}

	static bool isPVRTC(TextureFormat format)
	{
		return format == TEXF_COMPRESSED_PVRTC_565 ||
			format == TEXF_COMPRESSED_PVRTC_5551 ||
			format == TEXF_COMPRESSED_PVRTC_4444;
	}

	void createWhiteTexture()
	{
		unsigned char white[4] = { 255, 255, 255, 255 };
		MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:1 height:1 mipmapped:NO];
		descriptor.usage = MTLTextureUsageShaderRead;
		whiteTexture = [device newTextureWithDescriptor:descriptor];
		[whiteTexture replaceRegion:MTLRegionMake2D(0, 0, 1, 1) mipmapLevel:0 withBytes:white bytesPerRow:4];
	}

	static MTLPixelFormat metalPixelFormat(const TextureData& data)
	{
		switch (data.format)
		{
			case TEXF_COMPRESSED_PVRTC_565:
			case TEXF_COMPRESSED_PVRTC_5551:
			case TEXF_COMPRESSED_PVRTC_4444:
#if TARGET_OS_IPHONE || TARGET_OS_TV
				return data.transparent ? MTLPixelFormatPVRTC_RGBA_4BPP : MTLPixelFormatPVRTC_RGB_4BPP;
#else
				return MTLPixelFormatRGBA8Unorm;
#endif
			default:
				return MTLPixelFormatRGBA8Unorm;
		}
	}

	static bool prepareTextureUpload(const TextureData& data, std::vector<unsigned char>& converted, const void*& uploadData, NSUInteger& bytesPerRow, MTLPixelFormat& pixelFormat)
	{
		const int pixels = data.w * data.h;
		if (pixels <= 0)
			return false;

		if (isPVRTC(data.format)) {
#if TARGET_OS_IPHONE || TARGET_OS_TV
			bytesPerRow = std::max(32, data.w * 2);
			uploadData = data.data;
			return true;
#else
			return false;
#endif
		}

		pixelFormat = MTLPixelFormatRGBA8Unorm;
		bytesPerRow = data.w * 4;

		if (data.format == TEXF_UNCOMPRESSED_8888 && data.transparent) {
			uploadData = data.data;
			return true;
		}

		converted.resize(pixels * 4);
		if (data.format == TEXF_UNCOMPRESSED_8888) {
			for (int i = 0; i < pixels; ++i) {
				converted[i * 4 + 0] = data.data[i * 3 + 0];
				converted[i * 4 + 1] = data.data[i * 3 + 1];
				converted[i * 4 + 2] = data.data[i * 3 + 2];
				converted[i * 4 + 3] = 255;
			}
		} else if (data.format == TEXF_UNCOMPRESSED_565) {
			const unsigned short* src = (const unsigned short*)data.data;
			for (int i = 0; i < pixels; ++i) {
				unsigned short p = src[i];
				converted[i * 4 + 0] = (unsigned char)(((p >> 11) & 0x1f) * 255 / 31);
				converted[i * 4 + 1] = (unsigned char)(((p >> 5) & 0x3f) * 255 / 63);
				converted[i * 4 + 2] = (unsigned char)((p & 0x1f) * 255 / 31);
				converted[i * 4 + 3] = 255;
			}
		} else if (data.format == TEXF_UNCOMPRESSED_4444) {
			const unsigned short* src = (const unsigned short*)data.data;
			for (int i = 0; i < pixels; ++i) {
				unsigned short p = src[i];
				converted[i * 4 + 0] = (unsigned char)(((p >> 12) & 0x0f) * 17);
				converted[i * 4 + 1] = (unsigned char)(((p >> 8) & 0x0f) * 17);
				converted[i * 4 + 2] = (unsigned char)(((p >> 4) & 0x0f) * 17);
				converted[i * 4 + 3] = (unsigned char)((p & 0x0f) * 17);
			}
		} else if (data.format == TEXF_UNCOMPRESSED_5551) {
			const unsigned short* src = (const unsigned short*)data.data;
			for (int i = 0; i < pixels; ++i) {
				unsigned short p = src[i];
				converted[i * 4 + 0] = (unsigned char)(((p >> 11) & 0x1f) * 255 / 31);
				converted[i * 4 + 1] = (unsigned char)(((p >> 6) & 0x1f) * 255 / 31);
				converted[i * 4 + 2] = (unsigned char)(((p >> 1) & 0x1f) * 255 / 31);
				converted[i * 4 + 3] = (p & 0x01) ? 255 : 0;
			}
		} else {
			return false;
		}

		uploadData = converted.empty() ? data.data : &converted[0];
		return true;
	}

	id<MTLDevice> device;
	id<MTLCommandQueue> queue;
	CAMetalLayer* layer;
	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> encoder;
	id<MTLRenderPipelineState> pipeline;
	id<MTLSamplerState> samplerLinear;
	id<MTLSamplerState> samplerNearest;
	id<MTLTexture> whiteTexture;
	TextureId currentTexture;
	GLuint nextBufferId;
	TextureId nextTextureId;
	int viewportWidth;
	int viewportHeight;
	bool blendEnabled;
	bool depthTestEnabled;
	bool cullFaceEnabled;
	MatrixUniforms uniforms;
	std::map<GLuint, id<MTLBuffer> > buffers;
	std::map<TextureId, id<MTLTexture> > textures;
};
}

RendererBackend* createMetalRendererBackend()
{
	return new MetalRendererBackend();
}

#endif
