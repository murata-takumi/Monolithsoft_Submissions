#include"Application.h"

/// <summary>
/// プログラム実行時最初に呼び出される関数
/// </summary>
/// <returns>関数が正常に終了したかどうか</returns>
#ifdef _DEBUG
int main() {
#else
#include <Windows.h>
int WINAPI WinMain(_In_ HINSTANCE,_In_opt_ HINSTANCE,_In_ LPSTR,_In_ int) {
#endif
	auto& app = Application::Instance();	//Applicationインスタンス作成

	//Applicationインスタンスの初期化が成功しなければ異常終了
	if (!app.Init())
	{
		return -1;
	}

	app.Run();			//ゲーム画面を描画
	app.Terminate();	//終了時の後始末

	return 0;			//正常終了
}