module;

#include <d2d1_2.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include <atlbase.h>
#include <chrono>
#include <array>
#include <random>
#include <cmath>
#include <fmt/format.h>
#include <fmt/xchar.h>

export module SwapChainWrapper;

import RefCounter;
import Log;

const WCHAR* VC3_NAME = L"VC3Mod 1.9";
const UINT VC3_LEN = (UINT)wcslen(VC3_NAME);

const float SCREEN_WIDTH = 1600.f;
const float SCREEN_HEIGHT = 900.f;
const float WATERMARK_TEXT_SIZE = 15.25f;

const D2D1::ColorF WATERMARK_COLOR = D2D1::ColorF(0.803f, 0.784f, 0.690f, 1.f);
const D2D1::ColorF SHADOW_COLOR = D2D1::ColorF(0.f, 0.f, 0.f, 0.3f);

const D2D1_BITMAP_PROPERTIES1 BITMAP_PROPERTIES = {
    { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
    96.f,
    96.f,
    D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
    nullptr
};

namespace DxWrappers {

export class DXGISwapChainWrapper : public IDXGISwapChain1 {
    RefCounter m_refCounter;
    CComPtr<IDXGISwapChain1> m_target;
    CComPtr<ID2D1DeviceContext> m_deviceContext;
    CComPtr<ID2D1Device> m_D2DDevice;
    CComPtr<IDWriteTextFormat> m_textFormat;
    CComPtr<ID2D1SolidColorBrush> m_brush;
    CComPtr<ID2D1SolidColorBrush> m_shadowBrush;

    bool m_dvdMode; // true when watermark should bounce around
    D2D1_VECTOR_2F m_location;
    D2D1_VECTOR_2F m_velocity;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrame;

    // FPS Display
    std::array<float, 60> m_frameTimes;
    size_t m_frameTimeIndex;

    void renderWatermark() {
        if (!m_brush || !m_shadowBrush) {
            return;
        }

        static const D2D1::Matrix3x2F root = D2D1::Matrix3x2F::Identity();
        std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> frameDelta = now - m_lastFrame;

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

        float xscale = screenSize.width * (1.f / SCREEN_WIDTH);
        float yscale = screenSize.height * (1.f / SCREEN_HEIGHT);
        float heightScale = WATERMARK_TEXT_SIZE * yscale;

        if (!m_textFormat) {
            CComPtr<IDWriteFactory> dwFactory;
            DWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), (IUnknown**)&dwFactory);
            dwFactory->CreateTextFormat(
                L"Consolas",
                NULL,
                DWRITE_FONT_WEIGHT_MEDIUM,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                heightScale,
                L"en-us",
                &m_textFormat
            );

            if (!m_textFormat) {
                AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateTextFormat.");
                return;
            }
        }

        FLOAT textHeight = m_textFormat->GetFontSize();
        FLOAT rectHeight = textHeight * 2; // * 2 for two lines, the mod name and the FPS count

        if (m_dvdMode) {
            // bounce off edges of screen
            if (m_location.x <= 0.f || (m_location.x + 120.f) >= screenSize.width)
                m_velocity.x *= -1.f;

            if (m_location.y <= 0.f || (m_location.y + rectHeight) >= screenSize.height)
                m_velocity.y *= -1.f;

            FLOAT deltaTime = frameDelta.count();
            m_location.x += m_velocity.x * deltaTime;
            m_location.y += m_velocity.y * deltaTime;

            // Clamp to the bounds if we skipped too far past screen boundry during an update
            // Otherwise we'll get stuck constantly reversing our velocity while being off-screen
            if (m_location.x <= 0.f || (m_location.x + 120.f) >= screenSize.width)
                m_location.x = std::fmax(std::fmin(m_location.x, screenSize.width - 120.f), 0.f);

            if (m_location.y <= 0.f || (m_location.y + rectHeight) >= screenSize.height)
                m_location.y = std::fmax(std::fmin(m_location.y, screenSize.height - rectHeight), 0.f);
        }

        D2D1_RECT_F rect = { m_location.x, m_location.y, m_location.x + 150.f * xscale, m_location.y + rectHeight };

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

        std::chrono::duration<float, std::milli> frameDeltaMilli = now - m_lastFrame;
        std::wstring fpsString = calculateFps(frameDeltaMilli.count());

        // Draw FPS Shadow
        m_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(2, textHeight + 2));
        m_deviceContext->DrawText(fpsString.c_str(), fpsString.length(), m_textFormat, rect, m_shadowBrush);
        m_deviceContext->SetTransform(root);

        // Draw FPS Counter
        m_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0, textHeight));
        m_deviceContext->DrawText(fpsString.c_str(), fpsString.length(), m_textFormat, rect, m_brush);

        m_deviceContext->EndDraw();
        m_deviceContext->SetTarget(oldTarget);
        m_lastFrame = now;
    }

    // Roates velocity in a random direction
    void rotateVelocity() {
        static std::random_device rd;
        static const FLOAT pi = 3.141592f;
        static std::mt19937 eng(rd());
        static std::uniform_real_distribution<float> dist(-pi, pi);

        const FLOAT angle = dist(eng);
        m_velocity.x = m_velocity.x * std::cos(angle) - m_velocity.y * std::sin(angle);
        m_velocity.y = m_velocity.x * std::sin(angle) - m_velocity.y * std::cos(angle);
    }

    // Resets location to the default position
    void resetLocation(D2D1_SIZE_F& screenSize) {
        m_location.x = screenSize.width * 0.0495f;
        m_location.y = screenSize.height * 0.105f;
        m_velocity = { 0.f, 200.f };
    }

    // Calculates current frame rate and returns formatted FPS display string
    // frameDelta must be in milliseconds
    std::wstring calculateFps(float frameDelta) {
        m_frameTimes[m_frameTimeIndex % m_frameTimes.size()] = frameDelta;
        ++m_frameTimeIndex;

        // Get average FPS
        float total = 0.f;
        for (float f : m_frameTimes)
            total += f;

        if (total < 1.f)
            total = 1.f;

        float fps = 1000 * m_frameTimes.size() / total;
        return fmt::format(L"{:.1f}FPS {:.2f}ms", fps, frameDelta);
    }

