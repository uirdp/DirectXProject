#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include "ComPtr.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Engine
{
public:
	enum { FRAME_BUFFER_COUNT = 2 };

public:
	bool Init(HWND hWnd, UINT windowWidth, UINT windowHeight);
	bool InitIrradianceMap();
	void DrawIrradianceMap();

	void BeginRender();
	void EndRender();

	void UpdateFrameCount();

public: // Getters
	ID3D12Device6* Device();
	ID3D12GraphicsCommandList* CommandList();
	UINT CurrentBackBufferIndex();
	UINT FrameCount();

private: // DX12初期化
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateSwapChain();
	bool CreateCommandList();
	bool CreateFence();
	void CreateViewPort();
	D3D12_VIEWPORT CreateViewPort(UINT height, UINT width);
	void CreateScissorRect();
	D3D12_RECT CreateScissorRect(UINT height, UINT width);

private:
	HWND m_hWnd;
	UINT m_FrameBufferWidth = 0;
	UINT m_FrameBufferHeight = 0;
	UINT m_CurrentBackBufferIndex = 0;
	UINT m_FrameCount = 0;

	ComPtr<ID3D12Device6> m_pDevice = nullptr; // デバイス
	ComPtr<ID3D12CommandQueue> m_pQueue = nullptr; // コマンドキュー
	ComPtr<IDXGISwapChain3> m_pSwapChain = nullptr; // スワップチェイン
	ComPtr<ID3D12CommandAllocator> m_pAllocator[FRAME_BUFFER_COUNT] = { nullptr }; // コマンドアロケーたー
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr; // コマンドリスト
	HANDLE m_fenceEvent = nullptr; // フェンスで使うイベント
	ComPtr<ID3D12Fence> m_pFence = nullptr; // フェンス
	UINT64 m_fenceValue[FRAME_BUFFER_COUNT]; // フェンスの値（ダブルバッファリング用に2個）
	D3D12_VIEWPORT m_Viewport; // ビューポート
	D3D12_RECT m_Scissor; // シザー矩形

private: // 描画に使うオブジェクトとその生成関数たち
	bool CreateRenderTarget(); // レンダーターゲットを生成
	bool CreateDepthStencil(); // 深度ステンシルバッファを生成

	UINT m_RtvDescriptorSize = 0; // レンダーターゲットビューのディスクリプタサイズ
	ComPtr<ID3D12DescriptorHeap> m_pRtvHeap = nullptr; // レンダーターゲットのディスクリプタヒープ
	ComPtr<ID3D12Resource> m_pRenderTargets[FRAME_BUFFER_COUNT] = { nullptr }; // レンダーターゲット（ダブルバッファリングするので2個）

	UINT m_DsvDescriptorSize = 0; // 深度ステンシルのディスクリプターサイズ
	ComPtr<ID3D12DescriptorHeap> m_pDsvHeap = nullptr; // 深度ステンシルのディスクリプタヒープ
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer = nullptr; // 深度ステンシルバッファ（こっちは1つでいい）

private: // 描画ループで使用するもの
	ID3D12Resource* m_currentRenderTarget = nullptr; // 現在のフレームのレンダーターゲットを一時的に保存しておく関数
	void WaitRender(); // 描画完了を待つ処理

};

extern Engine* g_Engine;