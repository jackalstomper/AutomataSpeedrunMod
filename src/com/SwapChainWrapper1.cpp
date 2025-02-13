#include "SwapChainWrapper1.hpp"
#include "AutomataMod.hpp"
#include "StickState.hpp"
#include "infra/Log.hpp"
#include "infra/constants.hpp"
#include "infra/defs.hpp"
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <random>
#include <string>

#undef max
#include <algorithm>

namespace {

const float SCREEN_WIDTH = 1600.f;
const float SCREEN_HEIGHT = 900.f;
const float WATERMARK_TEXT_SIZE = 15.25f;

const D2D1::ColorF WATERMARK_COLOR = D2D1::ColorF(0.803f, 0.784f, 0.690f, 1.f);
const D2D1::ColorF SHADOW_COLOR = D2D1::ColorF(0.f, 0.f, 0.f, 0.3f);

const D2D1_BITMAP_PROPERTIES1 BITMAP_PROPERTIES = {
		{DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED},
		96.f,
		96.f,
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		nullptr
};

} // namespace

namespace {

u64 frameCounter = 0;

// I couldn't get the ModChecker working here to get the
// pointers for joystickMagnitude so I just did the pointer
// stuff here.
const auto processRamStartAddr = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));

} // namespace

namespace DxWrappers {

void DXGISwapChainWrapper1::renderWatermark() {
	if (!_brush || !_shadowBrush) {
		return;
	}

	_ASSERT(_deviceContext);

	static const D2D1::Matrix3x2F root = D2D1::Matrix3x2F::Identity();
	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> frameDelta = now - _lastFrame;

	ComPtr<IDXGISurface> dxgiBackBuffer;
	HRESULT hr = _target->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void **>(dxgiBackBuffer.GetAddressOf()));
	if (!SUCCEEDED(hr)) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get buffer from swap chain");
		return;
	}

	ComPtr<ID2D1Bitmap1> bitmap;
	hr = _deviceContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &BITMAP_PROPERTIES, bitmap.GetAddressOf());
	if (!SUCCEEDED(hr)) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get bitmap from device context");
		return;
	}

	D2D1_SIZE_F screenSize = bitmap->GetSize();
	if (!_dvdMode) {
		resetLocation(screenSize);
	}

	float xscale = screenSize.width * (1.f / SCREEN_WIDTH) * 1.2;
	float yscale = screenSize.height * (1.f / SCREEN_HEIGHT) * 1.2;
	float heightScale = WATERMARK_TEXT_SIZE * yscale;

	if (!_textFormat) {
		hr = _dwFactory->CreateTextFormat(
				L"Consolas", NULL, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, heightScale,
				L"en-us", _textFormat.GetAddressOf()
		);

		if (!SUCCEEDED(hr)) {
			AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to create IDWriteTextFormat");
			return;
		}
	}

	FLOAT textHeight = _textFormat->GetFontSize();
	FLOAT rectHeight = textHeight * 3; // * 3 for three lines, the mod name and the FPS count
	std::wstring logo = getLogo();
	std::chrono::duration<float, std::milli> frameDeltaMilli = now - _lastFrame;
	std::wstring fpsString = calculateFps(frameDeltaMilli.count());
	std::wstring stickMagnitudeString = getJoystickMagnitude();

	std::wstring *maxLenString;
	if (fpsString.size() > logo.size()) {
		maxLenString = &fpsString;
	} else {
		maxLenString = &logo;
	}

	// Use IDWriteTextLayout to get accurate text rectangle size
	ComPtr<IDWriteTextLayout> layout;
	hr = _dwFactory->CreateTextLayout(
			maxLenString->c_str(), maxLenString->size(), _textFormat.Get(), screenSize.width, screenSize.height,
			layout.GetAddressOf()
	);
	if (!SUCCEEDED(hr)) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to create IDWriteTextLayout");
		return;
	}

	DWRITE_TEXT_METRICS metrics;
	hr = layout->GetMetrics(&metrics);
	if (!SUCCEEDED(hr)) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get DWRITE_TEXT_METRICS");
		return;
	}

	float rectWidth = metrics.width;
	if (_dvdMode) {
		float xBound = _location.x + rectWidth;
		float yBound = _location.y + rectHeight;
		// bounce off edges of screen
		if (_location.x <= 0.f || xBound >= screenSize.width)
			_velocity.x *= -1.f;

		if (_location.y <= 0.f || yBound >= screenSize.height)
			_velocity.y *= -1.f;

		FLOAT deltaTime = frameDelta.count();
		_location.x += _velocity.x * deltaTime;
		_location.y += _velocity.y * deltaTime;

		// Clamp to the bounds if we skipped too far past screen boundry during an
		// update Otherwise we'll get stuck constantly reversing our velocity while
		// being off-screen
		if (_location.x <= 0.f || xBound >= screenSize.width)
			_location.x = std::fmax(std::fmin(_location.x, screenSize.width - 120.f), 0.f);

		if (_location.y <= 0.f || yBound >= screenSize.height)
			_location.y = std::fmax(std::fmin(_location.y, screenSize.height - rectHeight), 0.f);
	}

	D2D1_RECT_F rect = {_location.x, _location.y, _location.x + rectWidth * xscale, _location.y + rectHeight};

	_ASSERT(_textFormat);
	_ASSERT(_shadowBrush);
	_ASSERT(_brush);
	_ASSERT(bitmap);

	ComPtr<ID2D1Image> oldTarget;
	_deviceContext->GetTarget(oldTarget.GetAddressOf());

	_deviceContext->BeginDraw();
	_deviceContext->SetTarget(bitmap.Get());
	_deviceContext->SetTransform(root);

	// Draw shadow behind our text
	_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(2, 2));
	_deviceContext->DrawText(logo.c_str(), logo.size(), _textFormat.Get(), rect, _shadowBrush.Get());
	_deviceContext->SetTransform(root);

	// Draw main text
	_deviceContext->DrawText(logo.c_str(), logo.size(), _textFormat.Get(), rect, _brush.Get());

	// Draw FPS and Frame Counter Shadow
	_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(2, textHeight + 2));
	_deviceContext->DrawText(fpsString.c_str(), fpsString.length(), _textFormat.Get(), rect, _shadowBrush.Get());
	_deviceContext->SetTransform(root);

	// Draw FPS and Frame Counter
	_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0, textHeight));
	_deviceContext->DrawText(fpsString.c_str(), fpsString.length(), _textFormat.Get(), rect, _brush.Get());

	// Draw Joystick Magnitude shadow
	_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(2, 2 + (textHeight * 2)));
	_deviceContext->DrawText(
			stickMagnitudeString.c_str(), stickMagnitudeString.length(), _textFormat.Get(), rect, _shadowBrush.Get()
	);
	_deviceContext->SetTransform(root);

	// Draw Joystick Magnitude
	_deviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0, textHeight * 2));
	_deviceContext->DrawText(
			stickMagnitudeString.c_str(), stickMagnitudeString.length(), _textFormat.Get(), rect, _brush.Get()
	);

	_deviceContext->EndDraw();
	_deviceContext->SetTarget(oldTarget.Get());
	_lastFrame = now;
}

