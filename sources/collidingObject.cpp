#include "collidingObject.h"
#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>

CollidingObject::CollidingObject(void)
{
}

CollidingObject::CollidingObject(char* category, OSG::Vec3f pos)
{
    _category = category;
    _position = pos;
}

CollidingObject::CollidingObject(char* category, OSG::Vec3f pos, float radius)
{
    _category = category;
    _position = pos;
    _radius = radius;
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

	bool testX = objectPos.x() > (thisPos.x() - r) && objectPos.x() < (thisPos.x() + r);
	bool testY = objectPos.y() > (thisPos.y() - r) && objectPos.y() < (thisPos.y() + r);
	bool testZ = objectPos.z() > (thisPos.z() - r) && objectPos.z() < (thisPos.z() + r);

	if(testX && testY && testZ)
		return true;
	else
		return false;
}
