////////////////////////////////////////////////////////////////////////////
//	Module 		: actor_script.cpp
//	Created 	: 17.01.2008
//  Modified 	: 17.01.2008
//	Author		: Dmitriy Iassenev
//	Description : actor script export
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "pch_script.h"
#include "Actor.h"
#include "level_changer.h"
#include "ActorCondition.h"
#include "Inventory.h"
#include "CharacterPhysicsSupport.h"
#include "PHMovementControl.h"

using namespace luabind;

bool isFirstPerson() { return Actor()->active_cam() == eacFirstEye; }
void setFirstPerson() { Actor()->cam_Set(eacFirstEye); }
void setThirdPerson() { Actor()->cam_Set(eacLookAt); }

namespace ActorParamsScript
{
	CActor* GetActorObject() { return g_actor; }

	CCameraBase* GetActorCamera(u16 Index)
	{
		CActor* Actor = smart_cast<CActor*>(Level().CurrentEntity());
		return Actor != nullptr ? Actor->cam_ByIndex(Index) : nullptr;
	}

	CActorCondition* GetCondition(CActor* Actor) { return &Actor->conditions(); }
	CHitImmunity* GetImmunities(CActor* Actor) { return &Actor->conditions(); }
	CInventory* GetInventory(CActor* Actor) { return &Actor->inventory(); }
	CPHMovementControl* GetMovement(CActor* Actor) { return Actor->character_physics_support()->movement(); }
	SRotation GetOrientation(CActor* Actor) { return Actor->Orientation(); }

	CEntity::SEntityState GetState(CActor* Actor)
	{
		CEntity::SEntityState State = {};
		Actor->g_State(State);
		return State;
	}

	float GetJumpSpeed(CActor* Actor) { return Actor->m_fJumpSpeed; }

	void SetJumpSpeed(CActor* Actor, float Speed)
	{
		Actor->m_fJumpSpeed = Speed;
		GetMovement(Actor)->SetJumpUpVelocity(Speed);
	}

	float GetHitProbability(CActor* Actor) { return Actor->HitProbability(); }
	void SetHitProbability(CActor* Actor, float Value) { Actor->SetHitProbability(Value); }

	float GetMass(CPHMovementControl* Movement) { return Movement->GetMass(); }
	void SetMass(CPHMovementControl* Movement, float Value) { Movement->SetMass(Value); }
	float GetAirControl(CPHMovementControl* Movement) { return Movement->GetAirControlParam(); }
	void SetAirControl(CPHMovementControl* Movement, float Value) { Movement->SetAirControlParam(Value); }

	bool IsCrouch(CEntity::SEntityState* State) { return !!State->bCrouch; }
	bool IsFall(CEntity::SEntityState* State) { return !!State->bFall; }
	bool IsJump(CEntity::SEntityState* State) { return !!State->bJump; }
	bool IsSprint(CEntity::SEntityState* State) { return !!State->bSprint; }
	float GetVelocity(CEntity::SEntityState* State) { return State->fVelocity; }
	float GetAngularVelocity(CEntity::SEntityState* State) { return State->fAVelocity; }
}

bool isGodMode() { return psActorFlags.test(AF_GOD_MODE); }
bool isActorShadow() { return psGameFlags.test(rsActorShadow); }
void setActorShadow(bool state) { psGameFlags.set(rsActorShadow, state); }

