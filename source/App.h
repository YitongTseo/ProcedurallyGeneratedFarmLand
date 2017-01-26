/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include <iostream>
#include <fstream>

#include "Mesh.h"
#include "SceneFile.h"

/*
Change Log:
- code base from Kenny (meshes)
- cylinder variables deleted by Melanie (midterm)
- all member variables added by Melanie, Kenny, Yitong, and Cole (midterm)
*/

/** \brief Application framework. */
class App : public GApp {
protected:
    
    // Heightfield variables
    float heightfieldXScale;
    float heightfieldYScale;
    float heightfieldZScale;
    String heightFieldImageFilePath;

    // Point sampling variables
    int numRandomSampledPts;
    int numImportanceSampledPts;
    bool addPointsAboveThreshold;
    float interestingThreshold;

    // Terrain generation variables
    int terrainSize; 
    int terrainGran;
    float terrainPower; 
    int terrainOffset;

    // Density of sheep
    float areaPerSheep;

    // Size of fields
    int numVoronoiCells;

    /** Called from onInit */
    void makeGUI();

public:
    
    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit() override;
    virtual void onAI() override;
    virtual void onNetwork() override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;

    // You can override onGraphics if you want more control over the rendering loop.
    // virtual void onGraphics(RenderDevice* rd, Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) override;

    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;

    virtual bool onEvent(const GEvent& e) override;
    virtual void onUserInput(UserInput* ui) override;
    virtual void onCleanup() override;
};
