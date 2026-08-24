#pragma once
//-----------------------------------------------------------------------------
// UI Slider Element (Horizontal / Vertical)
//-----------------------------------------------------------------------------
enum ESliderOrientation
{
	eSO_Horizontal = 0,
	eSO_Vertical
};

typedef std::function<void(float)> UISliderCallbackFn;

class CUISlider : public CUIElement
{
public:
	CUISlider(
		int lX, int lY,
		int lWidth, int lHeight,
		float fMin, float fMax, float fCurrent, float fStep,
		ESliderOrientation eOrientation = eSO_Horizontal);

	virtual ~CUISlider() override = default;

public:
	virtual void Update() override;
	virtual void Render(CRenderer* pRenderer) override;

public:
	void SetValue(float fValue);
	float GetValue() const { return m_fValue; }

	void SetOnValueChanged(const UISliderCallbackFn& fnCallback) { m_fnOnValueChanged = fnCallback; }

private:
	float m_fMin;
	float m_fMax;
	float m_fValue;
	float m_fStep;

	ESliderOrientation m_eOrientation;

	// UI States
	bool m_bDraggingThumb;
	bool m_bHoverMinus;
	bool m_bHoverPlus;
	bool m_bHoverTrack;

	// Styling
	unsigned int m_dwBgColor;
	unsigned int m_dwThumbColor;
	unsigned int m_dwBtnNormalColor;
	unsigned int m_dwBtnHoverColor;
	unsigned int m_dwBtnPressedColor;
	unsigned int m_dwTextColor;

	UISliderCallbackFn m_fnOnValueChanged;
};