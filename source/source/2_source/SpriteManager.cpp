#include "SpriteManager.h"
#include "BinaryFile.h"
#include "Dx12Wrapper.h"
#include "Functions.h"

const int ORIGIN = 0;
const int START_TOP = 480;			//スタートボタンの上座標
const int END_TOP = 555;			//終了ボタンの上座標
const int BUTTON_WIDTH = 200;		//ボタン幅
const int BUTTON_HEIGHT = 25;		//ボタン高さ
const int LOADING_WIDTH = 480;		//ロード画面で表示するアイコンの幅
const int LOADIN_HEIGHT = 270;		//ロード画面で表示するアイコンの高さ
const int TITLE_DIFF = 180;			//画面全体からタイトル画面への差分
const int HPGAGE_DIFF = 45;			//左上を基準としたHPゲージの差分

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="dx12">Dx12Wrapperインスタンス</param>
/// <param name="width">画面幅</param>
/// <param name="height">画面高さ</param>
SpriteManager::SpriteManager(Dx12Wrapper& dx12, LONG width, LONG height):_dx12(dx12),_width(width),_height(height)
{
	CreateSpriteRS();		//Sprite用ルートシグネチャを作成

	InitSpriteDevices();	//Sprite用オブジェクトを初期化

	_buttonLeft = (_width / 2) - (BUTTON_WIDTH / 2);	//ボタン用矩形の左座標
	_buttonRight = (_width / 2) + (BUTTON_WIDTH / 2);	//ボタン用矩形の右座標

	_wss.precision(7);		//wstringstreamの表示桁数を設定

	AdjustSpriteRect();		//各矩形をウィンドウに合わせる

	_titleWidth = _titleRect.right - _titleRect.left;
	_titleHeight = _titleRect.bottom - _titleRect.top;

	//差分のサイズを取得
	_incrementSize = _dx12.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateUIBufferView(L"5_image/background.png","back");					//背景
	CreateUIBufferView(L"5_image/Cursor.png", "cursor");					//マウスカーソル
	CreateUIBufferView(L"5_image/base_004_hover.png", "button");			//ボタン
	CreateUIBufferView(L"5_image/nc81777.png", "gage");						//体力ゲージ
	CreateUIBufferView(L"5_image/outline.png", "outline");					//体力ゲージの外枠

	CreateUIBufferView(L"5_image/loading/1.png", "load_1");					//ロード画面
	CreateUIBufferView(L"5_image/loading/2.png", "load_2");					//ロード画面
	CreateUIBufferView(L"5_image/loading/3.png", "load_3");					//ロード画面
	CreateUIBufferView(L"5_image/loading/4.png", "load_4");					//ロード画面
	CreateUIBufferView(L"5_image/loading/5.png", "load_5");					//ロード画面
	CreateUIBufferView(L"5_image/loading/6.png", "load_6");					//ロード画面
	CreateUIBufferView(L"5_image/loading/7.png", "load_7");					//ロード画面
	CreateUIBufferView(L"5_image/loading/8.png", "load_8");					//ロード画面

	CreateDataBufferView();							//体力ゲージ調整用ビューを作成
}

/// <summary>
/// SpriteBatch向けのルートシグネチャ・シェーダーを作成する関数
/// </summary>
/// <returns>処理が成功したかどうか</returns>
HRESULT
SpriteManager::CreateSpriteRS()
{
	//ディスクリプタレンジ(SRV用)
	CD3DX12_DESCRIPTOR_RANGE spriteTblRange[2] = {};
	spriteTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	spriteTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	//ルートパラメータ(SRV,CBV用)
	//[1]はConstantBufferView、[2]はDescriptorTableとして初期化
	CD3DX12_ROOT_PARAMETER spriteRootParam[3] = {};
	spriteRootParam[0].InitAsDescriptorTable(1, &spriteTblRange[0], D3D12_SHADER_VISIBILITY_PIXEL);
	spriteRootParam[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	spriteRootParam[2].InitAsDescriptorTable(1, &spriteTblRange[1], D3D12_SHADER_VISIBILITY_ALL);

	//サンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);											//初期化
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//ピクセルシェーダーから見えるよう設定

	//ルートシグネチャ作成用構造体
	CD3DX12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Init(3,spriteRootParam,1,&samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//ルートシグネチャの初期化
	ID3DBlob* rsBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rsBlob,
		&errorBlob);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	rsBlob->Release();	//使わないデータを開放

	//ルートシグネチャ作成
	result = _dx12.Device()->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(_spriteRS.ReleaseAndGetAddressOf())
	);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

#ifdef _DEBUG
	//ピクセルシェーダー読み込み
	result = D3DCompileFromFile(
		L"3_shader/SpritePixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"SpritePixelShader", "ps_5_0",
		0,
		0,
		&_psBlob, &errorBlob);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}
