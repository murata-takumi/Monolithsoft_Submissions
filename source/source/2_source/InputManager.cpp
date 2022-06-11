#include "InputManager.h"

/// <summary>
/// コンストラクタ
/// </summary>
InputManager::InputManager()
{
	_keyboard = make_unique<Keyboard>();	//Keyboardインスタンスを初期化
	_mouse = make_unique<Mouse>();			//Mouseインスタンスを初期化

	UpdateInput();							//入力を更新
}

/// <summary>
/// 入力情報を更新する関数
/// </summary>
void
InputManager::UpdateInput()
{
	//キーボードの入力を更新
	_keyState = _keyboard->GetState();
	_keyboardButtons.Update(_keyState);

	//マウスの入力を更新
	_mouseState = _mouse->GetState();	
	_tracker.Update(_mouseState);
}

/// <summary>
/// キー状態を返す関数
/// </summary>
/// <returns>キー状態</returns>
Keyboard::State
InputManager::KeyState()
{
	return _keyState;
}

/// <summary>
/// マウス状態を返す関数
/// </summary>
/// <returns>マウス状態</returns>
Mouse::State
InputManager::MouseState()
{
	return _mouseState;
}

/// <summary>
/// マウス状態を返す関数(ボタンを押した・離した瞬間を取得)
/// </summary>
/// <returns>マウス状態</returns>
Mouse::ButtonStateTracker
InputManager::MouseTracker()
{
	return _tracker;
}
