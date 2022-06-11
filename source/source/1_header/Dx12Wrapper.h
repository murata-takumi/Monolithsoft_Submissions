#include "Application.h"
#include "Functions.h"

/// <summary>
/// カメラをどちらの方に回転させるか決定する列挙体
/// </summary>
enum Degree
{
	Azimth,			//方位角
	Elevation,		//仰角
};

/// <summary>
/// 描画命令を実行するためのオブジェクト等を管理するクラス
/// </summary>
class SphericalCoordinates;
class Dx12Wrapper
{
	SIZE _winSize;	//ウィンドウサイズ

	XMFLOAT3 _initialPos;	//カメラの初期座標

	template<typename T>
	using ComPtr = ComPtr<T>;

	unique_ptr<SphericalCoordinates> _coordinates;	//カメラ座標用インスタンス

	//DXGI周り
	ComPtr<IDXGIFactory6> _dxgiFactory;	//ファクトリー
	ComPtr<ID3D12Device> _dev;			//デバイス

	//DX12(コマンド)周り
	ComPtr<ID3D12CommandAllocator> _cmdAllocator;	//コマンドアロケータ
	ComPtr<ID3D12GraphicsCommandList> _cmdList;		//コマンドリスト
	ComPtr<ID3D12CommandQueue> _cmdQueue;			//コマンドキュー
	ComPtr<IDXGISwapChain4> _swapchain;				//スワップチェーン

	//表示関連のバッファ回り
	vector<ID3D12Resource*> _backBuffers;		//レンダーターゲット用バッファー
	ComPtr<ID3D12DescriptorHeap> _rtvHeap;		//レンダーターゲットビュー用ヒープ
	ComPtr<ID3D12Resource> _depthBuffer;		//深度ステンシル用バッファー
	ComPtr<ID3D12DescriptorHeap> _dsvHeap;		//深度ステンシルビュー用ヒープ

	//unique_ptr<class>はclassを元とするインスタンスが1つしかない状態とするポインタ
	unique_ptr<D3D12_VIEWPORT> _viewPort = {};	//ビューポート
	unique_ptr<D3D12_RECT> _rect = {};			//シザー矩形

	//ビュープロジェクション用定数バッファ周り
	ComPtr<ID3D12Resource> _sceneConstBuff;				//ビュー・プロジェクション用バッファー
	ComPtr<ID3D12DescriptorHeap> _sceneDescHeap;		//ビュー・プロジェクション用ディスクリプタヒープ

	//歪みテクスチャ周り
	ComPtr<ID3D12DescriptorHeap> _effectSRVHeap;	//歪みテクスチャ用ヒープ
	ComPtr<ID3D12Resource> _effectTexBuffer;		//歪みテクスチャ用バッファー
	ComPtr<ID3D12DescriptorHeap> _factorCBVHeap;	//歪みデータ用ヒープ
	ComPtr<ID3D12Resource> _factorConstBuff;		//歪みデータ用バッファー

	/// <summary>
	/// シェーダーに歪み・フェードイン／アウトデータを渡すための構造体及びインスタンス
	/// </summary>
	struct Factor 
	{
		float dist;			//歪みデータ
		float fade;			//フェードイン／アウトデータ
	};
	Factor* _mappedFactor = nullptr;

	/// <summary>
	/// スロットに流し込むデータの構造体及びインスタンス
	/// 各行列を別個に流し込むのではなく、構造体としてまとめる
	/// </summary>
	struct SceneData
	{
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT3 eye;
	};
	SceneData* _mappedScene;

	ComPtr<ID3D12Fence> _fence;					//フェンス
	UINT64 _fenceVal;							//初期化値

	CD3DX12_RESOURCE_BARRIER _barrier;			//リソースバリア用変数

	XMFLOAT3 _eye;		//視点座標
	XMFLOAT3 _target;	//注視点座標
	XMFLOAT3 _up;		//上座標

	float _deltaTime;	//フレーム間の時間

	float _dist;		//画面を歪ませるかどうかを決めるデータ
	float _fade;		//フェードイン／アウトするかどうかを決めるデータ

	float _start, _end;	//フェードイン／アウトで線形補完する際の初期値、最終値

