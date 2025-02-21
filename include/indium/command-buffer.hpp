#pragma once

#include <memory>
#include <functional>

#include <indium/render-command-encoder.hpp>
#include <indium/blit-command-encoder.hpp>
#include <indium/compute-command-encoder.hpp>
#include <indium/base.hpp>

namespace Indium {
	class CommandQueue;
	class Device;
	class Drawable;

	class CommandBuffer {
	public:
		virtual ~CommandBuffer() = 0;

		virtual std::shared_ptr<RenderCommandEncoder> renderCommandEncoder(const RenderPassDescriptor& descriptor) = 0;
		virtual std::shared_ptr<BlitCommandEncoder> blitCommandEncoder() = 0;
		virtual std::shared_ptr<BlitCommandEncoder> blitCommandEncoder(const BlitPassDescriptor& descriptor) = 0;
		virtual std::shared_ptr<ComputeCommandEncoder> computeCommandEncoder() = 0;
		virtual std::shared_ptr<ComputeCommandEncoder> computeCommandEncoder(const ComputePassDescriptor& descriptor) = 0;
		virtual std::shared_ptr<ComputeCommandEncoder> computeCommandEncoder(DispatchType dispatchType) = 0;

		virtual void commit() = 0;
		virtual void presentDrawable(std::shared_ptr<Drawable> drawable) = 0;

		using Handler = std::function<void(std::shared_ptr<CommandBuffer>)>;
		virtual void addScheduledHandler(Handler handler) = 0;
		virtual void addCompletedHandler(Handler handler) = 0;
		virtual void waitUntilCompleted() = 0;

		virtual std::shared_ptr<CommandQueue> commandQueue() = 0;
		virtual std::shared_ptr<Device> device() = 0;
	};
};
