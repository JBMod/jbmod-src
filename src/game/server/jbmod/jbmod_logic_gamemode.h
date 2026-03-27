// Copyright 2026 The JBMod Authors
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

#ifndef JBMOD_LOGIC_GAMEMODE_H
#define JBMOD_LOGIC_GAMEMODE_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#define SF_GAMEMODE_START_DISABLED ( 1 << 0 )

class CJBModLogicGamemode : public CPointEntity
{
public:
	DECLARE_CLASS( CJBModLogicGamemode, CPointEntity );
	DECLARE_DATADESC();

	CJBModLogicGamemode();

	virtual void Activate();

	void InputEnable( inputdata_t &inputdata );

private:
	void ApplySettings();

	string_t m_iszGameMode;
};

#endif // JBMOD_LOGIC_GAMEMODE_H
