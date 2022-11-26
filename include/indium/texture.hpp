#pragma once

#include <indium/base.hpp>
#include <indium/types.hpp>
#include <indium/resource.hpp>

namespace Indium {
	class Device;
	class Buffer;

	struct TextureDescriptor {
		TextureType textureType = TextureType::e2D;
		PixelFormat pixelFormat = PixelFormat::RGBA8Unorm;
		size_t width = 1;
		size_t height = 1;
		size_t depth = 1;
		size_t mipmapLevelCount = 1;
		size_t sampleCount = 1;
		size_t arrayLength = 1;
		ResourceOptions resourceOptions = ResourceOptions::CPUCacheModeDefaultCache | ResourceOptions::StorageModeManaged | ResourceOptions::HazardTrackingModeDefault;
		bool allowGPUOptimizedContents = true;
		TextureUsage usage = TextureUsage::ShaderRead;
		TextureSwizzleChannels swizzle;
	};

	class Texture: public Resource {
	public:
		virtual ~Texture() = 0;

		// derived classes are required to implement the following accessors properly
		virtual TextureType textureType() const = 0;
		virtual PixelFormat pixelFormat() const = 0;
		virtual size_t width() const = 0;
		virtual size_t height() const = 0;
		virtual size_t depth() const = 0;
		virtual size_t mipmapLevelCount() const = 0;
		virtual size_t arrayLength() const = 0;
		virtual size_t sampleCount() const = 0;
		virtual bool framebufferOnly() const = 0;
		//virtual TextureUsage usage() const = 0;
		virtual bool allowGPUOptimizedContents() const = 0;
		virtual bool shareable() const = 0;
		virtual TextureSwizzleChannels swizzle() const = 0;

		// derived classes can optionally implement these
		//
		// the default implementations return `nullptr` and `0` (as appropriate)
		virtual std::shared_ptr<Texture> parentTexture() const;
		virtual size_t parentRelativeLevel() const;
		virtual size_t parentRelativeSlice() const;
		virtual std::shared_ptr<Buffer> buffer() const;
		virtual size_t bufferOffset() const;
		virtual size_t bufferBytesPerRow() const;

		virtual std::shared_ptr<Texture> newTextureView(PixelFormat pixelFormat) = 0;
		virtual std::shared_ptr<Texture> newTextureView(PixelFormat pixelFormat, TextureType textureType, const Range<uint32_t>& levels, const Range<uint32_t>& layers) = 0;
		virtual std::shared_ptr<Texture> newTextureView(PixelFormat pixelFormat, TextureType textureType, const Range<uint32_t>& levels, const Range<uint32_t>& layers, const TextureSwizzleChannels& swizzle) = 0;

		virtual void replaceRegion(Region region, size_t mipmapLevel, const void* bytes, size_t bytesPerRow) = 0;
		virtual void replaceRegion(Region region, size_t mipmapLevel, size_t slice, const void* bytes, size_t bytesPerRow, size_t bytesPerImage) = 0;
	};
};
