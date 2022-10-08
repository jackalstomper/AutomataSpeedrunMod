#pragma once

#include "RefCounter.hpp"
#include <array>
#include <chrono>
#include <d2d1_2.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace DxWrappers {

using namespace Microsoft::WRL;

class DXGISwapChainWrapper : public IDXGISwapChain1 {
	RefCounter m_refCounter;
	ComPtr<IDXGISwapChain1> m_target;
	ComPtr<ID2D1DeviceContext> m_deviceContext;
	ComPtr<ID2D1Device> m_D2DDevice;
	ComPtr<IDWriteTextFormat> m_textFormat;
	ComPtr<ID2D1SolidColorBrush> m_brush;
	ComPtr<ID2D1SolidColorBrush> m_shadowBrush;

	bool m_dvdMode; // true when watermark should bounce around
	D2D1_VECTOR_2F m_location;
	D2D1_VECTOR_2F m_velocity;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrame;

	// FPS Display
	std::array<float, 60> m_frameTimes;
	size_t m_frameTimeIndex;

	void renderWatermark();
	void rotateVelocity();
	void resetLocation(D2D1_SIZE_F &screenSize);
	std::wstring calculateFps(float frameDelta);

public:
	DXGISwapChainWrapper(IUnknown *pDevice, ComPtr<IDXGISwapChain1> target, ComPtr<ID2D1Factory2> d2dFactory);
	virtual ~DXGISwapChainWrapper();
	void toggleDvdMode(bool enabled);

	virtual HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
	virtual ULONG __stdcall AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;
	virtual HRESULT __stdcall SetPrivateData(REFGUID Name, UINT DataSize, const void *pData) override;
	virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID Name, const IUnknown *pUnknown) override;
	virtual HRESULT __stdcall GetPrivateData(REFGUID Name, UINT *pDataSize, void *pData) override;
	virtual HRESULT __stdcall GetParent(REFIID riid, void **ppParent) override;
	virtual HRESULT __stdcall GetDevice(REFIID riid, void **ppDevice) override;
	virtual HRESULT __stdcall Present(UINT SyncInterval, UINT Flags) override;
	virtual HRESULT __stdcall GetBuffer(UINT Buffer, REFIID riid, void **ppSurface) override;
	virtual HRESULT __stdcall SetFullscreenState(BOOL Fullscreen, IDXGIOutput *pTarget) override;
	virtual HRESULT __stdcall GetFullscreenState(BOOL *pFullscreen, IDXGIOutput **ppTarget) override;
	virtual HRESULT __stdcall GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc) override;
	virtual HRESULT __stdcall ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
																					UINT SwapChainFlags) override;
	virtual HRESULT __stdcall ResizeTarget(const DXGI_MODE_DESC *pNewTargetParameters) override;
	virtual HRESULT __stdcall GetContainingOutput(IDXGIOutput **ppOutput) override;
	virtual HRESULT __stdcall GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats) override;
	virtual HRESULT __stdcall GetLastPresentCount(UINT *pLastPresentCount) override;
	virtual HRESULT __stdcall GetDesc1(DXGI_SWAP_CHAIN_DESC1 *pDesc) override;
	virtual HRESULT __stdcall GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pDesc) override;
	virtual HRESULT __stdcall GetHwnd(HWND *pHwnd) override;
	virtual HRESULT __stdcall GetCoreWindow(REFIID refiid, void **ppUnk) override;
	virtual HRESULT __stdcall Present1(UINT SyncInterval, UINT PresentFlags,
																		 const DXGI_PRESENT_PARAMETERS *pPresentParameters) override;
	virtual BOOL __stdcall IsTemporaryMonoSupported(void) override;
	virtual HRESULT __stdcall GetRestrictToOutput(IDXGIOutput **ppRestrictToOutput) override;
	virtual HRESULT __stdcall SetBackgroundColor(const DXGI_RGBA *pColor) override;
	virtual HRESULT __stdcall GetBackgroundColor(DXGI_RGBA *pColor) override;
	virtual HRESULT __stdcall SetRotation(DXGI_MODE_ROTATION Rotation) override;
	virtual HRESULT __stdcall GetRotation(DXGI_MODE_ROTATION *pRotation) override;
};

} // namespace DxWrappers
