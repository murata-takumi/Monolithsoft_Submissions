#include "SpriteManager.h"
#include "BinaryFile.h"
#include "Dx12Wrapper.h"
#include "Functions.h"

const int ORIGIN = 0;
const int START_TOP = 480;			//�X�^�[�g�{�^���̏���W
const int END_TOP = 555;			//�I���{�^���̏���W
const int BUTTON_WIDTH = 200;		//�{�^����
const int BUTTON_HEIGHT = 25;		//�{�^������
const int LOADING_WIDTH = 480;		//���[�h��ʂŕ\������A�C�R���̕�
const int LOADIN_HEIGHT = 270;		//���[�h��ʂŕ\������A�C�R���̍���
const int TITLE_DIFF = 180;			//��ʑS�̂���^�C�g����ʂւ̍���
const int HPGAGE_DIFF = 45;			//�������Ƃ���HP�Q�[�W�̍���

/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="dx12">Dx12Wrapper�C���X�^���X</param>
/// <param name="width">��ʕ�</param>
/// <param name="height">��ʍ���</param>
SpriteManager::SpriteManager(Dx12Wrapper& dx12, LONG width, LONG height):_dx12(dx12),_width(width),_height(height)
{
	CreateSpriteRS();		//Sprite�p���[�g�V�O�l�`�����쐬

	InitSpriteDevices();	//Sprite�p�I�u�W�F�N�g��������

	_buttonLeft = (_width / 2) - (BUTTON_WIDTH / 2);	//�{�^���p��`�̍����W
	_buttonRight = (_width / 2) + (BUTTON_WIDTH / 2);	//�{�^���p��`�̉E���W

	_wss.precision(7);		//wstringstream�̕\��������ݒ�

	AdjustSpriteRect();		//�e��`���E�B���h�E�ɍ��킹��

	_titleWidth = _titleRect.right - _titleRect.left;
	_titleHeight = _titleRect.bottom - _titleRect.top;

	//�����̃T�C�Y���擾
	_incrementSize = _dx12.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateUIBufferView(L"5_image/background.png","back");					//�w�i
	CreateUIBufferView(L"5_image/Cursor.png", "cursor");					//�}�E�X�J�[�\��
	CreateUIBufferView(L"5_image/base_004_hover.png", "button");			//�{�^��
	CreateUIBufferView(L"5_image/nc81777.png", "gage");						//�̗̓Q�[�W
	CreateUIBufferView(L"5_image/outline.png", "outline");					//�̗̓Q�[�W�̊O�g

	CreateUIBufferView(L"5_image/loading/1.png", "load_1");					//���[�h���
	CreateUIBufferView(L"5_image/loading/2.png", "load_2");					//���[�h���
	CreateUIBufferView(L"5_image/loading/3.png", "load_3");					//���[�h���
	CreateUIBufferView(L"5_image/loading/4.png", "load_4");					//���[�h���
	CreateUIBufferView(L"5_image/loading/5.png", "load_5");					//���[�h���
	CreateUIBufferView(L"5_image/loading/6.png", "load_6");					//���[�h���
	CreateUIBufferView(L"5_image/loading/7.png", "load_7");					//���[�h���
	CreateUIBufferView(L"5_image/loading/8.png", "load_8");					//���[�h���

	CreateDataBufferView();							//�̗̓Q�[�W�����p�r���[���쐬
}

