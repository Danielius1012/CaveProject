#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <ios>
#include <list>
#include <chrono>

#include <OpenSG/OSGMaterialGroup.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGSimpleTexturedMaterial.h>

#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGMultiDisplayWindow.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <OSGCSM/OSGCAVESceneManager.h>
#include <OSGCSM/OSGCAVEConfig.h>
#include <OSGCSM/appctrl.h>

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>

#include "collidingObject.h"

OSG_USING_NAMESPACE

//------------------------------------------------------------------------------
// ------------------------------ GLOBAL VARIABLES -----------------------------
//------------------------------------------------------------------------------
OSGCSM::CAVEConfig cfg;
OSGCSM::CAVESceneManager *mgr = nullptr;
vrpn_Tracker_Remote* tracker =  nullptr;
vrpn_Button_Remote* button = nullptr;
vrpn_Analog_Remote* analog = nullptr;
NodeRecPtr root;

//Skybox skybox;                        // scene surroundings

// MOVEMENT VALUES
const float BOOST_VALUE = 2.5f * 1e-3;	// min. 2 , every below is just button smashing
const float GRAVITY_PULL = 1e-12;
const float VELOCITY_THRESHOLD = 5.f;
const float MOON_SURFACE = -2000.f;
const float HEIGHT_START = 1900.f;		// take surface model into account
const int BOOST_TIME_THRESHOLD = 1e9;	// 1 billion nanoseconds = 1s
const int RESET_COUNTDOWN = 5 * 1e9;	// first number = seconds on the moon
const int FUEL_AMOUNT = 100;			// The lower, the harder the game will be

// USER
CollidingObject playerCollider = CollidingObject("Player", Vec3f(0.f,0.f,0.f), 2.f); 

auto userMovement = Vec3f(0.f, 0.f, 0.f);

// Start time for boost 
auto boostStartTime = std::chrono::high_resolution_clock::now();

// the last measured time - for differnce 
auto startTime = std::chrono::high_resolution_clock::now();

// The current time
auto currentTime = std::chrono::high_resolution_clock::now();

// player speed - manipulated by gravity and boost
float currentVelocity = 0.f;

// current height above moon surface
float height = HEIGHT_START;

// The user starts with 100 units of fuel - every boost burns 10 units
int fuel = FUEL_AMOUNT;

// OBJECTS
float objectRotationValue = 0.f;
std::list<CollidingObject*> objectList; 
bool isReset = true;
bool justStarted = true;
auto startResetCounter = std::chrono::high_resolution_clock::now();


// Trasformable Objects

// FUEL
NodeRecPtr fuel1TransNode;
NodeRecPtr fuel2TransNode;
NodeRecPtr fuel3TransNode;
NodeRecPtr fuel4TransNode;

// EARTH
NodeRecPtr earthTransNode;

// MOON
NodeRecPtr moonTransNode;

// TEAPOTS
NodeRecPtr winTeapotTransNode;
NodeRecPtr lostTeapotTransNode;

// LANDING
NodeRecPtr landingStripTransNode;

void cleanup()
{
	delete mgr;
	delete tracker;
	delete button;
	delete analog;
}

void print_tracker();

void addCollisionVolumes(void)
{
	CollidingObject* fuel1Collider = new CollidingObject("Fuel1", Vec3f(-100.f,0.f,-400.f), 100.f); 
	CollidingObject* fuel2Collider = new CollidingObject("Fuel2", Vec3f(100.f,-200.f,-600.f), 100.f); 
	CollidingObject* fuel3Collider = new CollidingObject("Fuel3", Vec3f(200.f,-600.f,-400.f), 100.f); 
	CollidingObject* fuel4Collider = new CollidingObject("Fuel4", Vec3f(-200.f,-800.f,-200.f), 100.f); 

	objectList.push_front(fuel4Collider);
	objectList.push_front(fuel3Collider);
	objectList.push_front(fuel2Collider);
	objectList.push_front(fuel1Collider);

	// ADD NODES TO SCENE
	root->addChild(fuel1TransNode);
	root->addChild(fuel2TransNode);
	root->addChild(fuel3TransNode);
	root->addChild(fuel4TransNode);
}

