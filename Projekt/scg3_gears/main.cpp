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
#include <thread>

using namespace scg;

/**
 * \brief Typical application using a customized viewer to create a teapot or table scene.
 */
void customViewer();
void keyBoardFunction(ViewerSP viewer);

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

void keyBoardFunction(ViewerSP viewer){

	GLFWwindow* fenster = viewer->getWindow();
	while(!glfwWindowShouldClose(fenster)){
		 if(glfwGetKey(fenster, GLFW_KEY_T) == GLFW_PRESS){
			 while(glfwGetKey(fenster, GLFW_KEY_T) == GLFW_PRESS){
				 //wait
			 }
		  std::cout << "Test Taste" << std::endl;
		}
	}
}


// Typical application using a customized viewer.
void customViewer() {

	// create viewer and renderer
	auto viewer = Viewer::create();
	auto renderer = StandardRenderer::create();
	viewer->init(renderer)->createWindow("s c g 3   e x a m p l e", 1024, 768);
	std::thread keyBoard(keyBoardFunction, viewer);
	// create camera
	auto camera = PerspectiveCamera::create();
	renderer->setCamera(camera);

	// create scene
	GroupSP scene;
	createGearScene(viewer, camera, scene);

	renderer->setScene(scene);

	// start animations, enter main loop
	viewer->startAnimations()->startMainLoop();
	keyBoard.join();
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


	// camera controllers
	camera->translate(glm::vec3(0.f, 0.5f, 1.f))
	//->rotateElevationRad(1.0f)	//um horizontale Achse durch den Fokus
	->rotateElevation(-3.5f)
	//->rotateAzimuthRad(1.0f);		//um vertikale Achse durch den Fokus
	//->rotateAzimuth(12.0f);
	//->rotateRollRad(2.0f)			//Rotation um die optische Achse
	//->rotateRoll(2.0f)
	//->rotatePitchRad(-1.0f) 		//um horizintale Kameraachse
	//->rotateYawRad(1.0f);			//um vertikale Kameraachse
	//->rotate(2.0f, glm::vec3(0.f, 0.f, 1.f))

	->dolly(-0.2f);	//Abstand zum Fokus(Bewegungen der Kamera in Blickrichtung)

	glm::vec3 _eye(0, 0, 10), _center(2, 0, 0), _up(0, 1, 0);

#ifdef SCG_CPP11_INITIALIZER_LISTS
	viewer->addControllers( { KeyboardController::create(camera),
			MouseController::create(camera) });
#else
  viewer->addController(KeyboardController::create(camera))
        ->addController(MouseController::create(camera));
#endif

	// lights
	auto light = Light::create();
	light->setDiffuseAndSpecular(glm::vec4(1.f, 1.f, 1.f, 1.f))
			->setPosition(glm::vec4(1.f, 7.2f, -1.f, 1.f))

		->init();

	auto light2 = Light::create();
	light2->setDiffuseAndSpecular(glm::vec4(1.f, 1.f, 1.f, 1.f))
					->setPosition(glm::vec4(0.f, 4.5f, -3.f, 1.f))
				->setSpot(glm::vec3(0.f, -0.8f, -1.f), 45.f, 1.f)
				->init();


	/*
	 *
	 *  Materialien
	 *
	 */
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

	auto matGrey = MaterialCore::create();
	matGrey->setAmbientAndDiffuse(glm::vec4(.5f, 0.5f, 0.5f, 1.f))->init();

	auto matDarkGrey = MaterialCore::create();
		matDarkGrey->setAmbientAndDiffuse(glm::vec4(0.25f, 0.25f, 0.25f, 1.f))->init();

	auto matGold = MaterialCore::create();
	matGold->setAmbient(glm::vec4(0.25f, 0.22f, 0.06f, 1.f))->setDiffuse(
			glm::vec4(0.35f, 0.31f, 0.09f, 1.f))->setSpecular(
			glm::vec4(0.80f, 0.72f, 0.21f, 1.f))->setShininess(13.2f)->init();

	auto matMessing = MaterialCore::create();
	matMessing->setAmbient(glm::vec4(0.33f, 0.22f, 0.03f, 1.0f))->setDiffuse(
			glm::vec4(0.78f, 0.57f, 0.11f, 1.0f))->setSpecular(
			glm::vec4(0.99f, 0.94f, 0.81f, 1.0f))->setShininess(27.9f)->init();

	auto matSilber = MaterialCore::create();
	matSilber->setAmbient(glm::vec4(0.19f, 0.19f, 0.19f, 1.f))->setDiffuse(
			glm::vec4(0.51f, 0.51f, 0.51f, 1.f))->setSpecular(
			glm::vec4(0.51f, 0.51f, 0.51f, 1.f))->setShininess(51.2f)->init();

	auto matChrom = MaterialCore::create();
	matChrom->setAmbient(glm::vec4(0.25f, 0.25f, 0.25f, 1.f))->setDiffuse(
			glm::vec4(0.40f, 0.40f, 0.40f, 1.f))->setSpecular(
			glm::vec4(0.77f, 0.77f, 0.77f, 1.f))->setShininess(76.8f)->init();

	auto matBronze = MaterialCore::create();
	matBronze->setAmbient(glm::vec4(0.21f, 0.13f, 0.05f, 1.f))->setDiffuse(
			glm::vec4(0.71f, 0.43f, 0.18f, 1.f))->setSpecular(
			glm::vec4(0.39f, 0.27f, 0.17f, 1.f))->setShininess(25.6f)->init();




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
//	auto texCeiling = textureFactory.create2DTextureFromFile("ceiling.png",
	//		GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);




	GeometryCoreFactory geometryFactory;
	auto wallCore = geometryFactory.createCuboid(glm::vec3(15.f, 0.05f, 10.f));
	ShapeSP walls[6];
	TransformationSP wallsTrans[6];

	auto zimmer = Group::create();
	zimmer->addCore(shaderPhongTex)
			->addCore(matGrey);

	auto wandLR = Group::create();
		wandLR->addCore(texCem1);
	auto wandVH = Group::create();
		wandVH->addCore(texCem2);
	auto wandNT = Group::create();
		wandNT->addCore(texCem3);

	for(int i = 0; i < 6; i++){
		walls[i] = Shape::create(wallCore);
		wallsTrans[i] = Transformation::create();
		wallsTrans[i]->addChild(walls[i]);
	}

	//walls[0]->addCore(texCem3);
	//walls[1]->addCore(texCeiling);

	wandLR->addChild(wallsTrans[2])
			->addChild(wallsTrans[4]);
	wandVH->addChild(wallsTrans[3])
			->addChild(wallsTrans[5]);
	wandNT->addChild(wallsTrans[0])
			->addChild(wallsTrans[1]);
	zimmer->addChild(wandLR)
			->addChild(wandVH)
			//->addChild(wallsTrans[0])
			->addChild(wandNT);

	wallsTrans[0]->translate(glm::vec3(0.f, -0.5f, 0.f));												//floor
	wallsTrans[1]->translate(glm::vec3(0.f, 9.5f, 0.f))->rotate(-180.f,glm::vec3(1.f, 0.f, 0.f));		//decke
	wallsTrans[2]->translate(glm::vec3(-7.5f, 4.5f, 0.f))->rotate(-90.f,glm::vec3(0.f, 0.f, 1.f));		//links
	wallsTrans[3]->translate(glm::vec3(0.0f, 4.5f, -5.f))->rotate(-90.f,glm::vec3(1.f, 0.f, 0.f));		//hinten
	wallsTrans[4]->translate(glm::vec3(7.5f, 4.5f, 0.f))->rotate(-90.f,glm::vec3(0.f, 0.f, 1.f));		//rechts
	wallsTrans[5]->translate(glm::vec3(0.0f, 4.5f, 5.f))->rotate(-90.f,glm::vec3(1.f, 0.f, 0.f));		//vorn


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

<<<<<<< Updated upstream
=======
/*
	auto clockAxis = Shape::create(); clockAxis->addCore(matGold)->addCore(clockAxisCore);
	auto clockGear1 = Shape::create(); clockGear1->addCore(matGold)->addCore(clockGear1Core);
	auto clockGear2 = Shape::create(); clockGear2->addCore(matGold)->addCore(clockGear2Core);
	auto clockGear3 = Shape::create(); clockGear3->addCore(matGold)->addCore(clockGear3Core);
	auto clockGear4 = Shape::create(); clockGear4->addCore(matGold)->addCore(clockGear4Core);
	auto clockHandSmall = Shape::create(); clockHandSmall->addCore(matMessing)->addCore(clockHandSmallCore);
	auto clockHandBig = Shape::create(); clockHandBig->addCore(matMessing)->addCore(clockHandBigCore);
	auto clockAxis2nd = Shape::create(); clockAxis2nd->addCore(matGold)->addCore(clockAxis2ndCore);
	auto clockGear2nd = Shape::create(); clockGear2nd->addCore(matGold)->addCore(clockGear2ndCore);
*/



>>>>>>> Stashed changes
	// Dinge

	ShapeSP clockGears[7];
	ShapeSP clockHands[2];
	TransformationSP clockGearsTrans[7];
	TransformationSP clockHandsTrans[2];

	auto clock = Group::create();
	auto gearsAxis = Group::create();
	gearsAxis->addCore(matGold);
	auto zeiger = Group::create();
	zeiger->addCore(matMessing);

	clockGears[0] = Shape::create(clockAxisCore);
	clockGears[1] = Shape::create(clockGear1Core);
	clockGears[2] = Shape::create(clockGear2Core);
	clockGears[3] = Shape::create(clockGear3Core);
	clockGears[4] = Shape::create(clockGear4Core);
	clockGears[5] = Shape::create(clockAxis2ndCore);
	clockGears[6] = Shape::create(clockGear2ndCore);

	clockHands[0] = Shape::create(clockHandSmallCore);
	clockHands[1] = Shape::create(clockHandBigCore);

	for(int i = 0; i < 7; i++) {
		clockGearsTrans[i] = Transformation::create();
	}

	for(int i = 0; i < 2; i++) {
		clockHandsTrans[i] = Transformation::create();
	}

	clockGearsTrans[0]->translate(glm::vec3(0.0f, 2.f, -4.85f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	clockGearsTrans[1]->translate(glm::vec3(0.0f, 2.f, -4.7f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	clockGearsTrans[2]->translate(glm::vec3(0.0f, 2.f, -4.5f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	clockGearsTrans[3]->translate(glm::vec3(0.0f, 2.f, -4.25f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	clockGearsTrans[4]->translate(glm::vec3(0.0f, 2.f, -4.f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	clockGearsTrans[5]->translate(glm::vec3(-1.96f, 2.f, -4.85f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	clockGearsTrans[6]->translate(glm::vec3(-1.96f, 2.f, -4.47f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(1.85f, 1.85f, 1.85f));

	clockHandsTrans[0]->translate(glm::vec3(0.0f, 2.f, -4.1f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->rotate(-70.f, glm::vec3(0.f, 1.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));
	clockHandsTrans[1]->translate(glm::vec3(0.0f, 2.f, -3.9f))->rotate(-90.f, glm::vec3(1.f, 0.f, 0.f))->scale(glm::vec3(2.0f, 2.0f, 2.0f));



	//Animationen von ClockGears
	//
	 auto clockAnim1 = TransformAnimation::create();
		float angularVelClock1 = 3.f; //Geschwindigkeit
		glm::vec3 axisClock1(0.f, 1.f, 0.f); //Rotation um y-Achse
		clockAnim1->setUpdateFunc(
				[angularVelClock1, axisClock1](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
					animation->rotate(angularVelClock1*static_cast<GLfloat>(diffTime), axisClock1);
				});
		viewer->addAnimation(clockAnim1);
		// add transformation (translation) to be applied before animation
		auto clockAnimTrans1 = Transformation::create();
		clockAnimTrans1->translate(glm::vec3(0.0f, 0.f, 0.f));

	auto clockAnim2 = TransformAnimation::create();
		float angularVelClock2 = 3.f; //Geschwindigkeit
		glm::vec3 axisClock2(0.f, 1.f, 0.f); //Rotation um y-Achse
		clockAnim2->setUpdateFunc(
				[angularVelClock2, axisClock2](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
					animation->rotate(-angularVelClock2*static_cast<GLfloat>(diffTime), axisClock2);
				});
		viewer->addAnimation(clockAnim2);
		// add transformation (translation) to be applied before animation
		auto clockAnimTrans2 = Transformation::create();
		clockAnimTrans2->translate(glm::vec3(0.0f, 0.f, 0.f));

	auto clockAnim3 = TransformAnimation::create();
		float angularVelClock3 = 3.f; //Geschwindigkeit
		glm::vec3 axisClock3(0.f, 1.f, 0.f); //Rotation um y-Achse
		clockAnim3->setUpdateFunc(
				[angularVelClock3, axisClock3](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
					animation->rotate(angularVelClock3*static_cast<GLfloat>(diffTime), axisClock3);
				});
		viewer->addAnimation(clockAnim3);
		// add transformation (translation) to be applied before animation
		auto clockAnimTrans3 = Transformation::create();
		clockAnimTrans3->translate(glm::vec3(0.0f, 0.f, 0.f));

	auto clockAnim4 = TransformAnimation::create();
		float angularVelClock4 = 3.5f; //Geschwindigkeit
		glm::vec3 axisClock4(0.f, 1.f, 0.f); //Rotation um y-Achse
		clockAnim4->setUpdateFunc(
				[angularVelClock4, axisClock4](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
					animation->rotate(-angularVelClock4*static_cast<GLfloat>(diffTime), axisClock4);
				});
		viewer->addAnimation(clockAnim4);
		// add transformation (translation) to be applied before animation
		auto clockAnimTrans4 = Transformation::create();
		clockAnimTrans4->translate(glm::vec3(0.0f, 0.f, 0.f));

	auto clockAnim5 = TransformAnimation::create();
		float angularVelClock5 = 3.f; //Geschwindigkeit
		glm::vec3 axisClock5(0.f, 1.f, 0.f); //Rotation um y-Achse
		clockAnim5->setUpdateFunc(
				[angularVelClock5, axisClock5](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
					animation->rotate(angularVelClock5*static_cast<GLfloat>(diffTime), axisClock5);
				});
		viewer->addAnimation(clockAnim5);
		// add transformation (translation) to be applied before animation
		auto clockAnimTrans5 = Transformation::create();
		clockAnimTrans5->translate(glm::vec3(0.0f, 0.f, 0.f));

	clockGearsTrans[0]->addChild(clockGears[0]);
	clockGearsTrans[5]->addChild(clockGears[5]);

	clockGearsTrans[1]->addChild(clockAnim1);
	clockAnim1->addChild(clockAnimTrans1);
	clockAnimTrans1->addChild(clockGears[1]);

	clockGearsTrans[2]->addChild(clockAnim2);
	clockAnim2->addChild(clockAnimTrans2);
	clockAnimTrans2->addChild(clockGears[2]);

	clockGearsTrans[3]->addChild(clockAnim3);
	clockAnim3->addChild(clockAnimTrans3);
	clockAnimTrans3->addChild(clockGears[3]);

	clockGearsTrans[4]->addChild(clockAnim4);
	clockAnim4->addChild(clockAnimTrans4);
	clockAnimTrans4->addChild(clockGears[4]);

	clockGearsTrans[6]->addChild(clockAnim5);
	clockAnim5->addChild(clockAnimTrans5);
	clockAnimTrans5->addChild(clockGears[6]);




	//Animation von kleinen Zeiger
	 auto clockHandAnim1 = TransformAnimation::create();
		float angularVelClockHand1 = 1.f; //Geschwindigkeit
		glm::vec3 axisClockHand1(0.f, 1.f, 0.f); //Rotation um y-Achse
		clockHandAnim1->setUpdateFunc(
				[angularVelClockHand1, axisClockHand1](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
					animation->rotate(angularVelClockHand1*static_cast<GLfloat>(diffTime), axisClockHand1);
				});
		viewer->addAnimation(clockHandAnim1);
		// add transformation (translation) to be applied before animation
		auto clockHandAnimTrans1 = Transformation::create();
		clockHandAnimTrans1->translate(glm::vec3(0.0f, 0.f, 0.f));

	//Animation von kleinen Zeiger
	auto clockHandAnim2 = TransformAnimation::create();
		float angularVelClockHand2 = 8.f; //Geschwindigkeit
		glm::vec3 axisClockHand2(0.f, 1.f, 0.f); //Rotation um z-Achse
		clockHandAnim2->setUpdateFunc(
				[angularVelClockHand2, axisClockHand2](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
					animation->rotate(angularVelClockHand2*static_cast<GLfloat>(diffTime), axisClockHand2);
				});
		viewer->addAnimation(clockHandAnim2);
		// add transformation (translation) to be applied before animation
		auto clockHandAnimTrans2 = Transformation::create();
		clockHandAnimTrans2->translate(glm::vec3(0.0f, 0.f, 0.f));


	clockHandsTrans[0]->addChild(clockHandAnim1);
	clockHandAnim1->addChild(clockHandAnimTrans1);
	clockHandAnimTrans1->addChild(clockHands[0]);

	clockHandsTrans[1]->addChild(clockHandAnim2);
	clockHandAnim2->addChild(clockHandAnimTrans2);
	clockHandAnimTrans2->addChild(clockHands[1]);

	for(int i = 0; i < 7; i++) {
		gearsAxis->addChild(clockGearsTrans[i]);
	}

	zeiger->addChild(clockHandsTrans[0])->addChild(clockHandsTrans[1]);

	clock->addChild(gearsAxis)
			->addChild(zeiger);


	/*
	 *
	 *
	 * Zahnrad Model
	 *
	 *
	 */
	auto gear = Group::create();
	auto gearCore = geometryFactory.createModelFromOBJFile("obj/gear.obj");
	auto gearCenterCore = geometryFactory.createModelFromOBJFile("obj/gear_center.obj");
		ShapeSP gears[7];
		ShapeSP gearsCenter[7];
		TransformationSP gearsTrans[7];
		TransformationSP gearsCenterTrans[7];

	auto gearChrom = Group::create();
		gearChrom->addCore(matChrom);

	auto gearMessing = Group::create();
		gearMessing->addCore(matMessing);

	auto gearSilber = Group::create();
		gearSilber->addCore(matSilber);

	//Animation
		    auto gearsAnim1 = TransformAnimation::create();
		    float angularVelGears1 = 20.f; //Geschwindigkeit
			glm::vec3 axisGears1(0.f, 1.f, 0.f); //Rotation um z-Achse
			gearsAnim1->setUpdateFunc(
					[angularVelGears1, axisGears1](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
						animation->rotate(angularVelGears1*static_cast<GLfloat>(diffTime), axisGears1);
					});
			viewer->addAnimation(gearsAnim1);
			// add transformation (translation) to be applied before animation
			auto gearsAnimTrans1 = Transformation::create();
			gearsAnimTrans1->translate(glm::vec3(0.0f, 0.f, 0.f));

			auto gearsAnim2 = TransformAnimation::create();
			float angularVelGears2 = 20.f; //Geschwindigkeit
			glm::vec3 axisGears2(0.f, 1.f, 0.f); //Rotation um z-Achse
			gearsAnim2->setUpdateFunc(
					[angularVelGears2, axisGears2](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
						animation->rotate(-angularVelGears2*static_cast<GLfloat>(diffTime), axisGears2);
					});
			viewer->addAnimation(gearsAnim2);
			// add transformation (translation) to be applied before animation
			auto gearsAnimTrans2 = Transformation::create();
			gearsAnimTrans2->translate(glm::vec3(0.0f, 0.f, 0.f));

			auto gearsAnim3 = TransformAnimation::create();
			float angularVelGears3 = 20.f; //Geschwindigkeit
			glm::vec3 axisGears3(0.f, 1.f, 0.f); //Rotation um z-Achse
			gearsAnim3->setUpdateFunc(
					[angularVelGears3, axisGears3](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
						animation->rotate(-angularVelGears3*static_cast<GLfloat>(diffTime), axisGears3);
					});
			viewer->addAnimation(gearsAnim3);
			// add transformation (translation) to be applied before animation
			auto gearsAnimTrans3 = Transformation::create();
			gearsAnimTrans3->translate(glm::vec3(0.0f, 0.f, 0.f));

			auto gearsAnim4 = TransformAnimation::create();
			float angularVelGears4 = 20.f; //Geschwindigkeit
			glm::vec3 axisGears4(0.f, 1.f, 0.f); //Rotation um z-Achse
			gearsAnim4->setUpdateFunc(
					[angularVelGears4, axisGears4](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
						animation->rotate(-angularVelGears4*static_cast<GLfloat>(diffTime), axisGears4);
					});
			viewer->addAnimation(gearsAnim4);
			// add transformation (translation) to be applied before animation
			auto gearsAnimTrans4 = Transformation::create();
			gearsAnimTrans4->translate(glm::vec3(0.0f, 0.f, 0.f));

			auto gearsAnim5 = TransformAnimation::create();
			float angularVelGears5 = 20.f; //Geschwindigkeit
			glm::vec3 axisGears5(0.f, 1.f, 0.f); //Rotation um z-Achse
			gearsAnim5->setUpdateFunc(
					[angularVelGears5, axisGears5](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
						animation->rotate(-angularVelGears5*static_cast<GLfloat>(diffTime), axisGears5);
					});
			viewer->addAnimation(gearsAnim5);
			// add transformation (translation) to be applied before animation
			auto gearsAnimTrans5 = Transformation::create();
			gearsAnimTrans5->translate(glm::vec3(0.0f, 0.f, 0.f));

			auto gearsAnim6 = TransformAnimation::create();
			float angularVelGears6 = 20.f; //Geschwindigkeit
			glm::vec3 axisGears6(0.f, 1.f, 0.f); //Rotation um z-Achse
			gearsAnim6->setUpdateFunc(
					[angularVelGears6, axisGears6](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
						animation->rotate(angularVelGears6*static_cast<GLfloat>(diffTime), axisGears6);
					});
			viewer->addAnimation(gearsAnim6);
			// add transformation (translation) to be applied before animation
			auto gearsAnimTrans6 = Transformation::create();
			gearsAnimTrans6->translate(glm::vec3(0.0f, 0.f, 0.f));

			auto gearsAnim7 = TransformAnimation::create();
			float angularVelGears7 = 20.f; //Geschwindigkeit
			glm::vec3 axisGears7(0.f, 1.f, 0.f); //Rotation um z-Achse
			gearsAnim7->setUpdateFunc(
					[angularVelGears7, axisGears7](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
						animation->rotate(angularVelGears7*static_cast<GLfloat>(diffTime), axisGears7);
					});
			viewer->addAnimation(gearsAnim7);
			// add transformation (translation) to be applied before animation
			auto gearsAnimTrans7 = Transformation::create();
			gearsAnimTrans7->translate(glm::vec3(0.0f, 0.f, 0.f));


		for(int i = 0; i < 7; i++) {
			gears[i]=Shape::create(gearCore);
			gearsTrans[i] = Transformation::create();
			gearsCenter[i]=Shape::create(gearCenterCore);
			gearsCenterTrans[i] = Transformation::create();
		}

		gearsTrans[0]->translate(glm::vec3(-4.85f, 2.f, 0.f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsTrans[1]->translate(glm::vec3(-4.85f, 2.f, -1.4f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsTrans[2]->translate(glm::vec3(-4.85f, 3.4f, 0.f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsTrans[3]->translate(glm::vec3(4.85f, 3.4f, 0.f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsTrans[4]->translate(glm::vec3(4.85f, 2.f, -1.4f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsTrans[5]->translate(glm::vec3(4.85f, 2.f, 0.f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsTrans[6]->translate(glm::vec3(4.85f, 3.4f, 1.4f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsCenterTrans[0]->translate(glm::vec3(-4.85f, 2.f, 0.f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsCenterTrans[1]->translate(glm::vec3(-4.85f, 2.f, -1.4f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsCenterTrans[2]->translate(glm::vec3(-4.85f, 3.4f, 0.f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsCenterTrans[3]->translate(glm::vec3(4.85f, 3.4f, 0.f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsCenterTrans[4]->translate(glm::vec3(4.85f, 2.f, -1.4f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsCenterTrans[5]->translate(glm::vec3(4.85f, 2.f, 0.f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
		gearsCenterTrans[6]->translate(glm::vec3(4.85f, 3.4f, 1.4f))->rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));

		//Animation hinzufügen
		gearsTrans[0]->addChild(gearsAnim1);
		gearsAnim1->addChild(gearsAnimTrans1);
		gearsAnimTrans1->addChild(gears[0]);

		gearsTrans[1]->addChild(gearsAnim2);
		gearsAnim2->addChild(gearsAnimTrans2);
		gearsAnimTrans2->addChild(gears[1]);

		gearsTrans[2]->addChild(gearsAnim3);
		gearsAnim3->addChild(gearsAnimTrans3);
		gearsAnimTrans3->addChild(gears[2]);

		gearsTrans[3]->addChild(gearsAnim4);
		gearsAnim4->addChild(gearsAnimTrans4);
		gearsAnimTrans4->addChild(gears[3]);

		gearsTrans[4]->addChild(gearsAnim5);
		gearsAnim5->addChild(gearsAnimTrans5);
		gearsAnimTrans5->addChild(gears[4]);

		gearsTrans[5]->addChild(gearsAnim6);
		gearsAnim6->addChild(gearsAnimTrans6);
		gearsAnimTrans6->addChild(gears[5]);

		gearsTrans[6]->addChild(gearsAnim7);
		gearsAnim7->addChild(gearsAnimTrans7);
		gearsAnimTrans7->addChild(gears[6]);


		for(int i = 0; i < 7; i++) {
			gearsCenterTrans[i]->addChild(gearsCenter[i]);
		}

		gearChrom->addChild(gearsTrans[0])->addChild(gearsTrans[4])->addChild(gearsTrans[6])->addChild(gearsCenterTrans[1])->addChild(gearsCenterTrans[5]);
		gearMessing->addChild(gearsTrans[1])->addChild(gearsTrans[2])->addChild(gearsTrans[5])->addChild(gearsCenterTrans[0])->addChild(gearsCenterTrans[3]);
		gearSilber->addChild(gearsTrans[3])->addChild(gearsCenterTrans[2])->addChild(gearsCenterTrans[4])->addChild(gearsCenterTrans[6]);


		gear->addChild(gearChrom)->addChild(gearMessing)->addChild(gearSilber);

	//Create Zahnrad auf dem Boden
	auto gearFloorCore = geometryFactory.createModelFromOBJFile("obj/gear.obj");
	auto gearFloor = Shape::create();
	gearFloor->addCore(matChrom)->addCore(gearFloorCore);
	auto gearFloorTrans = Transformation::create();
	gearFloorTrans->translate(glm::vec3(0.f, -0.4f, 0.f));
	gearFloorTrans->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//->rotate(90.f, glm::vec3(1.f, 0.f, 0.f));

	auto stabFloorCore = geometryFactory.createModelFromOBJFile("obj/gear_center.obj");
	auto stabFloor = Shape::create();
	stabFloor->addCore(matSilber)->addCore(stabFloorCore);
	auto stabFloorTrans = Transformation::create();
	stabFloorTrans->translate(glm::vec3(0.f, -0.5f, 0.f));
	stabFloorTrans->scale(glm::vec3(1.5f, 1.5f, 1.5f));
	//stabFloorTrans->rotate(90.f, glm::vec3(1.f, 0.f, 0.f));

//	Animation für Zahnrad auf dem Boden
	auto gearFloorAnim = TransformAnimation::create();
	float angularVel7 = 20.f; //Geschwindigkeit
	glm::vec3 axis7(0.f, 1.f, 0.f); //Rotation um z-Achse
	gearFloorAnim->setUpdateFunc(
			[angularVel7, axis7](TransformAnimation*animation,double currTime, double diffTime, double totalTime) {
				animation->rotate(angularVel7*static_cast<GLfloat>(diffTime), axis7);
			});
	viewer->addAnimation(gearFloorAnim);
	// add transformation (translation) to be applied before animation
	auto gearFloorAnimTrans = Transformation::create();
	gearFloorAnimTrans->translate(glm::vec3(0.0f, 0.f, 0.f));

<<<<<<< Updated upstream

	//Create Lampe
	auto lampeCore = geometryFactory.createModelFromOBJFile("obj/lamp1.obj");
		auto lampe = Shape::create();
		lampe->addCore(matChrom)->addCore(lampeCore);
		auto lampeTrans = Transformation::create();
		lampeTrans->translate(glm::vec3(1.f, 8.1f, -1.f));
		//lampeTrans->scale(glm::vec3(1.5f, 1.5f, 1.5f));

	auto sphereCore = geometryFactory.createSphere(1.0f, 50, 50) ;
=======
	/*auto sphereCore = geometryFactory.createSphere(1.0f, 50, 50) ;
>>>>>>> Stashed changes
	   auto sphere = Shape::create();
	   sphere->addCore(matWhite)
			->addCore(sphereCore);
	   auto sphereTrans = Transformation::create();
<<<<<<< Updated upstream
	   sphereTrans->translate(glm::vec3(1.0f, 7.95f, -1.f))
		->scale(glm::vec3(0.2f, 0.2f, 0.2f));;

	   //Create SpotLampe
	auto spotCore = geometryFactory.createModelFromOBJFile("obj/spot.obj");
		auto spot = Shape::create();
		spot->addCore(matGrey)->addCore(spotCore);
		auto spotTrans = Transformation::create();
		spotTrans->translate(glm::vec3(0.0f, 5.5f, -5.f));
		spotTrans->rotate(-90.0f, glm::vec3(0.f, 1.f, 0.f));
		//spotTrans->scale(glm::vec3(0.8f, 0.8f, 0.8f));
	auto sphere2Core = geometryFactory.createSphere(1.0f, 50, 50) ;
	   auto sphere2 = Shape::create();
	   sphere2->addCore(matWhite)
			->addCore(sphere2Core);
	   auto sphere2Trans = Transformation::create();
	   sphere2Trans->translate(glm::vec3(0.0f, 4.5f, -4.5f))
		->scale(glm::vec3(0.17f, 0.17f, 0.17f));;
=======
	   sphereTrans->translate(glm::vec3(3.0f, 2.5f, -3.f))
		->scale(glm::vec3(0.5f, 0.5f, 0.5f));;
*/
>>>>>>> Stashed changes

	// create scene graph
	/*
	   scene
	     |------------------|
	   camera			  light
	   	   	   	   	   	    |
	   	   	   	   	   	  light2------------------
	   	   	   	   	   	    |					 |
	   	   	   	   	   	  floortrans

	 */
	scene = Group::create();
	scene->addCore(shaderPhong);
	scene->addChild(camera)
			->addChild(light);
	light ->addChild(light2);
	light2->addChild(zimmer)
			->addChild(clock)
<<<<<<< Updated upstream
			->addChild(gear)
			->addChild(lampeTrans)
			->addChild(sphereTrans)
			->addChild(spotTrans)
			->addChild(sphere2Trans)
			->addChild(gearFloorTrans)
			->addChild(stabFloorTrans);
	lampeTrans->addChild(lampe);
	sphereTrans->addChild(sphere);
	sphere2Trans->addChild(sphere2);
	spotTrans->addChild(spot);
	stabFloorTrans->addChild(stabFloor);
=======
			->addChild(gearFloorTrans);

	//sphereTrans->addChild(sphere);
	//gearTrans->addChild(gear);
	//gear2Trans->addChild(gear2);
//	gearFloorTrans->addChild(gearFloor);

//	stabTrans->addChild(stab);
//	stab2Trans->addChild(stab2);
>>>>>>> Stashed changes

//	gearFloorTrans->addChild(gearFloor);

	/*clockAxisTrans->addChild(clockAxis);
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

	*/
	gearFloorTrans->addChild(gearFloorAnim);
		gearFloorAnim->addChild(gearFloorAnimTrans);
		gearFloorAnimTrans
	//	->addChild(camera)
		->addChild(gearFloor);
}
