#pragma once

class CKeyboard
{
	DECLARE_SINGLETON(CKeyboard);

public:
	CKeyboard();
	~CKeyboard();

public:
	// --- Init n update per frame --- 
	void Update();

	// --- WndPrc processing ---
	void ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

	// --- Check Key.State (use VirtualKey - VK_*, like VK_SPACE)
	/// <summary>
	/// Is key VK_* was hold | pressed
	/// </summary>
	/// <param name="key">VK_*</param>
	/// <returns></returns>
	bool IsKeyDown(int key) const;
	/// <summary>
	/// Is key VK_* was pressed IN cur frame
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	bool IsKeyPressed(int key) const;
	/// <summary>
	/// Is key VK_* was up IN cur frame
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	bool IsKeyReleased(int key) const;  // Клавиша была отпущена именно в этом кадре

	// --- Deactive states by request --
	// in case if user click/do staff in UI we reset values 
	// cause instead we get double trigger: UI process click and World process same click
	void ConsumeKey(int key);
private:
	bool m_bCurrentState[256];
	bool m_bPreviousState[256];
};