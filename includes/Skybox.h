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

#ifndef _SKYBOX_H
#define _SKYBOX_H

#include <string>

#include <gmtl/Vec.h>

#include <OpenSG/OSGVector.h>
#include <OpenSG/OSGMatrix.h>
#include <OpenSG/OSGQuaternion.h>
#include <OpenSG/OSGBaseTypes.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleTexturedMaterial.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGBlendChunk.h>
#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGMaterialGroup.h>
#include <OpenSG/OSGSwitch.h>
#if OSG_MAJOR_VERSION >= 2
# include <OpenSG/OSGTypedGeoIntegralProperty.h>
# include <OpenSG/OSGCubeTextureObjChunk.h>
#else
# include <OpenSG/OSGCubeTextureChunk.h>
namespace OSG {
	typedef TextureChunkRefPtr TextureChunkObjRefPtr;
	typedef CubeTextureChunk CubeTextureObjChunk;
	typedef CubeTextureChunkRefPtr CubeTextureObjChunkRefPtr;
	typedef NodePtr NodeTransitPtr;
}
#endif

#include "TextureManager/EffectTextureManager.h"

#ifdef WIN32
	#ifdef INVRSSKYBOX_EXPORTS
		#define INVRS_SKYBOX_API __declspec(dllexport)
	#else
		#define INVRS_SKYBOX_API __declspec(dllimport)
	#endif
#else
	#define INVRS_SKYBOX_API
#endif


class SkyboxTextureManager : public EffectTextureManager {
protected:

	virtual void initializeTexture(std::string path, OSG::TextureObjChunkRefPtr tex);
};

/**
 * Simple class for box shaped skyboxes. Allows the size and textures
 * to be changed at runtime (via setTextures() and setSize() methods).
 * Usage:
 *    First call init() to specify the initial properties of the skybox.
 *    Call getNodePtr() to integrate the skybox into your scene graph.
 *    Call setupRender() each frame before drawing
 */
class INVRS_SKYBOX_API Skybox {
public:
	Skybox();
	~Skybox();

	bool init(float xSize, float ySize, float zSize, float farClipDist, std::string texNegY,
			std::string texPosY, std::string texNegX, std::string texPosX, std::string texNegZ,
			std::string texPosZ, int sortKey = 0);
	bool setTextures(std::string texNegY, std::string texPosY, std::string texNegX,
			std::string texPosX, std::string texNegZ, std::string texPosZ);
	void setSize(float xSize, float ySize, float zSize, float farClipDist);
	OSG::NodeTransitPtr getNodePtr();

	/**
	 * adjusts the position and size of the skybox
	 * this method must be called whenever the camera position has been changed
	 */
	void setupRender(OSG::Vec3f camPos);
	void setupRender(gmtl::Vec3f camPos);
	OSG::CubeTextureObjChunkRefPtr getSkyboxCubemap();

protected:
	enum SIDE {
		SIDE_NEGY = 0, SIDE_POSY = 1, SIDE_NEGX = 2, SIDE_POSX = 3, SIDE_NEGZ = 4, SIDE_POSZ = 5
	};

	//bool LoadTexture(TextureChunkRefPtr dst, std::string path);

	OSG::GeometryRefPtr createQuad(OSG::Pnt3f v0, OSG::Pnt3f v1, OSG::Pnt3f v2, OSG::Pnt3f v3, OSG::TextureObjChunkRefPtr texture);
	void createSide(SIDE s, OSG::Pnt3f v0, OSG::Pnt3f v1, OSG::Pnt3f v2, OSG::Pnt3f v3);

	OSG::TextureObjChunkRefPtr textures[6];
	OSG::CubeTextureObjChunkRefPtr asCubemap;
	std::string texPaths[6];

	float xSize;
	float ySize;
	float zSize;
	float farClipDistance;
	OSG::NodeRefPtr transf;
	OSG::TransformRefPtr transfCore;
	OSG::NodeRefPtr skybox;
	OSG::GroupRefPtr skyboxCore;
	OSG::NodeRefPtr side[6];
	OSG::GeometryRefPtr sideGeometryCore[6];
	OSG::Matrix4f scale;
	bool initialized;

	int sortKey;

	SkyboxTextureManager textureManager;

};

#endif
