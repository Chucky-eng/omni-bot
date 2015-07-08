////////////////////////////////////////////////////////////////////////////////
//
// $LastChangedBy$
// $LastChangedDate$
// $LastChangedRevision$
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __PATHPLANNERRECAST_H__
#define __PATHPLANNERRECAST_H__

#include "PathPlannerBase.h"
#include "RecastInterfaces.h"

#include "DetourTileCache.h"
#include "DetourTileCacheBuilder.h"
#include "DetourNavMesh.h"

#include "CollisionModel.h"

#include "btBulletCollisionCommon.h"

struct aiScene;
struct aiNode;
struct sqlite3;

namespace modeldata
{
	class Scene;
	class Node;
};

//////////////////////////////////////////////////////////////////////////

struct TileRebuild
{
	int mX, mY;

	TileRebuild( int x = 0, int y = 0 );
};
struct TileAddData
{
	int					mX, mY;

	unsigned char *		mNavData = NULL;
	int					mNavDataSize = 0;

	TileAddData( int x = 0, int y = 0 );
};

//////////////////////////////////////////////////////////////////////////

// class: PathPlannerRecast
//		Path planner interface for the navmesh system for hl2
class PathPlannerRecast : public PathPlannerBase
{
public:
	friend class RecastPathInterface;
	
	static const int VERSION_MODELCACHE = 1;
	
	bool Init( System & system );
	void Update( System & system );
	void Shutdown();
	bool IsReady() const;
		
	bool Load( const std::string &_mapname, bool _dl = true );
	bool Save( const std::string &_mapname );
	void Unload();
	
	virtual void RegisterScriptFunctions( gmMachine *a_machine );

	bool GetNavInfo( const Vector3f &pos, int32_t &_id, std::string &_name );

	void AddEntityConnection( const Event_EntityConnection &_conn );
	void RemoveEntityConnection( GameEntity _ent );
	void EntityCreated( const EntityInstance &ei );
	void EntityDeleted( const EntityInstance &ei );

	void BuildConfig( rcConfig & config );
	void BuildNav( bool saveToFile );
	void BuildNavTile();

	PathInterface * AllocPathInterface( Client * client );

	const char *GetPlannerName() const;
	int GetPlannerType() const;
		
	typedef std::vector<TileRebuild> TileRebuildList;
	typedef std::vector<TileAddData> AddTileList;

	bool AsyncGetTileRebuild( TileRebuild & buildTile );

	void MarkTileForBuilding( const dtMeshTile * tile );
	void MarkTileForBuilding( const Vector3f & pos );
	void MarkTileForBuilding( const Vector3f & mins, const Vector3f & maxs );
	void MarkTileForBuilding( const AxisAlignedBox3f & oldBounds, const AxisAlignedBox3f & newBounds );
	void MarkTileForBuilding( const int tx, const int ty );

	void RasterizeTileLayers( int tax, int ty );

	PathPlannerRecast();
	virtual ~PathPlannerRecast();
protected:
	friend class ToolCreateBounds;
	friend class ToolCreateOffMeshConnection;

	void OpenCachedDatabase();

	void InitCommands();
	void cmdNavSave( const StringVector & args );
	void cmdNavLoad( const StringVector & args );
	void cmdNavView( const StringVector & args );
	void cmdNavViewModels( const StringVector & args );
	void cmdNavViewConnections( const StringVector & args );

	void cmdBuildNav( const StringVector & args );
	void cmdBuildNavTile( const StringVector & args );

	void cmdUndo( const StringVector & args );
	void cmdNavAddExclusionZone( const StringVector & args );
	void cmdNavAddLink( const StringVector & args );
	void cmdCommitPoly( const StringVector & args );
	void cmdAutoBuildFeatures( const StringVector & args );
	void cmdSaveToObjFile( const StringVector & args );
	void cmdModelEnable( const StringVector & args );
	void cmdModelShape( const StringVector & args );
	void cmdModelSetSolid( const StringVector & args );
	void cmdModelDynamic( const StringVector & args );
	void cmdModelSetTriangleSurface( const StringVector & args );

