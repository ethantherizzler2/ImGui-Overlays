#include "render.h"
#include "ui.h"
#include "other/notification.h"
#include <Windows.h>
#include "dependencies/Minhook/MinHook.h"
#include <d3d11.h>

#include "dependencies/imgui/imgui.h"
#include "dependencies/imgui/imgui_impl_dx11.h"
#include "dependencies/imgui/imgui_impl_win32.h"

#pragma comment(lib, "d3d11.lib")

using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
using ResizeBuffersFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

PresentFn oPresent = nullptr;
ResizeBuffersFn oResizeBuffers = nullptr;

ID3D11Device* g_Device = nullptr;
ID3D11DeviceContext* g_Context = nullptr;
ID3D11RenderTargetView* g_RTV = nullptr;
HWND g_Window = nullptr;
WNDPROC oWndProc = nullptr;

bool g_Initialized = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_Initialized && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}
void CreateRTV(IDXGISwapChain* sc)
{
    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
        return; 

    g_Device->CreateRenderTargetView(backBuffer, nullptr, &g_RTV);
    backBuffer->Release();
}

void InitImGui(IDXGISwapChain* sc)
{
    sc->GetDevice(__uuidof(ID3D11Device), (void**)&g_Device);
    g_Device->GetImmediateContext(&g_Context);

    DXGI_SWAP_CHAIN_DESC desc{};
    sc->GetDesc(&desc);
    g_Window = desc.OutputWindow;

    CreateRTV(sc);

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(g_Window);
    ImGui_ImplDX11_Init(g_Device, g_Context);

    oWndProc = (WNDPROC)SetWindowLongPtr(g_Window, GWLP_WNDPROC, (LONG_PTR)WndProc); 

    UI::Init();

    g_Initialized = true;
}

HRESULT __stdcall hkPresent(IDXGISwapChain* sc, UINT sync, UINT flags)
{
    if (!g_Initialized)
        InitImGui(sc);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    UI::Render();
    Notify::Render();

    ImGui::Render();

    g_Context->OMSetRenderTargets(1, &g_RTV, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(sc, sync, flags);
}

HRESULT __stdcall ResizeBuffers(IDXGISwapChain* sc, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (g_RTV)
    {
        g_Context->OMSetRenderTargets(0, nullptr, nullptr);
        g_RTV->Release();
        g_RTV = nullptr;
    }

    HRESULT hr = oResizeBuffers(sc, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    if (SUCCEEDED(hr))
    {
        CreateRTV(sc);
    }

    return hr;
}


void Render::Init()
{
    if (MH_Initialize() != MH_OK)
    {
        return;
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = GetForegroundWindow(); 
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    IDXGISwapChain* sc = nullptr;
    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &sd, &sc, &dev, nullptr, &ctx
    ))) {
        MH_Uninitialize();
        return;
    }

    void** vtable = *reinterpret_cast<void***>(sc);

    if (MH_CreateHook(vtable[8], hkPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK)
    {
        
        goto cleanup;
    }

    if (MH_CreateHook(vtable[13], hkResizeBuffers, reinterpret_cast<void**>(&oResizeBuffers)) != MH_OK)
    {
        goto cleanup;
    }
    MH_EnableHook(MH_ALL_HOOKS);

cleanup:
    sc->Release();
    dev->Release();
    ctx->Release();
}

void Render::Shutdown()
{
    if (g_Window && oWndProc)
        SetWindowLongPtr(g_Window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (g_RTV) g_RTV->Release();
    if (g_Context) g_Context->Release();
    if (g_Device) g_Device->Release();

    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

}
