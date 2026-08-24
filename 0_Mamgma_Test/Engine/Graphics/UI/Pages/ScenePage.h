#pragma once

class CScenePage : public CUIWindow
{
public:
	CScenePage();
	virtual ~CScenePage();
	void Init();
	void RebuildSceneTree();

private:
	CUIButton* m_pBtnSceneNew;
	CUIButton* m_pBtnSceneLoad;
	CUIButton* m_pBtnSceneSave;
	CUIButton* m_pBtnSceneExport;
public:
	void SceneNew();
	void SceneLoad();
	void SceneSave();
	void SceneExport();
private:
	CUISmartTree* m_pSceneTree;

	CUIButton* m_pBtnSlicerAdd;
	CUIButton* m_pBtnSlicerRun;
	CUIButton* m_pBtnSlicerReset;
	void SlicerAdd();
	void SlicerRun();
	void SlicerReset();

};