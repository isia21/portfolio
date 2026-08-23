#pragma once

class CMouse
{
	DECLARE_SINGLETON(CMouse);

public:
	enum EMouseButton
	{
		Button_Left = 0,
		Button_Right,
		Button_Middle,
		Button_Count
	};

public:
	CMouse();
	~CMouse();

public:
	// --- Init n update per frame --- 
	void Update();

	// --- WndPrc processing ---
	void ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

	// --- Check Key.State (use EMouseButton) ---
	bool IsButtonDown(EMouseButton button) const;
	bool IsButtonPressed(EMouseButton button) const;
	bool IsButtonReleased(EMouseButton button) const;
	// --- Raw Mouse State (Consume ignore) ---
	bool IsButtonDownRaw(EMouseButton button) const;

	// --- Get Mouse Pos (Cur n Delta) ---
	int GetX() const { return m_lX; }
	int GetY() const { return m_lY; }
	int GetDeltaX() const { return m_lDeltaX; }
	int GetDeltaY() const { return m_lDeltaY; }

	// --- Get Mouse Wheel Delta (will reset per frame) ---
	int GetWheelDelta() const;

	// --- Deactive states by request --
	// in case if user click/do staff in UI we reset values 
	// cause instead we get double trigger: UI process click and World process same click
	void ConsumeButton(EMouseButton button);
	void ConsumeWheel();
private:
	bool m_bCurrentState[Button_Count];
	bool m_bPreviousState[Button_Count];

	bool m_bConsumed[Button_Count];
	bool m_bWheelConsumed;

	int m_lX;
	int m_lY;
	int m_lPreviousX;
	int m_lPreviousY;

	int m_lDeltaX;
	int m_lDeltaY;

	int m_lWheelDelta;
	int m_lWheelDeltaAccum;
};