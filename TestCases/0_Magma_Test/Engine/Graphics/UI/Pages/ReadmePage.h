#pragma once

class CReadmePage : public CUIWindow
{
public:
	CReadmePage();
	virtual ~CReadmePage();
	void Init();

private:

	struct SShortcutHint {
		const char* szKey;
		const char* szDesc;
		unsigned int dwColor;
	};

	std::vector<CUITextBox*> m_vTxtKey;
	std::vector<CUITextBox*> m_vTxtText;
};