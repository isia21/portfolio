#pragma once
//-----------------------------------------------------------------------------
// UI Callback definitions
//-----------------------------------------------------------------------------
typedef void(*UIEventCallback)();
typedef std::function<void()> UICallbackFn;

//-----------------------------------------------------------------------------
// Base UI Element
//-----------------------------------------------------------------------------
class CUIElement
{
public:
	CUIElement(int lX = 0, int lY = 0, int lWidth = 0, int lHeight = 0);
	virtual ~CUIElement() = default;

public:
	virtual void Update() {};
	virtual void Render(CRenderer* pRenderer) = 0;

	//	// Mouse event handlers (return true if event was consumed by element)
	//	virtual bool OnMouseMove(int lMouseX, int lMouseY);
	//	virtual bool OnMouseDown(int lMouseX, int lMouseY, int lButton);
	//	virtual bool OnMouseUp(int lMouseX, int lMouseY, int lButton);
	//	virtual bool OnMouseWheel(int lMouseX, int lMouseY, int zDelta) { return false; }
public:
	void SetPosition(int lX, int lY) { m_lX = lX; m_lY = lY; }
	void SetSize(int lWidth, int lHeight) { m_lWidth = lWidth; m_lHeight = lHeight; }

	int GetX() const { return m_lX; }
	int GetY() const { return m_lY; }
	int GetWidth() const { return m_lWidth; }
	int GetHeight() const { return m_lHeight; }

	void SetVisible(bool bVisible) { m_bVisible = bVisible; }
	bool IsVisible() const { return m_bVisible; }

	void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }
	bool IsEnabled() const { return m_bEnabled; }

	bool IsHovered() const { return m_bHovered; }
	bool IsPressed() const { return m_bPressed; }

	bool IsPointInside(int lX, int lY) const;

	// --- Parent-Child Relationship ---
	void SetParent(CUIElement* pParent) { m_pParent = pParent; }
	CUIElement* GetParent() const { return m_pParent; }

	// --- Absolute Positioning (relative to parent hierarchy) ---
	int GetAbsoluteX() const { return m_lX + (m_pParent != nullptr ? m_pParent->GetAbsoluteX() : 0); }
	int GetAbsoluteY() const { return m_lY + (m_pParent != nullptr ? m_pParent->GetAbsoluteY() : 0); }


protected:
	int m_lX;
	int m_lY;
	int m_lWidth;
	int m_lHeight;

	CUIElement* m_pParent;

	bool m_bVisible;
	bool m_bEnabled;
	bool m_bHovered;
	bool m_bPressed;
};

//-----------------------------------------------------------------------------
// TextBox Element (Background Rect + Text Label)
//-----------------------------------------------------------------------------
class CUITextBox : public CUIElement
{
public:
	CUITextBox(
		int lX = 0, int lY = 0,
		int lWidth = 100, int lHeight = 30,
		const char* pszText = "",
		unsigned int dwBgColor = 0x303030FF,
		unsigned int dwTextColor = 0xFFFFFFFF,
		int lFontSize = 14,
		ETextAlignment eAlignment = TEXT_ALIGN_CENTER);

	virtual ~CUITextBox() override = default;

public:
	virtual void Render(CRenderer* pRenderer) override;

public:
	void SetText(const char* pszText);
	const char* GetText() const { return m_szText.c_str(); }

	void SetBgColor(unsigned int dwColor) { m_dwBgColor = dwColor; }
	unsigned int GetBgColor() const { return m_dwBgColor; }

	void SetTextColor(unsigned int dwColor) { m_dwTextColor = dwColor; }
	unsigned int GetTextColor() const { return m_dwTextColor; }

	void SetFontSize(int lSize) { m_lFontSize = lSize; }
	int GetFontSize() const { return m_lFontSize; }

	void SetTextAlignment(ETextAlignment eAlignment) { m_eAlignment = eAlignment; }
	ETextAlignment GetTextAlignment() const { return m_eAlignment; }

protected:
	std::string m_szText;

	unsigned int m_dwBgColor;
	unsigned int m_dwTextColor;

	int m_lFontSize;
	ETextAlignment m_eAlignment;
};


//-----------------------------------------------------------------------------
// Button Element (TextBox + Hover/Press States + Callback)
//-----------------------------------------------------------------------------
class CUIButton : public CUITextBox
{
public:
	CUIButton(
		int lX = 0, int lY = 0,
		int lWidth = 120, int lHeight = 35,
		const char* pszText = "Button",
		const UICallbackFn& fnCallback = nullptr);

	virtual ~CUIButton() override = default;

public:
	virtual void Update() override;
	virtual void Render(CRenderer* pRenderer) override;

	//	virtual bool OnMouseMove(int lMouseX, int lMouseY) override;
	//	virtual bool OnMouseDown(int lMouseX, int lMouseY, int lButton) override;
	//	virtual bool OnMouseUp(int lMouseX, int lMouseY, int lButton) override;

public:
	void SetOnClick(UIEventCallback pfnCallback) { m_fnOnClick = pfnCallback; }
	void SetOnClick(const UICallbackFn& fnCallback) { m_fnOnClick = fnCallback; }

	void SetNormalColor(unsigned int dwColor) { m_dwNormalColor = dwColor; }
	void SetHoverColor(unsigned int dwColor) { m_dwHoverColor = dwColor; }
	void SetPressedColor(unsigned int dwColor) { m_dwPressedColor = dwColor; }

protected:
	unsigned int m_dwNormalColor;
	unsigned int m_dwHoverColor;
	unsigned int m_dwPressedColor;

