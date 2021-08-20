#include "ImguiHandler.hpp"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

ImguiHandler::ImguiHandler(HWND hwnd, ATL::CComPtr<ID3D11Device> dxDevice, ATL::CComPtr<ID3D11DeviceContext> dxDeviceContext) {
    m_dxDevice = dxDevice;
    m_dxDeviceContext = dxDeviceContext;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(m_dxDevice, m_dxDeviceContext);
}

ImguiHandler::~ImguiHandler() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImguiHandler::render() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    m_debugMenu.render();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}