/// -------------------------------------------------------------------------------------------------------
/// ------------------------------------------- BUILD SCENE -----------------------------------------------
/// -------------------------------------------------------------------------------------------------------
NodeTransitPtr buildScene()
{
	// ---------------------------------------- SETUP ROOT ------------------------------------------------

	root = Node::create();
	root->setCore(Group::create());

	// ----------------------------------------------------------------------------------------------------

	// --------------------------------------- CREATE OBJECTS ---------------------------------------------

	// FUEL 1
	//NodeRecPtr fuel1 = SceneFileHandler::the()->read("models/fuel.3DS");
	NodeRecPtr fuel1 = makeSphere(8.f,30.f); 

	// FUEL 2
	NodeRecPtr fuel2 = makeSphere(8.f,30);

	// FUEL 3
	NodeRecPtr fuel3 = makeSphere(8.f,30);

	// FUEL 4
	NodeRecPtr fuel4 = makeSphere(8.f,30);
	// ----------------------------------------------------------------------------------------------------

	// ------------------------------------- TRANSFORMATION SETUP -----------------------------------------
	
	// FUEL 1 - TransCore
	TransformRecPtr fuel1TransCore = Transform::create();
	Matrix fuel1Matrix;

	// FUEL 2 - TransCore
	TransformRecPtr fuel2TransCore = Transform::create();
	Matrix fuel2Matrix;

	// FUEL 3 - TransCore
	TransformRecPtr fuel3TransCore = Transform::create();
	Matrix fuel3Matrix;

	// FUEL 4 - TransCore
	TransformRecPtr fuel4TransCore = Transform::create();
	Matrix fuel4Matrix;

	// ----------------------------------------------------------------------------------------------------

	// ---------------------------------------- MATRIX SETUP ----------------------------------------------

	// FUEL 1 Matrix
	fuel1Matrix.setIdentity();
	fuel1Matrix.setTranslate(0,40,-50);
	fuel1TransCore->setMatrix(fuel1Matrix);

	// FUEL 2 Matrix
	fuel2Matrix.setIdentity();
	fuel2Matrix.setTranslate(0,40,-50);
	fuel2TransCore->setMatrix(fuel2Matrix);

	// FUEL 3 Matrix
	fuel3Matrix.setIdentity();
	fuel3Matrix.setTranslate(0,40,-50);
	fuel3TransCore->setMatrix(fuel3Matrix);

	// FUEL 4 Matrix
	fuel4Matrix.setIdentity();
	fuel4Matrix.setTranslate(0,40,-50);
	fuel4TransCore->setMatrix(fuel4Matrix);

	// ----------------------------------------------------------------------------------------------------
	
	// ----------------------------------------- NODE SETUP -----------------------------------------------

	// FUEL 1 Node
	fuel1TransNode = makeNodeFor(fuel1TransCore);
	fuel1TransNode->addChild(fuel1);

	// FUEL 2 Node
	fuel2TransNode = makeNodeFor(fuel2TransCore);
	fuel2TransNode->addChild(fuel2);

	// FUEL 3 Node
	fuel3TransNode = makeNodeFor(fuel3TransCore);
	fuel3TransNode->addChild(fuel3);

	// FUEL 4 Node
	fuel4TransNode = makeNodeFor(fuel4TransCore);
	fuel4TransNode->addChild(fuel4);

	// ----------------------------------------------------------------------------------------------------

	// ------------------------------------------ TRANSFORM -----------------------------------------------

	// FUEL1 TRANSFORM
	ComponentTransformRecPtr fuel1Trans = ComponentTransform::create();
    fuel1Trans->setTranslation(Vec3f(-200.f,0.f,-500.f));
	fuel1Trans->setRotation(Quaternion(Vec3f(1.f,0.f,0.f),osgDegree2Rad(90)));
	fuel1Trans->setScale(Vec3f(2.f,2.f,2.f));

	fuel1TransNode = Node::create();
	fuel1TransNode->setCore(fuel1Trans);
	fuel1TransNode->addChild(fuel1);

	// FUEL2 TRANSFORM
	ComponentTransformRecPtr fuel2Trans = ComponentTransform::create();
    fuel2Trans->setTranslation(Vec3f(200.f,-400.f,-600.f));
	fuel2Trans->setRotation(Quaternion(Vec3f(1.f,0.f,0.f),osgDegree2Rad(90)));
	fuel2Trans->setScale(Vec3f(2.f,2.f,2.f));

	fuel2TransNode = Node::create();
	fuel2TransNode->setCore(fuel2Trans);
	fuel2TransNode->addChild(fuel2);

	// FUEL3 TRANSFORM
	ComponentTransformRecPtr fuel3Trans = ComponentTransform::create();
    fuel3Trans->setTranslation(Vec3f(300.f,-1200.f,-500.f));
	fuel3Trans->setRotation(Quaternion(Vec3f(1.f,0.f,0.f),osgDegree2Rad(90)));
	fuel3Trans->setScale(Vec3f(2.f,2.f,2.f));

	fuel3TransNode = Node::create();
	fuel3TransNode->setCore(fuel3Trans);
	fuel3TransNode->addChild(fuel3);

	// FUEL4 TRANSFORM
	ComponentTransformRecPtr fuel4Trans = ComponentTransform::create();
    	fuel4Trans->setTranslation(Vec3f(-200.f,-1600.f,-200.f));
	fuel4Trans->setRotation(Quaternion(Vec3f(1.f,0.f,0.f),osgDegree2Rad(90)));
	fuel4Trans->setScale(Vec3f(2.f,2.f,2.f));

	fuel4TransNode = Node::create();
	fuel4TransNode->setCore(fuel4Trans);
	fuel4TransNode->addChild(fuel4);

	// LANDING STRIP

	// ADD COLLISION VOLUMES TO NODES
	addCollisionVolumes();

	// ----------------------------------------------------------------------------------------------------

	// ADD EARTH MODEL 
	NodeRecPtr earth = SceneFileHandler::the()->read("models/earth.3DS");

	ComponentTransformRecPtr earthTrans = ComponentTransform::create();
	//earthTrans->setTranslation(Vec3f(1000.f,1000.f,-3000.f));
	earthTrans->setTranslation(Vec3f(100000.f,100000.f,-385000.f));
	earthTrans->setScale(Vec3f(1000.f,1000.f,1000.f));
	
	earthTransNode = makeNodeFor(earthTrans);
	earthTransNode->addChild(earth);

	root->addChild(earthTransNode);

	// ADD MOON SURFACE
	NodeRecPtr moonSurface = SceneFileHandler::the()->read("models/moon.3DS");

	ComponentTransformRecPtr moonTrans = ComponentTransform::create();
	moonTrans->setTranslation(Vec3f(1000.f,-2000.f,-1000.f));
	moonTrans->setScale(Vec3f(1000.f,1.f,1000.f));
	
	moonTransNode = makeNodeFor(moonTrans);
	moonTransNode->addChild(moonSurface);

	root->addChild(moonTransNode);

	// ------------------ WINNING TEAPOT - GREEN ------------------
	SimpleMaterialRecPtr winTeapotMat = SimpleMaterial::create();

	winTeapotMat->setDiffuse(Color3f(0.f,0.9f,0.2f));
	winTeapotMat->setAmbient(Color3f(0.f, 0.5f, 0.1f));
	winTeapotMat->setTransparency(0);
	//winTeapotMat->setLit(true);

	NodeRecPtr winTeapot = makeTeapot(8.f,8.f);;

	ComponentTransformRecPtr winTeapotTrans = ComponentTransform::create();
	winTeapotTrans->setTranslation(Vec3f(0.f,0.f,10000.f));
	winTeapotTrans->setScale(Vec3f(5.f,5.f,5.f));
	
	winTeapotTransNode = makeNodeFor(winTeapotTrans);
	winTeapotTransNode->addChild(winTeapot);

	root->addChild(winTeapotTransNode);
	GeometryRecPtr winTeapotGeo = dynamic_cast<Geometry*>(winTeapot->getCore());
	winTeapotGeo->setMaterial(winTeapotMat);

	// ------------------ LOSING TEAPOT - RED ------------------
	SimpleMaterialRecPtr lostTeapotMat = SimpleMaterial::create();

	lostTeapotMat->setDiffuse(Color3f(0.8f,0.2f,0.2f));
	lostTeapotMat->setAmbient(Color3f(0.5f, 0.1f, 0.1f));
	lostTeapotMat->setTransparency(0);
	//lostTeapotMat->setLit(true);

	NodeRecPtr lostTeapot = makeTeapot(8.f,8.f);;

	ComponentTransformRecPtr lostTeapotTrans = ComponentTransform::create();
	lostTeapotTrans->setTranslation(Vec3f(0.f,0.f,10000.f));
	lostTeapotTrans->setScale(Vec3f(5.f,5.f,5.f));
	
	lostTeapotTransNode = makeNodeFor(lostTeapotTrans);
	lostTeapotTransNode->addChild(lostTeapot);

	root->addChild(lostTeapotTransNode);
	GeometryRecPtr lostTeapotGeo = dynamic_cast<Geometry*>(lostTeapot->getCore());
	lostTeapotGeo->setMaterial(lostTeapotMat);

	// LANDING STRIP
	SimpleMaterialRecPtr landingStripMat = SimpleMaterial::create();

	landingStripMat->setDiffuse(Color3f(1.f,1.f,1.f));
	landingStripMat->setAmbient(Color3f(0.5f, 0.5f, 0.5f));
	landingStripMat->setTransparency(0);
	//landingStripMat->setLit(true);

	NodeRecPtr landingStrip = makeBox(50,1,50,10,10,10);;

	ComponentTransformRecPtr landingStripTrans = ComponentTransform::create();
	landingStripTrans->setTranslation(Vec3f(0.f,-1900.f,0.f));
	landingStripTrans->setScale(Vec3f(5.f,5.f,5.f));
	
	landingStripTransNode = makeNodeFor(landingStripTrans);
	landingStripTransNode->addChild(landingStrip);

	root->addChild(landingStripTransNode);
	GeometryRecPtr landingStripGeo = dynamic_cast<Geometry*>(landingStrip->getCore());
	landingStripGeo->setMaterial(landingStripMat);

	// Add color to spheres
	SimpleMaterialRecPtr sphereMat = SimpleMaterial::create();

	sphereMat->setDiffuse(Color3f(0.f,0.8f,1.f));
	sphereMat->setAmbient(Color3f(0.f, 0.4f, 0.5f));
	//sphereMat->setTransparency(0.25);

	GeometryRecPtr fuel1Geo = dynamic_cast<Geometry*>(fuel1->getCore());
	GeometryRecPtr fuel2Geo = dynamic_cast<Geometry*>(fuel2->getCore());
	GeometryRecPtr fuel3Geo = dynamic_cast<Geometry*>(fuel3->getCore());
	GeometryRecPtr fuel4Geo = dynamic_cast<Geometry*>(fuel4->getCore());
	fuel1Geo->setMaterial(sphereMat);
	fuel2Geo->setMaterial(sphereMat);
	fuel3Geo->setMaterial(sphereMat);
	fuel4Geo->setMaterial(sphereMat);

	return NodeTransitPtr(root);
}