#endif // DEBUG

	return result;
}

/// <summary>
/// Sprite向けオブジェクトを初期化する関数
/// </summary>
void
SpriteManager::InitSpriteDevices()
{
	//グラフィックスメモリの初期化
	_gmemory = make_unique<GraphicsMemory>(_dx12.Device());

	//スプライト表示用オブジェクトの初期化
	ResourceUploadBatch resUploadBatch(_dx12.Device());
	resUploadBatch.Begin();

	RenderTargetState rtState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);

	unique_ptr<CommonStates> _states = make_unique<CommonStates>(_dx12.Device());		//サンプラーを取得するためStateオブジェクトを初期化
	D3D12_GPU_DESCRIPTOR_HANDLE wrap = _states->AnisotropicWrap();						//GPUハンドル

	SpriteBatchPipelineStateDescription pd1(rtState, nullptr, nullptr, nullptr, &wrap);
	pd1.customRootSignature = _spriteRS.Get();												//ルートシグネチャ

#ifdef _DEBUG
	pd1.customPixelShader = CD3DX12_SHADER_BYTECODE(_psBlob);								//ピクセルシェーダー
#else
	pd1.customPixelShader.BytecodeLength = sizeof(g_SpritePixelShader);						//ピクセルシェーダー
	pd1.customPixelShader.pShaderBytecode = &g_SpritePixelShader;
#endif // DEBUG


	SpriteBatchPipelineStateDescription pd2(rtState, &CommonStates::NonPremultiplied);

	_spriteBatch1 = make_unique<SpriteBatch>(_dx12.Device(), resUploadBatch, pd1);	//スプライト表示用オブジェクト
	_spriteBatch2 = make_unique<SpriteBatch>(_dx12.Device(), resUploadBatch, pd2);	//スプライト表示用オブジェクト

	//フォント表示用オブジェクトの初期化
	_heapForSpriteFont = _dx12.CreateDescriptorHeapForSpriteFont();				//フォント・画像表示用ヒープ
	_tmpCPUHandle = _heapForSpriteFont->GetCPUDescriptorHandleForHeapStart();	//ヒープハンドル(CPU)
	_tmpGPUHandle = _heapForSpriteFont->GetGPUDescriptorHandleForHeapStart();	//ヒープハンドル(GPU)

	//フォント表示用オブジェクト
	_spriteFont = make_unique<SpriteFont>(
		_dx12.Device(),
		resUploadBatch,
		L"4_font/fonttest.spritefont",
		_tmpCPUHandle,
		_tmpGPUHandle
		);
	auto future = resUploadBatch.End(_dx12.CommandQueue());	//GPU側へ転送

	//GPUが使用可能になるまで待機
	_dx12.WaitForCommandQueue();
	future.wait();

	_spriteBatch1->SetViewport(*_dx12.ViewPort());	//スプライト表示用オブジェクトをビューポートへ登録
	_spriteBatch2->SetViewport(*_dx12.ViewPort());	//スプライト表示用オブジェクトをビューポートへ登録

	return;
}

