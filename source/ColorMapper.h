#pragma once

#include <G3D/G3DAll.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>
#include <set>

#include "SceneFile.h"

/*
Change Log:
- file created by Melanie, Yitong, Kenny, and Cole (midterm)
*/

/** Helper fract function for vectors, which converts each value to something between 0 and 1. **/
static Vector2 fract(const Vector2& x) {
    return Vector2(x.x - floor(x.x), x.y - floor(x.y));
};

/** Helper floor function for vectors. **/
static Vector2 floor(const Vector2& x) {
    return Vector2(floor(x.x), floor(x.y));
};

/** Helper fract function, which converts each value to someting between 0 and 1. **/
static float fract(float x) {
    return x - floor(x);
};

/**  Hash 2 floats to 1 float. **/
static float Hash12(Vector2 p) {
	p  = fract(p / Vector2(3.07965, 7.4235));
    const float t = p.xy().dot(Vector2(p.y + 19.19f, p.x + 19.19f));
    p.x += t;
    p.y += t;
    return fract(p.x * p.y);
};

/** A class which implements the continues coloring function demonstrated by Shader Toy user Dave_Hoskins (https://www.shadertoy.com/view/4slGD4). **/
class ColorMapper {
protected:

    /** A noise function. **/
    float noise(const Vector2& x);

public:

    ColorMapper();

    /** Returns the color for the vertex with the given position and normal. **/
    Color3 getColor(const Vector3& pos, const Vector3& normal);
};