// Roates velocity in a random direction
void DXGISwapChainWrapper1::rotateVelocity() {
	static std::random_device rd;
	static const FLOAT pi = 3.141592f;
	static std::mt19937 eng(rd());
	static std::uniform_real_distribution<float> dist(-pi, pi);

	const FLOAT angle = dist(eng);
	_velocity.x = _velocity.x * std::cos(angle) - _velocity.y * std::sin(angle);
	_velocity.y = _velocity.x * std::sin(angle) - _velocity.y * std::cos(angle);
}

// Resets location to the default position
void DXGISwapChainWrapper1::resetLocation(D2D1_SIZE_F &screenSize) {
	_location.x = screenSize.width * 0.0495f;
	_location.y = screenSize.height * 0.105f;
	_velocity = {0.f, 200.f};
}

// Calculates current frame rate and returns formatted FPS display string
// frameDelta must be in milliseconds
std::wstring DXGISwapChainWrapper1::calculateFps(float frameDelta) {
	_frameTimes[_frameTimeIndex % _frameTimes.size()] = frameDelta;
	++_frameTimeIndex;

	// Get average FPS
	float total = 0.f;
	for (float f : _frameTimes)
		total += f;

	if (total < 1.f)
		total = 1.f;

	float fps = 1000 * _frameTimes.size() / total;
	return fmt::format(L"{:.1f}FPS {:.2f}ms {}", fps, frameDelta, frameCounter);
}

