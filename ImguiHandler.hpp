#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <atlbase.h>
#include "DebugMenu.hpp"

class ImguiHandler {
    UI::DebugMenu m_debugMenu;
    ATL::CComPtr<ID3D11Device> m_dxDevice;
    ATL::CComPtr<ID3D11DeviceContext> m_dxDeviceContext;

public:
    ImguiHandler(HWND hwnd, ATL::CComPtr<ID3D11Device> dxDevice, ATL::CComPtr<ID3D11DeviceContext> dxDeviceContext);
    ~ImguiHandler();
    void render();
};