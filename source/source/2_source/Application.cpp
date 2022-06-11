#include "Application.h"
#include "Dx12Wrapper.h"
#include "PeraRenderer.h"
#include "FBXActor.h"
#include "Renderer.h"
#include "SpriteManager.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "TitleScene.h"
#include "PlayScene.h"
#include "ResultScene.h"

const unsigned int DISPLAY_WIDTH = GetSystemMetrics(SM_CXSCREEN);	//�f�B�X�v���C��
const unsigned int WINDOW_WIDTH = 1280;								//�E�B���h�E��
const unsigned int DISPLAY_HEIGHT = GetSystemMetrics(SM_CYSCREEN);	//�f�B�X�v���C����
const unsigned int WINDOW_HEIGHT = 720;								//�E�B���h�E����

/// <summary>
/// OS���瑗���Ă���f�[�^(���b�Z�[�W)����������֐�
/// </summary>
/// <param name="hwnd">   �E�B���h�E�����ʂ���f�[�^</param>
/// <param name="msg">    OS���瑗���Ă���f�[�^</param>
/// <param name="wpraram">1�ڂ̃��b�Z�[�W�̕ϐ�</param>
/// <param name="lparam" >2�ڂ̃��b�Z�[�W�̕ϐ�</param>
/// <returns></returns>
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);	//OS�ɃA�v���̏I����`����
		return 0;

	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(msg, wparam, lparam);
		Mouse::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_SYSKEYDOWN:
		Keyboard::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(msg, wparam, lparam);
		break;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);	//����̏������s��
}

/// <summary>
/// �Q�[���p�E�B���h�E���쐬����֐�
/// </summary>
/// <param name="hwnd">       �E�B���h�E�����ʂ���f�[�^</param>
/// <param name="windowClass">�E�B���h�E�쐬�p�f�[�^���i�[����\����</param>
void 
Application::CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass)
{
	windowClass.cbSize = sizeof(WNDCLASSEX);			//�\���̂̃������T�C�Y�w��
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure;	//�R�[���o�b�N�֐��̎w��
	windowClass.lpszClassName = _T("DX12Sample");		//�A�v���P�[�V�����N���X��
	windowClass.hInstance = GetModuleHandle(nullptr);	//�C���X�^���X(�A�v���P�[�V����)�n���h���̎擾

	RegisterClassEx(&windowClass);	                    //�A�v���P�[�V�����N���X�̎w���OS�ɓ`����

	RECT wrc = { 0,0,(LONG)WINDOW_WIDTH,(LONG)WINDOW_HEIGHT };			//�E�B���h�E�T�C�Y�̌���
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);	//�E�B���h�E�T�C�Y���v�Z

	hwnd = CreateWindow(windowClass.lpszClassName,		//�E�B���h�E�n���h���̓o�^
		_T("Program"),
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX ,
		(DISPLAY_WIDTH/2) - (WINDOW_WIDTH/2),
		(DISPLAY_HEIGHT / 2) - (WINDOW_HEIGHT / 2),
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		windowClass.hInstance,
		nullptr
	);

	//�t���[�����[�g���擾
	auto hdc = GetDC(hwnd);
	_rate = GetDeviceCaps(hdc, VREFRESH);

	//�t���[�����[�g�ɉ����ăt���b�v�Ԋu��ݒ�
	if (_rate <= 60) { _interval = 1; }
	if (_rate >= 120) { _interval = 2; }

	_deltaTime = (float)1 / (float)_rate;	//1�t���[�����̌o�ߕb�����v�Z
}

/// <summary>
/// �R���X�g���N�^
/// ���ɏ����͍s��Ȃ�
/// </summary>
Application::Application()
{

}

/// <summary>
/// �ÓI�C���X�^���X��Ԃ��֐�
/// </summary>
/// <returns>�C���X�^���X�̃A�h���X</returns>
Application&
Application::Instance()
{
	static Application instance;
	return instance;
}

