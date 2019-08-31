#include "DxWrappers.hpp"

#ifndef AUTOMATA_RELEASE_TARGET
#include <codecvt>
namespace {

static std::wstring_convert<std::codecvt_utf8<wchar_t>> g_converter;

}
#endif

namespace DxWrappers {

const WCHAR* DXGISwapChainWrapper::VC3_NAME = L"VC3 Mod 1.4";

const D2D1_BITMAP_PROPERTIES1 DXGIFactoryWrapper::BITMAP_PROPERTIES = {
       { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
       96.f,
       96.f,
       D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
       nullptr
};

void DXGISwapChainWrapper::renderWatermark() {
    FLOAT x = 20.f, y = 25.f;
    D2D1_RECT_F r = { x, y, x + 300.f, y + m_textFormat->GetFontSize() };

    m_deviceContext->BeginDraw();
    m_deviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
    m_deviceContext->DrawTextA(VC3_NAME, (UINT)wcslen(VC3_NAME), m_textFormat, r, m_brush);

#ifndef AUTOMATA_RELEASE_TARGET
    // Render 16 debug log lines on screen
    if (m_logLines.size() > 0) {
        FLOAT dx = 20.f, dy = 90.f;
        FLOAT fontSize = m_textFormat->GetFontSize();
        D2D1_RECT_F dRect = { dx, dy, dx + 1600.f, dy + fontSize };
        auto i = m_logLines.begin();
        if (m_logLines.size() > 16)
            i = m_logLines.end() - 16;

        for (; i != m_logLines.end(); ++i) {
            std::wstring wstr = g_converter.from_bytes(*i);
            m_deviceContext->DrawTextA(wstr.c_str(), wstr.length(), m_textFormat, dRect, m_brush);
            dRect.top += fontSize + 3.f;
            dRect.bottom += fontSize + 3.f;
        }
    }
#endif

    if (m_deviceContext->EndDraw() == D2DERR_RECREATE_TARGET) {
        AutomataMod::showErrorBox("Failed to render, need to remake frame!");
    }
}

DXGISwapChainWrapper::DXGISwapChainWrapper(IDXGISwapChain* target, ID2D1DeviceContext* deviceContext, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* brush) {
    m_target = target;
    m_deviceContext = deviceContext;
    m_textFormat = textFormat;
    m_brush = brush;
}

DXGISwapChainWrapper::~DXGISwapChainWrapper() {
    m_target->Release();
}

HRESULT __stdcall DXGISwapChainWrapper::QueryInterface(REFIID riid, void** ppvObject) {
    return m_target->QueryInterface(riid, ppvObject);
}

ULONG __stdcall DXGISwapChainWrapper::AddRef() {
    return m_target->AddRef();
}

ULONG __stdcall DXGISwapChainWrapper::Release() {
    return m_target->Release();
}

HRESULT __stdcall DXGISwapChainWrapper::SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) {
    return m_target->SetPrivateData(Name, DataSize, pData);
}

HRESULT __stdcall DXGISwapChainWrapper::SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) {
    return m_target->SetPrivateDataInterface(Name, pUnknown);
}

HRESULT __stdcall DXGISwapChainWrapper::GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) {
    return m_target->GetPrivateData(Name, pDataSize, pData);
}

HRESULT __stdcall DXGISwapChainWrapper::GetParent(REFIID riid, void** ppParent) {
    return m_target->GetParent(riid, ppParent);
}

HRESULT __stdcall DXGISwapChainWrapper::GetDevice(REFIID riid, void** ppDevice) {
    return m_target->GetDevice(riid, ppDevice);
}

HRESULT __stdcall DXGISwapChainWrapper::Present(UINT SyncInterval, UINT Flags) {
    renderWatermark();
    return m_target->Present(SyncInterval, Flags);;
}

HRESULT __stdcall DXGISwapChainWrapper::GetBuffer(UINT Buffer, REFIID riid, void** ppSurface) {
    return m_target->GetBuffer(Buffer, riid, ppSurface);
}

HRESULT __stdcall DXGISwapChainWrapper::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget) {
    return m_target->SetFullscreenState(Fullscreen, pTarget);
}

HRESULT __stdcall DXGISwapChainWrapper::GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget) {
    return m_target->GetFullscreenState(pFullscreen, ppTarget);
}

HRESULT __stdcall DXGISwapChainWrapper::GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc) {
    return m_target->GetDesc(pDesc);
}

HRESULT __stdcall DXGISwapChainWrapper::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    return m_target->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

HRESULT __stdcall DXGISwapChainWrapper::ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters) {
    return m_target->ResizeTarget(pNewTargetParameters);
}

HRESULT __stdcall DXGISwapChainWrapper::GetContainingOutput(IDXGIOutput** ppOutput) {
    return m_target->GetContainingOutput(ppOutput);
}

HRESULT __stdcall DXGISwapChainWrapper::GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats) {
    return m_target->GetFrameStatistics(pStats);
}

