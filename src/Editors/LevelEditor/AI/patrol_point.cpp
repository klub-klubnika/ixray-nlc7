////////////////////////////////////////////////////////////////////////////
//	Module 		: patrol_point.cpp
//	Created 	: 15.06.2004
//  Modified 	: 15.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Patrol point
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "patrol_point.h"
#include "level_graph.h"
#include "level_graph.h"
#include "game_level_cross_table.h"
#include "game_graph.h"

#ifdef DEBUG
#	include "patrol_path.h"
#endif

namespace
{
	u32 FindPatrolVertex(const ILevelGraph* LevelGraph, const Fvector& Position, const shared_str& PointName, const char* PathName)
	{
		if (!LevelGraph)
		{
			return u32(-1);
		}

		u32 VertexId = u32(-1);
		if (LevelGraph->valid_vertex_position(Position))
		{
			Fvector LookupPosition = Position;
			LookupPosition.y += .15f;
			VertexId = LevelGraph->vertex_id(LookupPosition);
		}

		if (LevelGraph->valid_vertex_id(VertexId))
		{
			return VertexId;
		}

		VertexId = LevelGraph->vertex(u32(-1), Position);
		Msg("! Patrol point [%s] in path [%s] at [%.3f %.3f %.3f] is outside the AI map, snapped to vertex [%u]", PointName.c_str(), PathName, VPUSH(Position), VertexId);
		return VertexId;
	}
}

CPatrolPoint::CPatrolPoint									(const CPatrolPath *path)
{
#ifdef DEBUG
	m_path				= path;
	m_initialized		= false;
#endif
}

#ifdef DEBUG
void CPatrolPoint::verify_vertex_id							(const ILevelGraph *level_graph, const IGameLevelCrossTable *cross, const IGameGraph *game_graph) const
{
	if (!level_graph)
		return;

	if (level_graph->valid_vertex_id(m_level_vertex_id)) {
		return;
	}

	VERIFY			(m_path);
	string1024		temp;
	xr_sprintf			(temp,"\n! Patrol point %s in path %s is not on the level graph vertex!",*m_name,*m_path->m_name);
	R_ASSERT2			(level_graph->valid_vertex_id(m_level_vertex_id),temp);
}
#endif

IC	void CPatrolPoint::correct_position						(const ILevelGraph *level_graph, const IGameLevelCrossTable *cross, const IGameGraph *game_graph)
{
	if (!level_graph || !level_graph->valid_vertex_position(position()) || !level_graph->valid_vertex_id(m_level_vertex_id))
		return;

	if (!level_graph->inside(level_vertex_id(level_graph,cross,game_graph),position()))
		m_position		= level_graph->vertex_position(level_vertex_id(level_graph,cross,game_graph));

	m_game_vertex_id	= cross->vertex(level_vertex_id(level_graph,cross,game_graph)).game_vertex_id();
}

CPatrolPoint::CPatrolPoint									(const ILevelGraph *level_graph, const IGameLevelCrossTable *cross, const IGameGraph *game_graph, const CPatrolPath *path, const Fvector &position, u32 level_vertex_id, u32 flags, const shared_str& name)
{
#ifdef DEBUG
	VERIFY				(path);
	m_path				= path;
#endif
	m_position			= position;
	m_level_vertex_id	= level_vertex_id;
	m_flags				= flags;
	m_name				= name;
#ifdef DEBUG
	m_initialized		= true;
#endif
	correct_position	(level_graph,cross,game_graph);
}

CPatrolPoint &CPatrolPoint::load_raw						(const ILevelGraph *level_graph, const IGameLevelCrossTable *cross, const IGameGraph *game_graph, IReader &stream)
{
	stream.r_fvector3	(m_position);
	m_flags				= stream.r_u32();
	stream.r_stringZ	(m_name);
	m_level_vertex_id	= FindPatrolVertex(level_graph, m_position, m_name, "?");
#ifdef DEBUG
	m_initialized		= true;
#endif
	correct_position	(level_graph,cross,game_graph);
	return				(*this);
}

CPatrolPoint& CPatrolPoint::load_editor(const ILevelGraph* level_graph, const IGameLevelCrossTable* cross, const IGameGraph* game_graph, CWayObject* object, size_t id)
{
	m_position = object->m_WayPoints[id]->m_vPosition;
	m_flags = object->m_WayPoints[id]->m_Flags.get();
	m_name = object->m_WayPoints[id]->m_Name;
	m_level_vertex_id = FindPatrolVertex(level_graph, m_position, m_name, object->GetName());
#ifdef DEBUG
	m_initialized = true;
#endif
	correct_position(level_graph, cross, game_graph);
	return *this;
}

void CPatrolPoint::load										(IReader &stream)
{
	load_data			(m_name,stream);
	load_data			(m_position,stream);
	load_data			(m_flags,stream);
	load_data			(m_level_vertex_id,stream);
	load_data			(m_game_vertex_id,stream);

#ifdef DEBUG
	m_initialized		= true;
#endif
}

void CPatrolPoint::save										(IWriter &stream)
{
	save_data			(m_name,stream);
	save_data			(m_position,stream);
	save_data			(m_flags,stream);
	save_data			(m_level_vertex_id,stream);
	save_data			(m_game_vertex_id,stream);
}