std::wstring DXGISwapChainWrapper1::getLogo() {
	std::wstring activatedString;
	auto checker = AutomataMod::ModChecker::get();
	if (checker && checker->getModActive()) {
		activatedString = L"VC3Mod";
	} else {
		activatedString = L"Vanilla";
	}

	if (checker && checker->getInMenu()) {
		return fmt::format(
				L"SpeedrunMod {} ({}), Press Home or hold X & Y to toggle mod", AutomataMod::Constants::getWVersion(),
				activatedString
		);
	}

	return fmt::format(L"SpeedrunMod {} ({})", AutomataMod::Constants::getWVersion(), activatedString);
} // namespace DxWrappers

std::wstring DXGISwapChainWrapper1::getJoystickMagnitude() {
	using namespace AutomataMod;
	StickState *state = reinterpret_cast<StickState *>(processRamStartAddr + 0x13FCC10);
	float magnitude = std::sqrt(state->leftY * state->leftY + state->leftX * state->leftX) * 0.001f;
	wchar_t mode = L'?';
	if (auto checker = AutomataMod::ModChecker::get()) {
		switch (checker->getWindowMode()) {
		case 0:
			mode = L'F';
			break;
		case 1:
			mode = L'W';
			break;
		case 2:
			mode = L'B';
			break;
		default:
			mode = L'?';
			break;
		}
	}
	return fmt::format(L"Stick: {:.3f} ({})", magnitude, mode);
}

DXGISwapChainWrapper1::~DXGISwapChainWrapper1() {}

DXGISwapChainWrapper1::DXGISwapChainWrapper1(
		IUnknown *pDevice, ComPtr<IDXGISwapChain1> target, ComPtr<ID2D1Factory2> d2dFactory
) {
	_target = target;
	_dvdMode = false;

	ComPtr<IDXGIDevice> dxgiDevice;
	HRESULT queryResult =
			pDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(dxgiDevice.GetAddressOf()));
	if (!SUCCEEDED(queryResult)) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed to get DXGIDevice. Error code: {}", queryResult);
		return;
	}

	queryResult = d2dFactory->CreateDevice(dxgiDevice.Get(), _D2DDevice.GetAddressOf());
	if (!SUCCEEDED(queryResult)) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateDevice. Error code: {}", queryResult);
		return;
	}

	queryResult = _D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, _deviceContext.GetAddressOf());
	if (!SUCCEEDED(queryResult)) {
		AutomataMod::log(
				AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateDeviceContext. Error code: {}", queryResult
		);
		return;
	}

	queryResult = _deviceContext->CreateSolidColorBrush(WATERMARK_COLOR, _brush.GetAddressOf());
	if (!SUCCEEDED(queryResult)) {
		AutomataMod::log(
				AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateSolidColorBrush. Error code: {}", queryResult
		);
		return;
	}

	queryResult = _deviceContext->CreateSolidColorBrush(SHADOW_COLOR, _shadowBrush.GetAddressOf());
	if (!SUCCEEDED(queryResult)) {
		AutomataMod::log(
				AutomataMod::LogLevel::LOG_ERROR, "Failed calling CreateSolidColorBrush. Error code: {}", queryResult
		);
		return;
	}

	queryResult = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(_dwFactory.GetAddressOf())
	);

	if (!SUCCEEDED(queryResult)) {
		AutomataMod::log(AutomataMod::LogLevel::LOG_ERROR, "Failed calling IDWriteFactory. Error code: {}", queryResult);
		return;
	}

	_lastFrame = std::chrono::high_resolution_clock::now();
}

void DXGISwapChainWrapper1::toggleDvdMode(bool enabled) {
	rotateVelocity(); // Rotate the velocity in a random direction
	this->_dvdMode = enabled;
}