void activateBoost(void)
{
	auto boostCurrentTime = std::chrono::high_resolution_clock::now();
	
	// if the time between boost is greater then the threshold
	if( (boostCurrentTime - boostStartTime).count() > BOOST_TIME_THRESHOLD)
	{
            if(fuel > 0)
            {
				std::cout << "--------------------------- BOOST --------------------------" 
					<< "\nTIME: " << (boostCurrentTime - boostStartTime).count()
					<< "\nFUEL: " << fuel  
					<< "\n------------------------------------------------------------\n";                
				currentVelocity += BOOST_VALUE;
                fuel -= 5;
                boostStartTime = boostCurrentTime;
            }
	}
}

template<typename T>
T scale_tracker2cm(const T& value)
{
	static const float scale = OSGCSM::convert_length(cfg.getUnits(), 1.f, OSGCSM::CAVEConfig::CAVEUnitCentimeters);
	return value * scale;
}

auto head_orientation = Quaternion(Vec3f(0.f, 1.f, 0.f), 3.141f);
auto head_position = Vec3f(0.f, 170.f, 200.f);	// a 1.7m Person 2m in front of the scene

void VRPN_CALLBACK callback_head_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	head_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	head_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

auto wand_orientation = Quaternion();
auto wand_position = Vec3f();
void VRPN_CALLBACK callback_wand_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	wand_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	wand_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

