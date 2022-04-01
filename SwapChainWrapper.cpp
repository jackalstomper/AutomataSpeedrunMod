#include "SwapChainWrapper.hpp"
#include "Log.hpp"
#include <cmath>
#include <random>

namespace DxWrappers {

const WCHAR* DXGISwapChainWrapper::VC3_NAME = L"VC3Mod 1.8";
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

void DXGISwapChainWrapper::resetLocation(D2D1_SIZE_F& screenSize) {
    m_location.x = screenSize.width * 0.0495f;
    m_location.y = screenSize.height * 0.105f;
    m_velocity = { 0.f, 200.f };
}

void DXGISwapChainWrapper::renderWatermark() {
    if (!m_brush || !m_shadowBrush) {
        return;
    }

    static const D2D1::Matrix3x2F root = D2D1::Matrix3x2F::Identity();
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();

    CComPtr<IDXGISurface> dxgiBackBuffer;
    m_target->GetBuffer(0, __uuidof(IDXGISurface), (void**)&dxgiBackBuffer);
    if (!dxgiBackBuffer) {
        return;
    }

    CComPtr<ID2D1Bitmap1> bitmap;
    m_deviceContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer, &BITMAP_PROPERTIES, &bitmap);
    if (!bitmap) {
        return;
    }

    D2D1_SIZE_F screenSize = bitmap->GetSize();
    if (!m_dvdMode) {
        resetLocation(screenSize);
    }

    if (!m_textFormat) {
        CComPtr<IDWriteFactory> dwFactory;
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), (IUnknown**)&dwFactory);
        dwFactory->CreateTextFormat(
            L"Consolas",
            NULL,
            DWRITE_FONT_WEIGHT_MEDIUM,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            screenSize.height * 0.02f,
            L"en-us",
            &m_textFormat
        );

        if (!m_textFormat) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateTextFormat.");
            return;
        }
    }

    FLOAT textHeight = m_textFormat->GetFontSize();

    if (m_dvdMode) {
        // bounce off edges of screen
        if (m_location.x <= 0.f || (m_location.x + 120.f) >= screenSize.width)
            m_velocity.x *= -1.f;

        if (m_location.y <= 0.f || (m_location.y + textHeight) >= screenSize.height)
            m_velocity.y *= -1.f;

        std::chrono::duration<float> diff = now - m_lastFrame;
        FLOAT deltaTime = diff.count();
        m_location.x += m_velocity.x * deltaTime;
        m_location.y += m_velocity.y * deltaTime;

        // Clamp to the bounds if we skipped too far past screen boundry during an update
        // Otherwise we'll get stuck constantly reversing our velocity while being off-screen
        if (m_location.x <= 0.f || (m_location.x + 120.f) >= screenSize.width)
            m_location.x = std::fmax(std::fmin(m_location.x, screenSize.width - 120.f), 0.f);

        if (m_location.y <= 0.f || (m_location.y + textHeight) >= screenSize.height)
            m_location.y = std::fmax(std::fmin(m_location.y, screenSize.height - textHeight), 0.f);
    }

    D2D1_RECT_F rect = { m_location.x, m_location.y, m_location.x + 150.f, m_location.y + textHeight };

    CComPtr<ID2D1Image> oldTarget;
    m_deviceContext->GetTarget(&oldTarget);
    m_deviceContext->SetTarget(bitmap);
    m_deviceContext->BeginDraw();
    m_deviceContext->SetTransform(root);

    // Draw shadow behind our text
    m_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(2, 2));
    m_deviceContext->DrawText(VC3_NAME, VC3_LEN, m_textFormat, rect, m_shadowBrush);
    m_deviceContext->SetTransform(root);

    // Draw main text
    m_deviceContext->DrawText(VC3_NAME, VC3_LEN, m_textFormat, rect, m_brush);
    m_deviceContext->EndDraw();
    m_deviceContext->SetTarget(oldTarget);
    m_lastFrame = now;
}