HRESULT __stdcall DXGISwapChainWrapper::GetLastPresentCount(UINT* pLastPresentCount) {
    return m_target->GetLastPresentCount(pLastPresentCount);
}

#ifndef AUTOMATA_RELEASE_TARGET
void DXGISwapChainWrapper::writeLog(const std::string& line) {
    m_logLines.push_back(line);
}
#endif


DXGIFactoryWrapper::DXGIFactoryWrapper(IDXGIFactory* target) {
    m_target = target;
    m_D2DFactory = nullptr;
    m_DWFactory = nullptr;
    m_D2DDevice = nullptr;
    m_D2DDeviceContext = nullptr;
    m_Brush = nullptr;
    m_Target = nullptr;
    m_DWTextFormat = nullptr;
    m_currentSwapChain = nullptr;

    D2D1_FACTORY_OPTIONS opt = { D2D1_DEBUG_LEVEL_ERROR };
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &opt, (void**)&m_D2DFactory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_DWFactory);
    m_DWFactory->CreateTextFormat(
        L"Consolas",
        NULL,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        16.0f,
        L"en-us",
        &m_DWTextFormat
    );
}

DXGIFactoryWrapper::~DXGIFactoryWrapper() {
    if (m_target)
        m_target->Release();

    if (m_D2DFactory)
        m_D2DFactory->Release();

    if (m_DWFactory)
        m_DWFactory->Release();

    if (m_D2DDevice)
        m_D2DDevice->Release();

    if (m_D2DDeviceContext)
        m_D2DDeviceContext->Release();

    if (m_Brush)
        m_Brush->Release();

    if (m_Target)
        m_Target->Release();

    if (m_DWTextFormat)
        m_DWTextFormat->Release();

    if (m_currentSwapChain) {
        delete m_currentSwapChain;
        m_currentSwapChain = nullptr;
    }
}

HRESULT __stdcall DXGIFactoryWrapper::EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter) {
    return m_target->EnumAdapters(Adapter, ppAdapter);
}

HRESULT __stdcall DXGIFactoryWrapper::MakeWindowAssociation(HWND WindowHandle, UINT Flags) {
    return m_target->MakeWindowAssociation(WindowHandle, Flags);
}

HRESULT __stdcall DXGIFactoryWrapper::GetWindowAssociation(HWND* pWindowHandle) {
    return m_target->GetWindowAssociation(pWindowHandle);
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChain(IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain) {
    IDXGISwapChain* swapChain;
    HRESULT result = m_target->CreateSwapChain(pDevice, pDesc, &swapChain);

    if (m_D2DDeviceContext == nullptr) {
        IDXGIDevice* pDXGIDevice = nullptr;
        pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
        m_D2DFactory->CreateDevice(pDXGIDevice, &m_D2DDevice);
        pDXGIDevice->Release();
        m_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_D2DDeviceContext);
        m_D2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond), &m_Brush);
    }

    if (SUCCEEDED(result)) {
        m_currentSwapChain = new DXGISwapChainWrapper(swapChain, m_D2DDeviceContext, m_DWTextFormat, m_Brush);
        *ppSwapChain = m_currentSwapChain;
        IDXGISurface* pBackBuffer = nullptr;
        if (m_Target)
            m_Target->Release();

        swapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pBackBuffer);
        m_D2DDeviceContext->CreateBitmapFromDxgiSurface(pBackBuffer, &BITMAP_PROPERTIES, &m_Target);
        pBackBuffer->Release();
        m_D2DDeviceContext->SetTarget(m_Target);
    }

    return result;
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter** ppAdapter) {
    return m_target->CreateSoftwareAdapter(Module, ppAdapter);
}

HRESULT __stdcall DXGIFactoryWrapper::QueryInterface(REFIID riid, void** ppvObject) {
    return m_target->QueryInterface(riid, ppvObject);
}

ULONG __stdcall DXGIFactoryWrapper::AddRef() {
    return m_target->AddRef();
}

ULONG __stdcall DXGIFactoryWrapper::Release() {
    return m_target->Release();
}

HRESULT __stdcall DXGIFactoryWrapper::SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) {
    return m_target->SetPrivateData(Name, DataSize, pData);
}

HRESULT __stdcall DXGIFactoryWrapper::SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) {
    return m_target->SetPrivateDataInterface(Name, pUnknown);
}

HRESULT __stdcall DXGIFactoryWrapper::GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) {
    return m_target->GetPrivateData(Name, pDataSize, pData);
}

HRESULT __stdcall DXGIFactoryWrapper::GetParent(REFIID riid, void** ppParent) {
    return m_target->GetParent(riid, ppParent);
}

#ifndef AUTOMATA_RELEASE_TARGET
void DXGIFactoryWrapper::writeLog(const std::string& line) {
    if (m_currentSwapChain)
        m_currentSwapChain->writeLog(line);
}
#endif

} // namespace DxWrappers