#include "SwapChainWrapper.hpp"
#include "Log.hpp"
#include <cmath>
#include <random>

namespace DxWrappers {

DXGISwapChainWrapper::DXGISwapChainWrapper(std::unique_ptr<ImguiHandler> imguiHandler, CComPtr<IDXGISwapChain1> target) :
    m_imguiHandler(std::move(imguiHandler))
{
    m_refCount = 1;
    m_target = target;
}

DXGISwapChainWrapper::~DXGISwapChainWrapper() {}

std::shared_ptr<ImguiHandler> DXGISwapChainWrapper::getImguiHandler() const {
    return m_imguiHandler;
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
    ++m_refCount;
    return m_refCount;
}

ULONG __stdcall DXGISwapChainWrapper::Release() {
    if (m_refCount > 0) {
        --m_refCount;
    }

    if (m_refCount == 0) {
        m_target = nullptr;
    }

    return m_refCount;
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
    m_imguiHandler->render();
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