/*test*/

/**
 * \file main.cpp
 * \brief A simple scg3 example application.
 *
 * Requires C++11 and OpenGL 3.2 (or later versions).
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

/*
 * Copyright 2014-2019 Volker Ahlers
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <stdexcept>
#include <vector>
#include <scg3.h>

#include <ctime>
#include <functional>
#include <random>

using namespace scg;

/**
 * \brief Configuration parameters.
 */
struct SCGConfiguration {
	static const int viewerType = 1;  // 0: simple, 1: customized
	// for customized viewer:
	static const int sceneType = 1;   // 0: teapot, 1: table
};

/**
 * \brief Minimal application using a simple viewer with default renderer, shaders,
 *   camera, and light to create a teapot scene.
 */
void useSimpleViewer();

/**
 * \brief Typical application using a customized viewer to create a teapot or table scene.
 */
void useCustomizedViewer();

/**
 * \brief Create a scene consisting of a teapot, a camera, and a light.
 */
void createTeapotScene(ViewerSP viewer, CameraSP camera, GroupSP& scene);

/**
 * \brief Create a scene consisting of a floor, a table, a teapot, a camera, and a light.
 */
void createGearScene(ViewerSP viewer, CameraSP camera, GroupSP& scene);

/**
 * \brief The main function.
 */
int main() {

	int result = 0;

	try {
		if (SCGConfiguration::viewerType == 0) {
			useSimpleViewer();
		} else {
			useCustomizedViewer();
		}
	} catch (const std::exception& exc) {
		std::cerr << std::endl << "Exception: " << exc.what() << std::endl;
		result = 1;
	}
	return result;
}

// Minimal application using a simple viewer.
void useSimpleViewer() {

	// create viewer with default renderer, camera, and light
	auto viewer = Viewer::create();
	CameraSP camera;
	GroupSP scene;
	LightSP light;
	viewer->initSimpleRenderer(camera, scene, light);

	// define red material
	auto matRed = MaterialCore::create();
	matRed->setAmbientAndDiffuse(glm::vec4(1.f, 0.5f, 0.5f, 1.f))->setSpecular(
			glm::vec4(1.f, 1.f, 1.f, 1.f))->setShininess(80.f)->init();
	/*
	 // add a Test to scene graph
	 GeometryCoreFactory gF;
	 auto test = Shape::create();
	 test->addCore(matRed)
	 ->addCore(gF.createTest());
	 light->addChild(test);
	 */

	// add gear shape to scene graph
	GeometryCoreFactory geometryFactory;
	auto gear = Shape::create();
	gear->addCore(matRed)->addCore(
			geometryFactory.createGear(0.5, 0.42, 0.1, 14.0, 16.0)); //GeometryCoreSP GeometryCoreFactory::createGear(double l, double k, double z, double w1, double w2)
	light->addChild(gear);

	// move camera backwards, enter main loop
	camera->translate(glm::vec3(0.f, 0.f, 1.f))->dolly(-1.f);
	viewer->startMainLoop();
}

// Typical application using a customized viewer.
void useCustomizedViewer() {

	// create viewer and renderer
	auto viewer = Viewer::create();
	auto renderer = StandardRenderer::create();
	viewer->init(renderer)->createWindow("s c g 3   e x a m p l e", 1024, 768);

	// create camera
	auto camera = PerspectiveCamera::create();
	renderer->setCamera(camera);

	// create scene
	GroupSP scene;
	switch (SCGConfiguration::sceneType) {
	case 0:
		createTeapotScene(viewer, camera, scene);
		break;
	case 1:
		createGearScene(viewer, camera, scene);
		break;
	default:
		throw std::runtime_error(
				"Invalid value of SCGConfiguration::sceneType [main()]");
	}
	renderer->setScene(scene);

	// start animations, enter main loop
	viewer->startAnimations()->startMainLoop();
}

