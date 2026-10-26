#pragma once
#include "inventory_item_object.h"
#include "../xrScripts/script_export_space.h"

class CScriptGameObject;

class CInventoryContainer final : public CInventoryItemObject
{
	using inherited = CInventoryItemObject;

	xr_vector<ALife::_OBJECT_ID> Items;
	bool Opened = true;

public:
	virtual void OnEvent(NET_Packet& P, u16 type) override;
	virtual bool net_Spawn(CSE_Abstract* DC) override;
	virtual u32 Cost() const override;
	virtual float Weight() const override;

	bool IsEmpty() const { return Items.empty(); }
	u32 ItemsCount() const { return static_cast<u32>(Items.size()); }
	CScriptGameObject* ItemByIndex(u32 index) const;
	CScriptGameObject* ItemByName(const char* name) const;
	void AddAvailableItems(TIItemContainer& items) const;

	bool IsOpened() const { return Opened; }
	void Open();
	void Close();

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
