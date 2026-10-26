////////////////////////////////////////////////////////////////////////////
//	Module 		: base_client_classes_script.cpp
//	Created 	: 20.12.2004
//  Modified 	: 20.12.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay base client classes script export
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "pch_script.h"
#include "base_client_classes.h"
#include "base_client_classes_wrappers.h"
#include "../xrEngine/Feel_Sound.h"
//#include "../Include/xrRender/RenderVisual.h"
#include "../Include/xrRender/RenderVisual.h"
#include "../Include/xrRender/Kinematics.h"
#include "ai/stalker/ai_stalker.h"

using namespace luabind;

#pragma optimize("s",on)
template<>
void DLL_PureScript::script_register	(lua_State *L)
{
	module(L)
	[
		class_<DLL_Pure, CDLL_PureWrapper>("DLL_Pure")
			.def(constructor<>())
			.def("_construct",&DLL_Pure::_construct,&CDLL_PureWrapper::_construct_static)
	];
}

template<>
void ISheduledScript::script_register	(lua_State *L)
{
	module(L)
	[
		class_<ISheduled, CISheduledWrapper>("ISheduled")
	];
}

template<>
void IRenderableScript::script_register	(lua_State *L)
{
	module(L)
	[
		class_<IRenderable, CIRenderableWrapper>("IRenderable")
	];
}

template<>
void ICollidableScript::script_register	(lua_State *L)
{
	module(L)
	[
		class_<ICollidable>("ICollidable")
			.def(constructor<>())
	];
}

namespace GameObjectSchedule
{
	void Set(CGameObject* Object, u32 MinTime, u32 MaxTime)
	{
		Object->shedule.t_min = MinTime;
		Object->shedule.t_max = MaxTime;
	}
}

template<>
void CObjectScript::script_register		(lua_State *L)
{
	module(L)
	[
		class_<CGameObject, bases<DLL_Pure, ISheduled, ICollidable, IRenderable>, CGameObjectWrapper>("CGameObject")
			.def(constructor<>())
			.def("_construct",			&CGameObject::_construct,	&CGameObjectWrapper::_construct_static)
			.def("Visual",				&CGameObject::Visual)

			.def("net_Export",			&CGameObject::net_Export,	&CGameObjectWrapper::net_Export_static)
			.def("net_Import",			&CGameObject::net_Import,	&CGameObjectWrapper::net_Import_static)
			.def("net_Spawn",			&CGameObject::net_Spawn,	&CGameObjectWrapper::net_Spawn_static)

			.def("use",					&CGameObject::use,			&CGameObjectWrapper::use_static)

			.def("getVisible",			&CGameObject::getVisible)
			.def("getEnabled",			&CGameObject::getEnabled)
			.def("set_schedule",		&GameObjectSchedule::Set)
	];
}

namespace KinematicsScript
{
	u16 BoneCount(IKinematics* Kinematics) { return Kinematics->LL_BoneCount(); }
	const char* BoneName(IKinematics* Kinematics, u16 BoneID) { return Kinematics->LL_BoneName_dbg(BoneID); }
	u16 BoneID(IKinematics* Kinematics, const char* BoneName) { return Kinematics->LL_BoneID(BoneName); }

	u16 AnimatedBoneCount(IKinematicsAnimated* Kinematics) { return BoneCount(Kinematics->dcast_PKinematics()); }
	const char* AnimatedBoneName(IKinematicsAnimated* Kinematics, u16 BoneID) { return BoneName(Kinematics->dcast_PKinematics(), BoneID); }
	u16 AnimatedBoneID(IKinematicsAnimated* Kinematics, const char* Name) { return BoneID(Kinematics->dcast_PKinematics(), Name); }
}

template<>
void IRender_VisualScript::script_register		(lua_State *L)
{
	module(L)
	[
		class_<IRenderVisual>("IRender_Visual")
			.def("dcast_PKinematicsAnimated",&IRenderVisual::dcast_PKinematicsAnimated)
			.def("dcast_PKinematics",		&IRenderVisual::dcast_PKinematics),
		class_<IKinematics>("IKinematics")
			.def("LL_BoneCount",			&KinematicsScript::BoneCount)
			.def("LL_BoneName",				&KinematicsScript::BoneName)
			.def("LL_BoneID",				&KinematicsScript::BoneID)
	];
}

void IKinematicsAnimated_PlayCycle(IKinematicsAnimated* sa, const char* anim)
{
	sa->PlayCycle(anim);
}

template<>
void IKinematicsAnimatedScript::script_register		(lua_State *L)
{
	module(L)
	[
		class_<IKinematicsAnimated>("IKinematicsAnimated")
			.def("PlayCycle", &IKinematicsAnimated_PlayCycle)
			.def("dcast_PKinematics", &IKinematicsAnimated::dcast_PKinematics)
			.def("LL_BoneCount", &KinematicsScript::AnimatedBoneCount)
			.def("LL_BoneName", &KinematicsScript::AnimatedBoneName)
			.def("LL_BoneID", &KinematicsScript::AnimatedBoneID)
	];
}

template<>
void CBlendScript::script_register		(lua_State *L)
{
	module(L)
	[
		class_<CBlend>("CBlend")
	];
}
