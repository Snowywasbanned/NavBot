#include <extension.h>
#include <ifaces_extern.h>
#include <server_class.h>
#include "helpers.h"
#include "entprops.h"

edict_t* UtilHelpers::BaseEntityToEdict(CBaseEntity* pEntity)
{
	IServerUnknown* pUnk = (IServerUnknown*)pEntity;
	IServerNetworkable* pNet = pUnk->GetNetworkable();

	if (!pNet)
	{
		return nullptr;
	}

	return pNet->GetEdict();
}

edict_t* UtilHelpers::GetEdict(int entity)
{

	edict_t* pEdict;
	if (!IndexToAThings(entity, nullptr, &pEdict))
	{
		return nullptr;
	}

	return pEdict;
}

CBaseEntity* UtilHelpers::GetEntity(int entity)
{
	CBaseEntity* pEntity;
	if (!IndexToAThings(entity, &pEntity, nullptr))
	{
		return nullptr;
	}

	return pEntity;
}

/* Given an entity reference or index, fill out a CBaseEntity and/or edict.
   If lookup is successful, returns true and writes back the two parameters.
   If lookup fails, returns false and doesn't touch the params.  */
bool UtilHelpers::IndexToAThings(int num, CBaseEntity** pEntData, edict_t** pEdictData)
{
	CBaseEntity* pEntity = gamehelpers->ReferenceToEntity(num);

	if (!pEntity)
	{
		return false;
	}

	int index = gamehelpers->ReferenceToIndex(num);
	if (index > 0 && index <= playerhelpers->GetMaxClients())
	{
		SourceMod::IGamePlayer* pPlayer = playerhelpers->GetGamePlayer(index);
		if (!pPlayer || !pPlayer->IsConnected())
		{
			return false;
		}
	}

	if (pEntData)
	{
		*pEntData = pEntity;
	}

	if (pEdictData)
	{
		edict_t* pEdict = BaseEntityToEdict(pEntity);
		if (!pEdict || pEdict->IsFree())
		{
			pEdict = nullptr;
		}

		*pEdictData = pEdict;
	}

	return true;
}

Vector UtilHelpers::getOBBCenter(edict_t* pEntity)
{
	Vector result = Vector(0, 0, 0);
	VectorLerp(pEntity->GetCollideable()->OBBMins(), pEntity->GetCollideable()->OBBMaxs(), 0.5f, result);
	return result;
}

Vector UtilHelpers::collisionToWorldSpace(const Vector& in, edict_t* pEntity)
{
	Vector result = Vector(0, 0, 0);

	if (!isBoundsDefinedInEntitySpace(pEntity) || pEntity->GetCollideable()->GetCollisionAngles() == vec3_angle)
	{
		VectorAdd(in, pEntity->GetCollideable()->GetCollisionOrigin(), result);
	}
	else
	{
		VectorTransform(in, pEntity->GetCollideable()->CollisionToWorldTransform(), result);
	}

	return result;
}

/**
 * Gets the entity world center. Clone of WorldSpaceCenter()
 * @param pEntity	The entity to get the center from
 * @return			Center vector
 **/
Vector UtilHelpers::getWorldSpaceCenter(edict_t* pEntity)
{
	Vector result = getOBBCenter(pEntity);
	result = collisionToWorldSpace(result, pEntity);
	return result;
}

/**
 * Checks if a point is within a trigger
 *
 * @param pEntity	The trigger entity
 * @param vPoint	The point to be tested
 * @return			True if the given point is within pEntity
 **/
bool UtilHelpers::pointIsWithinTrigger(edict_t* pEntity, const Vector& vPoint)
{
	Ray_t ray;
	trace_t tr;
	ICollideable* pCollide = pEntity->GetCollideable();
	ray.Init(vPoint, vPoint);
	enginetrace->ClipRayToCollideable(ray, MASK_ALL, pCollide, &tr);
	return (tr.startsolid);
}

bool UtilHelpers::isBoundsDefinedInEntitySpace(edict_t* pEntity)
{
	return ((pEntity->GetCollideable()->GetSolidFlags() & FSOLID_FORCE_WORLD_ALIGNED) == 0 &&
		pEntity->GetCollideable()->GetSolid() != SOLID_BBOX && pEntity->GetCollideable()->GetSolid() != SOLID_NONE);
}

