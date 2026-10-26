#include "StdAfx.h"
#include "UIOutfitSlot.h"
#include "../../xrUI/Widgets/UIStatic.h"
#include "UICellItem.h"
#include "../CustomOutfit.h"
#include "../Actor.h"
#include "UIInventoryUtilities.h"

CUIOutfitDragDropList::CUIOutfitDragDropList()
{
	m_background				= new CUI3dStatic();
	m_background->SetAutoDelete	(true);
	AttachChild					(m_background);

	FullIcon					= new CUIStatic();
	FullIcon->SetAutoDelete		(true);
	AttachChild					(FullIcon);
}

CUIOutfitDragDropList::~CUIOutfitDragDropList()
{
}

#include "../Level.h"

void CUIOutfitDragDropList::SetUseFullIcon2D(bool value)
{
	UseFullIcon2D = value;
}

void CUIOutfitDragDropList::SetOutfit2D(CUICellItem* itm)
{
	FullIcon->SetWndPos(Fvector2().set(0, 0));
	FullIcon->SetWndSize(Fvector2().set(GetWidth(), GetHeight()));
	FullIcon->SetStretchTexture(true);

	PIItem item = itm != nullptr ? static_cast<PIItem>(itm->m_pData) : nullptr;
	CCustomOutfit* outfit = item != nullptr ? item->cast_outfit() : nullptr;
	if (outfit != nullptr && outfit->GetFullIconName().size() != 0)
	{
		FullIcon->InitTexture(outfit->GetFullIconName().c_str(), false);
	}
	else
	{
		FullIcon->InitTexture("npc_icon_without_outfit", false);
	}
}

void CUIOutfitDragDropList::SetOutfit(CUICellItem* itm)
{
	if (UseFullIcon2D)
	{
		SetOutfit2D(itm);
		return;
	}

	m_background->SetWndPos(Fvector2().set(0, 0));
	m_background->SetWndSize(Fvector2().set(GetWidth(), GetHeight()));

	m_background->SetStretchTexture(true);
	CObject* current_entity = IsGameTypeSingle() ? Level().CurrentEntity() : Level().CurrentControlEntity();

	if (current_entity)
	{
		m_background->SetVisual(current_entity->Visual());
		m_background->SetXYZ(0, M_PI, 0);
	}
	else
	{
		m_background->SetVisual(nullptr);
	}

	if (m_background->GetVisual())
	{
		IKinematicsAnimated* K = m_background->GetVisual()->dcast_PKinematicsAnimated();
		if (K->ID_Cycle_Safe(m_visual_animation))
		{
			K->PlayCycle(m_visual_animation.c_str());
		}
		else
		{
			Msg("! Unable to find animation [%s] for outfit visual [%s]", m_visual_animation.c_str(), m_background->GetVisual()->getDebugName().c_str());
		}
	}
}

void CUIOutfitDragDropList::SetVisualAnimation(const char* animation)
{
	m_visual_animation = animation;
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm)
{
	if(itm)	inherited::SetItem			(itm);
	SetOutfit							(itm);
}

bool CUIOutfitDragDropList::SetItem(CUICellItem* itm, Fvector2 abs_pos)
{
	if(itm)	
		inherited::SetItem			(itm, abs_pos);
	SetOutfit							(itm);
	return true;
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm, Ivector2 cell_pos)
{
	if(itm)	inherited::SetItem			(itm, cell_pos);
	SetOutfit							(itm);
}

CUICellItem* CUIOutfitDragDropList::RemoveItem(CUICellItem* itm, bool force_root)
{
	VERIFY								(!force_root);
	CUICellItem* ci						= inherited::RemoveItem(itm, force_root);
	SetOutfit							(nullptr);
	return								ci;
}


void CUIOutfitDragDropList::Draw()
{
	if (UseFullIcon2D)
	{
		FullIcon->Draw();
		return;
	}

	m_background->Draw					();
//.	inherited::Draw						();
}