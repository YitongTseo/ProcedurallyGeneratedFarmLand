#include "SceneFile.h"

/*
Change Log:
- file created by Melanie, Yitong, Kenny, and Cole (midterm)
*/

SceneFile::SceneFile() {}

void SceneFile::addCloud(const Point3& loc, float yaw) {
    clouds.append(SceneObject(loc, yaw, 0.0f, 0.0f));
}

void SceneFile::addSheep(const Point3& loc, float yaw, float pitch, float roll) {
    sheep.append(SceneObject(loc, yaw, pitch, roll));
}

void SceneFile::addTree(const Point3& loc, float yaw, float pitch, float roll) {
    trees.append(SceneObject(loc, yaw, pitch, roll));
}

void SceneFile::addModels(std::string& stringOutput) {
    stringOutput +=  R"output(
    models = {

         heightfieldModel = ArticulatedModel::Specification { 
            filename = "model/heightfield.obj";
         };

         waterModel = ArticulatedModel::Specification {
            filename = "model/water.obj";
         };

         //when changing models be sure to update the sheepPersonalSpace variable in Mesh.cpp
         sheepModel = ArticulatedModel::Specification { 
            filename = "model/Low+Poly+Sheep.obj";
            preprocess = {
                transformGeometry(all(), Matrix4::scale(0.004, 0.004, 0.004));
            };
            
         };

         blackSheepModel = ArticulatedModel::Specification { 
            filename = "model/Low+Poly+Black+Sheep.obj";
            preprocess = {
                transformGeometry(all(), Matrix4::scale(0.004, 0.004, 0.004));
            };
         };
        
         treeModel0 = ArticulatedModel::Specification { 
            filename = "model/Lowpoly_tree_sample.obj";
            preprocess = {
                transformGeometry(all(), Matrix4::scale(0.1,0.1,0.1));
            };
         };

         treeModel1 = ArticulatedModel::Specification { 
            filename = "model/Cartoon_low_poly_tree_1.obj";
            preprocess = {
                transformGeometry(all(), Matrix4::scale(0.065,0.065,0.065));
            };
         };

         treeModel2 = ArticulatedModel::Specification { 
            filename = "model/Cartoon_low_poly_tree_2.obj";
            preprocess = {
                transformGeometry(all(), Matrix4::scale(0.075,0.075,0.075));
            };
         };

         cloudModel0 = ArticulatedModel::Specification {
            filename = "model/cloud1.obj";
            preprocess = {
                setMaterial(all(), Color3(1.0, 1.0, 1.0));
                transformGeometry(all(), Matrix4::scale(0.1,0.1,0.1));
            };
         };

         cloudModel1 = ArticulatedModel::Specification {
            filename = "model/cloud2.obj";
            preprocess = {
                setMaterial(all(), Color3(1.0, 1.0, 1.0));
                transformGeometry(all(), Matrix4::scale(0.3,0.3,0.3));
            };
         };

         cloudModel2 = ArticulatedModel::Specification {
            filename = "model/cloud3.obj";
            preprocess = {
                setMaterial(all(), Color3(1.0, 1.0, 1.0));
                transformGeometry(all(), Matrix4::scale(0.1,0.1,0.1));
            };
         };
    };
    )output";
}

void SceneFile::addCamera(std::string& stringOutput) {
    stringOutput += R"output(
        camera = Camera { 
            depthOfFieldSettings = DepthOfFieldSettings { 
                enabled = true; 
                farBlurRadiusFraction = 0.005; 
                farBlurryPlaneZ = -100; 
                farSharpPlaneZ = -40; 
                focusPlaneZ = -10; 
                lensRadius = 0.01; 
                model = "NONE"; 
                nearBlurRadiusFraction = 0.015; 
                nearBlurryPlaneZ = -0.25; 
                nearSharpPlaneZ = -1; 
            }; 
            
            filmSettings = FilmSettings { 
                antialiasingEnabled = true; 
                antialiasingFilterRadius = 0; 
                antialiasingHighQuality = true; 
                bloomRadiusFraction = 0.009; 
                bloomStrength = 0.2; 
                debugZoom = 1; 
                effectsEnabled = true; 
                gamma = 2.2; 
                sensitivity = 1; 
                toneCurve = Spline { 
                    control = ( 0, 0.1, 0.2, 0.5, 1 ); 
                    extrapolationMode = "LINEAR"; 
                    finalInterval = -1; 
                    interpolationMode = "CUBIC"; 
                    time = ( 0, 0.1, 0.2, 0.5, 1 ); 
                }; 
                
                vignetteBottomStrength = 0.05; 
                vignetteSizeFraction = 0.17; 
                vignetteTopStrength = 0.5; 
            }; 
            
            frame = CFrame::fromXYZYPRDegrees(0, 0, 0 ); 

            motionBlurSettings = MotionBlurSettings { 
                cameraMotionInfluence = 0.5; 
                enabled = false; 
                exposureFraction = 0.75; 
                maxBlurDiameterFraction = 0.1; 
                numSamples = 27; 
            }; 
            
            projection = Projection { 
                farPlaneZ = -150; 
                fovDegrees = 90; 
                fovDirection = "HORIZONTAL"; 
                nearPlaneZ = -0.15; 
                pixelOffset = Vector2(0, 0 ); 
            }; 
            
            visualizationScale = 1; 
        }; 
    )output";
}

