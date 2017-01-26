#include "Mesh.h"
#include <sstream>

/*
Change Log:
- File created by Melanie, Kenny, Yitong, and Cole (midterm)
*/

Color3 Mesh::thresholdColoring(float y, float flatness) {
    if (y >= .5 && flatness >= .75) return Color3(1,0.980392,0.941176); // Snow-capped peaks
    else if ( y >= .5 && flatness >= .5) return Color3(0.972549,0.972549,1); // Lighter grey mountain top
    else if (y >= .5) return Color3(0.439216,0.501961,0.564706); // Grey mountain top
    else if (y >= .3) return Color3(0.627451,0.321569,0.176471); // Brown mountain side
    else return Color3(0.133333,0.545098,0.133333); // Green grass
}

Color3 Mesh::continuousColoring(const Vector3& pos, const Vector3& normal) {
    ColorMapper mapper = ColorMapper();
    return mapper.getColor(pos, normal);
}

// Adds all points with 1 m spacing between point1 and point2 to triangleSidePositions.
// Used by setSheep() to keep sheep from being added on to the sides of triangles.
static void setTriangleSides(const Point3& point1, const Point3& point2, Array<Point3>& triangleSidePositions) {
    int len = (point2 - point1).length();
    for (int i = 0; i <= len; ++i) {
        triangleSidePositions.append(point1 + (point2 - point1) * i / len);
    }
}

// Thank you Prof. Morgan McGuire for this awesome function!
// Returns the CFrame needed to transform an object at position to stand 
// along the normal and spins the object along the Y axis randomly
static CFrame computeSceneObjectYPR(const Vector3& normal, const Point3& position) {    
    Vector3 x, y, z;
    y = normal;
    y.getTangents(z, x);
    Matrix3 matrix(Matrix3::fromColumns(x, y, z) * Matrix4::yawDegrees(Random::common().uniform(0, 360)).upper3x3());
    CFrame cFrame(matrix, position);
    return cFrame;
}

void Mesh::addClouds(int n) {
    for (int i = 0; i < n; ++i) {
        sceneFile->addCloud(
            Vector3(
                // Pick a random X and Z, and make Y 2x the max mountain height
                uniformRandom(0, xScale * image->width()),
                yScale * 2.0f,
                uniformRandom(0, zScale * image->height())
            ),
            uniformRandom(-180.0f, 180.0f)
        );
    }
}

void Mesh::addSheep(const Triangle& triangle) {
    const int totalSheep = triangle.area() / areaPerSheep;
    Point3 sheepPosition;
    Array<Point3> sheepPositions;
    Array<Point3> sidePositions; 
    const Vector3 normal(triangle.normal().direction());
    Random& rng = Random::threadCommon();
    const float sheepHeight = 0.0f; 
    const float sheepPersonalSpace = 1;
    const float sheepPersonalSpaceSquared = sheepPersonalSpace * sheepPersonalSpace;

    // Determine the possible tree positions so that sheep can avoid them
    setTriangleSides(triangle.vertex(0), triangle.vertex(1), sidePositions);
    setTriangleSides(triangle.vertex(1), triangle.vertex(2), sidePositions);
    setTriangleSides(triangle.vertex(2), triangle.vertex(0), sidePositions);

    for (int i = 0; i < totalSheep; ++i) {
        triangle.getRandomSurfacePoint(sheepPosition);

        // All sheep will be positioned 1/2 sheepHeight above the ground
        sheepPosition += normal * sheepHeight / 2;

        bool tooClose(false);

        // Check if sheepPosition is within the personal space bound of any existing sheep.
        for (int j = 0; j < sheepPositions.size(); ++j) {
            const Vector3 difference = sheepPositions[j] - sheepPosition;
            if (difference.dot(difference) < 2 * sheepPersonalSpaceSquared) {
                tooClose = true;
                break;
            }
        }

        // Check if the sheep might be too close to a tree
        for (int j = 0; j < sidePositions.size(); ++j) {
            const Vector3 difference = sidePositions[j] - sheepPosition;
            if (difference.dot(difference) < sheepPersonalSpaceSquared) {
                tooClose = true;
                break;
            }
        }

        // Add the sheep with the proper normal and a random rotation if it has a valid position!
        if (!tooClose) {
            float vecX, vecY, vecZ, yaw, pitch, roll;
            CFrame cFrame(computeSceneObjectYPR(normal, sheepPosition));
            cFrame.getXYZYPRDegrees(vecX, vecY, vecZ, yaw, pitch, roll);

            sceneFile->addSheep(sheepPosition, yaw, pitch, roll);
            sheepPositions.append(sheepPosition);
        }
    }
}

