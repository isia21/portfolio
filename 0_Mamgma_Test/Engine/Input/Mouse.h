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

	// --- Get Mouse Pos (Cur n Delta) ---
	int GetX() const { return m_lX; }
	int GetY() const { return m_lY; }
	int GetDeltaX() const { return m_lDeltaX; }
	int GetDeltaY() const { return m_lDeltaY; }

	// --- Get Mouse Wheel Delta (will reset per frame) ---
	int GetWheelDelta() const { return m_lWheelDelta; }

private:
	bool m_bCurrentState[Button_Count];
	bool m_bPreviousState[Button_Count];

	int m_lX;
	int m_lY;
	int m_lPreviousX;
	int m_lPreviousY;

	int m_lDeltaX;
	int m_lDeltaY;

	int m_lWheelDelta;
	int m_lWheelDeltaAccum;
};