/// <summary>
/// SpriteBatch�����̃��[�g�V�O�l�`���E�V�F�[�_�[���쐬����֐�
/// </summary>
/// <returns>�����������������ǂ���</returns>
HRESULT
SpriteManager::CreateSpriteRS()
{
	//�f�B�X�N���v�^�����W(SRV�p)
	CD3DX12_DESCRIPTOR_RANGE spriteTblRange[2] = {};
	spriteTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	spriteTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	//���[�g�p�����[�^(SRV,CBV�p)
	//[1]��ConstantBufferView�A[2]��DescriptorTable�Ƃ��ď�����
	CD3DX12_ROOT_PARAMETER spriteRootParam[3] = {};
	spriteRootParam[0].InitAsDescriptorTable(1, &spriteTblRange[0], D3D12_SHADER_VISIBILITY_PIXEL);
	spriteRootParam[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	spriteRootParam[2].InitAsDescriptorTable(1, &spriteTblRange[1], D3D12_SHADER_VISIBILITY_ALL);

	//�T���v���[
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);											//������
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//�s�N�Z���V�F�[�_�[���猩����悤�ݒ�

	//���[�g�V�O�l�`���쐬�p�\����
	CD3DX12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Init(3,spriteRootParam,1,&samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//���[�g�V�O�l�`���̏�����
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

	rsBlob->Release();	//�g��Ȃ��f�[�^���J��

	//���[�g�V�O�l�`���쐬
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
	//�s�N�Z���V�F�[�_�[�ǂݍ���
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
/// Sprite�����I�u�W�F�N�g������������֐�
/// </summary>
void
SpriteManager::InitSpriteDevices()
{
	//�O���t�B�b�N�X�������̏�����
	_gmemory = make_unique<GraphicsMemory>(_dx12.Device());

	//�X�v���C�g�\���p�I�u�W�F�N�g�̏�����
	ResourceUploadBatch resUploadBatch(_dx12.Device());
	resUploadBatch.Begin();

	RenderTargetState rtState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);

	unique_ptr<CommonStates> _states = make_unique<CommonStates>(_dx12.Device());		//�T���v���[���擾���邽��State�I�u�W�F�N�g��������
	D3D12_GPU_DESCRIPTOR_HANDLE wrap = _states->AnisotropicWrap();						//GPU�n���h��

	SpriteBatchPipelineStateDescription pd1(rtState, nullptr, nullptr, nullptr, &wrap);
	pd1.customRootSignature = _spriteRS.Get();												//���[�g�V�O�l�`��

#ifdef _DEBUG
	pd1.customPixelShader = CD3DX12_SHADER_BYTECODE(_psBlob);								//�s�N�Z���V�F�[�_�[
#else
	pd1.customPixelShader.BytecodeLength = sizeof(g_SpritePixelShader);						//�s�N�Z���V�F�[�_�[
	pd1.customPixelShader.pShaderBytecode = &g_SpritePixelShader;
#endif // DEBUG


	SpriteBatchPipelineStateDescription pd2(rtState, &CommonStates::NonPremultiplied);

	_spriteBatch1 = make_unique<SpriteBatch>(_dx12.Device(), resUploadBatch, pd1);	//�X�v���C�g�\���p�I�u�W�F�N�g
	_spriteBatch2 = make_unique<SpriteBatch>(_dx12.Device(), resUploadBatch, pd2);	//�X�v���C�g�\���p�I�u�W�F�N�g

	//�t�H���g�\���p�I�u�W�F�N�g�̏�����
	_heapForSpriteFont = _dx12.CreateDescriptorHeapForSpriteFont();				//�t�H���g�E�摜�\���p�q�[�v
	_tmpCPUHandle = _heapForSpriteFont->GetCPUDescriptorHandleForHeapStart();	//�q�[�v�n���h��(CPU)
	_tmpGPUHandle = _heapForSpriteFont->GetGPUDescriptorHandleForHeapStart();	//�q�[�v�n���h��(GPU)

	//�t�H���g�\���p�I�u�W�F�N�g
	_spriteFont = make_unique<SpriteFont>(
		_dx12.Device(),
		resUploadBatch,
		L"4_font/fonttest.spritefont",
		_tmpCPUHandle,
		_tmpGPUHandle
		);
	auto future = resUploadBatch.End(_dx12.CommandQueue());	//GPU���֓]��

	//GPU���g�p�\�ɂȂ�܂őҋ@
	_dx12.WaitForCommandQueue();
	future.wait();

	_spriteBatch1->SetViewport(*_dx12.ViewPort());	//�X�v���C�g�\���p�I�u�W�F�N�g���r���[�|�[�g�֓o�^
	_spriteBatch2->SetViewport(*_dx12.ViewPort());	//�X�v���C�g�\���p�I�u�W�F�N�g���r���[�|�[�g�֓o�^

	return;
}

/// <summary>
/// �摜�̃o�b�t�@�[�E�r���[���쐬����֐�
/// </summary>
/// <param name="path">�摜�̃p�X</param>
/// <param name="key">�A�z�z��̃L�[</param>
/// <returns>�����������������ǂ���</returns>
HRESULT
SpriteManager::CreateUIBufferView(const wchar_t* path,string key)
{
	//UI�摜�ǂݍ��ݗp�f�[�^
	TexMetadata meta = {};
	ScratchImage scratch = {};

	auto ext = FileExtension(path);		//�g���q���擾

	//�摜�f�[�^�̓ǂݍ���
	auto result = _dx12._loadLambdaTable[ToString(ext)](path, &meta, scratch);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	auto img = scratch.GetImage(0, 0, 0);	//���f�[�^���擾

	DXGI_FORMAT format = meta.format;							//�t�H�[�}�b�g
	size_t width = meta.width;									//��
	size_t height = meta.height;								//����
	UINT16 arraySize = static_cast<UINT16>(meta.arraySize);		//�e�N�X�`���T�C�Y
	UINT16 mipLevels = static_cast<UINT16>(meta.mipLevels);		
	void* pixels = img->pixels;
	UINT rowPitch = static_cast<UINT>(img->rowPitch);
	UINT slicePitch = static_cast<UINT>(img->slicePitch);

	ID3D12Resource* tmpUIBuff = nullptr;	//�摜�f�[�^�������ݗp�o�b�t�@

	//���\�[�X�쐬�p�f�[�^
	auto uiResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		(UINT)width,
		(UINT)height,
		arraySize,
		(UINT)mipLevels);

	//���\�[�X�쐬
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

	//�摜������������
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

	//�n���h�������炷
	ShiftHandles();

	//UI�r���[�p�\���̂̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = tmpUIBuff->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	_dx12.Device()->CreateShaderResourceView(tmpUIBuff, &srvDesc, _tmpCPUHandle);	//�r���[�쐬

	_GPUHandles[key] = _tmpGPUHandle;	//GPU�n���h�����x�N�g���Ɋi�[

	return result;
}