/// <summary>
/// 画像のバッファー・ビューを作成する関数
/// </summary>
/// <param name="path">画像のパス</param>
/// <param name="key">連想配列のキー</param>
/// <returns>処理が成功したかどうか</returns>
HRESULT
SpriteManager::CreateUIBufferView(const wchar_t* path,string key)
{
	//UI画像読み込み用データ
	TexMetadata meta = {};
	ScratchImage scratch = {};

	auto ext = FileExtension(path);		//拡張子を取得

	//画像データの読み込み
	auto result = _dx12._loadLambdaTable[ToString(ext)](path, &meta, scratch);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	auto img = scratch.GetImage(0, 0, 0);	//生データを取得

	DXGI_FORMAT format = meta.format;							//フォーマット
	size_t width = meta.width;									//幅
	size_t height = meta.height;								//高さ
	UINT16 arraySize = static_cast<UINT16>(meta.arraySize);		//テクスチャサイズ
	UINT16 mipLevels = static_cast<UINT16>(meta.mipLevels);		
	void* pixels = img->pixels;
	UINT rowPitch = static_cast<UINT>(img->rowPitch);
	UINT slicePitch = static_cast<UINT>(img->slicePitch);

	ID3D12Resource* tmpUIBuff = nullptr;	//画像データ書き込み用バッファ

	//リソース作成用データ
	auto uiResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		(UINT)width,
		(UINT)height,
		arraySize,
		(UINT)mipLevels);

	//リソース作成
	result = _dx12.Device()->CreateCommittedResource(
		&_writeHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&uiResDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&tmpUIBuff));
	if (FAILED(result))
	{
		assert(0);
		return result;;
	}

	//画像情報を書き込み
	result = tmpUIBuff->WriteToSubresource(0,
		nullptr,
		pixels,
		rowPitch,
		slicePitch);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//ハンドルをずらす
	ShiftHandles();

	//UIビュー用構造体の作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = tmpUIBuff->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	_dx12.Device()->CreateShaderResourceView(tmpUIBuff, &srvDesc, _tmpCPUHandle);	//ビュー作成

	_GPUHandles[key] = _tmpGPUHandle;	//GPUハンドルをベクトルに格納

	return result;
}

/// <summary>
/// 矩形の中央に文章を配置できるような座標を取得する関数
/// </summary>
/// <param name="rect">矩形</param>
/// <param name="wstr">文章</param>
/// <param name="rectWidth">矩形の幅</param>
/// <param name="rectHeight">矩形の高さ</param>
/// <returns>座標</returns>
XMFLOAT2
SpriteManager::GetCenterPos(RECT rect, const wchar_t* wstr, float rectWidth, float rectHeight)
{
	auto ret = _spriteFont->MeasureString(wstr);	//文字列のフォントサイズをベクトルの形で取得
	auto width = XMVectorGetX(ret);					//幅を取得
	auto height = XMVectorGetY(ret);				//高さを取得

	//幅、高さを元に中央揃えとなる座標を取得する
	float xPos = (float)rect.left + ((rectWidth - width) / 2);
	float yPos = (float)rect.top - 5;

	return XMFLOAT2(xPos, yPos);
}

/// <summary>
/// マウスがシザー矩形の中に入っているかチェックする関数
/// </summary>
/// <param name="rect">チェックしたい矩形</param>
/// <returns>入っているかどうか</returns>
bool
SpriteManager::IsInRect(RECT rect)
{
	if ((rect.left <= _x && _x <= rect.right) &&
		(rect.top <= _y && _y <= rect.bottom))
	{
		return true;
	}

	return false;
}

/// <summary>
/// HPゲージ用データのビューを作成する関数
/// </summary>
/// <param name="resource">UI加工用データ</param>
void
SpriteManager::CreateDataBufferView()
{
	ShiftHandles();											//ハンドルをずらす

	//バッファー作成
	ID3D12Resource* _dataBuff = nullptr;

	auto hpResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(HitPoint) + _constSize) & ~_constSize);

	auto result = _dx12.Device()->CreateCommittedResource
	(
		&_writeHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&hpResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_dataBuff)
	);
	if (FAILED(result))
	{
		assert(0);
		return;
	}

	result = _dataBuff->Map(0, nullptr, (void**)&_mappedHitPoint);
	if (FAILED(result))
	{
		assert(0);
		return;
	}

	//ビュー作成用構造体
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _dataBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)_dataBuff->GetDesc().Width;

	_dx12.Device()->CreateConstantBufferView(&cbvDesc, _tmpCPUHandle);	//ビュー作成

	_GPUHandles["data"] = _tmpGPUHandle;	//GPUハンドルを登録

	return;
}

/// <summary>
/// CPUとGPUのハンドルをずらす関数
/// </summary>
void
SpriteManager::ShiftHandles()
{
	//ハンドルをずらす
	_tmpCPUHandle.ptr += _incrementSize;
	_tmpGPUHandle.ptr += _incrementSize;
}

/// <summary>
/// HIゲージと表示されている体力を更新する関数
/// </summary>
/// <param name="currentHP">現在のHP</param>
void
SpriteManager::UpdateHP(float currentHP)
{
	//初期化
	_wss.str(L"");														
	_wss.clear(wstringstream::goodbit);
	//範囲を制限した体力を表示
	_wss << clamp(currentHP, 0.0f, _maxHp) << "/" << _maxHp << endl;

	_mappedHitPoint->hp = clamp(currentHP, 0.0f, _maxHp) / _maxHp;		//体力ゲージを更新
}