void Mesh::getColor(const shared_ptr<Image>& colormap, int terrainSize, const Vector3& triangleCenter, const Vector3& normal, Color3& color, shared_ptr<Image> biome, const Triangle& triangle, const int a, const int b, const int c) {
    // Scale and floor triangle coordinates to image coordinates
    int x = int(triangleCenter.x / terrainSize * colormap->width());
    int y = int(triangleCenter.z / terrainSize * colormap->height());

    // Check if this is a field
    if (sample(biome,int(triangleCenter.x),int(triangleCenter.z)) == 0  && normal.y >= .75) {
        // Retrieve color from generated Voronoi map
        colormap->get(Point2int32(x, y), color);
        
        // Add sheep if the field is the right color
        if (0.19 < color.r && color.r < 0.20 &&
            0.80 < color.g && color.g < 0.81 &&
            0.19 < color.b && color.b < 0.20) {
            addSheep(triangle);
        }

        // Add trees
        const Vector3 normal = triangle.normal().direction();
        addTrees(a, b, color, normal);
        addTrees(b, c, color, normal);
        addTrees(a, c, color, normal);
    } else {
        // Use the continuous coloring function for the rest of the terrain
        color = continuousColoring(triangleCenter, normal);
    }

    // Round the color values
    const float redRounded = floorf(color.r * 100) / 100;
    const float blueRounded = floorf(color.g * 100) / 100;
    const float greenRounded = floorf(color.b * 100) / 100;
    color = Color3(redRounded, blueRounded, greenRounded);
}

void Mesh::addTrees(const int a, const int b, Color3 color, const Vector3& normal) {
    // These edges are the same so we need to manipulate them both
    Edge key1 = Edge(a, b);
    Edge key2 = Edge(b, a);

    if (fieldEdges.find(key1) == fieldEdges.end()) {
        // If the edge has not been seen before then add the current field color
        fieldEdges[key1] = color;
        fieldEdges[key2] = color;
    } else if (fieldEdges[key1] != color) {
        // If the edge has been seen before touching a different color then add trees along this edge and remove the edge (it's been processed)
        lerpTrees(vertices[a], vertices[b], normal);
        fieldEdges.erase(key1);
        fieldEdges.erase(key2);
    } else {
        // If the edge has been seen before touching the same color then remove the edge (it's been processed)
        fieldEdges.erase(key1);
        fieldEdges.erase(key2);
    }
}

// Add trees (randomly spaced between 3-8 meters) between point1 and point2. All trees will be pointed along normal.
void Mesh::lerpTrees(Point3 point1, Point3 point2, const Vector3& normal) {
    Random& rng = Random::threadCommon();
    int len = (point2 - point1).length();
    int i = 0;

    // Randomly space the trees 3-8 meters apart and orient them along the correct normal
    while(i < len) {
        Point3 position = point1 + (point2 - point1) * i / len;
        CFrame cFrame(computeSceneObjectYPR(normal, position));
        float vecX, vecY, vecZ, yaw, pitch, roll;
        cFrame.getXYZYPRDegrees(vecX, vecY, vecZ, yaw, pitch, roll);
        sceneFile->addTree(position, yaw, pitch, roll);
        i += rng.uniform(3,8);
    }
}

