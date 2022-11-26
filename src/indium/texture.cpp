#include <indium/texture.private.hpp>
#include <indium/device.private.hpp>
#include <indium/types.private.hpp>

#include <cstring>

Indium::Texture::~Texture() {};
Indium::PrivateTexture::~PrivateTexture() {};

std::shared_ptr<Indium::Texture> Indium::Texture::parentTexture() const {
	return nullptr;
};

size_t Indium::Texture::parentRelativeLevel() const {
	return 0;
};

size_t Indium::Texture::parentRelativeSlice() const {
	return 0;
};

std::shared_ptr<Indium::Buffer> Indium::Texture::buffer() const {
	return nullptr;
};

size_t Indium::Texture::bufferOffset() const {
	return 0;
};

size_t Indium::Texture::bufferBytesPerRow() const {
	return 0;
};

std::shared_ptr<Indium::Texture> Indium::PrivateTexture::newTextureView(PixelFormat pixelFormat) {
	return newTextureView(
		pixelFormat,
		textureType(),
		Range<uint32_t> { static_cast<uint32_t>(parentRelativeLevel()), static_cast<uint32_t>(mipmapLevelCount()) },
		Range<uint32_t> { static_cast<uint32_t>(parentRelativeSlice()), static_cast<uint32_t>(arrayLength()) }
	);
};

std::shared_ptr<Indium::Texture> Indium::PrivateTexture::newTextureView(PixelFormat pixelFormat, TextureType textureType, const Range<uint32_t>& levels, const Range<uint32_t>& layers) {
	return newTextureView(
		pixelFormat,
		textureType,
		levels,
		layers,
		swizzle()
	);
};

std::shared_ptr<Indium::Texture> Indium::PrivateTexture::newTextureView(PixelFormat pixelFormat, TextureType textureType, const Range<uint32_t>& levels, const Range<uint32_t>& layers, const TextureSwizzleChannels& swizzle) {
	return std::make_shared<Indium::TextureView>(
		shared_from_this(),
		pixelFormat,
		textureType,
		VK_IMAGE_ASPECT_COLOR_BIT, // TODO
		swizzle,
		levels,
		layers
	);
};

