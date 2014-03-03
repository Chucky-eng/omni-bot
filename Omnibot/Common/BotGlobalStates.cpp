////////////////////////////////////////////////////////////////////////////////
//
// $LastChangedBy$
// $LastChangedDate$
// $LastChangedRevision$
//
////////////////////////////////////////////////////////////////////////////////

#include "BotGlobalStates.h"

namespace AiState
{
	RegionTriggers::RegionTriggers()
		: StateChild("RegionTriggers")
	{
	}

	//////////////////////////////////////////////////////////////////////////

	GlobalRoot::GlobalRoot() : StateFirstAvailable("GlobalRoot")
	{
		AppendState(new RegionTriggers);
	}
};