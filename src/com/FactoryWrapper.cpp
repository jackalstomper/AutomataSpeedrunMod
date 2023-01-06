#include "FactoryWrapper.hpp"
#include "infra/Log.hpp"

namespace DxWrappers {

DXGIFactoryWrapper::DXGIFactoryWrapper(ComPtr<IDXGIFactory2> target) {
	_target = target;
	D2D1_FACTORY_OPTIONS opt = {D2D1_DEBUG_LEVEL_ERROR};
	HRESULT hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory2), &opt,
			reinterpret_cast<void **>(_D2DFactory.GetAddressOf())
	);

	if (!SUCCEEDED(hr))
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "D2D1CreateFactory failed with code {}", hr);
}

DXGIFactoryWrapper::~DXGIFactoryWrapper() {}

void DXGIFactoryWrapper::toggleDvdMode(bool enabled) {
	if (_currentSwapChain)
		_currentSwapChain->toggleDvdMode(enabled);
}

void DXGIFactoryWrapper::setWindowMode(int mode) {
	if (_currentSwapChain)
		_currentSwapChain->setWindowMode(mode);
}

void DXGIFactoryWrapper::setModActive(bool active) {
	if (_currentSwapChain)
		_currentSwapChain->setModActive(active);
}

void DXGIFactoryWrapper::setInMenu(bool inMenu) {
	if (_currentSwapChain)
		_currentSwapChain->setInMenu(inMenu);
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

ULONG __stdcall DXGIFactoryWrapper::AddRef() { return _refCounter.incrementRef(); }

ULONG __stdcall DXGIFactoryWrapper::Release() {
	return _refCounter.decrementRef([this](ULONG refCount) {
		if (refCount == 0) {
			AutomataMod::log(AutomataMod::LogLevel::LOG_DEBUG, "DXGIFactoryWrapper ref count is zero. clearing.");
			_currentSwapChain = nullptr;
			_D2DFactory = nullptr;
			_target = nullptr;
		}
	});
}

HRESULT __stdcall DXGIFactoryWrapper::SetPrivateData(REFGUID Name, UINT DataSize, const void *pData) {
	return _target->SetPrivateData(Name, DataSize, pData);
}

HRESULT __stdcall DXGIFactoryWrapper::SetPrivateDataInterface(REFGUID Name, const IUnknown *pUnknown) {
	return _target->SetPrivateDataInterface(Name, pUnknown);
}

HRESULT __stdcall DXGIFactoryWrapper::GetPrivateData(REFGUID Name, UINT *pDataSize, void *pData) {
	return _target->GetPrivateData(Name, pDataSize, pData);
}

HRESULT __stdcall DXGIFactoryWrapper::GetParent(REFIID riid, void **ppParent) {
	return _target->GetParent(riid, ppParent);
}

HRESULT __stdcall DXGIFactoryWrapper::EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter) {
	return _target->EnumAdapters(Adapter, ppAdapter);
}

HRESULT __stdcall DXGIFactoryWrapper::MakeWindowAssociation(HWND WindowHandle, UINT Flags) {
	return _target->MakeWindowAssociation(WindowHandle, Flags);
}

HRESULT __stdcall DXGIFactoryWrapper::GetWindowAssociation(HWND *pWindowHandle) {
	return _target->GetWindowAssociation(pWindowHandle);
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChain(
		IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain
) {
	return _target->CreateSwapChain(pDevice, pDesc, ppSwapChain);
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter **ppAdapter) {
	return _target->CreateSoftwareAdapter(Module, ppAdapter);
}

HRESULT __stdcall DXGIFactoryWrapper::EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter) {
	return _target->EnumAdapters1(Adapter, ppAdapter);
}

BOOL __stdcall DXGIFactoryWrapper::IsCurrent() { return _target->IsCurrent(); }

BOOL __stdcall DXGIFactoryWrapper::IsWindowedStereoEnabled() { return _target->IsWindowedStereoEnabled(); }

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChainForHwnd(
		IUnknown *pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 *pDesc,
		const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc, IDXGIOutput *pRestrictToOutput,
		IDXGISwapChain1 **ppSwapChain
) {
	AutomataMod::log(AutomataMod::LogLevel::LOG_DEBUG, "CreateSwapChainForHwnd called");
	ComPtr<IDXGISwapChain1> swapChain;
	HRESULT result = _target->CreateSwapChainForHwnd(
			pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, swapChain.GetAddressOf()
	);

	if (!SUCCEEDED(result)) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to create swapchain in CreateSwapChainForHwnd");
		*ppSwapChain = nullptr;
		return result;
	}

	_currentSwapChain = new DXGISwapChainWrapper(pDevice, swapChain, _D2DFactory);
	_currentSwapChain->AddRef();
	*ppSwapChain = _currentSwapChain.get();
	return result;
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChainForCoreWindow(
		IUnknown *pDevice, IUnknown *pWindow, const DXGI_SWAP_CHAIN_DESC1 *pDesc, IDXGIOutput *pRestrictToOutput,
		IDXGISwapChain1 **ppSwapChain
) {
	return _target->CreateSwapChainForCoreWindow(pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

HRESULT __stdcall DXGIFactoryWrapper::GetSharedResourceAdapterLuid(HANDLE hResource, LUID *pLuid) {
	return _target->GetSharedResourceAdapterLuid(hResource, pLuid);
}

HRESULT __stdcall DXGIFactoryWrapper::RegisterStereoStatusWindow(HWND WindowHandle, UINT wMsg, DWORD *pdwCookie) {
	return _target->RegisterStereoStatusWindow(WindowHandle, wMsg, pdwCookie);
}

HRESULT __stdcall DXGIFactoryWrapper::RegisterStereoStatusEvent(HANDLE hEvent, DWORD *pdwCookie) {
	return _target->RegisterStereoStatusEvent(hEvent, pdwCookie);
}

void __stdcall DXGIFactoryWrapper::UnregisterStereoStatus(DWORD dwCookie) { _target->UnregisterStereoStatus(dwCookie); }

HRESULT __stdcall DXGIFactoryWrapper::RegisterOcclusionStatusWindow(HWND WindowHandle, UINT wMsg, DWORD *pdwCookie) {
	return _target->RegisterOcclusionStatusWindow(WindowHandle, wMsg, pdwCookie);
}

HRESULT __stdcall DXGIFactoryWrapper::RegisterOcclusionStatusEvent(HANDLE hEvent, DWORD *pdwCookie) {
	return _target->RegisterOcclusionStatusEvent(hEvent, pdwCookie);
}

void __stdcall DXGIFactoryWrapper::UnregisterOcclusionStatus(DWORD dwCookie) {
	_target->UnregisterOcclusionStatus(dwCookie);
}

HRESULT __stdcall DXGIFactoryWrapper::CreateSwapChainForComposition(
		IUnknown *pDevice, const DXGI_SWAP_CHAIN_DESC1 *pDesc, IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain
) {
	return _target->CreateSwapChainForComposition(pDevice, pDesc, pRestrictToOutput, ppSwapChain);
}

} // namespace DxWrappers
