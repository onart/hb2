#pragma once
#include <functional>
#include <d3d11.h>
#include <cstdint>

namespace ctx {
	extern ID3D11Device* const& device;
	extern ID3D11DeviceContext* const& context;
	extern const HWND& hwnd;

	bool init();
	void setUpdate(int64_t key, std::function<void()>);
	void update();
	void finalize();

}