auto analog_values = Vec3f();
void VRPN_CALLBACK callback_analog(void* userData, const vrpn_ANALOGCB analog)
{
	if (analog.num_channel >= 2)
		analog_values = Vec3f(analog.channel[0], 0, -analog.channel[1]);
}

void VRPN_CALLBACK callback_button(void* userData, const vrpn_BUTTONCB button)
{
	if (button.button == 0 && button.state == 1)
	{
		activateBoost();
	}
}

void InitTracker(OSGCSM::CAVEConfig &cfg)
{
	try
	{
		const char* const vrpn_name = "DTrack@localhost";
		tracker = new vrpn_Tracker_Remote(vrpn_name);
		tracker->shutup = true;
		tracker->register_change_handler(NULL, callback_head_tracker, cfg.getSensorIDHead());
		tracker->register_change_handler(NULL, callback_wand_tracker, cfg.getSensorIDController());
		button = new vrpn_Button_Remote(vrpn_name);
		button->shutup = true;
		button->register_change_handler(nullptr, callback_button);
		analog = new vrpn_Analog_Remote(vrpn_name);
		analog->shutup = true;
		analog->register_change_handler(NULL, callback_analog);
	}
	catch(const std::exception& e) 
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return;
	}
}

void check_tracker()
{
	tracker->mainloop();
	button->mainloop();
	analog->mainloop();
}

