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
 * \brief Typical application using a customized viewer to create a teapot or table scene.
 */
void customViewer();

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
		customViewer();
	} catch (const std::exception& exc) {
		std::cerr << std::endl << "Exception: " << exc.what() << std::endl;
		result = 1;
	}
	return result;
}


// Typical application using a customized viewer.
void customViewer() {

	// create viewer and renderer
	auto viewer = Viewer::create();
	auto renderer = StandardRenderer::create();
	viewer->init(renderer)->createWindow("s c g 3   e x a m p l e", 1024, 768);

	// create camera
	auto camera = PerspectiveCamera::create();
	renderer->setCamera(camera);

	// create scene
	GroupSP scene;
	createGearScene(viewer, camera, scene);

	renderer->setScene(scene);

	// start animations, enter main loop
	viewer->startAnimations()->startMainLoop();
}



/*
 *
 *
 *
 *     create Gear Scene
 *
 *
 *
 *
 */

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
	//glm::mat4 modelViewMatrix = glm::lookAt(_eye, _center, _up);
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

	/*
	 *
	 *  Materialien
	 *
	 */
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
	/*
	 *
	 *  Texturen
	 *
	 */
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
/*
 *
 *    Wände und Decken
 *
 */
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
	decke->addCore(shaderPhongTex)->addCore(matGrey)->addCore(
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

	/*
	 *
	 *
	 *  Clock Model
	 *
	 *
	 */
	auto clockAxisCore = geometryFactory.createModelFromOBJFile("obj/clock/06_achse.obj");
	auto clockGear1Core = geometryFactory.createModelFromOBJFile("obj/clock/00_Zahnrad_1.obj");
	auto clockGear2Core = geometryFactory.createModelFromOBJFile("obj/clock/01_Zahnrad_2.obj");
	auto clockGear3Core = geometryFactory.createModelFromOBJFile("obj/clock/02_Zahnrad_3.obj");
	auto clockGear4Core = geometryFactory.createModelFromOBJFile("obj/clock/03_Zahnrad_4.obj");
	auto clockHandSmallCore = geometryFactory.createModelFromOBJFile("obj/clock/04_Zeiger_klein.obj");
	auto clockHandBigCore = geometryFactory.createModelFromOBJFile("obj/clock/05_Zeiger_gross.obj");
	auto clockAxis2ndCore = geometryFactory.createModelFromOBJFile("obj/clock/07_achse_2.obj");
	auto clockGear2ndCore = geometryFactory.createModelFromOBJFile("obj/clock/08_zahnrad_abseits.obj");


	auto clockAxis = Shape::create(); clockAxis->addCore(matGold)->addCore(clockAxisCore);
	auto clockGear1 = Shape::create(); clockGear1->addCore(matGold)->addCore(clockGear1Core);
	auto clockGear2 = Shape::create(); clockGear2->addCore(matGold)->addCore(clockGear2Core);
	auto clockGear3 = Shape::create(); clockGear3->addCore(matGold)->addCore(clockGear3Core);
	auto clockGear4 = Shape::create(); clockGear4->addCore(matGold)->addCore(clockGear4Core);
	auto clockHandSmall = Shape::create(); clockHandSmall->addCore(matGold)->addCore(clockHandSmallCore);
	auto clockHandBig = Shape::create(); clockHandBig->addCore(matGold)->addCore(clockHandBigCore);
	auto clockAxis2nd = Shape::create(); clockAxis2nd->addCore(matGold)->addCore(clockAxis2ndCore);
	auto clockGear2nd = Shape::create(); clockGear2nd->addCore(matGold)->addCore(clockGear2ndCore);


	auto clockAxisTrans = Transformation::create(); clockAxisTrans->translate(glm::vec3(0.0f, 3.5f, -4.85f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	auto clockGear1Trans = Transformation::create(); clockGear1Trans->translate(glm::vec3(0.0f, 3.5f, -4.7f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	auto clockGear2Trans = Transformation::create(); clockGear2Trans->translate(glm::vec3(0.0f, 3.5f, -4.5f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	auto clockGear3Trans = Transformation::create(); clockGear3Trans->translate(glm::vec3(0.0f, 3.5f, -4.25f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	auto clockHandSmallTrans = Transformation::create(); clockHandSmallTrans->translate(glm::vec3(0.0f, 3.5f, -4.1f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->rotate(-70.f, glm::vec3(0.f, 1.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	auto clockGear4Trans = Transformation::create(); clockGear4Trans->translate(glm::vec3(0.0f, 3.5f, -4.f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	auto clockHandBigTrans = Transformation::create(); clockHandBigTrans->translate(glm::vec3(0.0f, 3.5f, -3.9f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	auto clockAxis2ndTrans = Transformation::create(); clockAxis2ndTrans->translate(glm::vec3(-2.125f, 3.5f, -4.85f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	auto clockGear2ndTrans = Transformation::create(); clockGear2ndTrans->translate(glm::vec3(-2.125f, 3.5f, -4.47f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));

	/*
	 *
	 */
	auto clockGearAnim = TransformAnimation::create();
	float angularVel5 = 3.f; //Geschwindigkeit
	glm::vec3 axis5(0.f, 1.f, 0.f); //Rotation um z-Achse
	clockGearAnim->setUpdateFunc(
			[angularVel5, axis5](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(angularVel5*static_cast<GLfloat>(diffTime), axis5);});
	viewer->addAnimation(clockGearAnim);

	auto clockGearAnimTrans = Transformation::create(); clockGearAnimTrans->translate(glm::vec3(0.0f, 0.f, 0.f));
	/*
	 *
	 */
	auto clockGear2ndAnim = TransformAnimation::create();
	float angularVel6 = 3.f; //Geschwindigkeit
	glm::vec3 axis6(0.f, 1.f, 0.f); //Rotation um z-Achse
	clockGear2ndAnim->setUpdateFunc(
			[angularVel6, axis6](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(-angularVel6*static_cast<GLfloat>(diffTime), axis6);
			});
	viewer->addAnimation(clockGear2ndAnim);
	auto clockGear2ndAnimTrans = Transformation::create(); clockGear2ndAnimTrans->translate(glm::vec3(0.0f, 0.f, 0.f));
	/*
	 *
	 */






	auto spotCore = geometryFactory.createModelFromOBJFile("obj/spot.obj");
	auto spot = Shape::create();
	spot->addCore(matGrey)->addCore(spotCore);
	auto spotTrans = Transformation::create();
/*	spotTrans->translate(glm::vec3(0.0f, 0.2f, -4.58f));
	spotTrans->rotate(-135.0f, glm::vec3(0.f, 1.f, 0.f));
	spotTrans->rotate(180.0f, glm::vec3(1.f, 0.f, 1.f));
	spotTrans->rotate(180.0f, glm::vec3(0.f, 1.f, 0.f));
*/	spotTrans->scale(glm::vec3(0.2f, 0.2f, 0.2f));

	auto stabCore = geometryFactory.createModelFromOBJFile("obj/gear_center.obj");
	auto stab = Shape::create();
	stab->addCore(matGold)->addCore(stabCore);
	auto stabTrans = Transformation::create();
	stabTrans->translate(glm::vec3(-0.45f, 0.75f, -4.55f));
	stabTrans->rotate(90.f, glm::vec3(1.f, 0.f, 0.f));


	auto gearCore = geometryFactory.createModelFromOBJFile("obj/gear.obj");
	auto gear = Shape::create();
	gear->addCore(matRed)->addCore(gearCore);
	auto gearTrans = Transformation::create();
	gearTrans->translate(glm::vec3(-0.45f, 0.75f, -4.7f));
	gearTrans->rotate(90.f, glm::vec3(1.f, 0.f, 0.f));

	//Animation für Zahnrad- Rotation
	auto gearAnim = TransformAnimation::create();
	float angularVel3 = 60.f; //Geschwindigkeit
	glm::vec3 axis3(0.f, 1.f, 0.f); //Rotation um z-Achse
	gearAnim->setUpdateFunc(
			[angularVel3, axis3](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(angularVel3*static_cast<GLfloat>(diffTime), axis3);
			});
	viewer->addAnimation(gearAnim);
	// add transformation (translation) to be applied before animation
	auto gearAnimTrans = Transformation::create();
	gearAnimTrans->translate(glm::vec3(0.0f, 0.f, 0.f));




	auto stab2Core = geometryFactory.createModelFromOBJFile("obj/gear_center.obj");
	auto stab2 = Shape::create();
	stab2->addCore(matRed)->addCore(stab2Core);
	auto stab2Trans = Transformation::create();
	stab2Trans->translate(glm::vec3(0.93f, 0.75f, -4.55f));
	stab2Trans->rotate(90.f, glm::vec3(1.f, 0.f, 0.f));


	// Create Zahnrad 2
	auto gear2Core = geometryFactory.createModelFromOBJFile("obj/gear.obj");
	auto gear2 = Shape::create();
	gear2->addCore(matGold)->addCore(gear2Core);
	auto gear2Trans = Transformation::create();
	gear2Trans->translate(glm::vec3(0.93f, 0.75f, -4.7f));
	gear2Trans->rotate(90.f, glm::vec3(1.f, 0.f, 0.f));
	//->rotate(-10.f, glm::vec3(0.f, 0.f, 1.f));

	//Animation für 2-Zahnrad- Rotation
	auto gear2Anim = TransformAnimation::create();
	float angularVel4 = 60.f; //Geschwindigkeit
	glm::vec3 axis4(0.f, 1.f, 0.f); //Rotation um z-Achse
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
	light->addChild(floorTrans)->addChild(deckeTrans)->addChild(wandLinksTrans)->addChild(
			wandHintenTrans)->addChild(wandRechtsTrans)->addChild(
			wandVorneTrans)->addChild(gearTrans)->addChild(gear2Trans)->addChild(stabTrans)->addChild(stab2Trans)->addChild(clockAxisTrans)
			->addChild(clockGear1Trans)->addChild(clockGear2Trans)->addChild(clockGear3Trans)->addChild(clockHandSmallTrans)
			->addChild(clockGear4Trans)->addChild(clockHandBigTrans)->addChild(clockGear2ndTrans)->addChild(clockAxis2ndTrans);

	//light2 ->addChild(gearTrans);
	floorTrans->addChild(floor);
	deckeTrans->addChild(decke);
	wandLinksTrans->addChild(wandLinks);
	wandHintenTrans->addChild(wandHinten);
	wandRechtsTrans->addChild(wandRechts);
	wandVorneTrans->addChild(wandVorne);
	//gearTrans->addChild(gear);
	//gear2Trans->addChild(gear2);

	stabTrans->addChild(stab);
	stab2Trans->addChild(stab2);


	clockAxisTrans->addChild(clockAxis);
	clockGear1Trans->addChild(clockGear1);
	// clockGear2Trans->addChild(clockGear2);
	clockGear3Trans->addChild(clockGear3);
	clockHandSmallTrans->addChild(clockHandSmall);
	clockGear4Trans->addChild(clockGear4);
	clockHandBigTrans->addChild(clockHandBig);
	// clockGear2ndTrans->addChild(clockGear2nd);
	clockAxis2ndTrans->addChild(clockAxis2nd);


	clockGear2ndTrans->addChild(clockGear2ndAnim);
	clockGear2ndAnim->addChild(clockGear2ndAnimTrans);
	clockGear2ndAnimTrans->addChild(clockGear2nd);

	clockGear2Trans->addChild(clockGearAnim);
	clockGearAnim->addChild(clockGearAnimTrans);
	clockGearAnimTrans->addChild(clockGear2);

	gearTrans->addChild(gearAnim);
	gearAnim->addChild(gearAnimTrans);
	gearAnimTrans->addChild(gear);
	gear2Trans->addChild(gear2Anim);
	gear2Anim->addChild(gear2AnimTrans);
	gear2AnimTrans->addChild(gear2);
}
