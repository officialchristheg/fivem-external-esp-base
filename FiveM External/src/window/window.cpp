#include "window.hpp"

#include <dwmapi.h>
#include <stdio.h>
#include "../game/game.h"
#include <thread>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// set up ImGui window procedure handler
	if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
		return true;

	// switch that disables alt application and checks for if the user tries to close the window.
	switch (msg)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu (imgui uses it in their example :shrug:)
			return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CLOSE:
		return 0;
	}

	// define the window procedure
	return DefWindowProc(window, msg, wParam, lParam);
}

float Overlay::GetRefreshRate()
{
	// get dxgi variables
	IDXGIFactory* dxgiFactory = nullptr;
	IDXGIAdapter* dxgiAdapter = nullptr;
	IDXGIOutput* dxgiOutput = nullptr;
	DXGI_MODE_DESC modeDesc;

	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&dxgiFactory))))
		return 60;

	if (FAILED(dxgiFactory->EnumAdapters(0, &dxgiAdapter))) {
		dxgiAdapter->Release();
		return 60;
	}

	if (FAILED(dxgiAdapter->EnumOutputs(0, &dxgiOutput))) {
		dxgiAdapter->Release();
		dxgiFactory->Release();
		return 60;
	}

	UINT numModes = 0;
	if (FAILED(dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr))) {
		dxgiOutput->Release();
		dxgiAdapter->Release();
		dxgiFactory->Release();
		return 60;
	}

	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (FAILED(dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, displayModeList))) {
		delete[] displayModeList;
		dxgiOutput->Release();
		dxgiAdapter->Release();
		dxgiFactory->Release();
		return 60;
	}

	float refreshRate = 60;
	for (int i = 0; i < numModes; ++i) {
		float hz = static_cast<float>(displayModeList[i].RefreshRate.Numerator) /
			static_cast<float>(displayModeList[i].RefreshRate.Denominator);
		
		if (hz != 0 && hz >= refreshRate)
			refreshRate = hz;
	}

	delete[] displayModeList;
	dxgiOutput->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	printf("[>>] Refresh rate: %f", refreshRate);
	printf("\n"); 

	return refreshRate;
}

bool Overlay::CreateDevice()
{
	// First we setup our swap chain, this basically just holds a bunch of descriptors for the swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	// set number of back buffers (this is double buffering)
	sd.BufferCount = 2;

	// width + height of buffer, (0 is automatic sizing)
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;

	// set the pixel format
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// get the fps from GetRefreshRate(). If anything fails it just returns 60 anyways.
	sd.BufferDesc.RefreshRate.Numerator = GetRefreshRate(); 
	sd.BufferDesc.RefreshRate.Denominator = 1;

	// allow mode switch (changing display modes)
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// set how the bbuffer will be used
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	sd.OutputWindow = overlay;

	// setup the multi-sampling
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// specify what Direct3D feature levels to use
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	
	// create device and swap chain
	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		featureLevelArray,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		&featureLevel,
		&device_context);

	// if the hardware isn't supported create with WARP (basically just a different renderer)
	if (result == DXGI_ERROR_UNSUPPORTED) {
		result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			0U,
			featureLevelArray,
			2, D3D11_SDK_VERSION,
			&sd,
			&swap_chain,
			&device,
			&featureLevel,
			&device_context);

		printf("[>>] DXGI_ERROR | Created with D3D_DRIVER_TYPE_WARP\n");
	}

	// can't do much more, if the hardware still isn't supported just return false.
	if (result != S_OK) {
		printf("[>>] Device Not Okay\n");
		return false;
	}

	// retrieve back_buffer, im defining it here since it isn't being used at any other point in time.
	ID3D11Texture2D* back_buffer{ nullptr };
	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

	// if back buffer is obtained then we can create render target view and release the back buffer again
	if (back_buffer) 
	{
		device->CreateRenderTargetView(back_buffer, nullptr, &render_targetview);
		back_buffer->Release();

		printf("[>>] Created Device\n");
		return true;
	}

	// if we reach this point then it failed to create the back buffer
	printf("[>>] Failed to create Device\n");
	return false;
}

void Overlay::DestroyDevice()
{
	// release everything that has to do with the device.
	if (device)
	{
		device->Release();
		device_context->Release();
		swap_chain->Release();
		render_targetview->Release();

		printf("[>>] Released Device\n");
	}
	else
		printf("[>>] Device Not Found when Exiting.\n");
}

void Overlay::CreateOverlay(const char* window_name)
{
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = GetModuleHandleA(0);
	wc.lpszClassName = "FiveM External";

	RegisterClassEx(&wc);

	overlay = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		wc.lpszClassName,
		window_name,
		WS_POPUP,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN), // 1920
		GetSystemMetrics(SM_CYSCREEN), // 1080
		NULL,
		NULL,
		wc.hInstance,
		NULL
	);

	if (overlay == NULL)
		printf("[>>] Failed to create Overlay\n");

	SetLayeredWindowAttributes(overlay, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	{
		RECT client_area{};
		RECT window_area{};

		GetClientRect(overlay, &client_area);
		GetWindowRect(overlay, &window_area);

		POINT diff{};
		ClientToScreen(overlay, &diff);

		const MARGINS margins{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};

		DwmExtendFrameIntoClientArea(overlay, &margins);
	}

	ShowWindow(overlay, SW_SHOW);
	UpdateWindow(overlay);

	printf("[>>] Overlay Created\n");
}

void Overlay::DestroyOverlay()
{
	DestroyWindow(overlay);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	style.WindowRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.TabRounding = 0.0f;
}

bool Overlay::CreateImGui()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	SetupImGuiStyle();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f);

	if (!ImGui_ImplWin32_Init(overlay)) {
		printf("[>>] Failed ImGui_ImplWin32_Init\n");
		return false;
	}
	
	if (!ImGui_ImplDX11_Init(device, device_context)) {
		printf("[>>] Failed ImGui_ImplDX11_Init\n");
		return false;
	}

	printf("[>>] ImGui Initialized\n");
	return true;
}

void Overlay::DestroyImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Overlay::StartRender()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		RenderMenu = !RenderMenu;

		if (RenderMenu) {
			SetWindowLong(overlay, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT);
		}
		else {
			SetWindowLong(overlay, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED);
		}
	}
}

void Overlay::EndRender()
{
	ImGui::Render();

	float color[4]{ 0, 0, 0, 0 };

	device_context->OMSetRenderTargets(1, &render_targetview, nullptr);
	device_context->ClearRenderTargetView(render_targetview, color);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	swap_chain->Present(1U, 0U);
}

void Overlay::Render()
{
	ImGui::SetNextWindowSize({ 500, 300 });
	ImGui::Begin("FiveM External",
		nullptr,
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	if (ImGui::BeginTabBar("MainTabs"))
	{
		if (ImGui::BeginTabItem("Visuals"))
		{
			ImGui::Checkbox("Enable ESP", &drawESP);

			ImGui::Separator();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void Overlay::SetForeground(HWND window)
{
	if (!IsWindowInForeground(window))
		BringToForeground(window);
}