Mesh::Mesh(shared_ptr<Image>& _image, 
   shared_ptr<SceneFile>& _sceneFile,
                       float _xScale,
                       float _yScale,
                       float _zScale,
            int _numRandomSampledPts,
        int _numImportanceSampledPts,
       bool _addPointsAboveThreshold,
         float _interestingThreshold,
                    int _terrainSize,
                    int _terrainGran,
                 float _terrainPower,
                  int _terrainOffset,
                  float _areaPerSheep,
         shared_ptr<Image> _colormap) : image(_image), sceneFile(_sceneFile), colormap(_colormap) {
     xScale = _xScale;
     yScale = _yScale;
     zScale = _zScale;
     numRandomSampledPts = _numRandomSampledPts;
     numImportanceSampledPts = _numImportanceSampledPts;
     addPointsAboveThreshold = _addPointsAboveThreshold;
     interestingThreshold = _interestingThreshold;
     terrainSize = _terrainSize;
     terrainGran = _terrainGran;
     terrainPower = _terrainPower;
     terrainOffset = _terrainOffset; 
     areaPerSheep = _areaPerSheep;
}

void Mesh::savePythonInput(const std::string& fileName, Array<Vector3>& sampledVertices) {
    std::ofstream file;
    file.open(fileName);

    // Vertices
    for (auto it = sampledVertices.begin(); it != sampledVertices.end(); ++it) {
        file << format("%f %f\n", it->x, it->z).c_str();
    }

    file.close();
}

void Mesh::saveMtl(const std::string& fileName) const {
    std::ofstream file;
    file.open(fileName);

    for (auto entry = indexGroups.begin(); entry != indexGroups.end(); ++entry) {
        Color3& color(entry->key);
        file << "newmtl " << color.hashCode() << "\n";
        file << format("Kd %f %f %f\n", color.r, color.g, color.b).c_str();
        file << "Ks 0 0 0\n";
        file << "illum 0\n";
    }

    file.close();
}

void Mesh::saveObj(const std::string& fileName, const std::string& mtlFileName) const {
    std::ofstream file;
    file.open(fileName);
    file << "mtllib " << FilePath::baseExt(String(mtlFileName)).c_str() << "\n\n";

    // Vertices
    for (auto it = vertices.begin(); it != vertices.end(); ++it) {
        file << format("v %f %f %f\n", it->x, it->y, it->z).c_str();
    }

    // Vertice normals
    for (auto it = normals.begin(); it != normals.end(); ++it) {
        file << format("vn %f %f %f\n", it->x, it->y, it->z).c_str();
    }

    // Faces
    for (auto entry = indexGroups.begin(); entry != indexGroups.end(); ++entry) {
        Array<VertexNormalIndexPair> indices = entry->value;
        file << "\nusemtl " << entry->key.hashCode();
        for (int i = 0; i < indices.size(); ++i) {
            if (i % 3 == 0) {
                file << "\nf";
            }
            file << " " << (indices[i].vertexIndex + 1) << "//" << (indices[i].normalIndex + 1);
        }
    }

    file << "\n";
    file.close();
}

void Mesh::saveObjMtlFiles(const std::string& objFileName, const std::string& mtlFileName, const std::string& pythFileName) {
    computeNormals();
    saveObj(objFileName, mtlFileName);
    saveMtl(mtlFileName);
}

/** Helper function for retrieving heightmap height from an image pixel.
 */
static float getHeight(const shared_ptr<Image>& image, int x, int y) {
    Color3 color;
    image->get(Point2int32(x, y), color);
    return color.average();
}

// Compute the normal of a face
static Vector3 getAdjacentNormal(const shared_ptr<Image>& image, int x, int y1, int x1, int y, float xScale, float yScale, float zScale) {
    Vector3 V1 = Point3(0,getHeight(image,x,y1)*yScale,zScale) - Point3(0,getHeight(image,x,y)*yScale,0); 
    Vector3 V2 = Point3(xScale,getHeight(image,x1,y)*yScale,0) - Point3(0,getHeight(image,x,y)*yScale,0);
    return (V1.cross(V2)).direction();
}

/** returns a float (between 0 and 1) representing the inverse influence of point (x,y).
The less interesting the point, the higher the returned float.
inverse influence is computed from the difference in slope of point (x,y) and the points surrounding (x,y).
Used by importance Sampling. */
static float assessInverseInfluenceFloat(const shared_ptr<Image>& image, int x, int y, float xScale, float yScale, float zScale) {
       // Adjacent faces' normals
       Vector3 N1 = getAdjacentNormal(image, x, y+1, x+1, y, xScale, yScale, zScale);
       Vector3 N2 = getAdjacentNormal(image, x, y+1, x-1, y, xScale, yScale, zScale);
       Vector3 N3 = getAdjacentNormal(image, x, y-1, x-1, y, xScale, yScale, zScale);
       Vector3 N4 = getAdjacentNormal(image, x, y-1, x+1, y, xScale, yScale, zScale);

       // Evaluate the similarity between normals of neighboring faces
       const float rating(N1.dot(N2) * N2.dot(N3) * N3.dot(N4) * N4.dot(N1) + 0.1f);
       return rating * rating * rating;
}

