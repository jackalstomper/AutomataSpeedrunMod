module;

#include <d2d1_2.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include <atlbase.h>

export module FactoryWrapper;

import RefCounter;
import SwapChainWrapper;

namespace DxWrappers {

export class DXGIFactoryWrapper : public IDXGIFactory2 {
    RefCounter m_refCounter;
    CComPtr<IDXGIFactory2> m_target;
    CComPtr<ID2D1Factory2> m_D2DFactory;
    CComPtr<DXGISwapChainWrapper> m_currentSwapChain;

public:
    DXGIFactoryWrapper(CComPtr<IDXGIFactory2> target) {
        m_target = target;

        D2D1_FACTORY_OPTIONS opt = { D2D1_DEBUG_LEVEL_ERROR };
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory2), &opt, (void**)&m_D2DFactory);
    }

    virtual ~DXGIFactoryWrapper() {}

    void toggleDvdMode(bool enabled) {
        if (m_currentSwapChain)
            m_currentSwapChain->toggleDvdMode(enabled);
    }

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == __uuidof(IDXGIFactory2) || riid == __uuidof(IDXGIFactory1) ||
            riid == __uuidof(IDXGIFactory) || riid == __uuidof(IDXGIObject) || riid == __uuidof(IUnknown)) {
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
                m_D2DFactory = nullptr;
                m_currentSwapChain = nullptr;
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

    virtual HRESULT __stdcall EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter) override {
        return m_target->EnumAdapters(Adapter, ppAdapter);
    }

    virtual HRESULT __stdcall MakeWindowAssociation(HWND WindowHandle, UINT Flags) override {
        return m_target->MakeWindowAssociation(WindowHandle, Flags);
    }

    virtual HRESULT __stdcall GetWindowAssociation(HWND* pWindowHandle) override {
        return m_target->GetWindowAssociation(pWindowHandle);
    }

    virtual HRESULT __stdcall CreateSwapChain(IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain) override {
        return m_target->CreateSwapChain(pDevice, pDesc, ppSwapChain);
    }

    virtual HRESULT __stdcall CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter** ppAdapter) override {
        return m_target->CreateSoftwareAdapter(Module, ppAdapter);
    }

    virtual HRESULT __stdcall EnumAdapters1(UINT Adapter, IDXGIAdapter1** ppAdapter) override {
        return m_target->EnumAdapters1(Adapter, ppAdapter);
    }

    virtual BOOL __stdcall IsCurrent() override {
        return m_target->IsCurrent();
    }

    virtual BOOL __stdcall IsWindowedStereoEnabled() override {
        return m_target->IsWindowedStereoEnabled();
    }

    virtual HRESULT __stdcall CreateSwapChainForHwnd(IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain) override {
        CComPtr<IDXGISwapChain1> swapChain;

        HRESULT result = m_target->CreateSwapChainForHwnd(pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, &swapChain);
        if (SUCCEEDED(result)) {
            m_currentSwapChain = new DXGISwapChainWrapper(pDevice, swapChain, m_D2DFactory);
            *ppSwapChain = m_currentSwapChain;
            return result;
        }

        *ppSwapChain = nullptr;
        return result;
    }

    virtual HRESULT __stdcall CreateSwapChainForCoreWindow(IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain) override {
        return m_target->CreateSwapChainForCoreWindow(pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
    }

    virtual HRESULT __stdcall GetSharedResourceAdapterLuid(HANDLE hResource, LUID* pLuid) override {
        return m_target->GetSharedResourceAdapterLuid(hResource, pLuid);
    }

    virtual HRESULT __stdcall RegisterStereoStatusWindow(HWND WindowHandle, UINT wMsg, DWORD* pdwCookie) override {
        return m_target->RegisterStereoStatusWindow(WindowHandle, wMsg, pdwCookie);
    }

    virtual HRESULT __stdcall RegisterStereoStatusEvent(HANDLE hEvent, DWORD* pdwCookie) override {
        return m_target->RegisterStereoStatusEvent(hEvent, pdwCookie);
    }

    virtual void __stdcall UnregisterStereoStatus(DWORD dwCookie) override {
        m_target->UnregisterStereoStatus(dwCookie);
    }

    virtual HRESULT __stdcall RegisterOcclusionStatusWindow(HWND WindowHandle, UINT wMsg, DWORD* pdwCookie) override {
        return m_target->RegisterOcclusionStatusWindow(WindowHandle, wMsg, pdwCookie);
    }

    virtual HRESULT __stdcall RegisterOcclusionStatusEvent(HANDLE hEvent, DWORD* pdwCookie) override {
        return m_target->RegisterOcclusionStatusEvent(hEvent, pdwCookie);
    }

    virtual void __stdcall UnregisterOcclusionStatus(DWORD dwCookie) override {
        m_target->UnregisterOcclusionStatus(dwCookie);
    }

    virtual HRESULT __stdcall CreateSwapChainForComposition(IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain) override {
        return m_target->CreateSwapChainForComposition(pDevice, pDesc, pRestrictToOutput, ppSwapChain);
    }
};

} // namespace DxWrappers