#include "StdAfx.h"
#include "pch_script.h"
#include "PhraseScript.h"
#include "../xrScripts/script_engine.h"
#include "ai_space.h"
#include "GameObject.h"
#include "script_game_object.h"
#include "InfoPortion.h"
#include "InventoryOwner.h"
#include "ai_debug.h"
#include "../../xrUI/xrUIXmlParser.h"
#include "Actor.h"


namespace DialogScriptCall
{
	bool HasArguments(const char* Name)
	{
		const char* Bracket = strchr(Name, '(');
		return Bracket != nullptr && strchr(Bracket, ')') != nullptr;
	}

	bool Run(const char* Code, const CGameObject* Speaker1, const CGameObject* Speaker2, const char* DialogID, const char* PhraseID, const char* NextPhraseID, bool NeedResult)
	{
		lua_State* L = ai().script_engine().lua();
		const int Top = lua_gettop(L);

		lua_newtable(L);
		lua_pushinteger(L, Speaker1->ID());
		lua_setfield(L, -2, "speaker_id");
		if (Speaker2 != nullptr)
		{
			lua_pushinteger(L, Speaker2->ID());
			lua_setfield(L, -2, "speaker2_id");
		}
		lua_pushstring(L, DialogID != nullptr ? DialogID : "");
		lua_setfield(L, -2, "dialog_id");
		lua_pushstring(L, PhraseID != nullptr ? PhraseID : "");
		lua_setfield(L, -2, "phrase_id");
		if (NextPhraseID != nullptr)
		{
			lua_pushstring(L, NextPhraseID);
			lua_setfield(L, -2, "next_phrase_id");
		}
		lua_setglobal(L, "dialog_params");

		const xr_string Chunk = NeedResult ? xr_string("return ") + Code : xr_string(Code);
		if (luaL_loadbuffer(L, Chunk.c_str(), Chunk.size(), Code) != 0 || lua_pcall(L, 0, 1, 0) != 0)
		{
			Msg("! Dialog script call [%s] failed: %s", Code, lua_tostring(L, -1));
			lua_settop(L, Top);
			return false;
		}

		const bool Result = !NeedResult || (lua_isboolean(L, -1) && lua_toboolean(L, -1));
		lua_settop(L, Top);
		return Result;
	}
}

//загрузка из XML файла
void CDialogScriptHelper::Load		(CUIXml* uiXml, XML_NODE* phrase_node)
{
	LoadSequence(uiXml,phrase_node, "precondition",		m_Preconditions);
	LoadSequence(uiXml,phrase_node, "action",			m_ScriptActions);
	
	LoadSequence(uiXml,phrase_node, "has_info",			m_HasInfo);
	LoadSequence(uiXml,phrase_node, "dont_has_info",	m_DontHasInfo);

	LoadSequence(uiXml,phrase_node, "give_info",		m_GiveInfo);
	LoadSequence(uiXml,phrase_node, "disable_info",		m_DisableInfo);
}

template<class T> 
void  CDialogScriptHelper::LoadSequence (CUIXml* uiXml, XML_NODE* phrase_node, 
								  const char* tag, T&  str_vector)
{
	int tag_num = uiXml->GetNodesNum(phrase_node, tag);
	str_vector.clear();
	for(int i=0; i<tag_num; ++i)
	{
		const char* tag_text = uiXml->Read(phrase_node, tag, i, nullptr);
		xr_string Text = tag_text != nullptr ? tag_text : "";
		_Trim(Text);
		if (!Text.empty())
		{
			str_vector.push_back(Text.c_str());
		}
	}
}

bool  CDialogScriptHelper::CheckInfo(const CInventoryOwner* pOwner) const
{
	THROW(pOwner);

	for(u32 i=0; i<m_HasInfo.size(); ++i) 
	{
		if (!Actor()->HasInfo(m_HasInfo[i])) {
#ifdef DEBUG
			if(psAI_Flags.test(aiDialogs) )
				Msg("----rejected: [%s] has info %s", pOwner->Name(), *m_HasInfo[i]);
#endif
			return false;
		}
	}

	for(u32 i=0; i<m_DontHasInfo.size(); i++) {
		if (Actor()->HasInfo(m_DontHasInfo[i])) {
#ifdef DEBUG
			if(psAI_Flags.test(aiDialogs) )
				Msg("----rejected: [%s] dont has info %s", pOwner->Name(), *m_DontHasInfo[i]);
#endif
			return false;
		}
	}
	return true;
}


void  CDialogScriptHelper::TransferInfo	(const CInventoryOwner* pOwner) const
{
	THROW(pOwner);

	for(u32 i=0; i<m_GiveInfo.size(); ++i)
		Actor()->TransferInfo(m_GiveInfo[i], true);

	for(u32 i=0; i<m_DisableInfo.size(); ++i)
		Actor()->TransferInfo(m_DisableInfo[i], false);
}

const char* CDialogScriptHelper::GetScriptText(const char* str_to_translate, const CGameObject* pSpeakerGO1, const CGameObject* pSpeakerGO2, const char* dialog_id, const char* phrase_id)
{
	if(!m_sScriptTextFunc.size())
		return str_to_translate;

	luabind::functor<const char*>		lua_function;
	bool functor_exists				= ai().script_engine().functor(m_sScriptTextFunc.c_str() ,lua_function);
	THROW3							(functor_exists, "Cannot find phrase script text ", m_sScriptTextFunc.c_str());

	const char* res						= lua_function(	pSpeakerGO1->lua_game_object(), 
													pSpeakerGO2->lua_game_object(), 
													dialog_id,
													phrase_id);

	return res;

}