/// <summary>
/// 画面サイズ変更時、矩形を調整する関数
/// </summary>
void
SpriteManager::AdjustSpriteRect()
{
	_backGroundRect = { ORIGIN,ORIGIN,_width,_height };												//背景用画像の設定
	_startButtonRect = { _buttonLeft,START_TOP,_buttonRight,START_TOP + BUTTON_HEIGHT };				//スタートボタンの設定
	_endButtonRect = { _buttonLeft,END_TOP,_buttonRight,END_TOP + BUTTON_HEIGHT };					//終了ボタンの設定
	_loadingRect = { LOADING_WIDTH,LOADIN_HEIGHT,_width - LOADING_WIDTH,_height - LOADIN_HEIGHT };	//ロード画面の設定
	_HPGageRect = { HPGAGE_DIFF,HPGAGE_DIFF,(_width / 3) + HPGAGE_DIFF,HPGAGE_DIFF + 5 };			//体力ゲージの設定
	_titleRect = { TITLE_DIFF,TITLE_DIFF,_width - TITLE_DIFF,_height - TITLE_DIFF };				//タイトルメニューの設定

	//ウィンドウサイズを計算
	AdjustWindowRect(&_backGroundRect, WS_OVERLAPPEDWINDOW, false);	
	AdjustWindowRect(&_startButtonRect, WS_OVERLAPPEDWINDOW, false);	
	AdjustWindowRect(&_endButtonRect, WS_OVERLAPPEDWINDOW, false);		
	AdjustWindowRect(&_loadingRect, WS_OVERLAPPEDWINDOW, false);		
	AdjustWindowRect(&_HPGageRect, WS_OVERLAPPEDWINDOW, false);		
	AdjustWindowRect(&_titleRect, WS_OVERLAPPEDWINDOW, false);			
}

/// <summary>
/// UIとして表示する最大HPを設定する関数
/// </summary>
/// <param name="maxHp">最大HP</param>
/// <param name="currentHp">現在のHP</param>
void 
SpriteManager::SetMaxHP(float maxHP, float currentHP)
{
	_maxHp = maxHP;	//最大HPを設定

	_mappedHitPoint->hp = clamp(currentHP,0.0f,_maxHp) / _maxHp;	//体力ゲージを初期値に設定

	UpdateHP(currentHP);	//体力に関連するUIを更新
}

/// <summary>
/// ローディング画面での画像を描画する関数
/// </summary>
void
SpriteManager::LoadingDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);		//ヒープをセット

	_spriteBatch2->Begin(_dx12.CommandList());								//バッチをセット

	//描画処理
	_spriteBatch2->Draw(_GPUHandles["back"], XMUINT2(1, 1), _backGroundRect, Colors::White);

	auto& app = Application::Instance();
	int rate = app.GetRate();

	int count = ((clock()/rate) % 8) + 1;
	_spriteBatch2->Draw(_GPUHandles["load_" + to_string(count)], XMUINT2(1, 1), _loadingRect, Colors::White);

	_spriteBatch2->End();	//バッチを解除
}