void print_tracker()
{
	std::cout << "Head position: " << head_position << " orientation: " << head_orientation << '\n';
	std::cout << "Wand position: " << wand_position << " orientation: " << wand_orientation << '\n';
	std::cout << "Analog: " << analog_values << '\n';
}

void objectMotion()
{
	objectRotationValue += 0.001f;
	Quaternion fuelRotation = Quaternion(Vec3f(0,1,0), osgDegree2Rad(90) + objectRotationValue) + Quaternion(Vec3f(0,0,1), osgDegree2Rad(90));

	// Transform Fuel 1
	ComponentTransformRecPtr fuel1DynTrans = dynamic_cast<ComponentTransform*>(fuel1TransNode->getCore());
    	fuel1DynTrans->setRotation(fuelRotation);
	
	// Transform Fuel 2
	ComponentTransformRecPtr fuel2DynTrans = dynamic_cast<ComponentTransform*>(fuel2TransNode->getCore());
    	fuel2DynTrans->setRotation(fuelRotation);

	// Transform Fuel 3
	ComponentTransformRecPtr fuel3DynTrans = dynamic_cast<ComponentTransform*>(fuel3TransNode->getCore());
    	fuel3DynTrans->setRotation(fuelRotation);

	// Transform Fuel 4
	ComponentTransformRecPtr fuel4DynTrans = dynamic_cast<ComponentTransform*>(fuel4TransNode->getCore());
    	fuel4DynTrans->setRotation(fuelRotation);

    	// Rotate Earth
	ComponentTransformRecPtr earthDynTrans = dynamic_cast<ComponentTransform*>(earthTransNode->getCore());
    	earthDynTrans->setRotation(Quaternion(Vec3f(0,1,1), osgDegree2Rad(90) + objectRotationValue));
    	earthDynTrans->setTranslation(Vec3f(100000.f,mgr->getTranslation().y() + 100000.f,-385000.f));

      	// EXAMPLES:
	//bt->setTranslation(Vec3f(10,5,0));
	//bt->setScale(Vec3f(0.001,0.001,0.001));

	//updateMesh(time);

	// -------------------------------------------------------------------------------------------
}

void keyboard(unsigned char k, int x, int y)
{
	Real32 ed;
	switch(k)
	{
		case 'q':
		case 27: 
			cleanup();
			exit(EXIT_SUCCESS);
			break;
		case 'e':
			ed = mgr->getEyeSeparation() * .9f;
			std::cout << "Eye distance: " << ed << '\n';
			mgr->setEyeSeparation(ed);
			break;
		case 'E':
			ed = mgr->getEyeSeparation() * 1.1f;
			std::cout << "Eye distance: " << ed << '\n';
			mgr->setEyeSeparation(ed);
			break;
		case 'h':
			cfg.setFollowHead(!cfg.getFollowHead());
			std::cout << "following head: " << std::boolalpha << cfg.getFollowHead() << '\n';
			break;
		case 'i':
			print_tracker();
			break;
		default:
			std::cout << "Key '" << k << "' ignored\n";
	}
}

