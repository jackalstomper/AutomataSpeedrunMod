#pragma once

#include <d2d1_1.h>
#include <dwrite.h>
#include "Log.hpp"

#ifndef AUTOMATA_RELEASE_TARGET
#include <vector>
#endif

namespace DxWrappers {

class DXGISwapChainWrapper : public IDXGISwapChain {
    static const WCHAR* VC3_NAME;

    IDXGISwapChain* m_target;
    ID2D1DeviceContext* m_deviceContext;
    IDWriteTextFormat* m_textFormat;
    ID2D1SolidColorBrush* m_brush;
#ifndef AUTOMATA_RELEASE_TARGET
    std::vector<std::string> m_logLines;
#endif

    void renderWatermark();

public:
    DXGISwapChainWrapper(IDXGISwapChain* target, ID2D1DeviceContext* deviceContext, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* brush);
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

#ifndef AUTOMATA_RELEASE_TARGET
    // Write the given line on screen
    void writeLog(const std::string& line);
#endif

};

class DXGIFactoryWrapper : public IDXGIFactory {
    static const D2D1_BITMAP_PROPERTIES1 BITMAP_PROPERTIES;

    IDXGIFactory* m_target;
    ID2D1Factory1* m_D2DFactory;
    IDWriteFactory* m_DWFactory;
    ID2D1Device* m_D2DDevice;
    ID2D1DeviceContext* m_D2DDeviceContext;
    ID2D1SolidColorBrush* m_Brush;
    ID2D1Bitmap1* m_Target;
    IDWriteTextFormat* m_DWTextFormat;
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

#ifndef AUTOMATA_RELEASE_TARGET
    // Write the given line on screen
    void writeLog(const std::string& line);
#endif

};

} // namespace DxWrappers