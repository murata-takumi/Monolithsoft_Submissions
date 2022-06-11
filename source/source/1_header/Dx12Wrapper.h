#include "Application.h"
#include "Functions.h"

/// <summary>
/// �J�������ǂ���̕��ɉ�]�����邩���肷��񋓑�
/// </summary>
enum Degree
{
	Azimth,			//���ʊp
	Elevation,		//�p
};

/// <summary>
/// �`�施�߂����s���邽�߂̃I�u�W�F�N�g�����Ǘ�����N���X
/// </summary>
class SphericalCoordinates;
class Dx12Wrapper
{
	SIZE _winSize;	//�E�B���h�E�T�C�Y

	XMFLOAT3 _initialPos;	//�J�����̏������W

	template<typename T>
	using ComPtr = ComPtr<T>;

	unique_ptr<SphericalCoordinates> _coordinates;	//�J�������W�p�C���X�^���X

	//DXGI����
	ComPtr<IDXGIFactory6> _dxgiFactory;	//�t�@�N�g���[
	ComPtr<ID3D12Device> _dev;			//�f�o�C�X

	//DX12(�R�}���h)����
	ComPtr<ID3D12CommandAllocator> _cmdAllocator;	//�R�}���h�A���P�[�^
	ComPtr<ID3D12GraphicsCommandList> _cmdList;		//�R�}���h���X�g
	ComPtr<ID3D12CommandQueue> _cmdQueue;			//�R�}���h�L���[
	ComPtr<IDXGISwapChain4> _swapchain;				//�X���b�v�`�F�[��

	//�\���֘A�̃o�b�t�@���
	vector<ID3D12Resource*> _backBuffers;		//�����_�[�^�[�Q�b�g�p�o�b�t�@�[
	ComPtr<ID3D12DescriptorHeap> _rtvHeap;		//�����_�[�^�[�Q�b�g�r���[�p�q�[�v
	ComPtr<ID3D12Resource> _depthBuffer;		//�[�x�X�e���V���p�o�b�t�@�[
	ComPtr<ID3D12DescriptorHeap> _dsvHeap;		//�[�x�X�e���V���r���[�p�q�[�v

	//unique_ptr<class>��class�����Ƃ���C���X�^���X��1�����Ȃ���ԂƂ���|�C���^
	unique_ptr<D3D12_VIEWPORT> _viewPort = {};	//�r���[�|�[�g
	unique_ptr<D3D12_RECT> _rect = {};			//�V�U�[��`

	//�r���[�v���W�F�N�V�����p�萔�o�b�t�@����
	ComPtr<ID3D12Resource> _sceneConstBuff;				//�r���[�E�v���W�F�N�V�����p�o�b�t�@�[
	ComPtr<ID3D12DescriptorHeap> _sceneDescHeap;		//�r���[�E�v���W�F�N�V�����p�f�B�X�N���v�^�q�[�v

	//�c�݃e�N�X�`������
	ComPtr<ID3D12DescriptorHeap> _effectSRVHeap;	//�c�݃e�N�X�`���p�q�[�v
	ComPtr<ID3D12Resource> _effectTexBuffer;		//�c�݃e�N�X�`���p�o�b�t�@�[
	ComPtr<ID3D12DescriptorHeap> _factorCBVHeap;	//�c�݃f�[�^�p�q�[�v
	ComPtr<ID3D12Resource> _factorConstBuff;		//�c�݃f�[�^�p�o�b�t�@�[

	/// <summary>
	/// �V�F�[�_�[�ɘc�݁E�t�F�[�h�C���^�A�E�g�f�[�^��n�����߂̍\���̋y�уC���X�^���X
	/// </summary>
	struct Factor 
	{
		float dist;			//�c�݃f�[�^
		float fade;			//�t�F�[�h�C���^�A�E�g�f�[�^
	};
	Factor* _mappedFactor = nullptr;

	/// <summary>
	/// �X���b�g�ɗ������ރf�[�^�̍\���̋y�уC���X�^���X
	/// �e�s���ʌɗ������ނ̂ł͂Ȃ��A�\���̂Ƃ��Ă܂Ƃ߂�
	/// </summary>
	struct SceneData
	{
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT3 eye;
	};
	SceneData* _mappedScene;

	ComPtr<ID3D12Fence> _fence;					//�t�F���X
	UINT64 _fenceVal;							//�������l

	CD3DX12_RESOURCE_BARRIER _barrier;			//���\�[�X�o���A�p�ϐ�

	XMFLOAT3 _eye;		//���_���W
	XMFLOAT3 _target;	//�����_���W
	XMFLOAT3 _up;		//����W

	float _deltaTime;	//�t���[���Ԃ̎���

	float _dist;		//��ʂ�c�܂��邩�ǂ��������߂�f�[�^
	float _fade;		//�t�F�[�h�C���^�A�E�g���邩�ǂ��������߂�f�[�^

	float _start, _end;	//�t�F�[�h�C���^�A�E�g�Ő��`�⊮����ۂ̏����l�A�ŏI�l

