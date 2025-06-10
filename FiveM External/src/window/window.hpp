#pragma once
#include <d3d11.h>
#include <dxgi.h>

#include "../../ext/ImGui/imgui.h"
#include "../../ext/ImGui/imgui_impl_dx11.h"
#include "../../ext/ImGui/imgui_impl_win32.h"

class Overlay {
private:
	// creation of device, window, and ImGui.
	bool CreateDevice();
	void CreateOverlay(const char* window_name);
	bool CreateImGui();

	// destruction of device, window and ImGui.
	void DestroyDevice();
	void DestroyOverlay();
	void DestroyImGui();

	// returns 60.f if it fails.
	float GetRefreshRate();
public:
	bool shouldRun = true;
	bool drawESP = true;
	void SetupOverlay(const char* window_name) {
		CreateOverlay(window_name);
		CreateDevice();
		CreateImGui();
	}

	~Overlay() {
		DestroyDevice();
		DestroyOverlay();
		DestroyImGui();
	}

	bool RenderMenu;
	void StartRender();
	void EndRender();

	void Render();

	bool IsWindowInForeground(HWND window) { return GetForegroundWindow() == window; }
	bool BringToForeground(HWND window) { return SetForegroundWindow(window); }

	void SetForeground(HWND window);
private:
	HWND overlay;
	WNDCLASSEX wc;

	ID3D11Device* device;
	ID3D11DeviceContext* device_context;
	IDXGISwapChain* swap_chain;
	ID3D11RenderTargetView* render_targetview;
};