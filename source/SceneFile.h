#pragma once

#include <G3D/G3DAll.h>
#include <iostream>
#include <fstream>

/*
Change Log:
- file created by Melanie, Yitong, Kenny, and Cole (midterm)
*/

/** A class to keep track of entity placement and generate a Scene file. **/
class SceneFile {
protected:

    /** A class to keep track of position/rotation data for a Scene entity. **/
    class SceneObject {
    public:
        Point3 position;
        float yaw;
        float pitch;
        float roll;
        SceneObject(Point3 a, float y, float p, float r) : 
            position(a), yaw(y), pitch(p), roll(r) {};
        SceneObject() {};
    };

    /** Add the specified models to the Scene file. Manually change the output string in this method. **/
    void addModels(std::string& stringOutuput);

    /** Add the specified camera to the Scene file. Manually change the output string in this method. **/
    void addCamera(std::string& stringOutuput);

    /** Add the specified entities to the Scene file. Iterates through the stored arrays of SceneObjects. **/
    void addEntities(std::string& stringOutuput);

    /** Add the specified lighting to the Scene file. Manually change the output string in this method. **/
    void addSun(std::string& stringOutuput);

    /** Add the specified lighting environment to the Scene file. Manually change the output string in this method. **/
    void addLightingEnvironment(std::string& stringOutuput);
    
    Array<SceneObject> clouds;
    Array<SceneObject> sheep;
    Array<SceneObject> trees;

public:
    SceneFile();

    /** Add a cloud to the scene. **/
    void addCloud(const Point3& loc, float yaw);

    /** Add a sheep to the scene. **/
    void addSheep(const Point3& loc, float yaw, float pitch, float roll);

    /** Add a tree to the scene. **/
    void addTree(const Point3& loc, float yaw, float pitch, float roll);

    /** Generate the Scene file. **/
    void generateFile(const std::string& file, const std::string& sceneName);
};