/*---------------------------------------------------------------------------*\
 *           interactive networked Virtual Reality system (inVRs)            *
 *                                                                           *
 *    Copyright (C) 2005-2009 by the Johannes Kepler University, Linz        *
 *                                                                           *
 *                            www.inVRs.org                                  *
 *                                                                           *
 *              contact: canthes@inVRs.org, rlander@inVRs.org                *
\*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*\
 *                                License                                    *
 *                                                                           *
 * This library is free software; you can redistribute it and/or modify it   *
 * under the terms of the GNU Library General Public License as published    *
 * by the Free Software Foundation, version 2.                               *
 *                                                                           *
 * This library is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public         *
 * License along with this library; if not, write to the Free Software       *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                 *
\*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*\
 *                                Changes                                    *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *                                                                           *
\*---------------------------------------------------------------------------*/

#ifndef _EFFECTTEXTUREMANAGER_H
#define _EFFECTTEXTUREMANAGER_H

// fix problems with Visual Studio 2008
#ifdef WIN32
#include <stdio.h>
#endif

#include <iostream>

#include <vector>
#include <string>

#include <OpenSG/OSGConfig.h>
#if OSG_MAJOR_VERSION >= 2
# include <OpenSG/OSGTextureObjChunk.h>
#else
# include <OpenSG/OSGTextureChunk.h>
namespace OSG {
	typedef TextureChunkPtr TextureObjChunkRefPtr;
}
#endif

#ifdef WIN32
	#ifdef INVRSTEXTUREMANAGER_EXPORTS
	#define INVRS_TEXTUREMANAGER_API __declspec(dllexport)
	#else
	#define INVRS_TEXTUREMANAGER_API __declspec(dllimport)
	#endif
#else
	#define INVRS_TEXTUREMANAGER_API
#endif



class EffectTextureManagerListEntry {
private:

	EffectTextureManagerListEntry(std::string path, OSG::TextureObjChunkRefPtr tex);

	std::string name;
	OSG::TextureObjChunkRefPtr tex;

	friend class EffectTextureManager;
};

class INVRS_TEXTUREMANAGER_API EffectTextureManager {
public:

	EffectTextureManager();
	virtual ~EffectTextureManager();

	virtual OSG::TextureObjChunkRefPtr loadTexture(std::string path);

protected:

	virtual void initializeTexture(std::string path, OSG::TextureObjChunkRefPtr tex);

	std::vector<EffectTextureManagerListEntry*> texturesLoaded;
};

#endif