/** Compares slopes of point (x,y) to the points around it. Returns true if difference in slope is above pickinessThreshold. */
static bool assessInfluence(const shared_ptr<Image>& image, int x, int y, float xScale, float yScale, float zScale, float pickinessThreshold) {
       // Adjacent faces' normals
       Vector3 N1 = getAdjacentNormal(image, x, y+1, x+1, y, xScale, yScale, zScale);
       Vector3 N2 = getAdjacentNormal(image, x, y+1, x-1, y, xScale, yScale, zScale);
       Vector3 N3 = getAdjacentNormal(image, x, y-1, x-1, y, xScale, yScale, zScale);
       Vector3 N4 = getAdjacentNormal(image, x, y-1, x+1, y, xScale, yScale, zScale);

       // Compare if neighboring faces have normals that are similar enough based on the pickinessThreshold
       return (N1.dot(N2) <= pickinessThreshold && N2.dot(N3) <= pickinessThreshold) && (N3.dot(N4) <= pickinessThreshold && N4.dot(N1) <= pickinessThreshold);
}


void Mesh::computeNormals() {
    for (auto entry = indexGroups.begin(); entry != indexGroups.end(); ++entry) {
        Array<VertexNormalIndexPair>& indices(entry->value);
        for (int i = 0; i < indices.size(); i += 3) {
            const Vector3& a(vertices[indices[i].vertexIndex]);
            const Vector3& b(vertices[indices[i + 1].vertexIndex]);
            const Vector3& c(vertices[indices[i + 2].vertexIndex]);
            
            const int normalIndex = normals.size();
            normals.append(Triangle(a, b, c).normal());

            indices[i].normalIndex = normalIndex;
            indices[i + 1].normalIndex = normalIndex;
            indices[i + 2].normalIndex = normalIndex;
        }
    }
}

void Mesh::setSample(shared_ptr<Image> heights, int x, int y, float value){
    heights->set(Point2int32(x,y),Color3(value));
}

float Mesh::sample(shared_ptr<Image> heights, int x, int y){
    Color3 c;
    heights->get(Point2int32(x,y),c);
    return c.average();
}

shared_ptr<Image> Mesh::genWaterDist(int n){
    shared_ptr<Image> water;
    int width, height;
    width = height = n;
    water = Image::create(width,height,ImageFormat::RGB32F());    
    water->setAll(Color3::black());
    Random& rng = Random::threadCommon();
    int c = rng.integer(-1 * pow(2,16), pow(2,16));
    for(int i = 0; i <width; ++i){
        for(int j = 0; j<height; ++j){
            int nx = i <<9;
            int ny = j <<9;
            float y = Noise::common().sampleFloat(nx + c, ny + c, c, 1);
            //1 is set to be where water should be, .25 indicates hills, which is used as boundary between water and other land forms (i.e. farms and mountains), 0 means the biome should be defined in the genBiomeDist function
            if( y < -.25){
              setSample(water,i,j, 1);
            } else if ( y < -.2){
                setSample(water,i,j,.25);
            } else {
                setSample(water,i,j,0);
            }
        }
    }
    return water;
}

void Mesh::genBiomeDist(shared_ptr<Image> water, int n){
    int width, height;
    width = height = n;
    Random& rng = Random::threadCommon();
    int c = rng.integer(-1 * pow(2,16), pow(2,16));
    for(int i = 0; i <width; ++i){
        for(int j = 0; j<height; ++j){
            //if genWaterDist has already decided biome then no further action is needed
            if(sample(water,i,j) <.05){
                int nx = i <<9;
                int ny = j <<9;
                float y = Noise::common().sampleFloat(nx, ny, c, 1);
                // 0 indicates farmland, .25 indicates rolling hills, .5 is foothills, and .75 is mountains
                // Because of this construction farmland should never be placed directly next to mountains which makes sense
                if( y < -.05){
                    setSample(water,i,j, 0);
                }else if ( y < .05){
                    setSample(water,i,j,.25);
                } else if ( y < .1){
                   setSample(water,i,j,.5);  
                } else {
                    setSample(water,i,j,.75);
                }
            }
        }
    }
}

