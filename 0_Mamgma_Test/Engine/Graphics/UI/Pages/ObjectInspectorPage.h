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
};