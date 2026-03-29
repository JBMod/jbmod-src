// Copyright 2025 The JBMod Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cbase.h"
#include "jbmod_achievements.h"

#ifdef CLIENT_DLL
#include "achievementmgr.h"
#include "baseachievement.h"
#include "steam/steam_api.h"
#include "vcr_shared.h"

CAchievementMgr g_AchievementMgrJB;

class CBaseJBModAchievement : public CBaseAchievement
{
	DECLARE_CLASS( CBaseJBModAchievement, CBaseAchievement );
public:
};

//-----------------------------------------------------------------------------
// JBMOD_PLAYED: Awarded for simply playing the game.
//-----------------------------------------------------------------------------
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_JBMOD_PLAYED, "JBMOD_PLAYED", 10 );

//-----------------------------------------------------------------------------
// JBMOD_PLAYED_ATLAUNCH: Awarded for playing on launch day (on any year).
//-----------------------------------------------------------------------------
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN( ACHIEVEMENT_JBMOD_PLAYED_ATLAUNCH, "JBMOD_PLAYED_ATLAUNCH", 10 );

//-----------------------------------------------------------------------------
// JBMOD_PLAYED_90DAYS: Awarded for playing 90 days after install.
//-----------------------------------------------------------------------------
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN( ACHIEVEMENT_JBMOD_PLAYED_90DAYS, "JBMOD_PLAYED_90DAYS", 10 );

//-----------------------------------------------------------------------------
// Startup check logic
//-----------------------------------------------------------------------------
void CheckStartupAchievements()
{
#ifndef NO_STEAM
	if ( !g_AchievementMgrJB.HasAchieved( "JBMOD_PLAYED" ) )
	{
		g_AchievementMgrJB.AwardAchievement( ACHIEVEMENT_JBMOD_PLAYED );
	}

	if ( steamapicontext && steamapicontext->SteamApps() && steamapicontext->SteamUtils() )
	{
		uint32 installed = steamapicontext->SteamApps()->GetEarliestPurchaseUnixTime( steamapicontext->SteamUtils()->GetAppID() );
		if ( installed > 0 )
		{
			if ( !g_AchievementMgrJB.HasAchieved( "JBMOD_PLAYED_ATLAUNCH" ) )
			{
				if ( installed <= 1666568334 ) // installed before October 23, 2022
				{
					g_AchievementMgrJB.AwardAchievement( ACHIEVEMENT_JBMOD_PLAYED_ATLAUNCH );
				}
				else
				{
					tm today;
					VCRHook_LocalTime( &today );
					if ( today.tm_mon == 9 && today.tm_mday == 22 ) // October 22 (month is 0-indexed)
					{
						g_AchievementMgrJB.AwardAchievement( ACHIEVEMENT_JBMOD_PLAYED_ATLAUNCH );
					}
				}
			}

			if ( !g_AchievementMgrJB.HasAchieved( "JBMOD_PLAYED_90DAYS" ) )
			{
				long today;
				VCRHook_Time( &today);
				if ( (int)today - (int)installed >= 7776000 ) // 90 * 24 * 60 * 60 = 7776000
				{
					g_AchievementMgrJB.AwardAchievement( ACHIEVEMENT_JBMOD_PLAYED_90DAYS );
				}
			}
		}
	}
#endif // NO_STEAM
}

#endif // CLIENT_DLL