/// <summary>
/// �A�v���P�[�V�����������֐�
/// </summary>
/// <returns>�������������������ǂ���</returns>
bool 
Application::Init()
{
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);						//�X���b�h�������s������COM���C�u������������
	CreateGameWindow(_hwnd, _windowClass);										//�Q�[���p�E�B���h�E�쐬

	ShowCursor(true);															//�}�E�X�J�[�\�����\���ɂ���

	_dx12.reset(new Dx12Wrapper(_hwnd,_deltaTime));								//Dx12Wrapper�C���X�^���X��������
	
	_sprite.reset(new SpriteManager(*_dx12,WINDOW_WIDTH,WINDOW_HEIGHT));		//DXTK12Manager�C���X�^���X��������
	
	_input.reset(new InputManager());											//InputManager�C���X�^���X��������

	_pera.reset(new PeraRenderer(*_dx12));										//PeraRenderer�C���X�^���X��������

	_renderer.reset(new Renderer(*_dx12));										//PMDRenderer�C���X�^���X��������

	_sound.reset(new SoundManager());											//SoundManager�C���X�^���X��������

	//���ꂼ��^�C�g���E�Q�[���E���U���g�V�[���̏�����
	_title.reset(new TitleScene(*_dx12,*_pera, *_renderer, *_sprite, *_input, *_sound));
	_play.reset(new PlayScene( *_dx12, *_pera, *_renderer,*_sprite,*_input,*_sound));
	_result.reset(new ResultScene( *_dx12, *_pera, *_renderer,*_sprite,*_input,*_sound));

	ShowWindow(_hwnd, SW_SHOW);			//�E�B���h�E�n���h���ɑΉ�����E�B���h�E��\��

	ChangeScene(SceneNames::Title);	//�^�C�g���V�[���֑J��

	return true;	//�������������������Ƃ�Ԃ�
}

/// <summary>
/// �Q�[����ʂ�`�悷��֐�
/// </summary>
void
Application::Run()
{
	//�Q�[�����[�v
	while (true)
	{
 		//�X���b�h�֌W�̏���
		if (PeekMessage(&_msg, nullptr, 0, 0, PM_REMOVE))	//���b�Z�[�W�𒲂ׂ�
		{
			TranslateMessage(&_msg);							//���b�Z�[�W��|��
			DispatchMessage(&_msg);								//�E�B���h�E�v���V�[�W���փ��b�Z�[�W�𑗂�

			if (_msg.message == WM_QUIT)						//�A�v�����I�������烋�[�v���I��������
			{
				break;
			}
		}
		else
		{
			_Scene->Update();	//�e�V�[���̍X�V����
		}
	}
}

/// <summary>
/// �A�v���I�����A��n�����s���֐�
/// </summary>
void 
Application::Terminate()
{
	UnregisterClass(_windowClass.lpszClassName, _windowClass.hInstance);	//�\���̂̐ݒ������
}

/// <summary>
/// �E�B���h�E�T�C�Y��Ԃ��֐�
/// </summary>
/// <returns>�E�B���h�E�T�C�Y�������\����</returns>
SIZE
Application::GetWindowSize()const
{
	SIZE ret;					//�\���̂�錾���A���ƍ�����ݒ�
	ret.cx = WINDOW_WIDTH;
	ret.cy = WINDOW_HEIGHT;

	return ret;					//�\���̂�Ԃ�
}

/// <summary>
/// �V�[����؂�ւ���֐�
/// </summary>
/// <param name="scene">�؂�ւ������V�[���I�u�W�F�N�g</param>
void
Application::SetScene(Scene* scene)
{
	if (_Scene != nullptr)_Scene->SceneEnd();	//�V�[���I�����̏���

	_Scene = scene;								//�V�[���؂�ւ�

	_Scene->SceneStart();						//�V�[���J�n���̏���
}

/// <summary>
/// �V�[����؂�ւ���֐�
/// </summary>
/// <param name="name">�؂�ւ������V�[��</param>
void
Application::ChangeScene(SceneNames name)
{
	//�����ɉ����đJ�ڐ�̃V�[�������߂�
	switch (name)
	{
	case SceneNames::Title:
		SetScene(_title.get());		//�^�C�g���V�[���֑J��
		break;
	case SceneNames::Play:
		SetScene(_play.get());		//�Q�[���V�[���֑J��
		break;
	case SceneNames::Result:
		SetScene(_result.get());	//���U���g�V�[���֑J��
		break;
	}
}

/// <summary>
/// �Q�[�����I��������
/// </summary>
void
Application::ExitApp()
{
	SendMessage(_hwnd,WM_DESTROY,0,0);	//�I���p���b�Z�[�W�𑗂�
}

/// <summary>
/// �����_�[�^�[�Q�b�g�̃t���b�v�Ԋu��Ԃ��֐�
/// </summary>
/// <returns>�t���b�v�Ԋu</returns>
int
Application::GetInterval()
{
	return _interval;
}

/// <summary>
/// 1�b�Ԃ̃t���[�����[�g��Ԃ��֐�
/// </summary>
/// <returns>�t���[�����[�g</returns>
int
Application::GetRate()
{
	return _rate;
}

/// <summary>
/// �f�R���X�g���N�^
/// ���ɏ����͍s��Ȃ�
/// </summary>
Application::~Application()
{

}