// Inspiration for the specifics of this algorithm taken from http://www.redblobgames.com/maps/terrain-from-noise/
// Explanation of noise function paramters:
// gran explains how many noise function calls will be made for a single spot, in this way raising gran should increase the smoothness of the function to a curve
// power explains the exponent that each noise variable will be raised to, as we tranpose the variables onto the scale 0,1, this will push numbers to the extremes of this range as it goes higher
// offset explains how gradaul the underlying noise function will act, it will be most noisy with an offset of 16 and least noisy with an offset of 1
shared_ptr<Image> Mesh::genPerlinTerrain(int n, int gran, float power, int offset, shared_ptr<Image> biome){
    shared_ptr<Image> heights;
    int width, height;
    width = height = n;
    heights = Image::create(width,height,ImageFormat::RGB32F());    
    heights->setAll(Color3::black());
    Random& rng = Random::threadCommon();
    int c = rng.integer(-1 * pow(2,16), pow(2,16));
    for(int i = 0; i <width; ++i){
        for(int j = 0; j<height; ++j){
            int nx = i <<offset;
            int ny = j <<offset;
            float y = 0;
            for(int k = 2; k <= gran+1; k++){
                y += (1/pow(2,k-2)) * Noise::common().sampleFloat(pow(2,k-2) * nx, pow(2,k-2) * ny, c, 1);
            }
            // we don't want y values outside of these bounds
            y = clamp(y, -1.0f, 1.0f);
            // we distinguish between the different biomes by muting the y values differently
           if (sample(biome,i,j) == 0) {
                //we add a +.1 so that the farmland is always on top of the water plane
                setSample(heights,i,j, pow((y+1)/8,power)+.1);
            } else if (sample(biome,i,j) < .30){
                //hills
                setSample(heights,i,j, pow((y+1)/4,power));
            } else if (sample(biome,i,j) < .55){
                //foothills
                setSample(heights,i,j, pow((y+1)/2.5,power));
            } else if (sample(biome,i,j) < .80){
                //mountains
                setSample(heights,i,j, pow((y+1)/1.5,power));
            } else {
                //water
                // we have a seperate water plane so we just want this part of the terrain map to be lower than it (hence the -1)
               setSample(heights,i,j, pow((y+1)/4,power)-1);
            }
        }
    }
    return heights;
}

/** Importance sampling that chooses less interesting points more often to fill in numVertices vertices into the Array<Vector3> vertices argument */
static void importanceSampling(const shared_ptr<Image>& image, int numVertices, float xScale, float yScale, float zScale, Array<Vector3>& vertices) {
    float totalInterest = 0.0f;
    Array<float> interestRatings;

    for (int z = 1; z < image->height() - 1; ++z) {
        for (int x = 1; x < image->width() - 1; ++x) {
            const float interest = assessInverseInfluenceFloat(image, x, z, xScale, yScale, zScale);
            totalInterest += interest;
            interestRatings.append(interest);
        }
    }

    Random& rng = Random::threadCommon();
    for (int vertex = 0; vertex < numVertices; ++vertex) {
        float r = totalInterest * rng.uniform();

        for (int i = 0; i < image->height() * image->width(); ++i) {
            r -= interestRatings[i];
            if (r < 0) {
                // "+1" b/c gotta account for the fact that none of the border vertices are considered
                const int z = (i / image->width()) + 1; 
                const int x = (i % image->width()) + 1;
                const Vector3 v = Vector3(xScale * x, yScale * getHeight(image, x, z), zScale * z);
                if (!vertices.contains(v)) {
                    vertices.append(v);
                }
                break;
            }
        }
    }
}

