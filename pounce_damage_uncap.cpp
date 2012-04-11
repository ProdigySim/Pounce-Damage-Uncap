/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * =============================================================================
 * Pounce Damage Uncap
 * Copyright (C) 2011 Michael "ProdigySim" Busby
 * All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the authors give you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 */

#include <stdio.h>
#include "pounce_damage_uncap.h"
#include "memutils.h"
#include "sourcehook.h"
#include "icvar.h"
#include "tier1/iconvar.h"
#include "tier1/convar.h"

PounceDamageUncap g_PounceDamageUncap;
IServerGameDLL *server = NULL;
ICvar* g_pCVar = NULL;

//static const char * pdsig_linux = "66 0F 6E ? F3 0F 51 ? F3 0F 11 ? ? ? ? ? D9 83 ? ? ? ? D9 85 ? ? ? ? DF E9 0F 86";

bool PatchPounceVars(void * pServerDll);
void recalculate_difference();
void min_range_changed(IConVar *var, const char *pOldValue, float flOldValue);
void max_range_changed(IConVar *var, const char *pOldValue, float flOldValue);
ConVar z_pounce_damage_range_max("z_pounce_damage_range_max", "1000.0", FCVAR_GAMEDLL|FCVAR_CHEAT, "Range at which a pounce is worth the maximum bonus damage.", true, 0.0, false, 0.0, max_range_changed);
ConVar z_pounce_damage_range_min("z_pounce_damage_range_min", "300.0", FCVAR_GAMEDLL|FCVAR_CHEAT, "Minimum range for a pounce to be worth bonus damage.", true, 0.0, false, 0.0, min_range_changed);


#if SH_SYS == SH_SYS_WIN32
// D9 E8 D9 C0 D9 05 ? ? ? ? D8 D3 DF E0 F6 C4 05
const char g_sPattern[] = "\xD9\xE8\xD9\xC0\xD9\x05\x2a\x2a\x2a\x2a\xD8\xD3\xDF\xE0\xf6\xc4\x05";
int g_iOffsets[3]={6, 36, 61};
#elif SH_SYS == SH_SYS_LINUX
// 66 0F 6E ? F3 0F 51 ? F3 0F 11 ? ? ? ? ? D9 83 ? ? ? ? D9 85 ? ? ? ? DF E9 0F 86
const char * g_sPattern[] = "\x66\x0F\x6E\x2A\xF3\x0F\x51\x2A\xF3\x0F\x11\x2A\x2A\x2A\x2A\x2A\xD9\x83\x2A\x2A\x2A\x2A\xD9\x85\x2A\x2A\x2A\x2A\xDF\xE9\x0F\x86";
int g_iOffsets[3]={18, 47, 71};
#endif

char *pPatchBaseAddr=NULL;

float * g_pMinRangeData=NULL;
float * g_pMaxRangeData=NULL;
float * g_pDifferenceData=NULL;

float g_flMinRange=300.0;
float g_flMaxRange=1000.0;
#if SH_SYS == SH_SYS_WIN32
float g_flDiffRatio=1.0/700.0;
#else
float g_flDifference=700.0;
#endif


PLUGIN_EXPOSE(PDUncap, g_PounceDamageUncap);

bool PounceDamageUncap::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);

	if(!PatchPounceVars(server))
	{
		Warning("Couldn't patch pounce variables. Giving up.\n");
		return false;
	}
	ConVar_Register(0, this);

	g_flMinRange = z_pounce_damage_range_min.GetFloat();
	g_flMaxRange = z_pounce_damage_range_max.GetFloat();
	recalculate_difference();

	return true;
}

bool PatchPounceVars(void * pServerDll)
{
	char *pAddr = pPatchBaseAddr = (char*)g_MemUtils.FindLibPattern(pServerDll, g_sPattern, sizeof(g_sPattern)-1);
	DevMsg("Found Pattern at %08x", pAddr);
	
	g_MemUtils.SetMemPatchable(pAddr, g_iOffsets[2]+sizeof(float*));
	float ** pPatchAddr = (float**)(pAddr + g_iOffsets[0]);
	g_pMinRangeData = *pPatchAddr;
	g_flMinRange=*g_pMinRangeData;
	*pPatchAddr=&g_flMinRange;

	pPatchAddr = (float**)(pAddr + g_iOffsets[1]);
	g_pMaxRangeData = *pPatchAddr;
	g_flMaxRange=*g_pMaxRangeData;
	*pPatchAddr=&g_flMaxRange;

	pPatchAddr = (float**)(pAddr + g_iOffsets[2]);
	g_pDifferenceData = *pPatchAddr;

#if SH_SYS == SH_SYS_WIN32
	g_flDiffRatio=*g_pDifferenceData;
	*pPatchAddr=&g_flDiffRatio;
#elif SH_SYS == SH_SYS_LINUX
	g_flDifference=*g_pDifferenceData;
	*pPatchAddr=&g_flDifference;
#endif

	return true;
}

void UnPatchPounceVars()
{
	char *pAddr = pPatchBaseAddr;
	
	float ** pPatchAddr = (float**)(pAddr + g_iOffsets[0]);
	*pPatchAddr=g_pMinRangeData;
	
	pPatchAddr = (float**)(pAddr + g_iOffsets[1]);
	*pPatchAddr=g_pMaxRangeData;

	pPatchAddr = (float**)(pAddr + g_iOffsets[2]);
	*pPatchAddr=g_pDifferenceData;
}

bool PounceDamageUncap::Unload(char *error, size_t maxlen)
{
	UnPatchPounceVars();
	return true;
}

void recalculate_difference()
{
	float diff = g_flMaxRange - g_flMinRange;
#if SH_SYS == SH_SYS_WIN32
	g_flDiffRatio = (diff == 0.0) ? FLT_MAX : 1.0/diff;
#else
	g_flDifference = (diff == 0.0) ? FLT_MIN : diff;
#endif
}

void min_range_changed(IConVar *var, const char *pOldValue, float flOldValue)
{
	g_flMinRange = z_pounce_damage_range_min.GetFloat();
	recalculate_difference();
}

void max_range_changed(IConVar *var, const char *pOldValue, float flOldValue)
{
	g_flMaxRange = z_pounce_damage_range_max.GetFloat();
	recalculate_difference();
}

const char *PounceDamageUncap::GetLicense()
{
	return "GPLv3";
}

const char *PounceDamageUncap::GetVersion()
{
	return "1.0.0.0";
}

const char *PounceDamageUncap::GetDate()
{
	return __DATE__;
}

const char *PounceDamageUncap::GetLogTag()
{
	return "PDUNCAP";
}

const char *PounceDamageUncap::GetAuthor()
{
	return "Michael \"ProdigySim\" Busby";
}

const char *PounceDamageUncap::GetDescription()
{
	return "Patch L4D2 to allow uncapping the pounce range limits.";
}

const char *PounceDamageUncap::GetName()
{
	return "Pounce Damage Uncap";
}

const char *PounceDamageUncap::GetURL()
{
	return "http://www.prodigysim.com/";
}

bool PounceDamageUncap::RegisterConCommandBase(ConCommandBase *pVar)
{
	return META_REGCVAR(pVar);
}