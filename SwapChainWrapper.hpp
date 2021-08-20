#pragma once

#include <dxgi1_2.h>
#include <atlbase.h>
#include <memory>
#include <optional>
#include "ImguiHandler.hpp"

namespace DxWrappers {

class DXGISwapChainWrapper : public IDXGISwapChain1 {
    std::shared_ptr<ImguiHandler> m_imguiHandler;
    ULONG m_refCount;
    CComPtr<IDXGISwapChain1> m_target;

public:
    DXGISwapChainWrapper(std::unique_ptr<ImguiHandler> imguiHandler, CComPtr<IDXGISwapChain1> target);
    virtual ~DXGISwapChainWrapper();
    std::shared_ptr<ImguiHandler> getImguiHandler() const;

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
    virtual HRESULT __stdcall GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc) override;
    virtual HRESULT __stdcall GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc) override;
    virtual HRESULT __stdcall GetHwnd(HWND* pHwnd) override;
    virtual HRESULT __stdcall GetCoreWindow(REFIID refiid, void** ppUnk) override;
    virtual HRESULT __stdcall Present1(UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters) override;
    virtual BOOL __stdcall IsTemporaryMonoSupported() override;
    virtual HRESULT __stdcall GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput) override;
    virtual HRESULT __stdcall SetBackgroundColor(const DXGI_RGBA* pColor) override;
    virtual HRESULT __stdcall GetBackgroundColor(DXGI_RGBA* pColor) override;
    virtual HRESULT __stdcall SetRotation(DXGI_MODE_ROTATION Rotation) override;
    virtual HRESULT __stdcall GetRotation(DXGI_MODE_ROTATION* pRotation) override;

};

} // namespace DxWrappers