	UICallbackFn m_fnOnClick;
};

//-----------------------------------------------------------------------------
// Tree Node Definition
//-----------------------------------------------------------------------------
struct SUITreeNode
{
	std::string sText;
	void* pUserData;

	bool bExpanded;
	bool bObjectVisible;
	bool bSelected;

	SUITreeNode* pParent;
	std::vector<SUITreeNode*> vChildren;

	SUITreeNode(const char* pszText = "", void* pData = nullptr, SUITreeNode* pParentNode = nullptr)
		: sText(pszText != nullptr ? pszText : "")
		, pUserData(pData)
		, bExpanded(true)
		, bObjectVisible(true)
		, bSelected(false)
		, pParent(pParentNode)
	{}

	~SUITreeNode()
	{
		for (SUITreeNode* pChild : vChildren)
			delete pChild;
		vChildren.clear();
	}

	SUITreeNode* AddChild(const char* pszChildText, void* pChildData = nullptr)
	{
		SUITreeNode* pNewChild = new SUITreeNode(pszChildText, pChildData, this);
		vChildren.push_back(pNewChild);
		return pNewChild;
	}
};


// --- Callbacks for SmartTree ---
typedef std::function<void(SUITreeNode* pNode)> UITreeSelectCallback;
typedef std::function<void(SUITreeNode* pNode, bool bVisible)> UITreeVisibilityCallback;

//-----------------------------------------------------------------------------
// CUISmartTree (Hierarchical Scene Outliner Element)
//-----------------------------------------------------------------------------
class CUISmartTree : public CUIElement
{
public:
	CUISmartTree(
		int lX = 0, int lY = 0,
		int lWidth = 200, int lHeight = 200,
		int lItemHeight = 20,
		int lFontSize = 12);

	virtual ~CUISmartTree() override;

public:
	virtual void Update() override;
	virtual void Render(CRenderer* pRenderer) override;

	//	virtual bool OnMouseMove(int lMouseX, int lMouseY) override;
	//	virtual bool OnMouseDown(int lMouseX, int lMouseY, int lButton) override;
	//	virtual bool OnMouseUp(int lMouseX, int lMouseY, int lButton) override;
	//	virtual bool OnMouseWheel(int lMouseX, int lMouseY, int zDelta) override;

public:
	SUITreeNode* AddRoot(const char* pszText, void* pUserData = nullptr);
	SUITreeNode* AddChild(SUITreeNode* pParent, const char* pszText, void* pUserData = nullptr);
	void Clear();

	void SetOnSelect(const UITreeSelectCallback& fn) { m_fnOnSelect = fn; }
	void SetOnToggleVisibility(const UITreeVisibilityCallback& fn) { m_fnOnToggleVisibility = fn; }

	SUITreeNode* GetSelectedNode() const { return m_pSelectedNode; }
	void SetSelectedNode(SUITreeNode* pNode);

	void SetBgColor(unsigned int dwColor) { m_dwBgColor = dwColor; }
	void SetSelectedColor(unsigned int dwColor) { m_dwSelectedBgColor = dwColor; }
	void SetHoverColor(unsigned int dwColor) { m_dwHoverBgColor = dwColor; }

private:
	// --- Service structure for flattening visible tree nodes for rendering and mouse hit-testing ---
	struct SFlatItem
	{
		SUITreeNode* pNode;
		int lLevel;
		int lItemY;
	};

	void FlattenVisibleNodes(SUITreeNode* pNode, int lLevel, int& lCurrentY, std::vector<SFlatItem>& vOutList) const;

private:
	std::vector<SUITreeNode*> m_vRoots;
	SUITreeNode* m_pSelectedNode;
	SUITreeNode* m_pHoveredNode;

	int m_lItemHeight;
	int m_lIndentSize;
	int m_lFontSize;

	int m_lScrollY;

	unsigned int m_dwBgColor;
	unsigned int m_dwSelectedBgColor;
	unsigned int m_dwHoverBgColor;
	unsigned int m_dwTextColor;

	UITreeSelectCallback m_fnOnSelect;
	UITreeVisibilityCallback m_fnOnToggleVisibility;
};

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
	void AddChild(CUIElement* pChild);
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

	CUIButton* m_pBtnCollapse;
	std::vector<CUIElement*> m_vChildren;
};

//-----------------------------------------------------------------------------
// UI Manager / Canvas (Stores elements, handles dispatching)
//-----------------------------------------------------------------------------
class CUIManager
{
public:
	CUIManager();
	~CUIManager();

public:
	void AddElement(CUIElement* pElement);
	void RemoveElement(CUIElement* pElement);
	void Clear();

	void Update();
	void Render(CRenderer* pRenderer);

	//	// --- Dispatchers for Win32 Messages (WM_MOUSEMOVE, WM_LBUTTONDOWN, WM_LBUTTONUP) ---
	//	bool ProcessMouseMove(int lMouseX, int lMouseY);
	//	bool ProcessMouseDown(int lMouseX, int lMouseY, int lButton = 0);
	//	bool ProcessMouseUp(int lMouseX, int lMouseY, int lButton = 0);
	//	bool ProcessMouseWheel(int lMouseX, int lMouseY, int zDelta);

private:
	std::vector<CUIElement*> m_vElements;
};