void reshape(int w, int h)
{
	mgr->resize(w, h);
	glutPostRedisplay();
}

void display(void)
{
	commitChanges();
	
	//skybox.setupRender(camera->getPosition()); // attach the SkyBox to the camera
	//skybox.setupRender(activeCamera->getPosition()); // attach the SkyBox to the camera

	mgr->redraw();

	//the changelist should be cleared - else things
	//could be copied multiple times
	OSG::Thread::getCurrentChangeList()->clear();

	// to ensure a black navigation window
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
}

void resetScene(void)
{
        playerCollider = CollidingObject("Player", Vec3f(0.f,0.f,0.f), 2.f);

        userMovement = Vec3f(0.f, 0.f, 0.f);

        // Start time for boost
        boostStartTime = std::chrono::high_resolution_clock::now();

        // the last measured time - for differnce
        startTime = std::chrono::high_resolution_clock::now();

        // The current time
        currentTime = std::chrono::high_resolution_clock::now();

        // player speed - manipulated by gravity and boost
        currentVelocity = 0.f;

        // current height above moon surface
        height = HEIGHT_START;

        // The user starts with 100 units of fuel - every boost burns 10 units
        fuel = FUEL_AMOUNT;

        // OBJECTS
        objectRotationValue = 0.f;
		objectList.clear();

        // Trasformable Objects

        // FUEL
        fuel1TransNode;
        fuel2TransNode;
        fuel3TransNode;
        fuel4TransNode;

        // EARTH
        earthTransNode;

        // MOON
        moonTransNode;

        // PLAYER POSITION RESET
    	mgr->setTranslation(Vec3f(0.f,0.f,0.f));

		// Add collision volumes
		addCollisionVolumes();
		
		
	// RESET TEAPOTS
	// winTeapotTransNode
	ComponentTransformRecPtr winTeapotDynTrans = dynamic_cast<ComponentTransform*>(winTeapotTransNode->getCore());
	winTeapotDynTrans->setTranslation(Vec3f(0.f,0.f,385000.f));

	// lostTeapotTransNode
	ComponentTransformRecPtr lostTeapotDynTrans = dynamic_cast<ComponentTransform*>(lostTeapotTransNode->getCore());
	lostTeapotDynTrans->setTranslation(Vec3f(0.f,0.f,385000.f));

	isReset = true;
}

// COLLISION - PLAYER WITH OBJECTS
void checkCollision(void)
{
	// update collision Object based on player 
	playerCollider = CollidingObject("Player", mgr->getTranslation() + userMovement, 100.f);

	// For every object in the objectList
	for (std::list<CollidingObject*>::iterator obj = objectList.begin(); obj != objectList.end(); )
	{
		// If colliding, check for category
		if(playerCollider.isColliding(**obj))
		{
			char* cat = (*obj)->getCategory();
			// std::cout << "CAT: " << cat << std::endl;

			// Check for Category - Fuel or Moon
			if(cat == "Fuel1")
			{
				// if fuel, add FUEL_AMOUNT and destroy object
				fuel += FUEL_AMOUNT;

				// Remove object from root
				// root->subChild(fuel1TransNode);
				delete (*obj);
				obj = objectList.erase(obj);
				std::cout << "------- FUEL 1 Picked up! -------" << std::endl;
			}
			else if(cat == "Fuel2")
			{
				// if fuel, add FUEL_AMOUNT and destroy object
				fuel += FUEL_AMOUNT;

				// Remove object from root
				// root->subChild(fuel2TransNode);
				delete (*obj);
				obj = objectList.erase(obj);
				std::cout << "------- FUEL 2 Picked up! -------" << std::endl;
			}
			else if(cat == "Fuel3")
			{
				// if fuel, add FUEL_AMOUNT and destroy object
				fuel += FUEL_AMOUNT;

				// Remove object from root
				// root->subChild(fuel3TransNode);
				delete (*obj);
				obj = objectList.erase(obj);
				std::cout << "------- FUEL 3 Picked up! -------" << std::endl;
			}
			else if(cat == "Fuel4")
			{
				// if fuel, add FUEL_AMOUNT and destroy object
				fuel += FUEL_AMOUNT;

				// Remove object from root
				// root->subChild(fuel4TransNode);
				delete (*obj);
				obj = objectList.erase(obj);
				std::cout << "------- FUEL 4 Picked up! -------" << std::endl;
			}
			else
			{	
				++obj;

			}
		}else {
			++obj;
		}	
	}
}