void createTeapotScene(ViewerSP viewer, CameraSP camera, GroupSP& scene) {

	ShaderCoreFactory shaderFactory("../scg3/shaders;../../scg3/shaders");

#ifdef SCG_CPP11_INITIALIZER_LISTS
	// Gouraud shader
	auto shaderGouraud = shaderFactory.createShaderFromSourceFiles( {
			ShaderFile("simple_gouraud_vert.glsl", GL_VERTEX_SHADER),
			ShaderFile("simple_gouraud_frag.glsl", GL_FRAGMENT_SHADER) });
#else
  std::vector<ShaderFile> shaderFiles;
  shaderFiles.push_back(ShaderFile("simple_gouraud_vert.glsl", GL_VERTEX_SHADER));
  shaderFiles.push_back(ShaderFile("simple_gouraud_frag.glsl", GL_FRAGMENT_SHADER));
  auto shaderGouraud = shaderFactory.createShaderFromSourceFiles(shaderFiles);
#endif

	// camera controllers
	camera->translate(glm::vec3(0.f, 0.f, 1.f))->dolly(-1.f);
#ifdef SCG_CPP11_INITIALIZER_LISTS
	viewer->addControllers( { KeyboardController::create(camera),
			MouseController::create(camera) });
#else
  viewer->addController(KeyboardController::create(camera))
      ->addController(MouseController::create(camera));
#endif

	// white point light at position (10,10,10)
	auto light = Light::create();
	light->setDiffuseAndSpecular(glm::vec4(1.f, 1.f, 1.f, 1.f))->setPosition(
			glm::vec4(10.f, 10.f, 10.f, 1.f))->init();

	// red material
	auto matRed = MaterialCore::create();
	matRed->setAmbientAndDiffuse(glm::vec4(1.f, 0.5f, 0.5f, 1.f))->setSpecular(
			glm::vec4(1.f, 1.f, 1.f, 1.f))->setShininess(20.f)->init();

	// teapot shape
	GeometryCoreFactory geometryFactory;
	auto teapotCore = geometryFactory.createTeapot(1.f);
	auto teapot = Shape::create();
	teapot->addCore(matRed)->addCore(teapotCore);

	// teapot transformation
	auto teapotTrans = Transformation::create();
	teapotTrans->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f));

	// create scene graph
	scene = Group::create();
	scene->addCore(shaderGouraud);
	scene->addChild(camera)->addChild(light);
	light->addChild(teapotTrans);
	teapotTrans->addChild(teapot);
}

