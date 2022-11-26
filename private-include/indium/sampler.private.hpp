#pragma once

#include <vulkan/vulkan.h>

#include <indium/sampler.hpp>

namespace Indium {
	class PrivateDevice;

	class PrivateSamplerState: public SamplerState {
	protected:
		std::shared_ptr<PrivateDevice> _privateDevice = nullptr;

	public:
		explicit PrivateSamplerState(std::shared_ptr<PrivateDevice> device, const SamplerDescriptor& descriptor);
		virtual ~PrivateSamplerState();

		virtual std::shared_ptr<Indium::Device> device() override;

		INDIUM_PROPERTY(VkSampler, s, S,ampler) = nullptr;
	};
};