/** Randomly fills in numVertices vertices into the Array<Vector3> vertices argument*/
static void randomSampling(const shared_ptr<Image>& image, int numVertices, float xScale, float yScale, float zScale, Array<Vector3>& vertices) {
    Random& rng = Random::threadCommon();
    for (int vertex = 0; vertex < numVertices; ++vertex) {
        const int r = image->width() * image->height() * rng.uniform();
        const int z = r / image->width();
        const int x = r % image->width();
        const Vector3 v = Vector3(xScale * x, yScale * getHeight(image, x, z), zScale * z);
        if (!vertices.contains(v)) {
            vertices.append(v);
        }
    }
}


/* Adds interesting points into vertices array based on assesInfluence and the interestingThreshold set in the GUI */
void Mesh::thresholdSampling(const shared_ptr<Image>& image, float interestingThreshold, float xScale, float yScale, float zScale, Array<Vector3>& vertices){
    for (int y = 1; y < image->height() - 1; ++y) {
        for (int x = 1; x < image->width() - 1; ++x) {
            float Xbump = (rand() % 25) * xScale / 35;
            float Zbump = (rand() % 25) * zScale / 35;
            if (assessInfluence(image, x, y, xScale, yScale, zScale, interestingThreshold)) {   
                vertices.append(Vector3(xScale * x + Xbump, yScale * getHeight(image, x, y), zScale * y + Zbump));
            }
        }
    }
}

void Mesh::generateDelaunayTriangulationMesh(shared_ptr<Image> biome, Array<Vector3>& vertices){
    int a, b, c;
    std::ifstream infile("model/out.txt");
    while (infile >> a >> b >> c)
    {
        const Triangle triangle(vertices[a], vertices[b], vertices[c]);
        const float xValue = (triangle.vertex(0).x + triangle.vertex(1).x + triangle.vertex(2).x) / 3;
        const float yValue = (triangle.vertex(0).y + triangle.vertex(1).y + triangle.vertex(2).y) / 3;
        const float zValue = (triangle.vertex(0).z + triangle.vertex(1).z + triangle.vertex(2).z) / 3;
        
        const Vector3 normal = triangle.normal();

        Color3 key;
        getColor(colormap, terrainSize, Vector3(xValue, yValue, zValue), normal.direction(), key, biome, triangle, a, b, c);
        Array<VertexNormalIndexPair>& indices(indexGroups.getCreate(key));

        indices.append(VertexNormalIndexPair(a)); 
        indices.append(VertexNormalIndexPair(b)); 
        indices.append(VertexNormalIndexPair(c)); 
    }
}


void Mesh::genHeightField() {
    // Generate the biome by first creating water areas and then breaking the remaining land into mountains and farm
    shared_ptr<Image> biome = genWaterDist(terrainSize);
    genBiomeDist(biome,terrainSize);

    // From the biome information, create an image using perlin noise that will get turned into a heightmap
    image = genPerlinTerrain(terrainSize, terrainGran, terrainPower, terrainOffset, biome);

    // Save terrain to an image so if it is good we can use it again
    image->convert(ImageFormat::RGB8());
    image->save("terrainGenerated.png");

    // Sample points for mesh simplification as specified in the GUI
    if (addPointsAboveThreshold) {
        thresholdSampling(image, interestingThreshold,xScale, yScale, zScale, vertices);
    }
    randomSampling(image, numRandomSampledPts, xScale, yScale, zScale, vertices);
    importanceSampling(image, numImportanceSampledPts, xScale, yScale, zScale, vertices);

    // Construct the Delauney triangulation for the sampled vertices
    savePythonInput("model/input.txt", vertices);
    //Call Bill Simons' 2009 Delaunay triangulator (https://svn.osgeo.org/qgis/trunk/qgis/python/plugins/fTools/tools/voronoi.py)
    system("\"C:/Python27/python.exe\" ../scripts/voronoi.py -t model/input.txt > model/out.txt");
    
    fieldEdges = std::map<Edge, Color3, cmpEdges>();
    
    generateDelaunayTriangulationMesh(biome,vertices);
    
    // Add trees to any edges that bordered mountains/water
    for (std::map<Edge, Color3>::iterator it = fieldEdges.begin(); it != fieldEdges.end(); ++it) {
        Edge edge = it->first;
        lerpTrees(vertices[edge.ind1], vertices[edge.ind2]);
    }

    // Specify amount of clouds for scene
    addClouds(10);
}
