#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <ios>

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

OSG_USING_NAMESPACE

//------------------------------------------------------------------------------
// ------------------------------ GLOBAL VARIABLES -----------------------------
//------------------------------------------------------------------------------
OSGCSM::CAVEConfig cfg;
OSGCSM::CAVESceneManager *mgr = nullptr;
vrpn_Tracker_Remote* tracker =  nullptr;
vrpn_Button_Remote* button = nullptr;
vrpn_Analog_Remote* analog = nullptr;

// Trasformable Objects
NodeRecPtr cubeTransNode;
NodeRecPtr torusTransNode;
NodeRecPtr earthTransNode;


void cleanup()
{
	delete mgr;
	delete tracker;
	delete button;
	delete analog;
}

void print_tracker();

/// -------------------------------------------------------------------------------------------------------
/// ------------------------------------------- BUILD SCENE -----------------------------------------------
/// -------------------------------------------------------------------------------------------------------
NodeTransitPtr buildScene()
{
	// ---------------------------------------- SETUP ROOT ------------------------------------------------

	NodeRecPtr root = Node::create();
	root->setCore(Group::create());

	// ----------------------------------------------------------------------------------------------------

	// --------------------------------------- CREATE OBJECTS ---------------------------------------------

	// Insert Test Torus
	NodeRecPtr testTorus = makeTorus(5.f, 10.f, 32.f, 64.f);
	root->addChild(testTorus);

	// Insert Cube
	NodeRecPtr testCube = makeBox(30,30,30,10,10,10);
	root->addChild(testCube);

	//decouple the nodes to be shifted in hierarchy from the scene
	root->subChild(testTorus);
	root->subChild(testCube);

	// ----------------------------------------------------------------------------------------------------

	// ------------------------------------- TRANSFORMATION SETUP -----------------------------------------
	
	// TORUS
	TransformRecPtr torusTransCore = Transform::create();
	Matrix torusMatrix;

	// CUBE
	TransformRecPtr cubeTransCore = Transform::create();
	Matrix cubeMatrix;

	// ----------------------------------------------------------------------------------------------------

	// ---------------------------------------- MATRIX SETUP ----------------------------------------------

	// TORUS
	torusMatrix.setIdentity();
	torusMatrix.setTranslate(0,40,0);
	torusTransCore->setMatrix(torusMatrix);

	// CUBE
	cubeMatrix.setIdentity();
	cubeMatrix.setTranslate(0,20,0);
	cubeTransCore->setMatrix(cubeMatrix);

	// ----------------------------------------------------------------------------------------------------
	
	// ----------------------------------------- NODE SETUP -----------------------------------------------

	// TORUS
	torusTransNode = makeNodeFor(torusTransCore);
	torusTransNode->addChild(testTorus);

	// CUBE
	cubeTransNode = makeNodeFor(cubeTransCore);
	cubeTransNode->addChild(testCube);

	// ----------------------------------------------------------------------------------------------------

	// ------------------------------------------ TRANSFORM -----------------------------------------------

	// TORUS
	ComponentTransformRecPtr torusTrans = ComponentTransform::create();
	torusTrans->setTranslation(Vec3f(-10.f,100.f,0.f));
	torusTrans->setRotation(Quaternion(Vec3f(1.f,0.f,0.f),osgDegree2Rad(90)));

	torusTransNode = Node::create();
	torusTransNode->setCore(torusTrans);
	torusTransNode->addChild(testTorus);

	// CUBE
	ComponentTransformRecPtr cubeTrans = ComponentTransform::create();
	cubeTrans->setTranslation(Vec3f(10.f,40.f,-40.f));
	cubeTrans->setRotation(Quaternion(Vec3f(1.f,0.f,0.f),osgDegree2Rad(90)));

	cubeTransNode = Node::create();
	cubeTransNode->setCore(cubeTrans);
	cubeTransNode->addChild(testCube);

	// -------------------------------------------- COLOR -------------------------------------------------

	//// MAKE EARTH BLUE
	//SimpleMaterialRecPtr boxMat = SimpleMaterial::create();

	//boxMat->setDiffuse(Color3f(0,0.6f,0.6f));
	//boxMat->setAmbient(Color3f(0, 0.4f, 0.4f));
	//// boxMat->setTransparency(0.25);
	////boxMat->setLit(false);

	//GeometryRecPtr boxGeo = dynamic_cast<Geometry*>(earth->getCore());
	//boxGeo->setMaterial(boxMat);

	// ----------------------------------------------------------------------------------------------------

	// ADD NODES TO SCENE
	root->addChild(torusTransNode);
	root->addChild(cubeTransNode);

	// ----------------------------------------------------------------------------------------------------

	// ADD EARTH MODEL 
	NodeRecPtr earth = SceneFileHandler::the()->read("models/moon.3DS");

	ComponentTransformRecPtr earthTrans = ComponentTransform::create();
	//earthTrans->setTranslation(Vec3f(1000.f,1000.f,-3000.f));
	earthTrans->setTranslation(Vec3f(1000.f,1000.f,-3000.f));
	earthTrans->setRotation(Quaternion(Vec3f(0,1,0),osgDegree2Rad(0)));
	earthTrans->setScale(Vec3f(5.f,5.f,5.f));

	earthTransNode = makeNodeFor(earthTrans);
	earthTransNode->addChild(earth);

	root->addChild(earthTransNode);
	
	return NodeTransitPtr(root);
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
		print_tracker();
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
	const float speed = 1000.f * std::clock() / CLOCKS_PER_SEC;

	// Transform Cube
	ComponentTransformRecPtr cubeDynTrans = dynamic_cast<ComponentTransform*>(cubeTransNode->getCore());
	cubeDynTrans->setRotation(Quaternion(Vec3f(0,1,0), osgDegree2Rad(270) + 0.001f * speed));
	
	// Transform Torus
	ComponentTransformRecPtr torusDynTrans = dynamic_cast<ComponentTransform*>(torusTransNode->getCore());
	torusDynTrans->setRotation(Quaternion(Vec3f(1,0,0), osgDegree2Rad(270) + 0.001f * speed));

	// Transform Earth
	ComponentTransformRecPtr earthDynTrans = dynamic_cast<ComponentTransform*>(earthTransNode->getCore());
	earthDynTrans->setRotation(Quaternion(Vec3f(0,1,0), osgDegree2Rad(30) + 0.0001f * speed));

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
	// transform the objects
	objectMotion();

	commitChanges();
	mgr->redraw();

	//the changelist should be cleared - else things
	//could be copied multiple times
	OSG::Thread::getCurrentChangeList()->clear();

	// to ensure a black navigation window
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
}

void idle(void)
{
	check_tracker();
	const auto speed = 1.f;
	mgr->setUserTransform(head_position, head_orientation);
	mgr->setTranslation(mgr->getTranslation() + speed * analog_values);
	
	// transform the objects
	objectMotion();

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
			scene = buildScene();
		commitChanges();

		mgr = new OSGCSM::CAVESceneManager(&cfg);
		mgr->setWindow(mwin );
		mgr->setRoot(scene);
		mgr->showAll();

		// ------------------------------------ BACKGROUND --------------------------------------- 

		ImageRecPtr backimage = Image::create();
		backimage->read("models/universe1.jpg");

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