	void LoadWorldModel();

	void LoadModel( const GameModelInfo & modelInfo, GameEntity entity, const IceMaths::Matrix4x4 & xform, bool baseStaticMesh );

	EditTool<PathPlannerRecast> * mCurrentTool;

	//////////////////////////////////////////////////////////////////////////
	// Friend functions
	friend int GM_CDECL gmfRecastView( gmThread *a_thread );
	friend int GM_CDECL gmfRecastViewConnections( gmThread *a_thread );
	friend int GM_CDECL gmfRecastEnableStep( gmThread *a_thread );
	friend int GM_CDECL gmfRecastStep( gmThread *a_thread );
	friend int GM_CDECL gmfRecastAddFloodSeed( gmThread *a_thread );
	friend int GM_CDECL gmfRecastBuildNavMesh( gmThread *a_thread );
	friend int GM_CDECL gmfRecastFloodFill( gmThread *a_thread );
	friend int GM_CDECL gmfRecastTrimSectors( gmThread *a_thread );

	//////////////////////////////////////////////////////////////////////////
protected:

	//////////////////////////////////////////////////////////////////////////
	// Internal Implementations of base class functionality
	std::string _GetNavFileExtension()
	{
		return ".nm";
	}

private:
	struct RecastSettings
	{
		float	AgentHeightStand;
		float	AgentHeightCrouch;
		float	AgentRadius;
		float	AgentMaxClimb;

		float	WalkableSlopeAngle;

		float	CellSize;
		float	CellHeight;

		float	EdgeMaxLen;
		float	MaxSimplificationError;
		int		TileSize;
		float	DetailSampleDist;
		float	DetailSampleMaxError;
		
		RecastSettings();
	} mSettings;

	typedef std::vector<OffMeshConnection> OffMeshConnections;
	OffMeshConnections				mOffMeshConnections;

	RecastBuildContext				mContext;
	dtNavMesh	*					mNavMesh;
	
	CollisionWorld					mCollision;
	
	std::vector<GameEntity>			mDeferredModel;
	
	TileRebuildList					mTileBuildQueue;
	AddTileList						mAddTileQueue;

	std::vector<AxisAlignedBox3f>	mExclusionZones;

	struct Flags
	{
		uint32_t					mViewMode : 2;
		uint32_t					mViewConnections : 1;
		uint32_t					mViewModels : 2;

		Flags()
			: mViewMode( 0 )
			, mViewConnections( 0 )
			, mViewModels( 0 )
		{
		}
	}								mFlags;

	void InitNavmesh();

	template<typename T, typename... Args>
	void SetCurrentTool( Args... args )
	{
		if ( mCurrentTool != NULL && typeid( *mCurrentTool ) == typeid( T ) )
		{
			if ( !mCurrentTool->ReEnter( this ) )
				OB_DELETE( mCurrentTool );
		}
		else
		{
			OB_DELETE( mCurrentTool );
			mCurrentTool = new T( args... );
			if ( !mCurrentTool->Enter( this ) )
				OB_DELETE( mCurrentTool );
		}
	}

	sqlite3 * mCacheDb;

	boost::recursive_mutex		mGuardBuildQueue;
	boost::recursive_mutex		mGuardAddTile;

	boost::thread_group			mThreadGroup;
	bool						mDeferredSaveNav;

	void UpdateDeferredModels();
	void UpdateModelState( bool forcePositionUpdate );
	
	bool GetAimedAtModel( RayResult& result, SurfaceFlags ignoreSurfaces );

	bool CreateEntityModel( const GameEntity& entity, const EntityInfo & entInfo );

	void SendWorldModel();
	void SendTileModel( int tx, int ty );
};

#endif
