#pragma once

#include <d2d1_1.h>
#include <dwrite.h>
#include <chrono>

namespace DxWrappers {

class DXGISwapChainWrapper : public IDXGISwapChain {
    static const WCHAR* VC3_NAME;
    static const UINT VC3_LEN;
    static const D2D1::ColorF WATERMARK_COLOR;
    static const D2D1::ColorF SHADOW_COLOR;
    static const D2D1_BITMAP_PROPERTIES1 BITMAP_PROPERTIES;

    IDXGISwapChain* m_target;
    ID2D1DeviceContext* m_deviceContext;
    IDWriteTextFormat* m_textFormat;
    ID2D1SolidColorBrush* m_brush;
    ID2D1SolidColorBrush* m_shadowBrush;
    ID2D1Bitmap1* m_bitmap;

    bool m_dvdMode; // true when watermark should bounce around
    D2D1_VECTOR_2F m_location;
    D2D1_VECTOR_2F m_velocity;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrame;
    D2D1_SIZE_F m_screenSize;

    void renderWatermark();

    // Roates velocity in a random direction
    void rotateVelocity();

    // Resets location to the default position
    void resetLocation();

public:
    DXGISwapChainWrapper(IDXGISwapChain* target, ID2D1DeviceContext* deviceContext);
    virtual ~DXGISwapChainWrapper();
    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    virtual ULONG __stdcall AddRef() override;
    virtual ULONG __stdcall Release() override;
    virtual HRESULT __stdcall SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) override;
    virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) override;
    virtual HRESULT __stdcall GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) override;
    virtual HRESULT __stdcall GetParent(REFIID riid, void** ppParent) override;
    virtual HRESULT __stdcall GetDevice(REFIID riid, void** ppDevice) override;
    virtual HRESULT __stdcall Present(UINT SyncInterval, UINT Flags) override;
    virtual HRESULT __stdcall GetBuffer(UINT Buffer, REFIID riid, void** ppSurface) override;
    virtual HRESULT __stdcall SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget) override;
    virtual HRESULT __stdcall GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget) override;
    virtual HRESULT __stdcall GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc) override;
    virtual HRESULT __stdcall ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) override;
    virtual HRESULT __stdcall ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters) override;
    virtual HRESULT __stdcall GetContainingOutput(IDXGIOutput** ppOutput) override;
    virtual HRESULT __stdcall GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats) override;
    virtual HRESULT __stdcall GetLastPresentCount(UINT* pLastPresentCount) override;

    void toggleDvdMode(bool enabled);
};

class DXGIFactoryWrapper : public IDXGIFactory {

    IDXGIFactory* m_target;
    ID2D1Factory1* m_D2DFactory;
    ID2D1Device* m_D2DDevice;
    ID2D1DeviceContext* m_D2DDeviceContext;
    DXGISwapChainWrapper* m_currentSwapChain;

public:
    DXGIFactoryWrapper(IDXGIFactory* target);
    virtual ~DXGIFactoryWrapper();
    virtual HRESULT __stdcall EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter) override;
    virtual HRESULT __stdcall MakeWindowAssociation(HWND WindowHandle, UINT Flags) override;
    virtual HRESULT __stdcall GetWindowAssociation(HWND* pWindowHandle) override;
    virtual HRESULT __stdcall CreateSwapChain(IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain) override;
    virtual HRESULT __stdcall CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter** ppAdapter) override;
    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    virtual ULONG __stdcall AddRef() override;
    virtual ULONG __stdcall Release() override;
    virtual HRESULT __stdcall SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) override;
    virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) override;
    virtual HRESULT __stdcall GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) override;
    virtual HRESULT __stdcall GetParent(REFIID riid, void** ppParent) override;

    void toggleDvdMode(bool enabled);
};

} // namespace DxWrappers