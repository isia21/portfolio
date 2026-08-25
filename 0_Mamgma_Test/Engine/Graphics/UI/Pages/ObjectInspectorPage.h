#pragma once

class CObjectInspectorPage : public CUIWindow
{
public:
	CObjectInspectorPage();
	virtual ~CObjectInspectorPage();
	void Init();
	void UpdateInspector();

private:

	CUITextBox* m_pTxtInspectorName;
	CUITextBox* m_pTxtInspectorStats;
	CUITextBox* m_pTxtInspectorTransform;
	CUIButton* m_pBtnToggleWireframe;
	CUIButton* m_pBtnDeleteObject;

	void ObjectToggleWireframe();
	void ObjectDelete();


	CUITextBox* m_pTxtRotX;
	CUISlider* m_pSliderRotX;
	CUITextBox* m_pTxtRotY;
	CUISlider* m_pSliderRotY;
	CUITextBox* m_pTxtRotZ;
	CUISlider* m_pSliderRotZ;

	CUITextBox* m_pTxtPosX;
	CUISlider* m_pSliderPosX;
	CUITextBox* m_pTxtPosY;
	CUISlider* m_pSliderPosY;
	CUITextBox* m_pTxtPosZ;
	CUISlider* m_pSliderPosZ;
};