// GAME LOOP - Every frame
void update(void)
{
	// Get current time
	currentTime = std::chrono::high_resolution_clock::now();
	
	// std::chrono::duration<double> difference = currentTime - startTime;
	auto difference = currentTime - startTime;

	// Calculate Gravity and apply to velocity
	if(justStarted)
    {
		justStarted = false;
		currentVelocity = 0;
		startTime = currentTime;
	}
	else
	{
		currentVelocity -= GRAVITY_PULL * difference.count();
		startTime = currentTime;
	}

    	// APPLY FORCES
	userMovement += Vec3f(0.f ,currentVelocity, 0.f);
	height = HEIGHT_START + mgr->getTranslation().y() + userMovement.y();

    	// transform the objects
    	objectMotion();

	// Check for collision
	checkCollision();

	// std::cout << "Velo: " << currentVelocity << "\n";
	// std::cout << "High: " << height << "\n";

	
}

void idle(void)
{
	check_tracker();
	const auto speed = 1.f;

	update();
	
	if(height < 0)
	{
		if(isReset)
		{
			startResetCounter = std::chrono::high_resolution_clock::now();
		}

		mgr->setUserTransform(head_position, head_orientation);
    		mgr->setTranslation(mgr->getTranslation() + speed * analog_values);
		
		Vec3f movementNearPlayer = Vec3f(0.f,0.f,-400.f);

		if(isReset)
		{
			std::cout << "SPEED: " << abs(currentVelocity) * 1000.f << std::endl;

			// Check velocity
			if(abs(currentVelocity * 1000.f) < VELOCITY_THRESHOLD)
			{
				// win
				ComponentTransformRecPtr winTeapotDynTrans = dynamic_cast<ComponentTransform*>(winTeapotTransNode->getCore());
				winTeapotDynTrans->setTranslation(mgr->getTranslation() + userMovement + movementNearPlayer);
				winTeapotDynTrans->setRotation(Quaternion(Vec3f(1,0,0), osgDegree2Rad(270)));
			}
			else
			{
				// lose
				ComponentTransformRecPtr lostTeapotDynTrans = dynamic_cast<ComponentTransform*>(lostTeapotTransNode->getCore());
				lostTeapotDynTrans->setTranslation(mgr->getTranslation() + userMovement + movementNearPlayer);
				lostTeapotDynTrans->setRotation(Quaternion(Vec3f(1,0,0), osgDegree2Rad(270)));
			}
			isReset = false;
		}
		
		
		auto endResetcounter = std::chrono::high_resolution_clock::now();

		if((endResetcounter - startResetCounter).count() > RESET_COUNTDOWN)
		{
			resetScene();
			currentVelocity = 0;
		}
	}
	else
	{
		// TRANSFORM AND TRANSLATE
		mgr->setUserTransform(head_position, head_orientation);
		mgr->setTranslation(mgr->getTranslation()  + userMovement + speed * analog_values);
		//std::cout << "USER: " << userMovement<< "\n";
		//std::cout << "MNGR: " << mgr->getTranslation() << "\n";
		//std::cout << "ANLG: " << analog_values<< "\n";
		//std::cout << "SPED: " << speed<< "\n";
	}	
	
	


	commitChanges();
	mgr->redraw();
	// the changelist should be cleared - else things could be copied multiple times
	OSG::Thread::getCurrentChangeList()->clear();
}



void setupGLUT(int *argc, char *argv[])
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGB  |GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("OpenSG CSMDemo with VRPN API");
	
	// ---------------------------------------   DISPLAY   -------------------------------------------
	glutDisplayFunc(display);
	// ---------------------------------------   RESHAPE   -------------------------------------------
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	// ----------------------------------------   IDLE   ---------------------------------------------
	glutIdleFunc(idle);
}