#pragma optimize("s",on)
void CActor::script_register(lua_State* L)
{
	module(L)
		[
			class_<CameraFocusParams>("camera_focus_params")
				.def(constructor<>())
				.def_readwrite("fov", &CameraFocusParams::Fov)
				.def_readwrite("blend_in", &CameraFocusParams::BlendIn)
				.def_readwrite("hold_time", &CameraFocusParams::HoldTime)
				.def_readwrite("blend_out", &CameraFocusParams::BlendOut)
				.def_readwrite("tracking_speed", &CameraFocusParams::TrackingSpeed)
				.def_readwrite("lock_input", &CameraFocusParams::LockInput)
				.def_readwrite("track", &CameraFocusParams::Track)
				.def_readwrite("restore_direction", &CameraFocusParams::RestoreDirection),
			class_<CActor, CGameObject>("CActor")
				.def(constructor<>())
			 .def("conditions", &CActor::conditions)
			 .def("inventory_disabled", &CActor::inventory_disabled)
			 .def("set_inventory_disabled", &CActor::set_inventory_disabled)
			 .property("condition", &ActorParamsScript::GetCondition)
			 .property("immunities", &ActorParamsScript::GetImmunities)
			 .property("inventory", &ActorParamsScript::GetInventory)
			 .property("movement", &ActorParamsScript::GetMovement)
			 .property("orientation", &ActorParamsScript::GetOrientation)
			 .property("state", &ActorParamsScript::GetState)
			 .property("jump_speed", &ActorParamsScript::GetJumpSpeed, &ActorParamsScript::SetJumpSpeed)
			 .property("hit_probability", &ActorParamsScript::GetHitProbability, &ActorParamsScript::SetHitProbability)
			 .def_readwrite("walk_accel", &CActor::m_fWalkAccel)
			 .def_readwrite("run_coef", &CActor::m_fRunFactor)
			 .def_readwrite("run_back_coef", &CActor::m_fRunBackFactor)
			 .def_readwrite("walk_back_coef", &CActor::m_fWalkBackFactor)
			 .def_readwrite("crouch_coef", &CActor::m_fCrouchFactor)
			 .def_readwrite("climb_coef", &CActor::m_fClimbFactor)
			 .def_readwrite("sprint_koef", &CActor::m_fSprintFactor)
			 .def_readwrite("walk_strafe_coef", &CActor::m_fWalk_StrafeFactor)
			 .def_readwrite("run_strafe_coef", &CActor::m_fRun_StrafeFactor),
			class_<CPHMovementControl>("CPHMovementControl")
				.property("ph_mass", &ActorParamsScript::GetMass, &ActorParamsScript::SetMass)
				.property("air_control_param", &ActorParamsScript::GetAirControl, &ActorParamsScript::SetAirControl)
				.def("set_jump_up_velocity", &CPHMovementControl::SetJumpUpVelocity),
			class_<CEntity::SEntityState>("SEntityState")
				.property("crouch", &ActorParamsScript::IsCrouch)
				.property("fall", &ActorParamsScript::IsFall)
				.property("jump", &ActorParamsScript::IsJump)
				.property("sprint", &ActorParamsScript::IsSprint)
				.property("velocity", &ActorParamsScript::GetVelocity)
				.property("a_velocity", &ActorParamsScript::GetAngularVelocity),
			def("get_actor_obj", &ActorParamsScript::GetActorObject),
			class_<CCameraBase>("CCameraBase")
				.def_readwrite("yaw", &CCameraBase::yaw)
				.def_readwrite("pitch", &CCameraBase::pitch)
				.def_readwrite("roll", &CCameraBase::roll)
				.def_readwrite("fov", &CCameraBase::f_fov)
				.def_readwrite("aspect", &CCameraBase::f_aspect)
				.def_readwrite("position", &CCameraBase::vPosition)
				.def_readonly("direction", &CCameraBase::vDirection)
				.def_readwrite("lim_yaw", &CCameraBase::lim_yaw)
				.def_readwrite("lim_pitch", &CCameraBase::lim_pitch)
				.def_readwrite("lim_roll", &CCameraBase::lim_roll),
			def("actor_camera", &ActorParamsScript::GetActorCamera),
				class_<CLevelChanger, CGameObject>("CLevelChanger")
				.def(constructor<>()),
				def("is_first_person", isFirstPerson),
				def("set_first_person", setFirstPerson),
				def("set_third_person", setThirdPerson),
				def("is_god_mode", isGodMode),
				def("is_actor_shadow", isActorShadow),
				def("set_actor_shadow", setActorShadow),

				class_<enum_exporter<EBoostParams>>("EBoostParams").enum_("eboostparams")[
					value("eBoostHpRestore", eBoostHpRestore),
					value("eBoostPowerRestore", eBoostPowerRestore),
					value("eBoostRadiationRestore", eBoostRadiationRestore),
					value("eBoostBleedingRestore", eBoostBleedingRestore),
					value("eBoostMaxWeight", eBoostMaxWeight),
					value("eBoostRadiationProtection", eBoostRadiationProtection),
					value("eBoostTelepaticProtection", eBoostTelepaticProtection),
					value("eBoostChemicalBurnProtection", eBoostChemicalBurnProtection),
					value("eBoostBurnImmunity", eBoostBurnImmunity),
					value("eBoostShockImmunity", eBoostShockImmunity),
					value("eBoostRadiationImmunity", eBoostRadiationImmunity),
					value("eBoostTelepaticImmunity", eBoostTelepaticImmunity),
					value("eBoostChemicalBurnImmunity", eBoostChemicalBurnImmunity),
					value("eBoostExplImmunity", eBoostExplImmunity),
					value("eBoostStrikeImmunity", eBoostStrikeImmunity),
					value("eBoostFireWoundImmunity", eBoostFireWoundImmunity),
					value("eBoostWoundImmunity", eBoostWoundImmunity),
					value("eBoostMaxCount", eBoostMaxCount)
			],

			class_<enum_exporter<EMovementStates>>("EMovementStates").enum_("emovementstates")
			[
				value("eOld", eOld),
				value("eWishful", eWishful),
				value("eReal", eReal)
			],

			class_<enum_exporter<EMoveCommand>>("EMoveCommand").enum_("emovecommand")
			[
				value("mcFwd", mcFwd),
				value("mcBack", mcBack),
				value("mcLStrafe", mcLStrafe),
				value("mcRStrafe", mcRStrafe),
				value("mcCrouch", mcCrouch),
				value("mcAccel", mcAccel),
				value("mcTurn", mcTurn),
				value("mcJump", mcJump),
				value("mcFall", mcFall),
				value("mcLanding", mcLanding),
				value("mcLanding2", mcLanding2),
				value("mcClimb", mcClimb),
				value("mcSprint", mcSprint),
				value("mcLLookout", mcLLookout),
				value("mcRLookout", mcRLookout),
				value("mcAnyMove", mcAnyMove),
				value("mcAnyAction", mcAnyAction),
				value("mcAnyState", mcAnyState),
				value("mcLookout", mcLookout),
				value("mcJumpSeq", mcJumpSeq)

			]
		];
}
