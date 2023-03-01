#pragma once

#include <indium/device.hpp>

namespace Indium {
	class Texture;

	class INDIUM_API Drawable {
	public:
		virtual ~Drawable() = 0;

		virtual void present() = 0;
	};
};