void audio(void)
{
	
}

int main(int argc, char **argv)
{
#if WIN32
	OSG::preloadSharedObject("OSGFileIO");
	OSG::preloadSharedObject("OSGImageFileIO");
#endif
	try
	{
		bool cfgIsSet = false;
		NodeRefPtr scene = nullptr;

		// ChangeList needs to be set for OpenSG 1.4
		ChangeList::setReadWriteDefault();
		osgInit(argc,argv);

		// PLAY SOUNDS
		audio();

		// evaluate intial params
		for(int a=1 ; a<argc ; ++a)
		{
			if( argv[a][0] == '-' )
			{
				if ( strcmp(argv[a],"-f") == 0 ) 
				{
					char* cfgFile = argv[a][2] ? &argv[a][2] : &argv[++a][0];
					if (!cfg.loadFile(cfgFile)) 
					{
						std::cout << "ERROR: could not load config file '" << cfgFile << "'\n";
						return EXIT_FAILURE;
					}
					cfgIsSet = true;
				}
			} else {
				std::cout << "Loading scene file '" << argv[a] << "'\n";
				scene = SceneFileHandler::the()->read(argv[a], NULL);
			}
		}

		// load the CAVE setup config file if it was not loaded already:
		if (!cfgIsSet) 
		{
			const char* const default_config_filename = "config/mono.csm";
			if (!cfg.loadFile(default_config_filename)) 
			{
				std::cout << "ERROR: could not load default config file '" << default_config_filename << "'\n";
				return EXIT_FAILURE;
			}
		}

		cfg.printConfig();

		// start servers for video rendering
		if ( startServers(cfg) < 0 ) 
		{
			std::cout << "ERROR: Failed to start servers\n";
			return EXIT_FAILURE;
		}

		setupGLUT(&argc, argv);

		InitTracker(cfg);

		MultiDisplayWindowRefPtr mwin = createAppWindow(cfg, cfg.getBroadcastaddress());

		if (!scene) 
		{
			scene = buildScene();	
		}
		commitChanges();

		mgr = new OSGCSM::CAVESceneManager(&cfg);
		mgr->setWindow(mwin );
		mgr->setRoot(scene);
		mgr->showAll();

		// ------------------------------------ BACKGROUND --------------------------------------- 

		ImageRecPtr backimage = Image::create();
		backimage->read("models/universe2.jpg");

		TextureObjChunkRecPtr bkgTex = TextureObjChunk::create();
		bkgTex->setImage(backimage);
		bkgTex->setScale(false);
		TextureBackgroundRecPtr imBkg = TextureBackground::create();
		imBkg->setTexture(bkgTex);
		imBkg->setColor(Color4f(1.0,1.0,1.0,0.0f));

		// alternatively use a gradient background
		//GradientBackgroundRecPtr bkg = GradientBackground::create();
		//bkg->addLine(Color3f(0.7f, 0.7f, 0.8f), 0);
		//bkg->addLine(Color3f(0.0f, 0.1f, 0.3f), 1);

		mwin->getPort(0)->setBackground(imBkg);
		mwin->getPort(1)->setBackground(imBkg);
		mwin->getPort(2)->setBackground(imBkg);
		mwin->getPort(3)->setBackground(imBkg);
		mwin->getPort(4)->setBackground(imBkg);
		mwin->getPort(5)->setBackground(imBkg);
		mwin->getPort(6)->setBackground(imBkg);
		mwin->getPort(7)->setBackground(imBkg);
		mwin->getPort(8)->setBackground(imBkg);
		mwin->getPort(9)->setBackground(imBkg);
		mwin->getPort(10)->setBackground(imBkg);
		mwin->getPort(11)->setBackground(imBkg);
		mwin->getPort(12)->setBackground(imBkg);
		mwin->getPort(13)->setBackground(imBkg);
		mwin->getPort(14)->setBackground(imBkg);
		mwin->getPort(15)->setBackground(imBkg);
		// ----------------------------------------------------------------------------------------

		mgr->getWindow()->init();
		mgr->turnWandOff();
	}
	catch(const std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	glutMainLoop();
}

