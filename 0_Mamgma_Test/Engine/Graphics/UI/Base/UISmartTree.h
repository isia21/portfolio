#pragma once
class CRenderer;
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
