#pragma once
#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>

/// <summary>
/// A new colliding object. <para>Includes a position and its radius.</para>
/// </summary>
class CollidingObject
{
	public:
		/// <summary>
		/// The Standart constructor for the collidingObject class <seealso cref="collidingObject"/>
		/// </summary>
		CollidingObject(void);
		
		/// <summary>
		/// Constructor: Set the value for the position of the collidingObject.
		/// </summary>
		CollidingObject(char* _category, OSG::Vec3f pos);

		/// <summary>
		/// Constructor: Set the value for the position and radius of the collidingObject.
		/// </summary>
		CollidingObject(char* _category, OSG::Vec3f pos, float radius);

		~CollidingObject(void);

		void setPosition(OSG::Vec3f pos);

		OSG::Vec3f getPosition();

		void setRadius(float radius);

		float getRadius();

		char* getCategory();

		bool isColliding(CollidingObject otherObject);

	private:
		/// <value>The 3D position of the object.</value>
		OSG::Vec3f _position;
		float _radius;
		char* _category;
};