/// <summary>
/// ��`�̒����ɕ��͂�z�u�ł���悤�ȍ��W���擾����֐�
/// </summary>
/// <param name="rect">��`</param>
/// <param name="wstr">����</param>
/// <param name="rectWidth">��`�̕�</param>
/// <param name="rectHeight">��`�̍���</param>
/// <returns>���W</returns>
XMFLOAT2
SpriteManager::GetCenterPos(RECT rect, const wchar_t* wstr, float rectWidth, float rectHeight)
{
	auto ret = _spriteFont->MeasureString(wstr);	//������̃t�H���g�T�C�Y���x�N�g���̌`�Ŏ擾
	auto width = XMVectorGetX(ret);					//�����擾
	auto height = XMVectorGetY(ret);				//�������擾

	//���A���������ɒ��������ƂȂ���W���擾����
	float xPos = (float)rect.left + ((rectWidth - width) / 2);
	float yPos = (float)rect.top - 5;

	return XMFLOAT2(xPos, yPos);
}

/// <summary>
/// �}�E�X���V�U�[��`�̒��ɓ����Ă��邩�`�F�b�N����֐�
/// </summary>
/// <param name="rect">�`�F�b�N��������`</param>
/// <returns>�����Ă��邩�ǂ���</returns>
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
/// HP�Q�[�W�p�f�[�^�̃r���[���쐬����֐�
/// </summary>
/// <param name="resource">UI���H�p�f�[�^</param>
void
SpriteManager::CreateDataBufferView()
{
	ShiftHandles();											//�n���h�������炷

	//�o�b�t�@�[�쐬
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

	//�r���[�쐬�p�\����
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _dataBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)_dataBuff->GetDesc().Width;

	_dx12.Device()->CreateConstantBufferView(&cbvDesc, _tmpCPUHandle);	//�r���[�쐬

	_GPUHandles["data"] = _tmpGPUHandle;	//GPU�n���h����o�^

	return;
}

/// <summary>
/// CPU��GPU�̃n���h�������炷�֐�
/// </summary>
void
SpriteManager::ShiftHandles()
{
	//�n���h�������炷
	_tmpCPUHandle.ptr += _incrementSize;
	_tmpGPUHandle.ptr += _incrementSize;
}

/// <summary>
/// HI�Q�[�W�ƕ\������Ă���̗͂��X�V����֐�
/// </summary>
/// <param name="currentHP">���݂�HP</param>
void
SpriteManager::UpdateHP(float currentHP)
{
	//������
	_wss.str(L"");														
	_wss.clear(wstringstream::goodbit);
	//�͈͂𐧌������̗͂�\��
	_wss << clamp(currentHP, 0.0f, _maxHp) << "/" << _maxHp << endl;

	_mappedHitPoint->hp = clamp(currentHP, 0.0f, _maxHp) / _maxHp;		//�̗̓Q�[�W���X�V
}

