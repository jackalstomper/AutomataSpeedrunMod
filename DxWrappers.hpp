#pragma once

#include <d2d1_1.h>
#include <dwrite.h>
#include "Log.hpp"

namespace DxWrappers {

class DXGISwapChainWrapper : public IDXGISwapChain {
    const WCHAR* VC3_NAME = L"VC3 Mod 1.4";

    IDXGISwapChain* m_target;
    ID2D1DeviceContext* m_deviceContext;
    IDWriteTextFormat* m_textFormat;
    ID2D1SolidColorBrush* m_brush;

    void renderWatermark() {
        FLOAT x = 20.f, y = 20.f;
        D2D1_RECT_F r = { x, y, x + 300.f, y + m_textFormat->GetFontSize() };

        m_deviceContext->BeginDraw();
        m_deviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
        m_deviceContext->DrawTextA(VC3_NAME, (UINT)wcslen(VC3_NAME), m_textFormat, r, m_brush);

        if (m_deviceContext->EndDraw() == D2DERR_RECREATE_TARGET) {
            AutomataMod::showErrorBox("Failed to render, need to remake frame!");
        }
    }

public:
    DXGISwapChainWrapper(IDXGISwapChain* target, ID2D1DeviceContext* deviceContext, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* brush) {
        m_target = target;
        m_deviceContext = deviceContext;
        m_textFormat = textFormat;
        m_brush = brush;
    }

    virtual ~DXGISwapChainWrapper() {
        m_target->Release();
    }

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
        return m_target->QueryInterface(riid, ppvObject);
    }

    virtual ULONG __stdcall AddRef() override {
        return m_target->AddRef();
    }

    virtual ULONG __stdcall Release() override {
        return m_target->Release();
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
        return m_target->Present(SyncInterval, Flags);;
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
};

class DXGIFactoryWrapper : public IDXGIFactory {
    IDXGIFactory* m_target;
    ID2D1Factory1* m_D2DFactory;
    IDWriteFactory* m_DWFactory;
    ID2D1Device* m_D2DDevice;
    ID2D1DeviceContext* m_D2DDeviceContext;
    ID2D1SolidColorBrush* m_Brush;
    ID2D1Bitmap1* m_Target;
    IDWriteTextFormat* m_DWTextFormat;

    const D2D1_BITMAP_PROPERTIES1 m_d2dBitmapProperties = {
        { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
        96.f,
        96.f,
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        nullptr
    };

public:
    DXGIFactoryWrapper(IDXGIFactory* target) {
        m_target = target;
        m_D2DFactory = nullptr;
        m_DWFactory = nullptr;
        m_D2DDevice = nullptr;
        m_D2DDeviceContext = nullptr;
        m_Brush = nullptr;
        m_Target = nullptr;
        m_DWTextFormat = nullptr;

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

    virtual ~DXGIFactoryWrapper() {
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
            *ppSwapChain = new DXGISwapChainWrapper(swapChain, m_D2DDeviceContext, m_DWTextFormat, m_Brush);
            IDXGISurface* pBackBuffer = nullptr;
            if (m_Target)
                m_Target->Release();

            swapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pBackBuffer);
            m_D2DDeviceContext->CreateBitmapFromDxgiSurface(pBackBuffer, &m_d2dBitmapProperties, &m_Target);
            pBackBuffer->Release();
            m_D2DDeviceContext->SetTarget(m_Target);
        }

        return result;
    }

    virtual HRESULT __stdcall CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter** ppAdapter) override {
        return m_target->CreateSoftwareAdapter(Module, ppAdapter);
    }

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
        return m_target->QueryInterface(riid, ppvObject);
    }

    virtual ULONG __stdcall AddRef() override {
        return m_target->AddRef();
    }

    virtual ULONG __stdcall Release() override {
        return m_target->Release();
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
};

} // namespace DxWrappers