DXGISwapChainWrapper::DXGISwapChainWrapper(IUnknown* pDevice, CComPtr<IDXGISwapChain1> target, CComPtr<ID2D1Factory2> d2dFactory) {
    m_target = target;
    m_dvdMode = false;
    
    rotateVelocity(); // Rotate the velocity in a random direction

    CComPtr<IDXGIDevice> pDXGIDevice;
    HRESULT queryResult = pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
    if (!SUCCEEDED(queryResult)) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get DXGIDevice. Error code: " + std::to_string(queryResult));
        return;
    }

    queryResult = d2dFactory->CreateDevice(pDXGIDevice, &m_D2DDevice);
    if (!SUCCEEDED(queryResult)) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateDevice. Error code: " + std::to_string(queryResult));
        return;
    }

    m_deviceContext = nullptr;

    queryResult = m_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_deviceContext);
    if (!SUCCEEDED(queryResult)) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateDeviceContext. Error code: " + std::to_string(queryResult));
        return;
    }

    m_lastFrame = std::chrono::high_resolution_clock::now();

    queryResult = m_deviceContext->CreateSolidColorBrush(WATERMARK_COLOR, &m_brush);
    if (!SUCCEEDED(queryResult)) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateSolidColorBrush. Error code: " + std::to_string(queryResult));
        return;
    }

    queryResult = m_deviceContext->CreateSolidColorBrush(SHADOW_COLOR, &m_shadowBrush);
    if (!SUCCEEDED(queryResult)) {
        AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateSolidColorBrush. Error code: " + std::to_string(queryResult));
        return;
    }
}

DXGISwapChainWrapper::~DXGISwapChainWrapper() {}

void DXGISwapChainWrapper::toggleDvdMode(bool enabled) {
    m_dvdMode = enabled;
    if (m_dvdMode)
        rotateVelocity();
}

HRESULT __stdcall DXGISwapChainWrapper::QueryInterface(REFIID riid, void** ppvObject) {
    if (riid == __uuidof(IDXGISwapChain1) || riid == __uuidof(IDXGISwapChain) || 
        riid == __uuidof(IDXGIDeviceSubObject) || riid == __uuidof(IDXGIObject) || riid == __uuidof(IUnknown)) {
        this->AddRef();
        *ppvObject = this;
        return S_OK;
    }

    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG __stdcall DXGISwapChainWrapper::AddRef() {
    return m_refCounter.incrementRef();
}

ULONG __stdcall DXGISwapChainWrapper::Release() {
    return m_refCounter.decrementRef([this](ULONG refCount) {
        if (refCount == 0) {
            m_brush = nullptr;
            m_shadowBrush = nullptr;
            m_textFormat = nullptr;
        }
    });
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

HRESULT __stdcall DXGISwapChainWrapper::GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc) {
    return m_target->GetDesc1(pDesc);
}

HRESULT __stdcall DXGISwapChainWrapper::GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc) {
    return m_target->GetFullscreenDesc(pDesc);
}

HRESULT __stdcall DXGISwapChainWrapper::GetHwnd(HWND* pHwnd) {
    return m_target->GetHwnd(pHwnd);
}

HRESULT __stdcall DXGISwapChainWrapper::GetCoreWindow(REFIID refiid, void** ppUnk) {
    return m_target->GetCoreWindow(refiid, ppUnk);
}

HRESULT __stdcall DXGISwapChainWrapper::Present1(UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters) {
    return m_target->Present1(SyncInterval, PresentFlags, pPresentParameters);
}

BOOL __stdcall DXGISwapChainWrapper::IsTemporaryMonoSupported() {
    return m_target->IsTemporaryMonoSupported();
}

HRESULT __stdcall DXGISwapChainWrapper::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput) {
    return m_target->GetRestrictToOutput(ppRestrictToOutput);
}

HRESULT __stdcall DXGISwapChainWrapper::SetBackgroundColor(const DXGI_RGBA* pColor) {
    return m_target->SetBackgroundColor(pColor);
}

HRESULT __stdcall DXGISwapChainWrapper::GetBackgroundColor(DXGI_RGBA* pColor) {
    return m_target->GetBackgroundColor(pColor);
}

HRESULT __stdcall DXGISwapChainWrapper::SetRotation(DXGI_MODE_ROTATION Rotation) {
    return m_target->SetRotation(Rotation);
}

HRESULT __stdcall DXGISwapChainWrapper::GetRotation(DXGI_MODE_ROTATION* pRotation) {
    return m_target->GetRotation(pRotation);
}

} // namespace DxWrappers