bool CDialogScriptHelper::Precondition	(const CGameObject* pSpeakerGO, const char* dialog_id, const char* phrase_id) const 
{
	bool predicate_result = true;

	if(!CheckInfo(smart_cast<const CInventoryOwner*>(pSpeakerGO)))
	{
		#ifdef DEBUG
			if (psAI_Flags.test(aiDialogs))
				Msg("dialog [%s] phrase[%s] rejected by CheckInfo",dialog_id,phrase_id);
		#endif
		return false;
	}

	for(u32 i = 0; i<Preconditions().size(); ++i)
	{
		THROW(*Preconditions()[i]);
		if (DialogScriptCall::HasArguments(*Preconditions()[i]))
		{
			predicate_result = DialogScriptCall::Run(*Preconditions()[i], pSpeakerGO, nullptr, dialog_id, phrase_id, nullptr, true);
			if (!predicate_result)
				break;
			continue;
		}

		luabind::functor<bool>	lua_function;
		bool functor_exists = ai().script_engine().functor(*Preconditions()[i] ,lua_function);
		THROW3(functor_exists, "Cannot find precondition", *Preconditions()[i]);
		predicate_result = lua_function	(pSpeakerGO->lua_game_object());
		if(!predicate_result){
		#ifdef DEBUG
			if (psAI_Flags.test(aiDialogs))
				Msg("dialog [%s] phrase[%s] rejected by script predicate", dialog_id, phrase_id);
		#endif
			break;
		} 
	}
	return predicate_result;
}

void CDialogScriptHelper::Action			(const CGameObject* pSpeakerGO, const char* dialog_id, const char* phrase_id) const 
{

	for(u32 i = 0; i<Actions().size(); ++i)
	{
		THROW(*Actions()[i]);
		if (DialogScriptCall::HasArguments(*Actions()[i]))
		{
			DialogScriptCall::Run(*Actions()[i], pSpeakerGO, nullptr, dialog_id, phrase_id, nullptr, false);
			continue;
		}

		luabind::functor<void>	lua_function;
		bool functor_exists = ai().script_engine().functor(*Actions()[i] ,lua_function);
		THROW3(functor_exists, "Cannot find phrase dialog script function", *Actions()[i]);
		lua_function		(pSpeakerGO->lua_game_object(), dialog_id);
	}
	TransferInfo(smart_cast<const CInventoryOwner*>(pSpeakerGO));
}

bool CDialogScriptHelper::Precondition	(	const CGameObject* pSpeakerGO1, 
										const CGameObject* pSpeakerGO2, 
										const char* dialog_id, 
										const char* phrase_id,
										const char* next_phrase_id) const 
{
	bool predicate_result = true;

	if(!CheckInfo(smart_cast<const CInventoryOwner*>(pSpeakerGO1))){
		#ifdef DEBUG
		if (psAI_Flags.test(aiDialogs))
			Msg("dialog [%s] phrase[%s] rejected by CheckInfo",dialog_id,phrase_id);
		#endif
		return false;
	}
	for(u32 i = 0; i<Preconditions().size(); ++i)
	{
		THROW(*Preconditions()[i]);
		if (DialogScriptCall::HasArguments(*Preconditions()[i]))
		{
			predicate_result = DialogScriptCall::Run(*Preconditions()[i], pSpeakerGO1, pSpeakerGO2, dialog_id, phrase_id, next_phrase_id, true);
			if (!predicate_result)
				break;
			continue;
		}

		luabind::functor<bool>	lua_function;
		bool functor_exists = ai().script_engine().functor(*Preconditions()[i] ,lua_function);
		THROW3(functor_exists, "Cannot find phrase precondition", *Preconditions()[i]);
		predicate_result = lua_function	(pSpeakerGO1->lua_game_object(), pSpeakerGO2->lua_game_object(), dialog_id, phrase_id, next_phrase_id);
		if(!predicate_result)
		{
		#ifdef DEBUG
			if (psAI_Flags.test(aiDialogs))
				Msg("dialog [%s] phrase[%s] rejected by script predicate",dialog_id,phrase_id);
		#endif
			break;
		}
	}
	return predicate_result;
}

void CDialogScriptHelper::Action			(const CGameObject* pSpeakerGO1, const CGameObject* pSpeakerGO2, const char* dialog_id, const char* phrase_id) const 
{
	TransferInfo(smart_cast<const CInventoryOwner*>(pSpeakerGO1));

	for(u32 i = 0; i<Actions().size(); ++i)
	{
		THROW(*Actions()[i]);
		if (DialogScriptCall::HasArguments(*Actions()[i]))
		{
			DialogScriptCall::Run(*Actions()[i], pSpeakerGO1, pSpeakerGO2, dialog_id, phrase_id, nullptr, false);
			continue;
		}

		luabind::functor<void>	lua_function;
		bool functor_exists = ai().script_engine().functor(*Actions()[i] ,lua_function);
		if (!functor_exists) {
			Msg("[ERROR] Cannot find phrase dialog script function %s", *Actions()[i]);
			return;
		}

		lua_function(pSpeakerGO1->lua_game_object(), pSpeakerGO2->lua_game_object(), dialog_id, phrase_id);
	}
}
