#pragma once
class CRenderer;
//-----------------------------------------------------------------------------
// UI Window / Panel (Container with Title Bar, Drag&Drop, Collapse & Children)
//-----------------------------------------------------------------------------
class CUIWindow : public CUIElement
{
public:
	CUIWindow(
		int lX = 0, int lY = 0,
		int lWidth = 200, int lHeight = 200,
		const char* pszTitle = "Window",
		unsigned int dwBgColor = 0x1E1E1EE6,
		unsigned int dwHeaderColor = 0x2C2C2CFF,
		unsigned int dwBorderColor = 0x484848FF,
		int lBorderSize = 1);

	virtual ~CUIWindow() override;

public:
	virtual void Update() override;
	virtual void Render(CRenderer* pRenderer) override;

	//	virtual bool OnMouseMove(int lMouseX, int lMouseY) override;
	//	virtual bool OnMouseDown(int lMouseX, int lMouseY, int lButton) override;
	//	virtual bool OnMouseUp(int lMouseX, int lMouseY, int lButton) override;
	//	virtual bool OnMouseWheel(int lMouseX, int lMouseY, int zDelta) override;
public:
	// - Add single elem (Vertical Stack)
	void AddChild(CUIElement* pChild);

	// - Add array of elems aka ROW (Horizontal Row / Flexbox)
	void AddChild(const std::vector<CUIElement*>& vRow);
	void AddChild(std::initializer_list<CUIElement*> list);

	// - just alias
	void AddRow(std::initializer_list<CUIElement*> list) { AddChild(list); }
	void AddRow(const std::vector<CUIElement*>& vRow) { AddChild(vRow); }

	void RemoveChild(CUIElement* pChild);
	void ClearChildren();

	void SetTitle(const char* pszTitle);
	const char* GetTitle() const { return m_szTitle.c_str(); }

	void SetCollapsed(bool bCollapsed);
	bool IsCollapsed() const { return m_bCollapsed; }

	int GetHeaderHeight() const { return m_lHeaderHeight; }
	void SetHeaderHeight(int lHeight) { m_lHeaderHeight = lHeight; }

	void SetBgColor(unsigned int dwColor) { m_dwBgColor = dwColor; }
	void SetHeaderColor(unsigned int dwColor) { m_dwHeaderColor = dwColor; }
	void SetBorderColor(unsigned int dwColor) { m_dwBorderColor = dwColor; }
	void SetBorderSize(int lSize) { m_lBorderSize = lSize; }

protected:
	bool IsPointInsideHeader(int lX, int lY) const;

	// find bottom edge Y
	int GetContentBottomY() const; 

public:
	void SetPadding(int lPadX, int lPadY) { m_lPaddingX = lPadX; m_lPaddingY = lPadY; }
	int GetPaddingX() const { return m_lPaddingX; }
	int GetPaddingY() const { return m_lPaddingY; }

protected:
	std::string m_szTitle;
	int m_lHeaderHeight;

	unsigned int m_dwBgColor;
	unsigned int m_dwHeaderColor;
	unsigned int m_dwBorderColor;
	int m_lBorderSize;

	// --- Window stats ---
	bool m_bCollapsed;
	bool m_bDragging;
	int m_lDragOffsetX;
	int m_lDragOffsetY;

	int m_lPaddingX = 6;
	int m_lPaddingY = 6;


	CUIButton* m_pBtnCollapse;
	std::vector<CUIElement*> m_vChildren;
};