/// <summary>
/// ��ʃT�C�Y�ύX���A��`�𒲐�����֐�
/// </summary>
void
SpriteManager::AdjustSpriteRect()
{
	_backGroundRect = { ORIGIN,ORIGIN,_width,_height };												//�w�i�p�摜�̐ݒ�
	_startButtonRect = { _buttonLeft,START_TOP,_buttonRight,START_TOP + BUTTON_HEIGHT };				//�X�^�[�g�{�^���̐ݒ�
	_endButtonRect = { _buttonLeft,END_TOP,_buttonRight,END_TOP + BUTTON_HEIGHT };					//�I���{�^���̐ݒ�
	_loadingRect = { LOADING_WIDTH,LOADIN_HEIGHT,_width - LOADING_WIDTH,_height - LOADIN_HEIGHT };	//���[�h��ʂ̐ݒ�
	_HPGageRect = { HPGAGE_DIFF,HPGAGE_DIFF,(_width / 3) + HPGAGE_DIFF,HPGAGE_DIFF + 5 };			//�̗̓Q�[�W�̐ݒ�
	_titleRect = { TITLE_DIFF,TITLE_DIFF,_width - TITLE_DIFF,_height - TITLE_DIFF };				//�^�C�g�����j���[�̐ݒ�

	//�E�B���h�E�T�C�Y���v�Z
	AdjustWindowRect(&_backGroundRect, WS_OVERLAPPEDWINDOW, false);	
	AdjustWindowRect(&_startButtonRect, WS_OVERLAPPEDWINDOW, false);	
	AdjustWindowRect(&_endButtonRect, WS_OVERLAPPEDWINDOW, false);		
	AdjustWindowRect(&_loadingRect, WS_OVERLAPPEDWINDOW, false);		
	AdjustWindowRect(&_HPGageRect, WS_OVERLAPPEDWINDOW, false);		
	AdjustWindowRect(&_titleRect, WS_OVERLAPPEDWINDOW, false);			
}

/// <summary>
/// UI�Ƃ��ĕ\������ő�HP��ݒ肷��֐�
/// </summary>
/// <param name="maxHp">�ő�HP</param>
/// <param name="currentHp">���݂�HP</param>
void 
SpriteManager::SetMaxHP(float maxHP, float currentHP)
{
	_maxHp = maxHP;	//�ő�HP��ݒ�

	_mappedHitPoint->hp = clamp(currentHP,0.0f,_maxHp) / _maxHp;	//�̗̓Q�[�W�������l�ɐݒ�

	UpdateHP(currentHP);	//�̗͂Ɋ֘A����UI���X�V
}

/// <summary>
/// ���[�f�B���O��ʂł̉摜��`�悷��֐�
/// </summary>
void
SpriteManager::LoadingDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);		//�q�[�v���Z�b�g

	_spriteBatch2->Begin(_dx12.CommandList());								//�o�b�`���Z�b�g

	//�`�揈��
	_spriteBatch2->Draw(_GPUHandles["back"], XMUINT2(1, 1), _backGroundRect, Colors::White);

	auto& app = Application::Instance();
	int rate = app.GetRate();

	int count = ((clock()/rate) % 8) + 1;
	_spriteBatch2->Draw(_GPUHandles["load_" + to_string(count)], XMUINT2(1, 1), _loadingRect, Colors::White);

	_spriteBatch2->End();	//�o�b�`������
}

