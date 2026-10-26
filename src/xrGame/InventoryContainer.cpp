#include "StdAfx.h"
#include "pch_script.h"
#include "InventoryContainer.h"
#include "Level.h"
#include "script_game_object.h"
#include "UIGameCustom.h"

void CInventoryContainer::OnEvent(NET_Packet& P, u16 type)
{
	switch (type)
	{
	case GE_TRADE_BUY:
	case GE_OWNERSHIP_TAKE:
	case GE_TRADE_SELL:
	case GE_OWNERSHIP_REJECT:
		break;
	default:
		inherited::OnEvent(P, type);
		return;
	}

	switch (type)
	{
	case GE_TRADE_BUY:
	case GE_OWNERSHIP_TAKE:
	{
		ALife::_OBJECT_ID id;
		P >> id;
		CObject* itm = Level().Objects.net_Find(id);
		if (itm == nullptr)
		{
			Msg("! CInventoryContainer [%s]: item [%u] not found on take", cName().c_str(), id);
			break;
		}

		Items.push_back(id);
		itm->H_SetParent(this);
		itm->setVisible(false);
		itm->setEnabled(false);
	}
	break;

	case GE_TRADE_SELL:
	case GE_OWNERSHIP_REJECT:
	{
		ALife::_OBJECT_ID id;
		P >> id;
		CObject* itm = Level().Objects.net_Find(id);
		auto it = std::find(Items.begin(), Items.end(), id);
		if (it != Items.end())
		{
			Items.erase(it);
		}

		if (itm == nullptr)
		{
			break;
		}

		const bool justBeforeDestroy = !P.r_eof() && P.r_u8();
		const bool dontCreateShell = (type == GE_TRADE_SELL) || justBeforeDestroy;
		itm->H_SetParent(nullptr, dontCreateShell);
	}
	break;
	}
}

bool CInventoryContainer::net_Spawn(CSE_Abstract* DC)
{
	const bool result = inherited::net_Spawn(DC);
	if (cNameSect() == "shadow_inventory")
	{
		Close();
	}
	else
	{
		Open();
	}
	return result;
}

u32 CInventoryContainer::Cost() const
{
	u32 cost = inherited::Cost();
	for (ALife::_OBJECT_ID id : Items)
	{
		CObject* object = Level().Objects.net_Find(id);
		if (CInventoryItem* item = object != nullptr ? object->cast_inventory_item() : nullptr)
		{
			cost += item->Cost();
		}
	}
	return cost;
}

float CInventoryContainer::Weight() const
{
	float weight = inherited::Weight();
	for (ALife::_OBJECT_ID id : Items)
	{
		CObject* object = Level().Objects.net_Find(id);
		if (CInventoryItem* item = object != nullptr ? object->cast_inventory_item() : nullptr)
		{
			weight += item->Weight();
		}
	}
	return weight;
}

CScriptGameObject* CInventoryContainer::ItemByIndex(u32 index) const
{
	if (index >= Items.size())
	{
		return nullptr;
	}

	CObject* object = Level().Objects.net_Find(Items[index]);
	CGameObject* gameObject = object != nullptr ? object->cast_game_object() : nullptr;
	return gameObject != nullptr ? gameObject->lua_game_object() : nullptr;
}

CScriptGameObject* CInventoryContainer::ItemByName(const char* name) const
{
	const shared_str itemName = name;
	CGameObject* bySection = nullptr;
	for (ALife::_OBJECT_ID id : Items)
	{
		CObject* object = Level().Objects.net_Find(id);
		CGameObject* gameObject = object != nullptr ? object->cast_game_object() : nullptr;
		if (gameObject == nullptr)
		{
			continue;
		}

		if (gameObject->cName() == itemName)
		{
			return gameObject->lua_game_object();
		}

		if (bySection == nullptr && gameObject->cNameSect() == itemName)
		{
			bySection = gameObject;
		}
	}
	return bySection != nullptr ? bySection->lua_game_object() : nullptr;
}

void CInventoryContainer::AddAvailableItems(TIItemContainer& items) const
{
	for (ALife::_OBJECT_ID id : Items)
	{
		CObject* object = Level().Objects.net_Find(id);
		if (CInventoryItem* item = object != nullptr ? object->cast_inventory_item() : nullptr)
		{
			items.push_back(item);
		}
	}
}

void CInventoryContainer::Open()
{
	Opened = true;
	set_tip_text("container_use");
}

void CInventoryContainer::Close()
{
	Opened = false;
	set_tip_text_default();
}
