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

#ifndef JBMOD_ACHIEVEMENTS_H
#define JBMOD_ACHIEVEMENTS_H
#ifdef _WIN32
#pragma once
#endif

#include "achievementmgr.h"

enum
{
	ACHIEVEMENT_JBMOD_PLAYED = 1001,
	ACHIEVEMENT_JBMOD_PLAYED_ATLAUNCH,
	ACHIEVEMENT_JBMOD_PLAYED_90DAYS,
};

extern CAchievementMgr g_AchievementMgrJB;

void CheckStartupAchievements();

#endif // JBMOD_ACHIEVEMENTS_H