Indium::TextureView::TextureView(std::shared_ptr<PrivateTexture> original, PixelFormat pixelFormat, TextureType textureType, VkImageAspectFlags imageAspect, TextureSwizzleChannels swizzle, const Range<uint32_t>& levels, const Range<uint32_t>& layers):
	PrivateTexture(std::dynamic_pointer_cast<PrivateDevice>(original->device())),
	_original(original),
	_pixelFormat(pixelFormat),
	_textureType(textureType),
	_imageAspect(imageAspect),
	_levels(levels),
	_layers(layers),
	_swizzle(swizzle)
{
	auto parentLevelStart = _original->parentRelativeLevel();
	auto parentLevelEnd = parentLevelStart + _original->mipmapLevelCount() - 1;
	auto levelStart = std::min(parentLevelStart + _levels.start, parentLevelEnd);
	auto levelEnd = std::min(levelStart + _levels.length - 1, parentLevelEnd);

	auto parentLayerStart = _original->parentRelativeSlice();
	auto parentLayerEnd = parentLayerStart + _original->arrayLength() - 1;
	auto layerStart = std::min(parentLayerStart + _layers.start, parentLayerEnd);
	auto layerEnd = std::min(layerStart + _layers.length - 1, parentLayerEnd);

	VkImageViewCreateInfo info {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.image = _original->image();
	info.viewType = textureTypeToVkImageViewType(_textureType);
	info.format = pixelFormatToVkFormat(_pixelFormat);
	info.components.r = textureSwizzleToVkComponentSwizzle(_swizzle.red);
	info.components.g = textureSwizzleToVkComponentSwizzle(_swizzle.green);
	info.components.b = textureSwizzleToVkComponentSwizzle(_swizzle.blue);
	info.components.a = textureSwizzleToVkComponentSwizzle(_swizzle.alpha);
	info.subresourceRange.aspectMask = _imageAspect;
	info.subresourceRange.baseMipLevel = levelStart;
	info.subresourceRange.levelCount = levelEnd - levelStart + 1;
	info.subresourceRange.baseArrayLayer = layerStart;
	info.subresourceRange.layerCount = layerEnd - layerStart + 1;

	if (vkCreateImageView(std::dynamic_pointer_cast<PrivateDevice>(_original->device())->device(), &info, nullptr, &_imageView) != VK_SUCCESS) {
		// TODO
		abort();
	}
};

Indium::TextureView::~TextureView() {
	vkDestroyImageView(std::dynamic_pointer_cast<PrivateDevice>(_original->device())->device(), _imageView, nullptr);
};

Indium::PixelFormat Indium::TextureView::pixelFormat() const {
	return _pixelFormat;
};

Indium::TextureType Indium::TextureView::textureType() const {
	return _textureType;
};

size_t Indium::TextureView::width() const {
	return _original->width();
};

size_t Indium::TextureView::height() const {
	return _original->height();
};

size_t Indium::TextureView::depth() const {
	return _original->depth();
};

size_t Indium::TextureView::mipmapLevelCount() const {
	return _levels.length;
};

size_t Indium::TextureView::arrayLength() const {
	return _layers.length;
};

size_t Indium::TextureView::sampleCount() const {
	return _original->sampleCount();
};

bool Indium::TextureView::framebufferOnly() const {
	return _original->framebufferOnly();
};

bool Indium::TextureView::allowGPUOptimizedContents() const {
	return _original->allowGPUOptimizedContents();
};

bool Indium::TextureView::shareable() const {
	return _original->shareable();
};

Indium::TextureSwizzleChannels Indium::TextureView::swizzle() const {
	return _swizzle;
};

std::shared_ptr<Indium::Texture> Indium::TextureView::parentTexture() const {
	return _original;
};

size_t Indium::TextureView::parentRelativeLevel() const {
	return _levels.start;
};

size_t Indium::TextureView::parentRelativeSlice() const {
	return _layers.start;
};

VkImageView Indium::TextureView::imageView() {
	return _imageView;
};

VkImage Indium::TextureView::image() {
	return _original->image();
};

std::shared_ptr<Indium::Device> Indium::TextureView::device() {
	return _original->device();
};

VkImageLayout Indium::TextureView::imageLayout() {
	return _original->imageLayout();
};

const Indium::TimelineSemaphore& Indium::TextureView::acquire(uint64_t& waitValue, std::shared_ptr<BinarySemaphore>& extraWaitSemaphore, uint64_t& signalValue) {
	return _original->acquire(waitValue, extraWaitSemaphore, signalValue);
};

void Indium::TextureView::beginUpdatingPresentationSemaphore(std::shared_ptr<BinarySemaphore> presentationSemaphore) {
	return _original->beginUpdatingPresentationSemaphore(presentationSemaphore);
};

void Indium::TextureView::endUpdatingPresentationSemaphore() {
	return _original->endUpdatingPresentationSemaphore();
};

std::shared_ptr<Indium::BinarySemaphore> Indium::TextureView::synchronizePresentation() {
	return _original->synchronizePresentation();
};

void Indium::TextureView::replaceRegion(Indium::Region region, size_t mipmapLevel, const void* bytes, size_t bytesPerRow) {
	throw std::runtime_error("TODO: support replaceRegion on texture views");
};

void Indium::TextureView::replaceRegion(Indium::Region region, size_t mipmapLevel, size_t slice, const void* bytes, size_t bytesPerRow, size_t bytesPerImage) {
	throw std::runtime_error("TODO: support replaceRegion on texture views");
};

const Indium::TimelineSemaphore& Indium::PrivateTexture::acquire(uint64_t& waitValue, std::shared_ptr<BinarySemaphore>& extraWaitSemaphore, uint64_t& signalValue) {
	std::unique_lock lock(_syncMutex);

	if (_extraWaitSemaphore) {
		extraWaitSemaphore = std::move(_extraWaitSemaphore);
		_extraWaitSemaphore = nullptr;
	}

	waitValue = _syncSemaphore->count;

	++_syncSemaphore->count;
	signalValue = _syncSemaphore->count;

	return *_syncSemaphore;
};

void Indium::PrivateTexture::beginUpdatingPresentationSemaphore(std::shared_ptr<BinarySemaphore> presentationSemaphore) {
	_presentationMutex.lock();
	_presentationSemaphore = presentationSemaphore;
};

void Indium::PrivateTexture::endUpdatingPresentationSemaphore() {
	_presentationMutex.unlock();
};

std::shared_ptr<Indium::BinarySemaphore> Indium::PrivateTexture::synchronizePresentation() {
	std::unique_lock lock(_presentationMutex);

	if (_presentationSemaphore) {
		auto sema = std::move(_presentationSemaphore);
		_presentationSemaphore = nullptr;
		return sema;
	}

	return nullptr;
};

Indium::PrivateTexture::PrivateTexture(std::shared_ptr<PrivateDevice> device):
	_device(device)
{
	_syncSemaphore = device->getWrappedTimelineSemaphore();
};

std::shared_ptr<Indium::Device> Indium::PrivateTexture::device() {
	return _device;
};

//
// concrete texture
//

Indium::ConcreteTexture::ConcreteTexture(std::shared_ptr<PrivateDevice> device, const TextureDescriptor& descriptor):
	PrivateTexture(device),
	_descriptor(descriptor)
{
	_storageMode = static_cast<StorageMode>((static_cast<size_t>(_descriptor.resourceOptions) >> 4) & 0xf);

	VkImageCreateInfo info {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.imageType = textureTypeToVkImageType(_descriptor.textureType);
	info.format = pixelFormatToVkFormat(_descriptor.pixelFormat);
	info.extent.width = _descriptor.width;
	info.extent.height = _descriptor.height;
	info.extent.depth = _descriptor.depth;
	info.mipLevels = _descriptor.mipmapLevelCount;
	info.arrayLayers = _descriptor.arrayLength;
	// TODO
	//info.samples = _descriptor.sampleCount;
	info.samples = VK_SAMPLE_COUNT_1_BIT;

	// TODO: allow GPU-optimized tiling
	//
	// we can't do this right now because `replaceRegion` does a direct host copy (and it's not supposed to use a command to do the copy)
	info.tiling = /*descriptor.allowGPUOptimizedContents ? VK_IMAGE_TILING_OPTIMAL :*/ VK_IMAGE_TILING_LINEAR;

	// we don't know ahead of time how the image is going to be used, so specify everything we support
	info.usage =
		VK_IMAGE_USAGE_SAMPLED_BIT |
		VK_IMAGE_USAGE_STORAGE_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		//VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		0
		;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: check if this should be "concurrent" instead. i think we're good, though.
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (vkCreateImage(_device->device(), &info, nullptr, &_image) != VK_SUCCESS) {
		// TODO
		abort();
	}

	VkMemoryRequirements reqs;

	vkGetImageMemoryRequirements(_device->device(), _image, &reqs);

	size_t targetIndex = SIZE_MAX;

	for (size_t i = 0; i < _device->memoryProperties().memoryTypeCount; ++i) {
		const auto& type = _device->memoryProperties().memoryTypes[i];

		if ((reqs.memoryTypeBits & (1 << i)) == 0) {
			continue;
		}

		if ((_storageMode == StorageMode::Managed || _storageMode == StorageMode::Shared) && (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
			continue;
		}

		if (_storageMode == StorageMode::Shared && (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
			continue;
		}

		// okay, this is good enough
		targetIndex = i;
		break;
	}

	if (targetIndex == SIZE_MAX) {
		throw std::runtime_error("No suitable memory region found for image with requested storage mode");
	}

	VkMemoryAllocateInfo allocateInfo {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = reqs.size;
	allocateInfo.memoryTypeIndex = targetIndex;

	if (vkAllocateMemory(_device->device(), &allocateInfo, nullptr, &_memory) != VK_SUCCESS) {
		// TODO
		abort();
	}

	vkBindImageMemory(_device->device(), _image, _memory, 0);

	// transition the image into the general layout
	VkCommandBufferAllocateInfo cmdBufAllocInfo {};
	cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocInfo.commandPool = _device->oneshotCommandPool();
	cmdBufAllocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuf;
	if (vkAllocateCommandBuffers(_device->device(), &cmdBufAllocInfo, &cmdBuf) != VK_SUCCESS) {
		// TODO
		abort();
	}

	VkCommandBufferBeginInfo cmdBufBeginInfo {};
	cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuf, &cmdBufBeginInfo);

	VkImageMemoryBarrier barrier {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_NONE;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = _image;
	// TODO: correctly determine which image aspect bits to use
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = _descriptor.mipmapLevelCount;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = _descriptor.arrayLength;

	vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_NONE, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	vkEndCommandBuffer(cmdBuf);

	VkFenceCreateInfo fenceCreateInfo {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkFence theFence = nullptr;

	if (vkCreateFence(_device->device(), &fenceCreateInfo, nullptr, &theFence) != VK_SUCCESS) {
		// TODO
		abort();
	}

	VkSubmitInfo submitInfo {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuf;

	vkQueueSubmit(_device->graphicsQueue(), 1, &submitInfo, theFence);
	if (vkWaitForFences(_device->device(), 1, &theFence, VK_TRUE, /* 1s */ 1ull * 1000 * 1000 * 1000) != VK_SUCCESS) {
		// TODO
		abort();
	}

	vkDestroyFence(_device->device(), theFence, nullptr);

	vkFreeCommandBuffers(_device->device(), _device->oneshotCommandPool(), 1, &cmdBuf);

	VkImageViewCreateInfo info2 {};
	info2.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info2.image = _image;
	info2.viewType = textureTypeToVkImageViewType(_descriptor.textureType);
	info2.format = pixelFormatToVkFormat(_descriptor.pixelFormat);
	info2.components.r = textureSwizzleToVkComponentSwizzle(_descriptor.swizzle.red);
	info2.components.g = textureSwizzleToVkComponentSwizzle(_descriptor.swizzle.green);
	info2.components.b = textureSwizzleToVkComponentSwizzle(_descriptor.swizzle.blue);
	info2.components.a = textureSwizzleToVkComponentSwizzle(_descriptor.swizzle.alpha);
	info2.subresourceRange = barrier.subresourceRange;

	if (vkCreateImageView(_device->device(), &info2, nullptr, &_imageView) != VK_SUCCESS) {
		// TODO
		abort();
	}
};

Indium::ConcreteTexture::~ConcreteTexture() {
	vkDestroyImageView(_device->device(), _imageView, nullptr);
	vkDestroyImage(_device->device(), _image, nullptr);
	vkFreeMemory(_device->device(), _memory, nullptr);
};

Indium::TextureType Indium::ConcreteTexture::textureType() const {
	return _descriptor.textureType;
};

Indium::PixelFormat Indium::ConcreteTexture::pixelFormat() const {
	return _descriptor.pixelFormat;
};

size_t Indium::ConcreteTexture::width() const {
	return _descriptor.width;
};

size_t Indium::ConcreteTexture::height() const {
	return _descriptor.height;
};

size_t Indium::ConcreteTexture::depth() const {
	return _descriptor.depth;
};

size_t Indium::ConcreteTexture::mipmapLevelCount() const {
	return _descriptor.mipmapLevelCount;
};

size_t Indium::ConcreteTexture::arrayLength() const {
	return _descriptor.arrayLength;
};

size_t Indium::ConcreteTexture::sampleCount() const {
	return _descriptor.sampleCount;
};

bool Indium::ConcreteTexture::framebufferOnly() const {
	return false;
};

bool Indium::ConcreteTexture::allowGPUOptimizedContents() const {
	return _descriptor.allowGPUOptimizedContents;
};

bool Indium::ConcreteTexture::shareable() const {
	return false;
};

Indium::TextureSwizzleChannels Indium::ConcreteTexture::swizzle() const {
	return _descriptor.swizzle;
};

VkImageView Indium::ConcreteTexture::imageView() {
	return _imageView;
};

VkImage Indium::ConcreteTexture::image() {
	return _image;
};

VkImageLayout Indium::ConcreteTexture::imageLayout() {
	return VK_IMAGE_LAYOUT_GENERAL;
};

void Indium::ConcreteTexture::replaceRegion(Indium::Region region, size_t mipmapLevel, const void* bytes, size_t bytesPerRow) {
	return replaceRegion(region, mipmapLevel, 0, bytes, bytesPerRow, 0);
};

void Indium::ConcreteTexture::replaceRegion(Indium::Region region, size_t mipmapLevel, size_t slice, const void* bytes, size_t bytesPerRow, size_t bytesPerImage) {
	if (_storageMode != StorageMode::Managed && _storageMode != StorageMode::Shared) {
		throw std::runtime_error("Invalid usage of replaceRegion on non-managed and non-shared texture");
	}

	void* mapped = NULL;

	if (vkMapMemory(_device->device(), _memory, 0, VK_WHOLE_SIZE, 0, &mapped) != VK_SUCCESS) {
		// TODO
		abort();
	}

	if (slice != 0) {
		throw std::runtime_error("TODO: support texture slices");
	}

	if (pixelFormatIsCompressed(_descriptor.pixelFormat)) {
		throw std::runtime_error("TODO: support compressed textures");
	}

	if (_descriptor.textureType != TextureType::e1D && _descriptor.textureType != TextureType::e2D && _descriptor.textureType != TextureType::e3D) {
		throw std::runtime_error("TODO: support other texture formats");
	}

	// FIXME: not sure how the pixels are laid out in the input buffer (`bytes`) in cases starting at a non-zero origin
	//        and i haven't been able to find any examples of such a case, either.

	size_t bytesPerPixel = pixelFormatToByteCount(_descriptor.pixelFormat);
	size_t bytesPerBufferRow = (bytesPerPixel * region.size.width) + (bytesPerRow - (bytesPerPixel * region.size.width));
	size_t bytesPerBufferImage = (bytesPerBufferRow * region.size.height) + (bytesPerImage - (bytesPerBufferRow * region.size.height));

	for (size_t zOffset = 0; zOffset < region.size.depth; ++zOffset) {
		size_t z = region.origin.z + zOffset;

		// as an optimization, we can copy in bigger chunks if we're using the entire row
		if (region.origin.x == 0 && region.size.width == _descriptor.width) {
			size_t startOffset = (z * bytesPerImage) + (region.origin.y * bytesPerRow);
			size_t bytesOffset = (zOffset * bytesPerBufferImage);
			size_t copyLength = region.size.height * bytesPerRow;

			std::memcpy(static_cast<char*>(mapped) + startOffset, static_cast<const char*>(bytes) + bytesOffset, copyLength);
		} else {
			for (size_t yOffset = 0; yOffset < region.size.height; ++yOffset) {
				size_t y = region.origin.y + yOffset;

				size_t startOffset = (z * bytesPerImage) + (y * bytesPerRow) + (region.origin.x * bytesPerPixel);
				size_t bytesOffset = (zOffset * bytesPerBufferImage) + (yOffset * bytesPerBufferRow);
				size_t copyLength = region.size.width * bytesPerPixel;

				std::memcpy(static_cast<char*>(mapped) + startOffset, static_cast<const char*>(bytes) + bytesOffset, copyLength);
			}
		}
	}

	VkMappedMemoryRange vulkanRange {};
	vulkanRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	vulkanRange.memory = _memory;
	vulkanRange.size = VK_WHOLE_SIZE;
	vulkanRange.offset = 0;
	if (vkFlushMappedMemoryRanges(_device->device(), 1, &vulkanRange) != VK_SUCCESS) {
		// TODO
		abort();
	}

	vkUnmapMemory(_device->device(), _memory);
};