public:
    DXGISwapChainWrapper(IUnknown* pDevice, CComPtr<IDXGISwapChain1> target, CComPtr<ID2D1Factory2> d2dFactory) {
        m_target = target;
        m_dvdMode = false;

        rotateVelocity(); // Rotate the velocity in a random direction

        CComPtr<IDXGIDevice> pDXGIDevice;
        HRESULT queryResult = pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
        if (!SUCCEEDED(queryResult)) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get DXGIDevice. Error code: {}", queryResult);
            return;
        }

        queryResult = d2dFactory->CreateDevice(pDXGIDevice, &m_D2DDevice);
        if (!SUCCEEDED(queryResult)) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateDevice. Error code: {}", queryResult);
            return;
        }

        m_deviceContext = nullptr;

        queryResult = m_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_deviceContext);
        if (!SUCCEEDED(queryResult)) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateDeviceContext. Error code: {}", queryResult);
            return;
        }

        m_lastFrame = std::chrono::high_resolution_clock::now();

        queryResult = m_deviceContext->CreateSolidColorBrush(WATERMARK_COLOR, &m_brush);
        if (!SUCCEEDED(queryResult)) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateSolidColorBrush. Error code: {}", queryResult);
            return;
        }

        queryResult = m_deviceContext->CreateSolidColorBrush(SHADOW_COLOR, &m_shadowBrush);
        if (!SUCCEEDED(queryResult)) {
            AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateSolidColorBrush. Error code: {}", queryResult);
            return;
        }
    }

    virtual ~DXGISwapChainWrapper() {}

    void toggleDvdMode(bool enabled) {
        m_dvdMode = enabled;
        if (m_dvdMode)
            rotateVelocity();
    }

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == __uuidof(IDXGISwapChain1) || riid == __uuidof(IDXGISwapChain) ||
            riid == __uuidof(IDXGIDeviceSubObject) || riid == __uuidof(IDXGIObject) || riid == __uuidof(IUnknown)) {
            this->AddRef();
            *ppvObject = this;
            return S_OK;
        }

        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    virtual ULONG __stdcall AddRef() override {
        return m_refCounter.incrementRef();
    }

    virtual ULONG __stdcall Release() override {
        return m_refCounter.decrementRef([this](ULONG refCount) {
            if (refCount == 0) {
                m_brush = nullptr;
                m_shadowBrush = nullptr;
                m_textFormat = nullptr;
            }
        });
    }

    virtual HRESULT __stdcall SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) override {
        return m_target->SetPrivateData(Name, DataSize, pData);
    }

    virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) override {
        return m_target->SetPrivateDataInterface(Name, pUnknown);
    }

    virtual HRESULT __stdcall GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) override {
        return m_target->GetPrivateData(Name, pDataSize, pData);
    }

    virtual HRESULT __stdcall GetParent(REFIID riid, void** ppParent) override {
        return m_target->GetParent(riid, ppParent);
    }

    virtual HRESULT __stdcall GetDevice(REFIID riid, void** ppDevice) override {
        return m_target->GetDevice(riid, ppDevice);
    }

    virtual HRESULT __stdcall Present(UINT SyncInterval, UINT Flags) override {
        renderWatermark();
        return m_target->Present(SyncInterval, Flags);
    }

    virtual HRESULT __stdcall GetBuffer(UINT Buffer, REFIID riid, void** ppSurface) override {
        return m_target->GetBuffer(Buffer, riid, ppSurface);
    }

    virtual HRESULT __stdcall SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget) override {
        return m_target->SetFullscreenState(Fullscreen, pTarget);
    }

    virtual HRESULT __stdcall GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget) override {
        return m_target->GetFullscreenState(pFullscreen, ppTarget);
    }

    virtual HRESULT __stdcall GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc) override {
        return m_target->GetDesc(pDesc);
    }

    virtual HRESULT __stdcall ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) override {
        m_textFormat = nullptr; // Trigger a text format refresh
        return m_target->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    virtual HRESULT __stdcall ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters) override {
        return m_target->ResizeTarget(pNewTargetParameters);
    }

    virtual HRESULT __stdcall GetContainingOutput(IDXGIOutput** ppOutput) override {
        return m_target->GetContainingOutput(ppOutput);
    }

    virtual HRESULT __stdcall GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats) override {
        return m_target->GetFrameStatistics(pStats);
    }

    virtual HRESULT __stdcall GetLastPresentCount(UINT* pLastPresentCount) override {
        return m_target->GetLastPresentCount(pLastPresentCount);
    }

    virtual HRESULT __stdcall GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc) override {
        return m_target->GetDesc1(pDesc);
    }

    virtual HRESULT __stdcall GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc) override {
        return m_target->GetFullscreenDesc(pDesc);
    }

    virtual HRESULT __stdcall GetHwnd(HWND* pHwnd) override {
        return m_target->GetHwnd(pHwnd);
    }

    virtual HRESULT __stdcall GetCoreWindow(REFIID refiid, void** ppUnk) override {
        return m_target->GetCoreWindow(refiid, ppUnk);
    }

    virtual HRESULT __stdcall Present1(UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters) override {
        return m_target->Present1(SyncInterval, PresentFlags, pPresentParameters);
    }

    virtual BOOL __stdcall IsTemporaryMonoSupported() override {
        return m_target->IsTemporaryMonoSupported();
    }

    virtual HRESULT __stdcall GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput) override {
        return m_target->GetRestrictToOutput(ppRestrictToOutput);
    }

    virtual HRESULT __stdcall SetBackgroundColor(const DXGI_RGBA* pColor) override {
        return m_target->SetBackgroundColor(pColor);
    }

    virtual HRESULT __stdcall GetBackgroundColor(DXGI_RGBA* pColor) override {
        return m_target->GetBackgroundColor(pColor);
    }

    virtual HRESULT __stdcall SetRotation(DXGI_MODE_ROTATION Rotation) override {
        return m_target->SetRotation(Rotation);
    }

    virtual HRESULT __stdcall GetRotation(DXGI_MODE_ROTATION* pRotation) override {
        return m_target->GetRotation(pRotation);
    }
};

} // namespace DxWrappers