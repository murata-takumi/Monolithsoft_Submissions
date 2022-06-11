#pragma once
#include "Application.h"
#include <Mouse.h>
#include <Keyboard.h>

/// <summary>
/// キーボードやマウス等の入力を管理するクラス
/// </summary>
class SpriteManager;
class InputManager
{
private:
	unique_ptr<Keyboard> _keyboard;						//キーボード
	Keyboard::State _keyState;							//キー状態
	Keyboard::KeyboardStateTracker _keyboardButtons;	//押されているボタン

	unique_ptr<Mouse> _mouse;				//マウス
	Mouse::State _mouseState;				//マウス状態
	Mouse::ButtonStateTracker _tracker;		//マウス状態（ボタンを押した・離した瞬間を取得）

public:
	InputManager();									//コンストラクタ

	void UpdateInput();								//入力情報を更新する関数

	Keyboard::State KeyState();						//キー状態を取得

	Mouse::State MouseState();						//マウス状態を取得

	Mouse::ButtonStateTracker MouseTracker();		//マウス状態を取得(ボタンを押した・離した瞬間を取得)
};
