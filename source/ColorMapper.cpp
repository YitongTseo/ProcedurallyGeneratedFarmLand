#include "ColorMapper.h"

/*
Change Log:
- code based on Shader Toy user Dave_Hoskins (https://www.shadertoy.com/view/4slGD4)
- file created by Melanie, Yitong, Kenny, and Cole (midterm)
*/

ColorMapper::ColorMapper() {}

float ColorMapper::noise(const Vector2& x) {
    Vector2 p(floor(x));
    Vector2 f(fract(x));
    f = f * f;
    Vector2 add(1.0, 0.0);
    float res = lerp(lerp( Hash12(p),          Hash12(p + add.xy()),f.x),
                     lerp( Hash12(p + add.yx()), Hash12(p + add.xx()),f.x), f.y);
    return res;
}

Color3 ColorMapper::getColor(const Vector3& pos, const Vector3& normal) {
	Vector3 mat;
	float specular(.0);
	float ambient(.1);
    const Vector3 matPos(pos * 12.0f); //bump this to make snow caps

	float f = clamp(noise(matPos.xz()*.05f), 0.0f, 1.0f);
	f += noise(matPos.xz()*.1+normal.yz()*1.08)*.85;
	f *= .65;
	Vector3 m = lerp(Vector3(.63*f+.2, .7*f+.1, .7*f+.1), Vector3(f*.43+.1, f*.3+.2, f*.35+.1), f*.65);
	mat = m * Vector3(f*m.x+.36, f*m.y+.30, f*m.z+.28);

	// Should have used smoothstep to add colours, but left it using 'if' for sanity...
	if (normal.y < .5) {
		float v = normal.y;
		float c = (.5-normal.y) * 4.0;
		c = clamp(c*c, 0.1f, 1.0f);
		f = noise(Vector2(matPos.x*.09, matPos.z*.095f + matPos.y*0.15));
		f += noise(Vector2(matPos.x*2.233, matPos.z*2.23))*0.5;
		mat = lerp(mat, Vector3(0.4f*f, 0.4f*f, 0.4f*f), c);
		specular+=.1;
	}

	// Grass. Use the normal to decide when to plonk grass down...
	if (matPos.y < 45.35 && normal.y > .65) {

		m = Vector3(noise(matPos.xz()*.023)*.5+.15, noise(matPos.xz()*.03)*.6+.25, 0.0);
		m *= (normal.y- 0.65)*.6;
		mat = lerp(mat, m, clamp((normal.y-.65)*1.3 * (45.35-matPos.y)*0.1, 0.5, 1.0));
    }

	// Snow topped mountains...
	if (matPos.y > 80.0 && normal.y > .42) {
		float snow = clamp((matPos.y - 80.0 - noise(matPos.xz() * .1)*28.0) * 0.035, 0.0, 1.0);
		mat = lerp(mat, Vector3(1.0f, 1.0f, 1.0f), snow);
		specular += snow;
		ambient+=snow *.3;
	}
    
	// Beach effect...
	if (matPos.y < 1.45) {
		if (normal.y > .4) {
			f = noise(matPos.xz() * .084)*1.5;
			f = clamp((1.45-f-matPos.y) * 1.34, 0.0, .67);
			float t = (normal.y-.4);
			t = (t*t);
			mat = lerp(mat, Vector3(.09+t, .07+t, .03+t), f);
		}
		// Cheap under water darkening...it's wet after all...
		if (matPos.y < 0.0) {
			mat *= .5;
		}
	}
	return Color3(mat.x, mat.y, mat.z);
}