	//�e�N�X�`�����[�h�p�����_�����i�[����^�̒�`
	//function<R,ArgsTypes>�Ƃ́AR��Ԃ�l�ɁAArgsTypes���������X�g�Ɏ��֐���ێ��ł���N���X�e���v���[�g
	//�����ł�const wstring&, TexMetadata*,ScratchImage&�������ɁAHRESULT��Ԃ�l�Ƃ����u�֐��̌^�v���`���Ă���
	using LoadLambda_t = function<
		HRESULT(const wstring& path, TexMetadata*, ScratchImage&)>;

	map<string, ID3D12Resource*> _resourceTable;	//string��key,���\�[�X��value�Ƃ����A�z�z��

	HRESULT InitializeDXGIDevice();					//�f�o�C�X�֘A������������֐�

	HRESULT InitializeCommand();					//�R�}���h�֘A������������֐�

	HRESULT CreateSwapChain(const HWND& hwnd);		//�X���b�v�`�F�[�����쐬����֐�

	HRESULT CreateRenderTargetsView();				//�����_�[�^�[�Q�b�g���쐬����֐�

	HRESULT CreateSceneView();						//�r���[�v���W�F�N�V�����p�r���[���쐬����֐�

	HRESULT CreateDepthStencilView();				//�[�x�X�e���V���r���[���쐬����֐�

	HRESULT CreateEffectBufferAndView();			//�c�݃e�N�X�`���p�q�[�v�E�o�b�t�@�[���쐬����֐�

	HRESULT CreateFactorBufferAndView();			//�G�t�F�N�g�K�p�����߂�f�[�^�p�̃q�[�v�E�o�b�t�@�[���쐬����֐�

	void CreateTextureLoaderTable();				//�e�N�X�`�����[�g�p�e�[�u�����쐬����֐�
public:
	map<string, LoadLambda_t> _loadLambdaTable;		//string��key,LoadLambda_t��value�Ƃ����A�z�z��

	Dx12Wrapper(HWND hwnd, float deltatime);		//�R���X�g���N�^
	~Dx12Wrapper();									//�f�R���X�g���N�^

	void BarrierTransition(				//���\�[�X��J�ڂ�����֐�
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, 
		D3D12_RESOURCE_STATES after);

	void TranslateSphericalCoordinates(int x);							//�J�������ߕt����E��������֐�
	void RotateSphericalCoordinates(Degree rotation, int direction);	//�J��������]������֐�
	void ResetSphericalCoordinates();									//�J�����̈ʒu������������֐�

	void SetScene();			//�r���[�v���W�F�N�V�����p�r���[���R�}���h���X�g�ɃZ�b�g����֐�

	void BeginGameDraw();		//�Q�[���p���\�[�X�̑J��(STATE_PRESENT��RENDER_TARGET)�ERTV�̃Z�b�g�����s����֐�
	void EndGameDraw();			//�Q�[���p���\�[�X�̑J��(RENDER_TARGET��STATE_PRESENT)�����s����֐�

	void WaitForCommandQueue();	//�����̓����҂����s���֐�

	ID3D12Resource* LoadTextureFromFile(const char* texPath);	//�e�N�X�`����ǂݍ��ފ֐�

	ID3D12DescriptorHeap* CreateDescriptorHeapForSpriteFont();	//DXTK12�p�̃q�[�v���쐬����֐�

	void SetPipelines(ID3D12RootSignature* rootSignature,	//���[�g�V�O�l�`���E�p�C�v���C���E�`����@���Z�b�g����֐�
		ID3D12PipelineState* pipeline, 
		D3D12_PRIMITIVE_TOPOLOGY topology);

	void UpdateFactor();	//��ʘc�݁E�t�F�[�h�C���^�A�E�g�f�[�^���V�F�[�_�[�ɔ��f����֐�

	void SetDist(float dist);		//��ʂ�c�܂���f�[�^��ݒ肷��֐�

	void Fade(function<void()> func,float start,float end);		//�t�F�[�h�C���^�A�E�g�����s����֐�

	ID3D12Device* Device();										//�f�o�C�X��Ԃ��֐�
	IDXGISwapChain4* Swapchain();								//�X���b�v�`�F�[����Ԃ��֐�
	ID3D12GraphicsCommandList* CommandList();					//�R�}���h���X�g��Ԃ��֐�
	ID3D12CommandQueue* CommandQueue();							//�R�}���h�L���[��Ԃ��֐�
	ID3D12Resource* BackBuffer() const;							//�o�b�N�o�b�t�@�[�i1���ځj��Ԃ��֐�
	ID3D12DescriptorHeap* RTVHeap() const;						//RTV�q�[�v��Ԃ��֐�
	ID3D12DescriptorHeap* DSVHeap() const;						//�[�x�X�e���V���q�[�v��Ԃ��֐�
	ID3D12DescriptorHeap* EffectSRVHeap() const;				//�G�t�F�N�g�p�q�[�v��Ԃ��֐�
	ID3D12DescriptorHeap* FactorCBVHeap() const;				//�����p�q�[�v��Ԃ��֐�
	D3D12_VIEWPORT* ViewPort() const;							//�r���[�|�[�g��Ԃ��֐�
	D3D12_RECT* Rect() const;									//�V�U�[��`��Ԃ��֐�

private:

};

