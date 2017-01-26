/** \file App.cpp */
#include "App.h"

/*
Change Log:
- code base from Kenny (meshes)
- custom GUI code added by Melanie, Kenny, Yitong, and Cole (midterm)
*/

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);

    // Change the window and other startup parameters by modifying the
    // settings class.  For example:
    settings.window.caption             = argv[0];

    // Set enable to catch more OpenGL errors
    // settings.window.debugContext     = true;

    // Some common resolutions:
    // settings.window.width            =  854; settings.window.height       = 480;
    // settings.window.width            = 1024; settings.window.height       = 768;
    settings.window.width               = 1280; settings.window.height       = 720;
    //settings.window.width             = 1920; settings.window.height       = 1080;
    // settings.window.width            = OSWindow::primaryDisplayWindowSize().x; settings.window.height = OSWindow::primaryDisplayWindowSize().y;
    settings.window.fullScreen          = false;
    settings.window.resizable           = ! settings.window.fullScreen;
    settings.window.framed              = ! settings.window.fullScreen;

    // Set to true for a significant performance boost if your app can't render at 60fps, or if
    // you *want* to render faster than the display.
    settings.window.asynchronous        = false;

    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(64, 64);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir                    = FileSystem::currentDirectory();
    settings.screenshotDirectory        = "../journal/";

    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = false;

    return App(settings).run();
}


App::App(const GApp::Settings& settings) :
    GApp(settings),

    // Set generation defaults.
    terrainSize(100), 
    terrainGran(4),
    terrainPower(1.5), 
    terrainOffset(12),
    numRandomSampledPts(0),
    areaPerSheep(5),
    numImportanceSampledPts(0),
    addPointsAboveThreshold(true),
    interestingThreshold(0.95f),
    heightfieldXScale(1),
    heightfieldYScale(20),
    heightfieldZScale(1),
    heightFieldImageFilePath("heightfield.png") {
}

// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be
// automatically caught.
void App::onInit() {
    GApp::onInit();

    setFrameDuration(1.0f / 120.0f);

    // Call setScene(shared_ptr<Scene>()) or setScene(MyScene::create()) to replace
    // the default scene here.
        
    debugPrintf("Target frame rate = %f Hz\n", realTimeTargetDuration());
    
    showRenderingStats      = false;

    makeGUI();
    // For higher-quality screenshots:
    // developerWindow->videoRecordDialog->setScreenShotFormat("PNG");
    // developerWindow->videoRecordDialog->setCaptureGui(false);
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));
    loadScene(
        //"G3D Sponza"
        "G3D Cornell Box" // Load something simple
        //developerWindow->sceneEditorWindow->selectedSceneName()  // Load the first scene encountered 
        );
}

void App::makeGUI() {
    // Initialize the developer HUD
    createDeveloperHUD();

    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);

    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
    
    // GUI controls for the heightfield
    GuiPane* heightfieldPane = debugPane->addPane("Heightfield", GuiTheme::ORNATE_PANE_STYLE);
    heightfieldPane->addNumberBox("X Scale", &heightfieldXScale, "m", GuiTheme::NO_SLIDER);
    heightfieldPane->addNumberBox("Y Scale", &heightfieldYScale, "m", GuiTheme::NO_SLIDER);
    heightfieldPane->addNumberBox("Z Scale", &heightfieldZScale, "m", GuiTheme::NO_SLIDER);
    heightfieldPane->addButton("Choose Image File", [this]() {
        FileDialog::getFilename(heightFieldImageFilePath);
    });
    heightfieldPane->addButton("Generate", [this](){
        shared_ptr<Image> image;
        try {
            drawMessage("Generating...");
            
            // Initialize variables
            image = Image::fromFile(heightFieldImageFilePath);
            int width = image->width();
            int height = image->height();
            shared_ptr<SceneFile> sceneFile = std::make_shared<SceneFile>();
            
            // Generate the voronoi diagram for the field coloring
            const std::string voronoiCall = "\"C:/Python27/python.exe\" ../scripts/voronoi-img.py " + std::to_string(width) + " " + std::to_string(height) + " " + std::to_string(numVoronoiCells) + " ../data-files/colormap.png";
            system(voronoiCall.c_str());
            
            // Generate the heightfield
            Mesh heightfield(
                image,
                sceneFile,
                heightfieldXScale,
                heightfieldYScale,
                heightfieldZScale,
                numRandomSampledPts,
                numImportanceSampledPts,
                addPointsAboveThreshold,
                interestingThreshold,
                terrainSize, 
                terrainGran,
                terrainPower, 
                terrainOffset,
                areaPerSheep,
                Image::fromFile("colormap.png")
            );
            heightfield.genHeightField();
            heightfield.saveObjMtlFiles("model/heightfield.obj", "model/heightfield.mtl", "model/input.txt");
            
            // Generate the Scene file
            sceneFile->generateFile("../data-files/scene/Farmland.Scene.Any", "Farmland");            

            // Reload scene
            ArticulatedModel::clearCache();
            scene()->load(scene()->name());
        } catch (...) {
            msgBox("Unable to load the image.", heightFieldImageFilePath);
        }
    });

    // GUI controls for the mesh simplification point sampling
	GuiPane* meshSimplificationPane = debugPane->addPane("Mesh Simplification");
	meshSimplificationPane->moveRightOf(heightfieldPane);
	meshSimplificationPane->setNewChildSize(240);
    meshSimplificationPane->addCheckBox("add threshold pts", &addPointsAboveThreshold);
    meshSimplificationPane->addNumberBox("threshold", &interestingThreshold, "low==picky",
		GuiTheme::LOG_SLIDER, 0.01f, 1.0f)->setUnitsSize(30);
    meshSimplificationPane->addNumberBox("# import s. pts", &numImportanceSampledPts, "",
		GuiTheme::LOG_SLIDER, 0, 100000)->setUnitsSize(30);
    meshSimplificationPane->addNumberBox("# rand. s. pts", &numRandomSampledPts, "",
		GuiTheme::LOG_SLIDER, 0, 100000)->setUnitsSize(30);
    meshSimplificationPane->addNumberBox("area per sheep", &areaPerSheep, "m^2",
		GuiTheme::LINEAR_SLIDER, 1.0f, 100.0f)->setUnitsSize(30);

    // GUI controls for the terrain generation parameters    
	GuiPane* terrainGenerationPane = debugPane->addPane("Terrain Generation");
	terrainGenerationPane->moveRightOf(meshSimplificationPane);
	terrainGenerationPane->setNewChildSize(400);
    terrainGenerationPane->addNumberBox("Size", &terrainSize, "Clear",
		GuiTheme::LOG_SLIDER, 0, 1000)->setUnitsSize(30);
    terrainGenerationPane->addNumberBox("Gran Noise", &terrainGran, "Higher -> more gran",
		GuiTheme::LINEAR_SLIDER, 1, 10)->setUnitsSize(30);
    terrainGenerationPane->addNumberBox("Exp Noise", &terrainPower, "Higher -> more extreme",
		GuiTheme::LINEAR_SLIDER, 0.01f, 5.0f)->setUnitsSize(30);
    terrainGenerationPane->addNumberBox("Smooth Noise", &terrainOffset, "Higher -> more variance",
		GuiTheme::LINEAR_SLIDER, 1, 16)->setUnitsSize(30);
    terrainGenerationPane->addNumberBox("Number of Fields", &numVoronoiCells, "Higher -> smaller fields",
        GuiTheme::LINEAR_SLIDER, 1, 2000)->setUnitsSize(50);
}

