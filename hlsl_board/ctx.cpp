#include "ctx.h"
#include <map>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace ctx {
	ID3D11Device* dev;
	ID3D11DeviceContext* cont;
	IDXGISwapChain* sc;
	HWND hw;
	IDXGISwapChain* const& swapchain = sc;
	ID3D11Device* const& device = dev;
	ID3D11DeviceContext* const& context = cont;
	const HWND& hwnd = hw;

	static std::map<int64_t, std::function<void()>> updates;

	bool init() {
		if (device) return true;
		const D3D_FEATURE_LEVEL fl[] = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL flOut = D3D_FEATURE_LEVEL_11_0;
		auto hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, fl, sizeof(fl) / sizeof(fl[0]), D3D11_SDK_VERSION, &dev, &flOut, &cont);
		if (FAILED(hr)) return false;

		CreateWindowW(L"STATIC", L"board", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, NULL, NULL, NULL, NULL);

		return true;
	}

	void finalize() {
		if (hwnd) { DestroyWindow(hwnd); hw = {}; }
		if (context) { cont->Release(); cont = {}; }
		if (device) { dev->Release(); dev = {}; }
	}

	void setUpdate(int64_t key, std::function<void()> f) {
		if (!f) updates.erase(key);
		else updates[key] = f;
	}

	void run() {
		for (auto& kv : updates) {
			kv.second();
		}
	}
}