/// @brief Searches for entities by classname
/// @return Entity index/reference or INVALID_EHANDLE_INDEX if none is found
int UtilHelpers::FindEntityByClassname(int start, const char* searchname)
{
#if SOURCE_ENGINE > SE_ORANGEBOX
	CBaseEntity* pEntity = servertools->FindEntityByClassname(GetEntity(start), searchname);
	return gamehelpers->EntityToBCompatRef(pEntity);
#else

	// from: https://cs.alliedmods.net/sourcemod/source/extensions/sdktools/vnatives.cpp#873

	CBaseEntity* pEntity = nullptr;

	if (start == -1)
	{
		pEntity = (CBaseEntity*)servertools->FirstEntity();
	}
	else
	{
		pEntity = gamehelpers->ReferenceToEntity(start);
		if (!pEntity)
		{
			return INVALID_EHANDLE_INDEX;
		}
		pEntity = (CBaseEntity*)servertools->NextEntity(pEntity);
	}

	// it's tough to find a good ent these days
	if (!pEntity)
	{
		return INVALID_EHANDLE_INDEX;
	}

	const char* classname = nullptr;
	int lastletterpos;

	static int offset = -1;
	if (offset == -1)
	{
		SourceMod::sm_datatable_info_t info;
		if (!gamehelpers->FindDataMapInfo(gamehelpers->GetDataMap(pEntity), "m_iClassname", &info))
		{
			return INVALID_EHANDLE_INDEX;
		}

		offset = info.actual_offset;
	}

	string_t s;

	while (pEntity)
	{
		if ((s = *(string_t*)((uint8_t*)pEntity + offset)) == NULL_STRING)
		{
			pEntity = (CBaseEntity*)servertools->NextEntity(pEntity);
			continue;
		}

		classname = STRING(s);

		lastletterpos = strlen(searchname) - 1;
		if (searchname[lastletterpos] == '*')
		{
			if (strncasecmp(searchname, classname, lastletterpos) == 0)
			{
				return gamehelpers->EntityToBCompatRef(pEntity);
			}
		}
		else if (strcasecmp(searchname, classname) == 0)
		{
			return gamehelpers->EntityToBCompatRef(pEntity);
		}

		pEntity = (CBaseEntity*)servertools->NextEntity(pEntity);
	}

	return INVALID_EHANDLE_INDEX;

#endif // SOURCE_ENGINE > SE_ORANGEBOX
}

/// @brief Searches for entities in a sphere
/// @return Entity index/reference or INVALID_EHANDLE_INDEX if none is found
int UtilHelpers::FindEntityInSphere(int start, Vector center, float radius)
{
#if SOURCE_ENGINE > SE_ORANGEBOX
	CBaseEntity* pEntity = servertools->FindEntityInSphere(GetEntity(start), center, radius);
	return gamehelpers->EntityToBCompatRef(pEntity);
#else
	CBaseEntity* pEntity = nullptr;

	if (start == -1)
	{
		pEntity = static_cast<CBaseEntity*>(servertools->FirstEntity());
	}
	else
	{
		pEntity = gamehelpers->ReferenceToEntity(start);

		if (!pEntity)
		{
			return INVALID_EHANDLE_INDEX;
		}

		pEntity = static_cast<CBaseEntity*>(servertools->NextEntity(pEntity));
	}

	if (!pEntity)
	{
		return INVALID_EHANDLE_INDEX;
	}

	while (pEntity)
	{
		int index = gamehelpers->EntityToBCompatRef(pEntity);
		auto pos = entprops->GetEntPropVector(index, Prop_Data, "m_vecOrigin");
		float distancesqr = (center - pos).LengthSqr();

		if (distancesqr <= radius * radius)
		{
			return gamehelpers->EntityToBCompatRef(pEntity);
		}

		pEntity = static_cast<CBaseEntity*>(servertools->NextEntity(pEntity));
	}

	return INVALID_EHANDLE_INDEX;
#endif // SOURCE_ENGINE > SE_ORANGEBOX
}

/// @brief Searches for entities by their networkable class
/// @return Entity index or INVALID_EHANDLE_INDEX if none is found
int UtilHelpers::FindEntityByNetClass(int start, const char* classname)
{
	edict_t* current;

	for (int i = ((start != -1) ? start : 0); i < gpGlobals->maxEntities; i++)
	{
		current = engine->PEntityOfEntIndex(i);
		if (current == nullptr || current->IsFree())
		{
			continue;
		}

		IServerNetworkable* network = current->GetNetworkable();

		if (network == nullptr)
		{
			continue;
		}

		ServerClass* sClass = network->GetServerClass();
		const char* name = sClass->GetName();


		if (strcmp(name, classname) == 0)
		{
			return i;
		}
	}

	return INVALID_EHANDLE_INDEX;
}

/// @brief check if a point is in the field of a view of an object. supports up to 180 degree fov.
/// @param vecSrcPosition Source position of the view.
/// @param vecTargetPosition Point to check if within view angle.
/// @param vecLookDirection The direction to look towards.  Note that this must be a forward angle vector.
/// @param flCosHalfFOV The width of the forward view cone as a dot product result.
/// @return True if the point is within view from the source position at the specified FOV.
/// @note https://github.com/ValveSoftware/source-sdk-2013/blob/beaae8ac45a2f322a792404092d4482065bef7ef/sp/src/public/mathlib/vector.h#L462-L477
bool UtilHelpers::PointWithinViewAngle(Vector const& vecSrcPosition, Vector const& vecTargetPosition, Vector const& vecLookDirection, float flCosHalfFOV)
{
	Vector vecDelta = vecTargetPosition - vecSrcPosition;
	float cosDiff = DotProduct(vecLookDirection, vecDelta);

	if (cosDiff < 0)
		return false;

	float flLen2 = vecDelta.LengthSqr();

	// a/sqrt(b) > c  == a^2 > b * c ^2
	return (cosDiff * cosDiff > flLen2 * flCosHalfFOV * flCosHalfFOV);
}

/// @brief Calculates the width of the forward view cone as a dot product result from the given angle.
/// This manually calculates the value of CBaseCombatCharacter's `m_flFieldOfView` data property.
/// @param angle The FOV value in degree
/// @return Width of the forward view cone as a dot product result
float UtilHelpers::GetForwardViewCone(float angle)
{
	return cosf(DEG2RAD(angle) / 2.0f);
}
