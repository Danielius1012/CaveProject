#include "collidingObject.h"
#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>

CollidingObject::CollidingObject(void)
{
}

CollidingObject::CollidingObject(char* _category, OSG::Vec3f pos)
	:_position(pos)
{

}

CollidingObject::CollidingObject(char* _category, OSG::Vec3f pos, float radius)
	:_position(pos) , _radius(radius)
{

}

CollidingObject::~CollidingObject(void)
{
	
}

void CollidingObject::setPosition(OSG::Vec3f pos)
{
	_position = pos;
}

void CollidingObject::setRadius(float radius)
{
	_radius = radius;
}

OSG::Vec3f CollidingObject::getPosition()
{
	return _position;
}

char* CollidingObject::getCategory()
{
	return this->_category;
}

float CollidingObject::getRadius()
{
	return this->_radius;
}

bool CollidingObject::isColliding(CollidingObject otherObject)
{
	float r = this->getRadius();
	OSG::Vec3f thisPos = this->getPosition();
	OSG::Vec3f objectPos = otherObject.getPosition();

	float objectX = objectPos.x();
	float objectY = objectPos.y();
	float objectZ = objectPos.z();

	float minX, maxX = thisPos.x();
	float minY, maxY = thisPos.y();
	float minZ, maxZ = thisPos.z(); 
	minX -= r;
	maxX += r;
	minY -= r;
	maxY += r;
	minZ -= r;
	maxZ += r;

	bool testX = objectX > minX && objectX < maxX;
	bool testY = objectY > minY && objectY < maxY;
	bool testZ = objectZ > minZ && objectZ < maxZ;

	if(testX && testY && testZ)
		return true;
	else
		return false;
}