void SceneFile::addEntities(std::string& stringOutput) {
    stringOutput += R"output(
        heightfield = VisibleEntity { 
            frame = CFrame::fromXYZYPRDegrees(0, 0, 0); 
            model = "heightfieldModel"; 
        };
    )output";

    stringOutput += R"output(
        water = VisibleEntity { 
            frame = CFrame::fromXYZYPRDegrees(0, 0, 0); 
            model = "waterModel"; 
        };
    )output";
    
    Random& rng = Random::threadCommon();
    for (int i = 0; i < trees.size(); ++i) {
        // Randomly sample a tree from the 3 models.
        const int treeNumber = rng.uniform() * 2.99;
        stringOutput += format(R"output(
            tree%d = VisibleEntity { 
                frame = CFrame::fromXYZYPRDegrees(%f, %f, %f, %f, %f, %f); 
                model = "treeModel%d"; 
            };
            )output", 
            i, 
            trees[i].position.x, 
            trees[i].position.y, 
            trees[i].position.z, 
            trees[i].yaw, 
            trees[i].pitch, 
            trees[i].roll, 
            treeNumber
        ).c_str();
    }

    for (int i = 0; i < sheep.size(); ++i) {
        // Make 1 in 42 sheep black
        const char* sheepType = (rng.uniform() * 42) < 1 ? "blackSheepModel" : "sheepModel";
        stringOutput += format(R"output(
            sheep%d = VisibleEntity { 
                frame = CFrame::fromXYZYPRDegrees(%f, %f, %f, %f, %f, %f); 
                model = "%s"; 
            };
            )output", 
            i, 
            sheep[i].position.x, 
            sheep[i].position.y, 
            sheep[i].position.z, 
            sheep[i].yaw, 
            sheep[i].pitch, 
            sheep[i].roll,
            sheepType
        ).c_str();
    }

    for (int i = 0; i < clouds.size(); ++i) {
        stringOutput += format(R"output(
        cloud%d = VisibleEntity { 
            frame = CFrame::fromXYZYPRDegrees(%f, %f, %f, %f); 
            model = "cloudModel%d"; 
        };
        )output", i, clouds[i].position.x, clouds[i].position.y, clouds[i].position.z, clouds[i].yaw, rand() % 3).c_str();
    }
}

void SceneFile::addSun(std::string& stringOutput) {
    stringOutput += R"output(
               sun = Light { 
            attenuation = ( 0.0001, 0, 1 ); 
            bulbPower = Color3(2e7,2e7,1e7); 
            canChange = false; 
            castsShadows = true; 
            enabled = true;
            frame = CFrame::fromXYZYPRDegrees(-449.06, 426.89, 395.66, -65.03, -33.273, 0); 
            nearPlaneZLimit = -30; 
            producesDirectIllumination = true; 
            producesIndirectIllumination = true; 
            shadowMapSize = Vector2int16(4096, 4096); 
            shadowMapBias = 0.04;
            spotHalfAngleDegrees = 20; 
            spotSquare = true; 
            type = "SPOT"; 
        }; 
    )output";
}

void SceneFile::addLightingEnvironment(std::string& stringOutput) {
    stringOutput += R"output(
    lightingEnvironment = LightingEnvironment { 
        ambientOcclusionSettings = AmbientOcclusionSettings { 
            bias = 0.012; 
            blurRadius = 4; 
            blurStepSize = 2; 
            depthPeelSeparationHint = 0.001; 
            edgeSharpness = 1; 
            enabled = false; 
            highQualityBlur = true; 
            intensity = 8.95833; 
            monotonicallyDecreasingBilateralWeights = false; 
            numSamples = 24; 
            packBlurKeys = false; 
            radius = 1.40702; 
            temporalFilterSettings = TemporalFilter::Settings { 
                falloffEndDistance = 0.07; 
                falloffStartDistance = 0.05; 
                hysteresis = 0.802083; 
            }; 
            
            temporallyVarySamples = true; 
            useDepthPeelBuffer = true; 
            useNormalBuffer = true; 
            useNormalsInBlur = true; 
            zStorage = "HALF"; 
        }; 
        
        environmentMap = "cubemap/gradient_blue/gradient_*.png"; 
    };)output";
}

void SceneFile::generateFile(const std::string& file, const std::string& sceneName) {
    std::ofstream outPutFile;
    outPutFile.open (file.c_str());

    std::string stringOutput = "/* -*- c++ -*- */\n{";
        
    addModels(stringOutput);

    stringOutput += "entities = {\n"; 

    addCamera(stringOutput);
    addEntities(stringOutput);
    addSun(stringOutput);
       
    stringOutput += "};\n"; 
    
    addLightingEnvironment(stringOutput);

    stringOutput += format("name = \"%s\"\n}", sceneName).c_str(); 

    outPutFile << stringOutput;
    outPutFile.close();
}