void createTableScene(ViewerSP viewer, CameraSP camera, GroupSP& scene) {

	ShaderCoreFactory shaderFactory("../scg3/shaders;../../scg3/shaders");

#ifdef SCG_CPP11_INITIALIZER_LISTS
	// Phong shader
	auto shaderPhong = shaderFactory.createShaderFromSourceFiles(
			{ ShaderFile("phong_vert.glsl", GL_VERTEX_SHADER), ShaderFile(
					"phong_frag.glsl", GL_FRAGMENT_SHADER), ShaderFile(
					"blinn_phong_lighting.glsl", GL_FRAGMENT_SHADER),
					ShaderFile("texture_none.glsl", GL_FRAGMENT_SHADER) });

	// Phong shader with texture mapping
	auto shaderPhongTex =
			shaderFactory.createShaderFromSourceFiles(
					{ ShaderFile("phong_vert.glsl", GL_VERTEX_SHADER),
							ShaderFile("phong_frag.glsl", GL_FRAGMENT_SHADER),
							ShaderFile("blinn_phong_lighting.glsl",
									GL_FRAGMENT_SHADER), ShaderFile(
									"texture2d_modulate.glsl",
									GL_FRAGMENT_SHADER) });
#else
  // Phong shader
  std::vector<ShaderFile> shaderFiles;
  shaderFiles.push_back(ShaderFile("phong_vert.glsl", GL_VERTEX_SHADER));
  shaderFiles.push_back(ShaderFile("phong_frag.glsl", GL_FRAGMENT_SHADER));
  shaderFiles.push_back(ShaderFile("blinn_phong_lighting.glsl", GL_FRAGMENT_SHADER));
  shaderFiles.push_back(ShaderFile("texture_none.glsl", GL_FRAGMENT_SHADER));
  auto shaderPhong = shaderFactory.createShaderFromSourceFiles(shaderFiles);

  // Phong shader with texture mapping
  shaderFiles.clear();
  shaderFiles.push_back(ShaderFile("phong_vert.glsl", GL_VERTEX_SHADER));
  shaderFiles.push_back(ShaderFile("phong_frag.glsl", GL_FRAGMENT_SHADER));
  shaderFiles.push_back(ShaderFile("blinn_phong_lighting.glsl", GL_FRAGMENT_SHADER));
  shaderFiles.push_back(ShaderFile("texture2d_modulate.glsl", GL_FRAGMENT_SHADER));
  auto shaderPhongTex = shaderFactory.createShaderFromSourceFiles(shaderFiles);
#endif

	// camera controllers
	camera->translate(glm::vec3(0.f, 0.5f, 1.f))->dolly(-1.f);
#ifdef SCG_CPP11_INITIALIZER_LISTS
	viewer->addControllers( { KeyboardController::create(camera),
			MouseController::create(camera) });
#else
  viewer->addController(KeyboardController::create(camera))
        ->addController(MouseController::create(camera));
#endif

	// lights
	auto light = Light::create();
	light->setDiffuseAndSpecular(glm::vec4(1.f, 1.f, 1.f, 1.f))->setPosition(
			glm::vec4(10.f, 10.f, 10.f, 1.f))->init();

	auto light2 = Light::create();
	light2->setDiffuseAndSpecular(glm::vec4(1.f, 0.f, 0.f, 1.f))->setPosition(
			glm::vec4(-10.f, -3.f, 10.f, 1.f))->init();

	// materials
	auto matRed = MaterialCore::create();
	matRed->setAmbientAndDiffuse(glm::vec4(1.f, 0.5f, 0.5f, 1.f))->setSpecular(
			glm::vec4(1.f, 1.f, 1.f, 1.f))->setShininess(20.f)->init();

	auto matGreen = MaterialCore::create();
	matGreen->setAmbientAndDiffuse(glm::vec4(0.1f, 0.8f, 0.3f, 1.f))->init();

	auto matWhite = MaterialCore::create();
	matWhite->setAmbientAndDiffuse(glm::vec4(1.f, 1.f, 1.f, 1.f))->setSpecular(
			glm::vec4(0.5f, 0.5f, 0.5f, 1.f))->setShininess(20.f)->init();

	auto matBlue = MaterialCore::create();
	matBlue->setAmbientAndDiffuse(glm::vec4(0.5f, 0.5f, 1.f, 1.f))->setSpecular(
			glm::vec4(0.8f, 0.8f, 0.8f, 1.f))->setShininess(20.f)->init();

	auto matOrange = MaterialCore::create();
	matOrange->setAmbientAndDiffuse(glm::vec4(.8f, 0.6f, 0.0f, 1.f))->init();

	auto matGold = MaterialCore::create();
	matGold->setAmbient(glm::vec4(0.25f, 0.22f, 0.06f, 1.f))->setDiffuse(
			glm::vec4(0.35f, 0.31f, 0.09f, 1.f))->setSpecular(
			glm::vec4(0.80f, 0.72f, 0.21f, 1.f))->setShininess(13.2f)->init();

	auto matTuerkis = MaterialCore::create();
	matTuerkis->setAmbient(glm::vec4(0.10f, 0.19f, 0.17f, 0.8f))->setDiffuse(
			glm::vec4(0.40f, 0.74f, 0.69f, 0.8f))->setSpecular(
			glm::vec4(0.30f, 0.31f, 0.31f, 0.8f))->setShininess(12.8f)->init();

	// textures
	TextureCoreFactory textureFactory("../scg3/textures;../../scg3/textures");
	auto texWood = textureFactory.create2DTextureFromFile("wood_256.png",
			GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	// set texture matrix
	//  texWood->scale2D(glm::vec2(4.f, 4.f));

	// floor shape and transformation
	GeometryCoreFactory geometryFactory;
	auto floorCore = geometryFactory.createCuboid(glm::vec3(20.f, 0.05f, 10.f));
	auto floor = Shape::create();
	floor->addCore(matGreen)->addCore(floorCore);
	auto floorTrans = Transformation::create();
	floorTrans->translate(glm::vec3(0.f, -0.5f, 0.f));

	// teapot shape and transformation
	auto teapotCore = geometryFactory.createTeapot(0.35f);
	auto teapot = Shape::create();
	teapot->addCore(matRed)->addCore(teapotCore);
	auto teapotTrans = Transformation::create();
	teapotTrans->translate(glm::vec3(.5f, 0.9f, 0.f))->rotate(-90.f,
			glm::vec3(1.f, 0.f, 0.f));

	// table group and transformation
	auto table = Group::create();
	table->addCore(shaderPhongTex)->addCore(matWhite)->addCore(texWood);
	auto tableTrans = Transformation::create();
	tableTrans->rotate(30.f, glm::vec3(0.f, 1.f, 0.f));

	auto tableTop = Shape::create(
			geometryFactory.createCuboid(glm::vec3(1.5f, 0.05f, 1.f)));
	auto tableTopTrans = Transformation::create();
	tableTopTrans->translate(glm::vec3(0.f, 0.5f, 0.f));
	table->addChild(tableTopTrans);
	tableTopTrans->addChild(tableTop);

	auto tableLegCore = geometryFactory.createCuboid(
			glm::vec3(0.1f, 1.f, 0.1f));
	ShapeSP tableLeg[4];
	TransformationSP tableLegTrans[4];
	for (int i = 0; i < 4; ++i) {
		tableLeg[i] = Shape::create(tableLegCore);
		tableLegTrans[i] = Transformation::create();
		table->addChild(tableLegTrans[i]);
		tableLegTrans[i]->addChild(tableLeg[i]);
	}
	tableLegTrans[0]->translate(glm::vec3(0.6f, 0.f, 0.35f));
	tableLegTrans[1]->translate(glm::vec3(0.6f, 0.f, -0.35f));
	tableLegTrans[2]->translate(glm::vec3(-0.6f, 0.f, -0.35f));
	tableLegTrans[3]->translate(glm::vec3(-0.6f, 0.f, 0.35f));

	// Mersenne Twister seeded with current time
	std::mt19937 randEngine(
			static_cast<std::mt19937::result_type>(std::time(nullptr)));
	auto random = std::bind(std::uniform_real_distribution<float>(0.0f, 1.0f),
			randEngine);

	auto cubeCore = geometryFactory.createCuboid(glm::vec3(.3f, .5f, 0.3f));
	ShapeSP cubeLeg[4];
	TransformationSP cubeLegTrans[4];
	for (int i = 0; i < 4; ++i) {
		//random scaling
		float scaling = random();

		cubeLeg[i] = Shape::create();
		cubeLeg[i]->addCore(matBlue)->addCore(cubeCore);
		cubeLegTrans[i] = Transformation::create();
		cubeLegTrans[i]->scale(glm::vec3(scaling, scaling, scaling));
		// cubeLegTrans[i]->translate(glm::vec3(scaling, scaling, scaling));
		light->addChild(cubeLegTrans[i]);
		cubeLegTrans[i]->addChild(cubeLeg[i]);
	}
	cubeLegTrans[0]->translate(glm::vec3(1.6f, -0.4f, 0.35f));
	cubeLegTrans[1]->translate(glm::vec3(1.6f, -0.4f, -0.35f));
	cubeLegTrans[2]->translate(glm::vec3(-1.6f, -0.4f, -0.35f));
	cubeLegTrans[3]->translate(glm::vec3(-1.6f, -0.4f, 0.35f));

	auto deckeCore = geometryFactory.createCuboid(glm::vec3(20.f, 10.f, 0.3f));
	auto decke = Shape::create();
	decke->addCore(matOrange)->addCore(deckeCore);
	auto deckeTrans = Transformation::create();
	deckeTrans->translate(glm::vec3(0.0f, 4.5f, -5.f));

	auto modelCore = geometryFactory.createModelFromOBJFile(
			"../scg3/models/icosahedron.obj");
	auto model = Shape::create();
	model->addCore(matGold)->addCore(modelCore);
	auto modelTrans = Transformation::create();
	modelTrans->translate(glm::vec3(-3.0f, 4.0f, -4.f));

	auto sphereCore = geometryFactory.createSphere(1.0f, 50, 40);
	auto sphere = Shape::create();
	sphere->addCore(matTuerkis)->addCore(sphereCore);
	auto sphereTrans = Transformation::create();
	sphereTrans->translate(glm::vec3(3.0f, 2.5f, -3.f));

	//Zahnrad
	auto gearCore = geometryFactory.createGear(0.5, 0.42, 0.1, 14.0, 16.0); //GeometryCoreSP GeometryCoreFactory::createGear(double l, double k, double z, double w1, double w2)
	auto gear = Shape::create();
	gear->addCore(matRed)->addCore(gearCore);
	auto gearTrans = Transformation::create();
	gearTrans->translate(glm::vec3(2.0f, 2.0f, 3.f));

	// add animation (rotation)
	auto teapotAnim = TransformAnimation::create();
	float angularVel = 50.f; //Geschwindigkeit
	glm::vec3 axis(0.f, 0.f, 1.f); //Rotation um z-Achse
	teapotAnim->setUpdateFunc(
			[angularVel, axis](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(angularVel*static_cast<GLfloat>(diffTime), axis);
			});
	viewer->addAnimation(teapotAnim);
	// add transformation (translation) to be applied before animation
	auto teapotAnimTrans = Transformation::create();
	teapotAnimTrans->translate(glm::vec3(0.3f, 0.f, 0.f));

	auto modelAnim = TransformAnimation::create();
	float angularVel2 = 100.f; //Geschwindigkeit
	glm::vec3 axis2(0.f, 1.f, 0.f); //Rotation um y-Achse
	modelAnim->setUpdateFunc(
			[angularVel2, axis2](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(angularVel2*static_cast<GLfloat>(diffTime), axis2);
			});
	viewer->addAnimation(modelAnim);
	auto modelAnimTrans = Transformation::create();
	modelAnimTrans->translate(glm::vec3(0.3f, 0.f, 0.f));

	//Animation für Zahnrad
	auto gearAnim = TransformAnimation::create();
	float angularVel3 = 100.f; //Geschwindigkeit
	glm::vec3 axis3(0.f, 0.f, 1.f); //Rotation um z-Achse
	gearAnim->setUpdateFunc(
			[angularVel3, axis3](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(angularVel3*static_cast<GLfloat>(diffTime), axis3);
			});
	viewer->addAnimation(gearAnim);
	// add transformation (translation) to be applied before animation
	auto gearAnimTrans = Transformation::create();
	gearAnimTrans->translate(glm::vec3(0.0f, 0.f, 0.f));

	// create scene graph
	scene = Group::create();
	scene->addCore(shaderPhong);
	scene->addChild(camera)->addChild(light)->addChild(light2);

	//Light*lichtPtr = light.get();

	light->addChild(floorTrans)->addChild(tableTrans)->addChild(deckeTrans)->addChild(
			sphereTrans)->addChild(modelTrans)->addChild(gearTrans);
	//light2 ->addChild(tableTrans);
	floorTrans->addChild(floor); //http://org.eclipse.ui.intro/showPage?id=samples&standby=false
	deckeTrans->addChild(decke);
	tableTrans->addChild(table)->addChild(teapotTrans);
	//		->addChild(cubeLegTrans);
	//cubeLegTrans->addChild(cubeLeg);
	teapotTrans->addChild(teapot);
	modelTrans->addChild(model);
	sphereTrans->addChild(sphere);
	//gearTrans->addChild(gear);

	gearTrans->addChild(gearAnim);
	gearAnim->addChild(gearAnimTrans);
	gearAnimTrans->addChild(gear);
	/*teapotTrans->addChild(teapotAnim);
	 teapotAnim->addChild(teapotAnimTrans);
	 teapotAnimTrans->addChild(teapot);

	 modelTrans->addChild(modelAnim);
	 modelAnim->addChild(modelAnimTrans);
	 modelAnimTrans->addChild(model);*/
}

void createGearScene(ViewerSP viewer, CameraSP camera, GroupSP& scene) {

	ShaderCoreFactory shaderFactory("../scg3/shaders;../../scg3/shaders");

#ifdef SCG_CPP11_INITIALIZER_LISTS
	// Phong shader
	auto shaderPhong = shaderFactory.createShaderFromSourceFiles(
			{ ShaderFile("phong_vert.glsl", GL_VERTEX_SHADER), ShaderFile(
					"phong_frag.glsl", GL_FRAGMENT_SHADER), ShaderFile(
					"blinn_phong_lighting.glsl", GL_FRAGMENT_SHADER),
					ShaderFile("texture_none.glsl", GL_FRAGMENT_SHADER) });

	// Phong shader with texture mapping
	auto shaderPhongTex =
			shaderFactory.createShaderFromSourceFiles(
					{ ShaderFile("phong_vert.glsl", GL_VERTEX_SHADER),
							ShaderFile("phong_frag.glsl", GL_FRAGMENT_SHADER),
							ShaderFile("blinn_phong_lighting.glsl",
									GL_FRAGMENT_SHADER), ShaderFile(
									"texture2d_modulate.glsl",
									GL_FRAGMENT_SHADER) });
#else
  // Phong shader
  std::vector<ShaderFile> shaderFiles;
  shaderFiles.push_back(ShaderFile("phong_vert.glsl", GL_VERTEX_SHADER));
  shaderFiles.push_back(ShaderFile("phong_frag.glsl", GL_FRAGMENT_SHADER));
  shaderFiles.push_back(ShaderFile("blinn_phong_lighting.glsl", GL_FRAGMENT_SHADER));
  shaderFiles.push_back(ShaderFile("texture_none.glsl", GL_FRAGMENT_SHADER));
  auto shaderPhong = shaderFactory.createShaderFromSourceFiles(shaderFiles);

  // Phong shader with texture mapping
  shaderFiles.clear();
  shaderFiles.push_back(ShaderFile("phong_vert.glsl", GL_VERTEX_SHADER));
  shaderFiles.push_back(ShaderFile("phong_frag.glsl", GL_FRAGMENT_SHADER));
  shaderFiles.push_back(ShaderFile("blinn_phong_lighting.glsl", GL_FRAGMENT_SHADER));
  shaderFiles.push_back(ShaderFile("texture2d_modulate.glsl", GL_FRAGMENT_SHADER));
  auto shaderPhongTex = shaderFactory.createShaderFromSourceFiles(shaderFiles);
#endif

	// camera controllers
	camera->translate(glm::vec3(0.f, 0.5f, 1.f))
	//->rotateElevationRad(1.0f)	//um horizontale Achse durch den Fokus
	//->rotateElevation(12.0f)
	//->rotateAzimuthRad(1.0f)		//um vertikale Achse durch den Fokus
	//->rotateAzimuth(12.0f)
	//->rotateRollRad(2.0f)			//Rotation um die optische Achse
	//->rotateRoll(2.0f)
	//->rotatePitchRad(-1.0f) 		//um horizintale Kameraachse
	//->rotateYawRad(1.0f)			//um vertikale Kameraachse
	//->rotate(2.0f, glm::vec3(0.f, 0.f, 1.f))

	->dolly(-2.5f);	//Abstand zum Fokus(Bewegungen der Kamera in Blickrichtung)

	glm::vec3 _eye(0, 0, 10), _center(2, 0, 0), _up(0, 1, 0);
	glm::mat4 modelViewMatrix = glm::lookAt(_eye, _center, _up);
	//glm::mat4 projectionMatrix = glm::perspective(glm::radians(45), 6./8., 0.1, 200.);

#ifdef SCG_CPP11_INITIALIZER_LISTS
	viewer->addControllers( { KeyboardController::create(camera),
			MouseController::create(camera) });
#else
  viewer->addController(KeyboardController::create(camera))
        ->addController(MouseController::create(camera));
#endif

	// lights
	auto light = Light::create();
	light->setDiffuseAndSpecular(glm::vec4(1.f, 1.f, 1.f, 1.f))->setPosition(
			glm::vec4(5.f, 4.f, 3.f, 1.f))->init();

	auto frameLight = Light::create();
	frameLight->setDiffuseAndSpecular(glm::vec4(1.f, 1.f, 1.f, 1.f))->setPosition(
			glm::vec4(-6.0f, 8.0f, -4.8f, 1.f))->init();

	auto lampLight = Light::create();
	lampLight->setDiffuseAndSpecular(glm::vec4(1.f, 1.f, 1.f, 1.f))->setPosition(
			glm::vec4(4.6f, 5.95f, -.9f, 1.f))->init();


	auto light2 = Light::create();
	light2->setDiffuseAndSpecular(glm::vec4(1.f, 0.f, 0.f, 1.f))
	//->setSpot(glm::vec3(1.f, 0.f, 0.f), 0.5f, 0.5f)
	->setPosition(glm::vec4(-5.f, 4.f, 3.f, 1.f))->init();

	// materials
	auto matRed = MaterialCore::create();
	matRed->setAmbientAndDiffuse(glm::vec4(1.f, 0.5f, 0.5f, 1.f))->setSpecular(
			glm::vec4(1.f, 1.f, 1.f, 1.f))->setShininess(20.f)->init();

	auto matYellow = MaterialCore::create();
	matYellow->setAmbientAndDiffuse(glm::vec4(.8f, 0.4f, 0.12f, 1.f))->setSpecular(
			glm::vec4(1.f, 1.f, 1.f, 1.f))->setShininess(20.f)->init();

	auto matGreen = MaterialCore::create();
	matGreen->setAmbientAndDiffuse(glm::vec4(0.1f, 0.8f, 0.3f, 1.f))->init();

	auto matWhite = MaterialCore::create();
	matWhite->setAmbientAndDiffuse(glm::vec4(1.f, 1.f, 1.f, 1.f))->setSpecular(
			glm::vec4(0.5f, 0.5f, 0.5f, 1.f))->setShininess(20.f)->init();

	auto matBlue = MaterialCore::create();
	matBlue->setAmbientAndDiffuse(glm::vec4(0.5f, 0.5f, 1.f, 1.f))->setSpecular(
			glm::vec4(0.8f, 0.8f, 0.8f, 1.f))->setShininess(20.f)->init();

	auto matGrey = MaterialCore::create();
	matGrey->setAmbientAndDiffuse(glm::vec4(.5f, 0.5f, 0.5f, 1.f))->init();

	auto matDarkGrey = MaterialCore::create();
		matDarkGrey->setAmbientAndDiffuse(glm::vec4(0.25f, 0.25f, 0.25f, 1.f))->init();

	auto matGold = MaterialCore::create();
	matGold->setAmbient(glm::vec4(0.25f, 0.22f, 0.06f, 1.f))->setDiffuse(
			glm::vec4(0.35f, 0.31f, 0.09f, 1.f))->setSpecular(
			glm::vec4(0.80f, 0.72f, 0.21f, 1.f))->setShininess(13.2f)->init();

	auto matTuerkis = MaterialCore::create();
	matTuerkis->setAmbient(glm::vec4(0.10f, 0.19f, 0.17f, 0.8f))->setDiffuse(
			glm::vec4(0.40f, 0.74f, 0.69f, 0.8f))->setSpecular(
			glm::vec4(0.30f, 0.31f, 0.31f, 0.8f))->setShininess(12.8f)->init();

	// textures
	TextureCoreFactory textureFactory("../scg3/textures;../../scg3/textures");
	// set texture matrix
	//texWood->scale2D(glm::vec2(4.f, 4.f));
	//TextureCoreFactory textureFactory2("../scg3/textures;../../scg3/textures");
	auto texCem1 = textureFactory.create2DTextureFromFile("cement1.jpg",
			GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	//TextureCoreFactory textureFactory3("../scg3/textures;../../scg3/textures");
	auto texCem2 = textureFactory.create2DTextureFromFile("cement2.jpg",
			GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	auto texCem3 = textureFactory.create2DTextureFromFile("cement3.jpg",
			GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	auto texCeiling = textureFactory.create2DTextureFromFile("ceiling.png",
			GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// floor shape and transformation
	GeometryCoreFactory geometryFactory;
	auto floorCore = geometryFactory.createCuboid(glm::vec3(15.f, 0.05f, 10.f));
	auto floor = Shape::create();
	floor->addCore(shaderPhongTex)->addCore(matGrey)->addCore(texCem3)->addCore(
			floorCore);
	auto floorTrans = Transformation::create();
	floorTrans->translate(glm::vec3(0.f, -0.5f, 0.f));

	auto deckeCore = geometryFactory.createCuboid(glm::vec3(15.f, 0.05f, 10.f));
	auto decke = Shape::create();
	decke->addCore(shaderPhongTex)->addCore(matGrey)->addCore(texCeiling)->addCore(
			deckeCore);
	auto deckeTrans = Transformation::create();
	deckeTrans->translate(glm::vec3(0.f, 9.5f, 0.f));
	// ->rotate(-180.f, glm::vec3(1.f, 0.f, 0.f));

	//4 Wände im Raum: linke, hintere, rechte, vordere
	auto wandLinksCore = geometryFactory.createCuboid(
			glm::vec3(10.f, 0.05f, 10.0f));
	auto wandLinks = Shape::create();
	wandLinks->addCore(shaderPhongTex)->addCore(matGrey)->addCore(texCem2)->addCore(
			wandLinksCore);
	auto wandLinksTrans = Transformation::create();
	wandLinksTrans->translate(glm::vec3(-7.5f, 4.5f, 0.f))->rotate(-90.f,
			glm::vec3(0.f, 0.f, 1.f));

	auto wandHintenCore = geometryFactory.createCuboid(
			glm::vec3(15.f, 10.f, 0.3f));
	auto wandHinten = Shape::create();
	wandHinten->addCore(shaderPhongTex)->addCore(matGrey)->addCore(texCem1)->addCore(
			wandHintenCore);
	auto wandHintenTrans = Transformation::create();
	wandHintenTrans->translate(glm::vec3(0.0f, 4.5f, -5.f));

	auto wandRechtsCore = geometryFactory.createCuboid(
			glm::vec3(10.f, 0.05f, 10.0f));
	auto wandRechts = Shape::create();
	wandRechts->addCore(shaderPhongTex)->addCore(matGrey)->addCore(texCem2)->addCore(
			wandRechtsCore);
	auto wandRechtsTrans = Transformation::create();
	wandRechtsTrans->translate(glm::vec3(7.5f, 4.5f, 0.f))->rotate(-90.f,
			glm::vec3(0.f, 0.f, 1.f));

	auto wandVorneCore = geometryFactory.createCuboid(
			glm::vec3(15.f, 10.f, 0.3f));
	auto wandVorne = Shape::create();
	wandVorne->addCore(shaderPhongTex)->addCore(matGrey)->addCore(texCem1)->addCore(
			wandVorneCore);
	auto wandVorneTrans = Transformation::create();
	wandVorneTrans->translate(glm::vec3(0.0f, 4.5f, 5.f));

	/*    auto table = Group::create();
	 table->addCore(shaderPhongTex)
	 ->addCore(matWhite)
	 ->addCore(texWood);
	 auto tableTrans = Transformation::create();
	 tableTrans->rotate(30.f, glm::vec3(0.f, 1.f, 0.f));
	 *
	 */
	// Create Frame
	auto frameCore = geometryFactory.createFrame();
	auto frame = Shape::create();
	frame->addCore(matGrey)->addCore(frameCore);
	auto frameTrans = Transformation::create();
	frameTrans->translate(glm::vec3(-6.0f, 8.0f, -4.85f));
	frameTrans->scale(glm::vec3(2.f, 2.f, 2.f));

	auto frameBGCore = geometryFactory.createCuboid(
			glm::vec3(1.3f, 0.7f, .01f));
	auto frameBG = Shape::create();
	frameBG->addCore(matBlue)->addCore(frameBGCore);
	auto frameBGTrans = Transformation::create();
	frameBGTrans->translate(glm::vec3(-4.9f, 7.3f, -4.85f));
	frameBGTrans->scale(glm::vec3(2.f, 2.f, 2.f));



	//Create Broccoli Wall-Art
	auto broccoliCore = geometryFactory.createBroccoli();
	auto broccoli = Shape::create();
	broccoli->addCore(matGreen)->addCore(broccoliCore);
	auto broccoliTrans = Transformation::create();
	broccoliTrans->translate(glm::vec3(6.0f, 1.0f, -4.85f));
	broccoliTrans->scale(glm::vec3(.5f, .5f, .5f));
	broccoliTrans->rotate(-14.f, glm::vec3(0.f, 0.f, 1.f));

	//Create Lamp
	auto lampCore = geometryFactory.createLamp();
	auto lamp = Shape::create();
	lamp->addCore(matDarkGrey)->addCore(lampCore);
	auto lampTrans = Transformation::create();
	lampTrans->translate(glm::vec3(5.0f, 9.5f, 1.85f));
	lampTrans->scale(glm::vec3(1.f, 1.f, 1.f));
	lampTrans->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f));

	//Create Bulb
	auto bulbCore = geometryFactory.createBulb();
	auto bulb = Shape::create();
	bulb->addCore(matYellow)->addCore(bulbCore);
	auto bulbTrans = Transformation::create();
	bulbTrans->translate(glm::vec3(4.6f, 5.95f, -.9f));
	bulbTrans->scale(glm::vec3(1.3f, 1.3f, 1.3f));
	bulbTrans->rotate(90.f, glm::vec3(1.f, 0.f, 0.f));

	// Creat Zahnrad
	auto gearCore = geometryFactory.createGear(0.5, 0.42, 0.1, 14.0, 16.0); //GeometryCoreSP GeometryCoreFactory::createGear(double l, double k, double z, double w1, double w2)
	auto gear = Shape::create();
	gear->addCore(matRed)->addCore(gearCore);
	auto gearTrans = Transformation::create();
	gearTrans->translate(glm::vec3(0.0f, 0.75f, 0.f));
	//->rotate(-180.f, glm::vec3(0.f, 1.f, 0.f));

	//Animation für Zahnrad- Rotation
	auto gearAnim = TransformAnimation::create();
	float angularVel3 = 60.f; //Geschwindigkeit
	glm::vec3 axis3(0.f, 0.f, 1.f); //Rotation um z-Achse
	gearAnim->setUpdateFunc(
			[angularVel3, axis3](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(angularVel3*static_cast<GLfloat>(diffTime), axis3);
			});
	viewer->addAnimation(gearAnim);
	// add transformation (translation) to be applied before animation
	auto gearAnimTrans = Transformation::create();
	gearAnimTrans->translate(glm::vec3(0.0f, 0.f, 0.f));

	// Create Zahnrad 2
	auto gear2Core = geometryFactory.createGear(0.5, 0.42, 0.1, 14.0, 16.0); //GeometryCoreSP GeometryCoreFactory::createGear(double l, double k, double z, double w1, double w2)
	auto gear2 = Shape::create();
	gear2->addCore(matGold)->addCore(gear2Core);
	auto gear2Trans = Transformation::create();
	gear2Trans->translate(glm::vec3(0.93f, 0.75f, 0.f));
	//->rotate(-10.f, glm::vec3(0.f, 0.f, 1.f));

	//Animation für 2-Zahnrad- Rotation
	auto gear2Anim = TransformAnimation::create();
	float angularVel4 = 60.f; //Geschwindigkeit
	glm::vec3 axis4(0.f, 0.f, 1.f); //Rotation um z-Achse
	gear2Anim->setUpdateFunc(
			[angularVel4, axis4](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(-angularVel4*static_cast<GLfloat>(diffTime), axis4);
			});
	viewer->addAnimation(gear2Anim);
	// add transformation (translation) to be applied before animation
	auto gear2AnimTrans = Transformation::create();
	gear2AnimTrans->translate(glm::vec3(0.0f, 0.f, 0.f));

/*	//Create Sonne
	auto sonneCore = geometryFactory.createSphere(0.6f, 150, 140);
	auto sonne = Shape::create();
	sonne->addCore(matTuerkis)->addCore(sonneCore);
	auto sonneTrans = Transformation::create();
	sonneTrans->translate(glm::vec3(3.6f, 2.3f, -1.f));
*/
	// create scene graph
	scene = Group::create();
	scene->addCore(shaderPhong);
	scene->addChild(camera)->addChild(light)->addChild(light2)->addChild(
			frameLight);
	frameLight->addChild(frameBGTrans);
	lampLight->addChild(bulbTrans);
	light->addChild(floorTrans)->addChild(deckeTrans)->addChild(wandLinksTrans)->addChild(
			wandHintenTrans)->addChild(wandRechtsTrans)->addChild(
			wandVorneTrans)->addChild(gearTrans)->addChild(gear2Trans)->addChild(frameBGTrans)->addChild(frameTrans)
			->addChild(broccoliTrans)->addChild(lampTrans)->addChild(bulbTrans);
	//light2 ->addChild(gearTrans);
	floorTrans->addChild(floor);
	deckeTrans->addChild(decke);
	wandLinksTrans->addChild(wandLinks);
	wandHintenTrans->addChild(wandHinten);
	wandRechtsTrans->addChild(wandRechts);
	wandVorneTrans->addChild(wandVorne);
	//gearTrans->addChild(gear);
	//gear2Trans->addChild(gear2);
	frameTrans->addChild(frame);
	frameBGTrans->addChild(frameBG);
	broccoliTrans->addChild(broccoli);
	//sonneTrans->addChild(sonne);
	lampTrans->addChild(lamp);
	bulbTrans->addChild(bulb);

	gearTrans->addChild(gearAnim);
	gearAnim->addChild(gearAnimTrans);
	gearAnimTrans->addChild(gear);

	gear2Trans->addChild(gear2Anim);
	gear2Anim->addChild(gear2AnimTrans);
	gear2AnimTrans->addChild(gear2);
}