// This default implementation is a direct copy of GApp::onGraphics3D to make it easy
// for you to modify. If you aren't changing the hardware rendering strategy, you can
// delete this override entirely.
void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    if (!scene()) {
        if ((submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) && (!rd->swapBuffersAutomatically())) {
            swapBuffers();
        }
        rd->clear();
        rd->pushState(); {
            rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
            drawDebugShapes();
        } rd->popState();
        return;
    }

    GBuffer::Specification gbufferSpec = m_gbufferSpecification;
    extendGBufferSpecification(gbufferSpec);
    m_gbuffer->setSpecification(gbufferSpec);
    m_gbuffer->resize(m_framebuffer->width(), m_framebuffer->height());
    m_gbuffer->prepare(rd, activeCamera(), 0, -(float)previousSimTimeStep(), m_settings.hdrFramebuffer.depthGuardBandThickness, m_settings.hdrFramebuffer.colorGuardBandThickness);

    m_renderer->render(rd, m_framebuffer, scene()->lightingEnvironment().ambientOcclusionSettings.enabled ? m_depthPeelFramebuffer : shared_ptr<Framebuffer>(),
        scene()->lightingEnvironment(), m_gbuffer, allSurfaces);

    // Debug visualizations and post-process effects
    rd->pushState(m_framebuffer); {
        // Call to make the App show the output of debugDraw(...)
        rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
        drawDebugShapes();
        const shared_ptr<Entity>& selectedEntity = (notNull(developerWindow) && notNull(developerWindow->sceneEditorWindow)) ? developerWindow->sceneEditorWindow->selectedEntity() : shared_ptr<Entity>();
        scene()->visualize(rd, selectedEntity, allSurfaces, sceneVisualizationSettings(), activeCamera());

        // Post-process special effects
        m_depthOfField->apply(rd, m_framebuffer->texture(0), m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(), m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);

        m_motionBlur->apply(rd, m_framebuffer->texture(0), m_gbuffer->texture(GBuffer::Field::SS_EXPRESSIVE_MOTION),
            m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(),
            m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);
    } rd->popState();

    // We're about to render to the actual back buffer, so swap the buffers now.
    // This call also allows the screenshot and video recording to capture the
    // previous frame just before it is displayed.
    if (submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) {
        swapBuffers();
    }

    // Clear the entire screen (needed even though we'll render over it, since
    // AFR uses clear() to detect that the buffer is not re-used.)
    rd->clear();

    // Perform gamma correction, bloom, and SSAA, and write to the native window frame buffer
    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0), settings().hdrFramebuffer.colorGuardBandThickness.x + settings().hdrFramebuffer.depthGuardBandThickness.x, settings().hdrFramebuffer.depthGuardBandThickness.x);
}


void App::onAI() {
    GApp::onAI();
    // Add non-simulation game logic and AI code here
}


void App::onNetwork() {
    GApp::onNetwork();
    // Poll net messages here
}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);

    // Example GUI dynamic layout code.  Resize the debugWindow to fill
    // the screen horizontally.
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }

    // If you need to track individual UI events, manage them here.
    // Return true if you want to prevent other parts of the system
    // from observing this specific event.
    //
    // For example,
    // if ((event.type == GEventType::GUI_ACTION) && (event.gui.control == m_button)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) { ... return true; }

    return false;
}

void App::onUserInput(UserInput* ui) {
    GApp::onUserInput(ui);
    (void)ui;
    // Add key handling here based on the keys currently held or
    // ones that changed in the last frame.
}


void App::onPose(Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) {
    GApp::onPose(surface, surface2D);

    // Append any models to the arrays that you want to later be rendered by onGraphics()
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    // Render 2D objects like Widgets.  These do not receive tone mapping or gamma correction.
    Surface2D::sortAndRender(rd, posed2D);
}


void App::onCleanup() {
    // Called after the application loop ends.  Place a majority of cleanup code
    // here instead of in the constructor so that exceptions can be caught.
}
