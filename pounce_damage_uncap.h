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

#ifndef _INCLUDE_POUNCE_DAMAGE_UNCAP_H_
#define _INCLUDE_POUNCE_DAMAGE_UNCAP_H_

#include <ISmmPlugin.h>

#if defined WIN32 && !defined snprintf
#define snprintf _snprintf
#endif

class PounceDamageUncap : public ISmmPlugin, public IConCommandBaseAccessor
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
public:
	const char *GetAuthor();
	const char *GetName();
	const char *GetDescription();
	const char *GetURL();
	const char *GetLicense();
	const char *GetVersion();
	const char *GetDate();
	const char *GetLogTag();
public:    //IConCommandBaseAccessor
	bool RegisterConCommandBase(ConCommandBase *pVar);
};

void Hook_ServerActivate(edict_t *pEdictList, int edictCount, int clientMax);

extern PounceDamageUncap g_pdUncapPlugin;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_POUNCE_DAMAGE_UNCAP_H_