	//テクスチャロード用ラムダ式を格納する型の定義
	//function<R,ArgsTypes>とは、Rを返り値に、ArgsTypesを引数リストに持つ関数を保持できるクラステンプレート
	//ここではconst wstring&, TexMetadata*,ScratchImage&を引数に、HRESULTを返り値とした「関数の型」を定義している
	using LoadLambda_t = function<
		HRESULT(const wstring& path, TexMetadata*, ScratchImage&)>;

	map<string, ID3D12Resource*> _resourceTable;	//stringをkey,リソースをvalueとした連想配列

	HRESULT InitializeDXGIDevice();					//デバイス関連を初期化する関数

	HRESULT InitializeCommand();					//コマンド関連を初期化する関数

	HRESULT CreateSwapChain(const HWND& hwnd);		//スワップチェーンを作成する関数

	HRESULT CreateRenderTargetsView();				//レンダーターゲットを作成する関数

	HRESULT CreateSceneView();						//ビュープロジェクション用ビューを作成する関数

	HRESULT CreateDepthStencilView();				//深度ステンシルビューを作成する関数

	HRESULT CreateEffectBufferAndView();			//歪みテクスチャ用ヒープ・バッファーを作成する関数

	HRESULT CreateFactorBufferAndView();			//エフェクト適用を決めるデータ用のヒープ・バッファーを作成する関数

	void CreateTextureLoaderTable();				//テクスチャロート用テーブルを作成する関数
public:
	map<string, LoadLambda_t> _loadLambdaTable;		//stringをkey,LoadLambda_tをvalueとした連想配列

	Dx12Wrapper(HWND hwnd, float deltatime);		//コンストラクタ
	~Dx12Wrapper();									//デコンストラクタ

	void BarrierTransition(				//リソースを遷移させる関数
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, 
		D3D12_RESOURCE_STATES after);

	void TranslateSphericalCoordinates(int x);							//カメラを近付ける・遠ざける関数
	void RotateSphericalCoordinates(Degree rotation, int direction);	//カメラを回転させる関数
	void ResetSphericalCoordinates();									//カメラの位置を初期化する関数

	void SetScene();			//ビュープロジェクション用ビューをコマンドリストにセットする関数

	void BeginGameDraw();		//ゲーム用リソースの遷移(STATE_PRESENT→RENDER_TARGET)・RTVのセットを実行する関数
	void EndGameDraw();			//ゲーム用リソースの遷移(RENDER_TARGET→STATE_PRESENT)を実行する関数

	void WaitForCommandQueue();	//処理の同期待ちを行う関数

	ID3D12Resource* LoadTextureFromFile(const char* texPath);	//テクスチャを読み込む関数

	ID3D12DescriptorHeap* CreateDescriptorHeapForSpriteFont();	//DXTK12用のヒープを作成する関数

	void SetPipelines(ID3D12RootSignature* rootSignature,	//ルートシグネチャ・パイプライン・描画方法をセットする関数
		ID3D12PipelineState* pipeline, 
		D3D12_PRIMITIVE_TOPOLOGY topology);

	void UpdateFactor();	//画面歪み・フェードイン／アウトデータをシェーダーに反映する関数

	void SetDist(float dist);		//画面を歪ませるデータを設定する関数

	void Fade(function<void()> func,float start,float end);		//フェードイン／アウトを実行する関数

	ID3D12Device* Device();										//デバイスを返す関数
	IDXGISwapChain4* Swapchain();								//スワップチェーンを返す関数
	ID3D12GraphicsCommandList* CommandList();					//コマンドリストを返す関数
	ID3D12CommandQueue* CommandQueue();							//コマンドキューを返す関数
	ID3D12Resource* BackBuffer() const;							//バックバッファー（1枚目）を返す関数
	ID3D12DescriptorHeap* RTVHeap() const;						//RTVヒープを返す関数
	ID3D12DescriptorHeap* DSVHeap() const;						//深度ステンシルヒープを返す関数
	ID3D12DescriptorHeap* EffectSRVHeap() const;				//エフェクト用ヒープを返す関数
	ID3D12DescriptorHeap* FactorCBVHeap() const;				//因数用ヒープを返す関数
	D3D12_VIEWPORT* ViewPort() const;							//ビューポートを返す関数
	D3D12_RECT* Rect() const;									//シザー矩形を返す関数

private:

};