/// <summary>
/// �^�C�g����ʂł̉摜�EUI��`�悷��֐�
/// </summary>
void
SpriteManager::TitleDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);		//�q�[�v���Z�b�g

	//�`�揈��
	_spriteBatch2->Begin(_dx12.CommandList());	//�o�b�`���Z�b�g

	_spriteBatch2->Draw(_GPUHandles["back"], XMUINT2(1, 1), _backGroundRect, Colors::White);
	_spriteBatch2->Draw(_GPUHandles["button"], XMUINT2(1, 1), _startButtonRect, Colors::White);
	_spriteBatch2->Draw(_GPUHandles["button"], XMUINT2(1, 1), _endButtonRect, Colors::White);

	//�^�C�g������\��
	_spriteFont->DrawString(_spriteBatch2.get(), L"Game",
		GetCenterPos(_titleRect, L"Game", (float)_titleWidth, (float)_titleHeight), Colors::White);

	//�{�^����̃e�L�X�g��`��
	_spriteFont->DrawString(_spriteBatch2.get(), L"Start",
		GetCenterPos(_startButtonRect, L"Start", (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT), Colors::Black);
	_spriteFont->DrawString(_spriteBatch2.get(), L"End",
		GetCenterPos(_endButtonRect, L"End", (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT), Colors::Black);

	_spriteBatch2->End();	//�o�b�`������
}

/// <summary>
/// ���U���g��ʂł̉摜��`�悷��֐�
/// </summary>
void
SpriteManager::ResultDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);		//�q�[�v���Z�b�g

	//�`�揈��
	_spriteBatch2->Begin(_dx12.CommandList());	//�o�b�`���Z�b�g

	_spriteBatch2->Draw(_GPUHandles["back"], XMUINT2(1, 1), _backGroundRect, Colors::White);
	_spriteBatch2->Draw(_GPUHandles["button"], XMUINT2(1, 1), _startButtonRect, Colors::White);
	_spriteBatch2->Draw(_GPUHandles["button"], XMUINT2(1, 1), _endButtonRect, Colors::White);

	//���U���g��\��
	_spriteFont->DrawString(_spriteBatch2.get(), L"Game Over",
		GetCenterPos(_titleRect, L"Game Over", (float)_titleWidth, (float)_titleHeight), Colors::White);

	//�{�^����̃e�L�X�g��`��
	_spriteFont->DrawString(_spriteBatch2.get(), L"Restart",
		GetCenterPos(_startButtonRect, L"Restart", (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT), Colors::Black);
	_spriteFont->DrawString(_spriteBatch2.get(), L"Title",
		GetCenterPos(_endButtonRect, L"Title", (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT), Colors::Black);

	_spriteBatch2->End();	//�o�b�`������
}

/// <summary>
/// �^�C�g����ʂŃX�^�[�g�{�^���̏�Ƀ}�E�X�����邩�`�F�b�N����֐�
/// </summary>
/// <returns>�}�E�X���W���X�^�[�g�{�^����ɂ��邩�ǂ���</returns>
bool
SpriteManager::TitleIsOnStart()
{
	return IsInRect(_startButtonRect);
}

/// <summary>
/// �^�C�g����ʂŏI���{�^���̏�Ƀ}�E�X�����邩�`�F�b�N����֐�
/// </summary>
/// <returns>�}�E�X���W���I���{�^���̏�ɂ��邩�ǂ���</returns>
bool
SpriteManager::TitleIsOnEnd()
{
	return IsInRect(_endButtonRect);
}

/// <summary>
/// �w�i��`�悷��֐�
/// </summary>
void
SpriteManager::BackGroundDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);		//�q�[�v���Z�b�g

	_spriteBatch2->Begin(_dx12.CommandList());	//�o�b�`���Z�b�g

	//�`�揈��
	_spriteBatch2->Draw(_GPUHandles["back"], XMUINT2(1, 1), _backGroundRect, Colors::White);

	_spriteBatch2->End();	//�o�b�`������
}

/// <summary>
/// �}�E�X�J�[�\���Ȃǂ�UI��`�悷��֐�
/// </summary>
void
SpriteManager::UIDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);

	//HP�Q�[�W�`�揈���i�V�F�[�_�[��K�p���邽�ߕ������Ă���j
	_spriteBatch1->Begin(_dx12.CommandList(), _GPUHandles["data"]);

	_spriteBatch1->Draw(_GPUHandles["gage"], XMUINT2(1, 1), _HPGageRect, Colors::White);

	_spriteBatch1->End();

	//���̑�UI�`�揈��
	_spriteBatch2->Begin(_dx12.CommandList());	//�o�b�`���Z�b�g

	_spriteBatch2->Draw(_GPUHandles["outline"], XMUINT2(1, 1), _HPGageRect);
	_spriteFont->DrawString(_spriteBatch2.get(), _wss.str().c_str(), XMFLOAT2(((float)_width / 3) + 60, 6), Colors::White);

	_spriteBatch2->End();	//�o�b�`������
}

/// <summary>
/// �}�E�X�J�[�\���p�̉摜��`�悷��֐�
/// </summary>
void
SpriteManager::CursorDraw()
{
	_dx12.CommandList()->SetDescriptorHeaps(1, &_heapForSpriteFont);

	//���̑�UI�`�揈��
	_spriteBatch2->Begin(_dx12.CommandList());	//�o�b�`���Z�b�g

	_spriteBatch2->Draw(_GPUHandles["cursor"], XMUINT2(50, 50), XMFLOAT2((float)_x, (float)_y), Colors::White);

	_spriteBatch2->End();	//�o�b�`������
}

/// <summary>
/// �O���t�B�b�N�X���������R�}���h�L���[�ɃZ�b�g����֐�
/// </summary>
void
SpriteManager::Commit()
{
	_gmemory->Commit(_dx12.CommandQueue());
}

/// <summary>
/// �}�E�X���W���Z�b�g����֐�
/// </summary>
/// <param name="x">X���W</param>
/// <param name="y">y���W</param>
void
SpriteManager::SetMousePosition(int x, int y)
{
	_x = x;
	_y = y;
}