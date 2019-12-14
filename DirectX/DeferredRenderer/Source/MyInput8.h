#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>	//dinput8.lib dxguid.libのリンクも必要です。

#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dinput8.lib")
//キーボード
class CInputKeyboard
{
	friend class CInput;

	IDirectInputDevice8*	m_pDIDevKB;			//デバイス
	bool					m_Keystate[256];	//更新時に押してたらON(押しっぱなしを拾う)
	bool					m_KeyAction[256];	//押した直後だけON(押しっぱなしはOFFになる)
												//配列インデックスはDIK_SPACEなど

	CInputKeyboard();//privateにしてる。クラスを作れるはCInputからだけ。
	bool Create(IDirectInput8* pDInput, HWND hWnd);
	void Release();
	void Update();

public:
	bool ChkKeyDown(int key) { return m_Keystate[key]; }
	bool ChkKeyAction(int key) { return m_KeyAction[key]; }
};

//マウス
class CInputMouse
{
	friend class CInput;

	IDirectInputDevice8*	m_pDIMouse;		//デバイス
	bool CreateMouse();
	void UpdateInput_Mouse();
	void ReleaseMouse();

	HWND					m_hWnd;
	int						m_wndWid;
	int						m_wndHgt;
public:
	bool					m_windowMode;
private:
	long					m_posX;	//マウス座標
	long					m_posY;

	long					m_moveX;//マウスの移動量
	long					m_moveY;//マウスの移動量

	long					m_moveWheel;

	bool					m_LDown;	//Update時点のボタン状態
	bool					m_RDown;
	bool					m_MDown;

	bool					m_LAction;	//ボタン押した直後だけONになる(クリックを拾うときに)
	bool					m_RAction;
	bool					m_MAction;

	bool					m_LClicked; //長押し用
	bool					m_RClicked;
	bool					m_MClicked;

	CInputMouse();
	bool Create(IDirectInput8* pDInput, HWND hWnd);
	void Release();
	void Update();

public:

	long PosX() { return m_posX; }
	long PosY() { return m_posY; }

	long MoveX() { return m_moveX; }
	long MoveY() { return m_moveY; }

	bool IsLAction() { return m_LAction; }
	bool IsRAction() { return m_RAction; }
	bool IsMAction() { return m_MAction; }

	bool IsLDown() { return m_LClicked; }
	bool IsRDown() { return m_RClicked; }
	bool IsMDown() { return m_MClicked; }

	//ウィンドウモードの指定
	//カーソル位置をAPIから取るか、DirectInputから取るかの違い
	void SetWindowMode(bool isWindowmode) { m_windowMode = isWindowmode; }//true:ウィンドウモード false:フルスクリーン

	void SetCursorPos(int x, int y)
	{
		m_posX = x;
		m_posY = y;
	}

	void SetCursorMove(int x, int y)
	{
		m_moveX = x; m_moveY = y;
	}
	long GetWheel() { return m_moveWheel; }
	void SetWheel(long n) { m_moveWheel = n; }
};

//ゲームパッド---------------------------------
//(十字キーと４つのボタンを扱う)
class CInputGamepad
{
	friend class CInput;

	IDirectInputDevice8*	m_pDIDevJS;			//デバイス
	bool					gamepadState[8];	//更新時に押してたらON
	bool					gamepadAction[8];	//押して次回更新するまでON
#define GamePadBtn_UP		0
#define GamePadBtn_DOWN		1
#define GamePadBtn_LEFT		2
#define GamePadBtn_RIGHT	3
#define GamePadBtn_Btn0		4
#define GamePadBtn_Btn1		5
#define GamePadBtn_Btn2		6
#define GamePadBtn_Btn3		7

	CInputGamepad();
	bool Create(IDirectInput8* pDInput, HWND hWnd);
	void Release();
	void Update();

public:

	bool ChkGamePadDown(int btn) { return gamepadState[btn]; }
	bool ChkGamePadAction(int btn) { return gamepadAction[btn]; }
};

//キーボード、マウス、ゲームパッド入力を管理するクラス
class CInput
{
	HWND					m_hWnd;

	//DirectInputオブジェクト
	IDirectInput8*			m_pDInput;

	//CInputで使うデバイス（初期化対象にする）
	int						m_useDevice;
#define UseInputDevice_KEYBOARD	0x01
#define UseInputDevice_MOUSE	0x02
#define UseInputDevice_GAMEPAD	0x04
#define UseInputDevice_ALL		(UseInputDevice_KEYBOARD | UseInputDevice_MOUSE | UseInputDevice_GAMEPAD)

	//各デバイス
	CInputKeyboard		m_keyboard;
	CInputMouse			m_mouse;
	CInputGamepad		m_gamepad;

public:

	CInput();
	~CInput();

	bool Create(HWND hWnd, int useDevice = UseInputDevice_ALL);
	void Release();

public:
	//入力状態を更新する
	void InputUpdate() {
		m_keyboard.Update();
		m_mouse.Update();
		m_gamepad.Update();
	}

	//各デバイスを参照する(ここから入力状態を取得する)
	CInputKeyboard* Keyboard() { return &m_keyboard; }
	CInputMouse* Mouse() { return &m_mouse; }
	CInputGamepad* Gamepad() { return &m_gamepad; }
};
