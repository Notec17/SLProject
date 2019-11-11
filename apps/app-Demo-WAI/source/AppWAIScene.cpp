#include <AppWAIScene.h>
#include <SLBox.h>
#include <SLLightSpot.h>
#include <SLCoordAxis.h>
#include <SLPoints.h>

AppWAIScene::AppWAIScene()
{
}

void AppWAIScene::rebuild()
{
    rootNode            = new SLNode("scene");
    cameraNode          = new SLCamera("Camera 1");
    mapNode             = new SLNode("map");
    mapPC               = new SLNode("MapPC");
    mapMatchedPC        = new SLNode("MapMatchedPC");
    mapLocalPC          = new SLNode("MapLocalPC");
    mapMarkerCornerPC   = new SLNode("MapMarkerCornerPC");
    keyFrameNode        = new SLNode("KeyFrames");
    covisibilityGraph   = new SLNode("CovisibilityGraph");
    spanningTree        = new SLNode("SpanningTree");
    loopEdges           = new SLNode("LoopEdges");
    SLLightSpot* light1 = new SLLightSpot(1, 1, 1, 0.3f);

    redMat = new SLMaterial(SLCol4f::RED, "Red");
    redMat->program(new SLGLGenericProgram("ColorUniformPoint.vert", "Color.frag"));
    redMat->program()->addUniform1f(new SLGLUniform1f(UT_const, "u_pointSize", 3.0f));
    greenMat = new SLMaterial(SLCol4f::GREEN, "Green");
    greenMat->program(new SLGLGenericProgram("ColorUniformPoint.vert", "Color.frag"));
    greenMat->program()->addUniform1f(new SLGLUniform1f(UT_const, "u_pointSize", 5.0f));
    blueMat = new SLMaterial(SLCol4f::BLUE, "Blue");
    blueMat->program(new SLGLGenericProgram("ColorUniformPoint.vert", "Color.frag"));
    blueMat->program()->addUniform1f(new SLGLUniform1f(UT_const, "u_pointSize", 4.0f));
    SLMaterial* yellow = new SLMaterial("mY", SLCol4f(1, 1, 0, 0.5f));
    SLfloat     l = 0.593f, b = 0.466f, h = 0.257f;
    //SLBox*      box1     = new SLBox(0.0f, 0.0f, 0.0f, l, h, b, "Box 1", yellow);
    SLBox*  box1     = new SLBox(0.0f, 0.0f, 0.0f, 0.355f, 0.2f, 0.1f, "Box 1", yellow);
    SLNode* boxNode1 = new SLNode(box1, "boxNode1");
    //boxNode1->rotate(-45.0f, 1.0f, 0.0f, 0.0f);
    //boxNode1->translate(10.0f, -5.0f, -0.1f);
    boxNode1->translate(0.316, -1.497f, -0.1f);
    SLBox*  box2     = new SLBox(0.0f, 0.0f, 0.0f, 0.355f, 0.2f, 0.1f, "Box 2", yellow);
    SLNode* boxNode2 = new SLNode(box2, "boxNode2");
    SLNode* axisNode = new SLNode(new SLCoordAxis(), "axis node");
    SLBox*  box3     = new SLBox(0.0f, 0.0f, 0.0f, 1.745f, 0.745, 0.81, "Box 2", yellow);
    SLNode* boxNode3 = new SLNode(box3, "boxNode3");
    boxNode3->translate(2.561f, -5.147f, -0.06f);

    //boxNode->addChild(axisNode);

    covisibilityGraphMat = new SLMaterial("YellowLines", SLCol4f::YELLOW);
    spanningTreeMat      = new SLMaterial("GreenLines", SLCol4f::GREEN);
    loopEdgesMat         = new SLMaterial("RedLines", SLCol4f::RED);

    //make some light
    light1->ambient(SLCol4f(0.2f, 0.2f, 0.2f));
    light1->diffuse(SLCol4f(0.8f, 0.8f, 0.8f));
    light1->specular(SLCol4f(1, 1, 1));
    light1->attenuation(1, 0, 0);

    cameraNode->translation(0, 0, 0.1f);
    cameraNode->lookAt(0, 0, 0);
    //for tracking we have to use the field of view from calibration
    cameraNode->clipNear(0.001f);
    cameraNode->clipFar(1000000.0f); // Increase to infinity?
    cameraNode->setInitialState();

    mapNode->addChild(mapPC);
    mapNode->addChild(mapMatchedPC);
    mapNode->addChild(mapLocalPC);
    mapNode->addChild(mapMarkerCornerPC);
    mapNode->addChild(keyFrameNode);
    mapNode->addChild(covisibilityGraph);
    mapNode->addChild(spanningTree);
    mapNode->addChild(loopEdges);
    mapNode->addChild(cameraNode);

    mapNode->rotate(180, 1, 0, 0);

    //setup scene
    rootNode->addChild(light1);
    rootNode->addChild(boxNode1);
    rootNode->addChild(boxNode2);
    rootNode->addChild(boxNode3);
    rootNode->addChild(mapNode);
}