HRESULT __stdcall DXGISwapChainWrapper1::QueryInterface(REFIID riid, void **ppvObject) {
	if (riid == __uuidof(IDXGISwapChain1) || riid == __uuidof(IDXGISwapChain) || riid == __uuidof(IDXGIDeviceSubObject) ||
			riid == __uuidof(IDXGIObject) || riid == __uuidof(IUnknown)) {
		this->AddRef();
		*ppvObject = this;
		return S_OK;
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG __stdcall DXGISwapChainWrapper1::AddRef() { return _refCounter.incrementRef(); }

ULONG __stdcall DXGISwapChainWrapper1::Release() {
	return _refCounter.decrementRef([this](ULONG refCount) {
		if (refCount == 0) {
			AutomataMod::log(AutomataMod::LogLevel::LOG_DEBUG, "DXGISwapChainWrapper1 ref count is zero. clearing.");
			_shadowBrush = nullptr;
			_brush = nullptr;
			_textFormat = nullptr;
			_D2DDevice = nullptr;
			_deviceContext = nullptr;
			_target = nullptr;
		}
	});
}

HRESULT __stdcall DXGISwapChainWrapper1::SetPrivateData(REFGUID Name, UINT DataSize, const void *pData) {
	return _target->SetPrivateData(Name, DataSize, pData);
}

HRESULT __stdcall DXGISwapChainWrapper1::SetPrivateDataInterface(REFGUID Name, const IUnknown *pUnknown) {
	return _target->SetPrivateDataInterface(Name, pUnknown);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetPrivateData(REFGUID Name, UINT *pDataSize, void *pData) {
	return _target->GetPrivateData(Name, pDataSize, pData);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetParent(REFIID riid, void **ppParent) {
	return _target->GetParent(riid, ppParent);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetDevice(REFIID riid, void **ppDevice) {
	return _target->GetDevice(riid, ppDevice);
}

HRESULT __stdcall DXGISwapChainWrapper1::Present(UINT SyncInterval, UINT Flags) {
	++frameCounter;
	renderWatermark();
	return _target->Present(SyncInterval, Flags);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetBuffer(UINT Buffer, REFIID riid, void **ppSurface) {
	return _target->GetBuffer(Buffer, riid, ppSurface);
}

HRESULT __stdcall DXGISwapChainWrapper1::SetFullscreenState(BOOL Fullscreen, IDXGIOutput *pTarget) {
	return _target->SetFullscreenState(Fullscreen, pTarget);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetFullscreenState(BOOL *pFullscreen, IDXGIOutput **ppTarget) {
	return _target->GetFullscreenState(pFullscreen, ppTarget);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc) { return _target->GetDesc(pDesc); }

HRESULT __stdcall DXGISwapChainWrapper1::ResizeBuffers(
		UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags
) {
	_textFormat = nullptr; // Trigger a text format refresh
	return _target->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

HRESULT __stdcall DXGISwapChainWrapper1::ResizeTarget(const DXGI_MODE_DESC *pNewTargetParameters) {
	return _target->ResizeTarget(pNewTargetParameters);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetContainingOutput(IDXGIOutput **ppOutput) {
	return _target->GetContainingOutput(ppOutput);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats) {
	return _target->GetFrameStatistics(pStats);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetLastPresentCount(UINT *pLastPresentCount) {
	return _target->GetLastPresentCount(pLastPresentCount);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetDesc1(DXGI_SWAP_CHAIN_DESC1 *pDesc) { return _target->GetDesc1(pDesc); }

HRESULT __stdcall DXGISwapChainWrapper1::GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pDesc) {
	return _target->GetFullscreenDesc(pDesc);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetHwnd(HWND *pHwnd) { return _target->GetHwnd(pHwnd); }

HRESULT __stdcall DXGISwapChainWrapper1::GetCoreWindow(REFIID refiid, void **ppUnk) {
	return _target->GetCoreWindow(refiid, ppUnk);
}

HRESULT __stdcall DXGISwapChainWrapper1::Present1(
		UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS *pPresentParameters
) {
	return _target->Present1(SyncInterval, PresentFlags, pPresentParameters);
}

BOOL __stdcall DXGISwapChainWrapper1::IsTemporaryMonoSupported() { return _target->IsTemporaryMonoSupported(); }

HRESULT __stdcall DXGISwapChainWrapper1::GetRestrictToOutput(IDXGIOutput **ppRestrictToOutput) {
	return _target->GetRestrictToOutput(ppRestrictToOutput);
}

HRESULT __stdcall DXGISwapChainWrapper1::SetBackgroundColor(const DXGI_RGBA *pColor) {
	return _target->SetBackgroundColor(pColor);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetBackgroundColor(DXGI_RGBA *pColor) {
	return _target->GetBackgroundColor(pColor);
}

HRESULT __stdcall DXGISwapChainWrapper1::SetRotation(DXGI_MODE_ROTATION Rotation) {
	return _target->SetRotation(Rotation);
}

HRESULT __stdcall DXGISwapChainWrapper1::GetRotation(DXGI_MODE_ROTATION *pRotation) {
	return _target->GetRotation(pRotation);
}

} // namespace DxWrappers