/// <summary>
/// タイトル画面での画像・UIを描画する関数
/// </summary>
void
SpriteManager::TitleDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);		//ヒープをセット

	//描画処理
	_spriteBatch2->Begin(_dx12.CommandList());	//バッチをセット

	_spriteBatch2->Draw(_GPUHandles["back"], XMUINT2(1, 1), _backGroundRect, Colors::White);
	_spriteBatch2->Draw(_GPUHandles["button"], XMUINT2(1, 1), _startButtonRect, Colors::White);
	_spriteBatch2->Draw(_GPUHandles["button"], XMUINT2(1, 1), _endButtonRect, Colors::White);

	//タイトル名を表示
	_spriteFont->DrawString(_spriteBatch2.get(), L"Game",
		GetCenterPos(_titleRect, L"Game", (float)_titleWidth, (float)_titleHeight), Colors::White);

	//ボタン上のテキストを描画
	_spriteFont->DrawString(_spriteBatch2.get(), L"Start",
		GetCenterPos(_startButtonRect, L"Start", (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT), Colors::Black);
	_spriteFont->DrawString(_spriteBatch2.get(), L"End",
		GetCenterPos(_endButtonRect, L"End", (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT), Colors::Black);

	_spriteBatch2->End();	//バッチを解除
}

/// <summary>
/// リザルト画面での画像を描画する関数
/// </summary>
void
SpriteManager::ResultDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);		//ヒープをセット

	//描画処理
	_spriteBatch2->Begin(_dx12.CommandList());	//バッチをセット

	_spriteBatch2->Draw(_GPUHandles["back"], XMUINT2(1, 1), _backGroundRect, Colors::White);
	_spriteBatch2->Draw(_GPUHandles["button"], XMUINT2(1, 1), _startButtonRect, Colors::White);
	_spriteBatch2->Draw(_GPUHandles["button"], XMUINT2(1, 1), _endButtonRect, Colors::White);

	//リザルトを表示
	_spriteFont->DrawString(_spriteBatch2.get(), L"Game Over",
		GetCenterPos(_titleRect, L"Game Over", (float)_titleWidth, (float)_titleHeight), Colors::White);

	//ボタン上のテキストを描画
	_spriteFont->DrawString(_spriteBatch2.get(), L"Restart",
		GetCenterPos(_startButtonRect, L"Restart", (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT), Colors::Black);
	_spriteFont->DrawString(_spriteBatch2.get(), L"Title",
		GetCenterPos(_endButtonRect, L"Title", (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT), Colors::Black);

	_spriteBatch2->End();	//バッチを解除
}

/// <summary>
/// タイトル画面でスタートボタンの上にマウスがあるかチェックする関数
/// </summary>
/// <returns>マウス座標がスタートボタン上にあるかどうか</returns>
bool
SpriteManager::TitleIsOnStart()
{
	return IsInRect(_startButtonRect);
}

/// <summary>
/// タイトル画面で終了ボタンの上にマウスがあるかチェックする関数
/// </summary>
/// <returns>マウス座標が終了ボタンの上にあるかどうか</returns>
bool
SpriteManager::TitleIsOnEnd()
{
	return IsInRect(_endButtonRect);
}

/// <summary>
/// 背景を描画する関数
/// </summary>
void
SpriteManager::BackGroundDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);		//ヒープをセット

	_spriteBatch2->Begin(_dx12.CommandList());	//バッチをセット

	//描画処理
	_spriteBatch2->Draw(_GPUHandles["back"], XMUINT2(1, 1), _backGroundRect, Colors::White);

	_spriteBatch2->End();	//バッチを解除
}

/// <summary>
/// マウスカーソルなどのUIを描画する関数
/// </summary>
void
SpriteManager::UIDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);

	//HPゲージ描画処理（シェーダーを適用するため分離している）
	_spriteBatch1->Begin(_dx12.CommandList(), _GPUHandles["data"]);

	_spriteBatch1->Draw(_GPUHandles["gage"], XMUINT2(1, 1), _HPGageRect, Colors::White);

	_spriteBatch1->End();

	//その他UI描画処理
	_spriteBatch2->Begin(_dx12.CommandList());	//バッチをセット

	_spriteBatch2->Draw(_GPUHandles["outline"], XMUINT2(1, 1), _HPGageRect);
	_spriteFont->DrawString(_spriteBatch2.get(), _wss.str().c_str(), XMFLOAT2(((float)_width / 3) + 60, 6), Colors::White);

	_spriteBatch2->End();	//バッチを解除
}

/// <summary>
/// マウスカーソル用の画像を描画する関数
/// </summary>
void
SpriteManager::CursorDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);

	//その他UI描画処理
	_spriteBatch2->Begin(_dx12.CommandList());	//バッチをセット

	_spriteBatch2->Draw(_GPUHandles["cursor"], XMUINT2(50, 50), XMFLOAT2((float)_x, (float)_y), Colors::White);

	_spriteBatch2->End();	//バッチを解除
}

/// <summary>
/// グラフィックスメモリをコマンドキューにセットする関数
/// </summary>
void
SpriteManager::Commit()
{
	_gmemory->Commit(_dx12.CommandQueue());
}

/// <summary>
/// マウス座標をセットする関数
/// </summary>
/// <param name="x">X座標</param>
/// <param name="y">y座標</param>
void
SpriteManager::SetMousePosition(int x, int y)
{
	_x = x;
	_y = y;
}