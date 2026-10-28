
#pragma once

#include "../../xrUI/Widgets/UIWindow.h"
#include "../../xrUI/Widgets/UIWndCallback.h"
#include "../encyclopedia_article_defs.h"
class CUINewsWnd;
class CUIFrameLineWnd;
class CUIFrameWindow;
class CUIAnimatedStatic;
class CUIStatic;
class CUITabControl;
class CUIScrollView;
class CUIListWnd;
class CUITreeViewItem;
class CEncyclopediaArticle;
class CUIGamepadLegend;
class CPdaUiSounds;

class CUIDiaryWnd: public CUIWindow, public CUIWndCallback
{
	typedef CUIWindow inherited;
	enum EDiaryFilter{
//			eInfo,
			eJournal=0,
			eNews,
			eEncyclopedia,
			eNone
	};
protected:
	EDiaryFilter		m_currFilter;

	CUINewsWnd*			m_UINewsWnd;

	CUIWindow*			m_UILeftWnd;
	CUIWindow*			m_UIRightWnd;
	CUIFrameWindow*		m_UILeftFrame;
	CUIFrameLineWnd*	m_UILeftHeader;
	CUIFrameWindow*		m_UIRightFrame;
	CUIFrameLineWnd*	m_UIRightHeader;
	CUIAnimatedStatic*	m_UIAnimation = nullptr;
	CUIListWnd*			m_SrcListWnd;
	CUIScrollView*		m_DescrView;
	CGameFont*			m_pTreeRootFont;
	u32					m_uTreeRootColor;
	CGameFont*			m_pTreeItemFont;
	u32					m_uTreeItemColor;
	CPdaUiSounds*		m_pUiSounds = nullptr;
	shared_str			TreeHighlightTexture;
	Frect				TreeHighlightRect{};

	xr_vector<Fvector2>	m_sign_places;
	CUIStatic*			m_updatedSectionImage = nullptr;
	CUIStatic*			m_oldSectionImage = nullptr;

	typedef xr_vector<CEncyclopediaArticle*>			ArticlesDB;
	typedef xr_vector<CEncyclopediaArticle*>::iterator	ArticlesDB_it;
	ArticlesDB				m_ArticlesDB;

			void 	OnFilterChanged			(CUIWindow*,void*);
			void 	OnSrcListItemClicked	(CUIWindow*,void*);
			void		UnloadJournalTab		();
			void		LoadJournalTab			(ARTICLE_DATA::EArticleType _type);
			void		UnloadNewsTab			();
			void		LoadNewsTab				();
			void		Reload					(EDiaryFilter new_filter);
			EDiaryFilter	ActiveFilter			() const;
			void		ApplyTreeHighlight		(CUITreeViewItem* Item);
			void		UpdateGamepadLegend		();
public:
	CUITabControl*		m_FilterTab;
						CUIDiaryWnd				();
	virtual ~CUIDiaryWnd			() override;

	virtual void		SendMessage				(CUIWindow* pWnd, s16 msg, void* pData) override;
	virtual void		Draw					() override;
	virtual void		Reset					() override;

			void		Init					();
			void		SetUiSounds				(CPdaUiSounds* uiSounds) { m_pUiSounds = uiSounds; }
			void		AddNews					();
			void		MarkNewsAsRead			(bool status);
	virtual void		Show					(bool status) override;
	virtual void		Update					() override;
	virtual bool		OnGamepadKeyAction		(int id, EUIMessages gamepad_action) override;
	virtual bool		OnGamepadKeyHold		(int id) override;

	CUIGamepadLegend*	m_gamepad_legend = nullptr;

	virtual CUIWindow* ui_cast_window() override { return this; }
};

