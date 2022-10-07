#include "FactoryWrapper.hpp"

namespace DxWrappers {

DXGIFactoryWrapper::DXGIFactoryWrapper(CComPtr<IDXGIFactory2> target) {
  m_target = target;
  D2D1_FACTORY_OPTIONS opt = {D2D1_DEBUG_LEVEL_ERROR};
  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory2), &opt, (void **)&m_D2DFactory);
}

DXGIFactoryWrapper::~DXGIFactoryWrapper() {}

void DXGIFactoryWrapper::toggleDvdMode(bool enabled) {
  if (m_currentSwapChain)
    m_currentSwapChain->toggleDvdMode(enabled);
}

HRESULT __stdcall DXGIFactoryWrapper::QueryInterface(REFIID riid, void **ppvObject) {
  if (riid == __uuidof(IDXGIFactory2) || riid == __uuidof(IDXGIFactory1) || riid == __uuidof(IDXGIFactory) ||
      riid == __uuidof(IDXGIObject) || riid == __uuidof(IUnknown)) {
    this->AddRef();
    *ppvObject = this;
    return S_OK;
  }

  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

ULONG __stdcall DXGIFactoryWrapper::AddRef() { return m_refCounter.incrementRef(); }

ULONG __stdcall DXGIFactoryWrapper::Release() {
  return m_refCounter.decrementRef([this](ULONG refCount) {
    if (refCount == 0) {
      m_D2DFactory = nullptr;
      m_currentSwapChain = nullptr;
    }
  });
}

HRESULT __stdcall DXGIFactoryWrapper::SetPrivateData(REFGUID Name, UINT DataSize, const void *pData) {
  return m_target->SetPrivateData(Name, DataSize, pData);
}

HRESULT __stdcall DXGIFactoryWrapper::SetPrivateDataInterface(REFGUID Name, const IUnknown *pUnknown) {
  return m_target->SetPrivateDataInterface(Name, pUnknown);
}

HRESULT __stdcall DXGIFactoryWrapper::GetPrivateData(REFGUID Name, UINT *pDataSize, void *pData) {
  return m_target->GetPrivateData(Name, pDataSize, pData);
}

HRESULT __stdcall DXGIFactoryWrapper::GetParent(REFIID riid, void **ppParent) {
  return m_target->GetParent(riid, ppParent);
}

HRESULT __stdcall DXGIFactoryWrapper::EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter) {
  return m_target->EnumAdapters(Adapter, ppAdapter);
}

HRESULT __stdcall DXGIFactoryWrapper::MakeWindowAssociation(HWND WindowHandle, UINT Flags) {
  return m_target->MakeWindowAssociation(WindowHandle, Flags);
}

HRESULT __stdcall DXGIFactoryWrapper::GetWindowAssociation(HWND *pWindowHandle) {
  return m_target->GetWindowAssociation(pWindowHandle);
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc,
                                                      IDXGISwapChain **ppSwapChain) {
  return m_target->CreateSwapChain(pDevice, pDesc, ppSwapChain);
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter **ppAdapter) {
  return m_target->CreateSoftwareAdapter(Module, ppAdapter);
}

HRESULT __stdcall DXGIFactoryWrapper::EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter) {
  return m_target->EnumAdapters1(Adapter, ppAdapter);
}

BOOL __stdcall DXGIFactoryWrapper::IsCurrent() { return m_target->IsCurrent(); }

BOOL __stdcall DXGIFactoryWrapper::IsWindowedStereoEnabled() { return m_target->IsWindowedStereoEnabled(); }

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChainForHwnd(IUnknown *pDevice, HWND hWnd,
                                                             const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                             const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc,
                                                             IDXGIOutput *pRestrictToOutput,
                                                             IDXGISwapChain1 **ppSwapChain) {
  CComPtr<IDXGISwapChain1> swapChain;

  HRESULT result =
      m_target->CreateSwapChainForHwnd(pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, &swapChain);
  if (SUCCEEDED(result)) {
    m_currentSwapChain = new DXGISwapChainWrapper(pDevice, swapChain, m_D2DFactory);
    *ppSwapChain = m_currentSwapChain;
    return result;
  }

  *ppSwapChain = nullptr;
  return result;
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChainForCoreWindow(IUnknown *pDevice, IUnknown *pWindow,
                                                                   const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                                   IDXGIOutput *pRestrictToOutput,
                                                                   IDXGISwapChain1 **ppSwapChain) {
  return m_target->CreateSwapChainForCoreWindow(pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

HRESULT __stdcall DXGIFactoryWrapper::GetSharedResourceAdapterLuid(HANDLE hResource, LUID *pLuid) {
  return m_target->GetSharedResourceAdapterLuid(hResource, pLuid);
}

HRESULT __stdcall DXGIFactoryWrapper::RegisterStereoStatusWindow(HWND WindowHandle, UINT wMsg, DWORD *pdwCookie) {
  return m_target->RegisterStereoStatusWindow(WindowHandle, wMsg, pdwCookie);
}

HRESULT __stdcall DXGIFactoryWrapper::RegisterStereoStatusEvent(HANDLE hEvent, DWORD *pdwCookie) {
  return m_target->RegisterStereoStatusEvent(hEvent, pdwCookie);
}

void __stdcall DXGIFactoryWrapper::UnregisterStereoStatus(DWORD dwCookie) {
  m_target->UnregisterStereoStatus(dwCookie);
}

HRESULT __stdcall DXGIFactoryWrapper::RegisterOcclusionStatusWindow(HWND WindowHandle, UINT wMsg, DWORD *pdwCookie) {
  return m_target->RegisterOcclusionStatusWindow(WindowHandle, wMsg, pdwCookie);
}

HRESULT __stdcall DXGIFactoryWrapper::RegisterOcclusionStatusEvent(HANDLE hEvent, DWORD *pdwCookie) {
  return m_target->RegisterOcclusionStatusEvent(hEvent, pdwCookie);
}

void __stdcall DXGIFactoryWrapper::UnregisterOcclusionStatus(DWORD dwCookie) {
  m_target->UnregisterOcclusionStatus(dwCookie);
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChainForComposition(IUnknown *pDevice,
                                                                    const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                                    IDXGIOutput *pRestrictToOutput,
                                                                    IDXGISwapChain1 **ppSwapChain) {
  return m_target->CreateSwapChainForComposition(pDevice, pDesc, pRestrictToOutput, ppSwapChain);
}

} // namespace DxWrappers