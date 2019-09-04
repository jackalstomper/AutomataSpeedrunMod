#include "DxWrappers.hpp"
#include "Log.hpp"
#include <cmath>
#include <random>

namespace DxWrappers {

const WCHAR* DXGISwapChainWrapper::VC3_NAME = L"VC3 Mod 1.4";
const UINT DXGISwapChainWrapper::VC3_LEN = (UINT)wcslen(VC3_NAME);

const D2D1::ColorF DXGISwapChainWrapper::WATERMARK_COLOR = D2D1::ColorF(0.803f, 0.784f, 0.690f, 1.f);
const D2D1::ColorF DXGISwapChainWrapper::SHADOW_COLOR = D2D1::ColorF(0.f, 0.f, 0.f, 0.3f);

const D2D1_BITMAP_PROPERTIES1 DXGISwapChainWrapper::BITMAP_PROPERTIES = {
       { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
       96.f,
       96.f,
       D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
       nullptr
};

void DXGISwapChainWrapper::rotateVelocity() {
    static std::random_device rd;
    static const FLOAT pi = 3.141592f;
    static std::mt19937 eng(rd());
    static std::uniform_real_distribution<float> dist(-pi, pi);

    const FLOAT angle = dist(eng);
    m_velocity.x = m_velocity.x * std::cos(angle) - m_velocity.y * std::sin(angle);
    m_velocity.y = m_velocity.x * std::sin(angle) - m_velocity.y * std::cos(angle);
}

void DXGISwapChainWrapper::resetLocation() {
    m_location.x = m_screenSize.width * 0.0495f;
    m_location.y = m_screenSize.height * 0.105f;
    m_velocity = { 0.f, 200.f };
}

void DXGISwapChainWrapper::renderWatermark() {
    static const  D2D1::Matrix3x2F root = D2D1::Matrix3x2F::Identity();
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
    FLOAT textHeight = m_textFormat->GetFontSize();

    if (m_dvdMode) {
        // bounce off edges of screen
        if (m_location.x <= 0.f || (m_location.x + 120.f) >= m_screenSize.width)
            m_velocity.x *= -1.f;

        if (m_location.y <= 0.f || (m_location.y + textHeight) >= m_screenSize.height)
            m_velocity.y *= -1.f;

        std::chrono::duration<float> diff = now - m_lastFrame;
        FLOAT deltaTime = diff.count();
        m_location.x += m_velocity.x * deltaTime;
        m_location.y += m_velocity.y * deltaTime;

        // Clamp to the bounds if we skipped too far past screen boundry during an update
        // Otherwise we'll get stuck constantly reversing our velocity while being off-screen
        if (m_location.x <= 0.f || (m_location.x + 120.f) >= m_screenSize.width)
            m_location.x = std::fmax(std::fmin(m_location.x, m_screenSize.width - 120.f), 0.f);

        if (m_location.y <= 0.f || (m_location.y + textHeight) >= m_screenSize.height)
            m_location.y = std::fmax(std::fmin(m_location.y, m_screenSize.height - textHeight), 0.f);
    }

    D2D1_RECT_F rect = { m_location.x, m_location.y, m_location.x + 150.f, m_location.y + textHeight };

    m_deviceContext->BeginDraw();
    m_deviceContext->SetTransform(root);

    // Draw shadow behind our text
    m_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(2, 2));
    m_deviceContext->DrawText(VC3_NAME, VC3_LEN, m_textFormat, rect, m_shadowBrush);
    m_deviceContext->SetTransform(root);

    // Draw main text
    m_deviceContext->DrawText(VC3_NAME, VC3_LEN, m_textFormat, rect, m_brush);
    m_deviceContext->EndDraw();
    m_lastFrame = now;
}

DXGISwapChainWrapper::DXGISwapChainWrapper(IDXGISwapChain* target, ID2D1DeviceContext* deviceContext) {
    m_target = target;
    m_deviceContext = deviceContext;
    m_brush = nullptr;
    m_shadowBrush = nullptr;
    m_bitmap = nullptr;

    m_dvdMode = false;
    rotateVelocity(); // Rotate the velocity in a random direction

    m_lastFrame = std::chrono::high_resolution_clock::now();

    m_deviceContext->CreateSolidColorBrush(WATERMARK_COLOR, &m_brush);
    m_deviceContext->CreateSolidColorBrush(SHADOW_COLOR, &m_shadowBrush);

    IDXGISurface* pBackBuffer = nullptr;
    m_target->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pBackBuffer);
    m_deviceContext->CreateBitmapFromDxgiSurface(pBackBuffer, &BITMAP_PROPERTIES, &m_bitmap);
    pBackBuffer->Release();
    m_deviceContext->SetTarget(m_bitmap);
    m_screenSize = m_bitmap->GetSize();
    resetLocation();

    IDWriteFactory* dwFactory = nullptr;
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&dwFactory);
    dwFactory->CreateTextFormat(
        L"Consolas",
        NULL,
        DWRITE_FONT_WEIGHT_MEDIUM,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        //20.0f,
        m_screenSize.height * 0.02f,
        L"en-us",
        &m_textFormat
    );

    dwFactory->Release();
}

DXGISwapChainWrapper::~DXGISwapChainWrapper() {
    if (m_brush)
        m_brush->Release();

    if (m_shadowBrush)
        m_shadowBrush->Release();

    if (m_bitmap)
        m_bitmap->Release();

    if (m_textFormat)
        m_textFormat->Release();

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
    return m_target->Present(SyncInterval, Flags);
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

void DXGISwapChainWrapper::toggleDvdMode(bool enabled) {
    m_dvdMode = enabled;
    if (m_dvdMode)
        rotateVelocity();
    else
        resetLocation();
}


DXGIFactoryWrapper::DXGIFactoryWrapper(IDXGIFactory* target) {
    m_target = target;
    m_D2DFactory = nullptr;
    m_D2DDevice = nullptr;
    m_D2DDeviceContext = nullptr;
    m_currentSwapChain = nullptr;

    D2D1_FACTORY_OPTIONS opt = { D2D1_DEBUG_LEVEL_ERROR };
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &opt, (void**)&m_D2DFactory);
}

DXGIFactoryWrapper::~DXGIFactoryWrapper() {
    if (m_currentSwapChain) {
        delete m_currentSwapChain;
        m_currentSwapChain = nullptr;
    }

    if (m_target)
        m_target->Release();

    if (m_D2DFactory)
        m_D2DFactory->Release();

    if (m_D2DDevice)
        m_D2DDevice->Release();

    if (m_D2DDeviceContext)
        m_D2DDeviceContext->Release();
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

    if (SUCCEEDED(result)) {
        if (m_currentSwapChain)
            delete m_currentSwapChain;

        if (m_D2DDeviceContext)
            m_D2DDeviceContext->Release();

        IDXGIDevice* pDXGIDevice = nullptr;
        pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)& pDXGIDevice);
        m_D2DFactory->CreateDevice(pDXGIDevice, &m_D2DDevice);
        pDXGIDevice->Release();
        m_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_D2DDeviceContext);

        m_currentSwapChain = new DXGISwapChainWrapper(swapChain, m_D2DDeviceContext);
        *ppSwapChain = m_currentSwapChain;
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

void DXGIFactoryWrapper::toggleDvdMode(bool enabled) {
    if (m_currentSwapChain)
        m_currentSwapChain->toggleDvdMode(enabled);
}

} // namespace DxWrappers