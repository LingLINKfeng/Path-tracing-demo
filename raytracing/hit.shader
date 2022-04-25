#shader vertex
#version 450 core

layout(location = 0) in vec3 dir;
layout(location = 1) in vec2 pos;

out vec3 direction;
out vec3 origin;
out vec2 point;
out vec2 wnh;

uniform vec3 eye;
uniform vec2 screen;
void main()
{
	wnh = screen;
	direction = dir;
	origin = eye;
	point = pos;
	gl_Position = vec4(2 * (pos[0] + 0.5f) / screen[0] - 1.0f, 2 * (pos[1] + 0.5f) / screen[1] - 1.0f, 0.0f, 1.0);
}
#shader fragment
#version 450 core
out vec4 FragColor;
in vec3 direction;
in vec3 origin;
in vec2 point;
in vec2 wnh;


uniform samplerBuffer tria;
uniform samplerBuffer rand;
uniform int triangleNum;

vec3 colorBuffer[gl_MaxVertexUniformVectors];
struct Ray {
	vec3 ori;
	vec3 dir;
	vec3 invDir;
	vec3 dirIsNeg;
};
struct BVH {
	vec3 pMin;
	vec3 pMax;
};
struct Inter {
	vec3 normal;
	vec3 position;
	float distanceI;
	bool isIntersect;
	vec3 color;
	bool isLight;
	float pdf;
};
struct Triangle {
	vec3 a;
	vec3 b;
	vec3 c;
	vec3 aNormal;
	vec3 bNormal;
	vec3 cNormal;
	vec3 color;
	bool isLight;
};
float random(int iterationTime, int index) {
	return texelFetch(rand, int((point.y*wnh[0] + point.x) * 28 + 7 * iterationTime + index)).r;
}
Inter intersectT(Ray ray, Triangle tria) {
	Inter inter;
	inter.color = vec3(0);
	inter.isIntersect = false;
	inter.distanceI = -7;
	vec3 E1, E2, S1, S2, S;
	float det, u, v, t_tmp = 0;
	vec3 normal = cross(tria.c - tria.a, tria.b - tria.a);
	E1 = tria.b - tria.a;
	E2 = tria.c - tria.a;
	S = ray.ori - tria.a;
	S1 = cross(-ray.dir, E2);
	S2 = cross(S, E1);
	det = dot(E1, S1);
	//Check if it's faceing the camera
	if (dot(ray.dir, normal) >= 0)
		return inter;
	//Check if it's parallel
	if (abs(dot(normalize(E1), normalize(S1))) < 0.0001)
		return inter;
	//intersection point out of triangle
	u = dot(S, S1) / det;
	v = dot(-ray.dir, S2) / det;
	t_tmp = -dot(E2, S2) / det;
	if (u < 0 || u > 1)
		return inter;
	if (v < 0 || u + v > 1)
		return inter;
	if (t_tmp < 0)
		return inter;
	inter.isIntersect = true;
	inter.position = tria.a*(1 - u - v) + tria.b*u + tria.c*v;
	inter.normal = normalize(tria.aNormal*(1 - u - v) + tria.bNormal*u + tria.cNormal*v);
	inter.distanceI = length(ray.ori - inter.position) / 2;
	inter.color = tria.color;
	inter.isLight = tria.isLight;
	return inter;
}

vec3 eval(vec3 wo, vec3 N, vec3 Kd) {
	vec3 diffuse = vec3(0);
	float cosalpha = dot(N, wo);
	if (cosalpha >= 0.0f) {
		diffuse = (1 / 3.1415926535)*Kd;
	}
	return diffuse;
}
float triangleArea(vec3 a, vec3 b, vec3 c) {
	return length(cross(c - a, b - a))*0.5;
}
Inter sampleLightArea(Inter pos, Triangle trian, int iterationTime) {
	Inter inter;
	float x = sqrt(random(iterationTime, 1)),
		y = random(iterationTime, 2);
	vec2 result;
	inter.color = trian.color;
	inter.position = trian.a * (1.0f - x) + trian.b * (x * (1.0f - y)) + trian.c * (x * y);
	inter.normal = normalize(cross(trian.c - trian.a, trian.b - trian.a));
	inter.isLight = true;
	inter.pdf = 0.5f / abs(triangleArea(trian.a, trian.b, trian.c));
	return inter;
}
Inter sampleLight(Inter pos, int iterationTime) {
	float emit_area_sum = 0;
	float p;
	float pdf;
	Triangle trian;
	Inter inter;
	inter.color = vec3(0);
	inter.isIntersect = false;
	inter.distanceI = -1;
	for (int idx = 0; idx < triangleNum; ++idx) {
		trian = Triangle(texelFetch(tria, idx * 8).rgb, texelFetch(tria, idx * 8 + 1).rgb,
			texelFetch(tria, idx * 8 + 2).rgb, texelFetch(tria, idx * 8 + 3).rgb,
			texelFetch(tria, idx * 8 + 4).rgb, texelFetch(tria, idx * 8 + 5).rgb,
			texelFetch(tria, idx * 8 + 6).rgb, texelFetch(tria, idx * 8 + 7).r > 0);
		if (trian.isLight) {
			inter = sampleLightArea(pos, trian, iterationTime);
			inter.distanceI = length(inter.position - pos.position);
			break;
		}
	}
	return inter;
}
Inter normMethod(Ray ray) {
	Inter inter = Inter(vec3(0), vec3(0), -1, false, vec3(0), false, 0);
	float minDistance = -1;
	Inter judge;
	Triangle trian;
	for (int idx = 0; idx < triangleNum; idx++) {
		trian = Triangle(texelFetch(tria, idx * 8).rgb, texelFetch(tria, idx * 8 + 1).rgb,
			texelFetch(tria, idx * 8 + 2).rgb, texelFetch(tria, idx * 8 + 3).rgb,
			texelFetch(tria, idx * 8 + 4).rgb, texelFetch(tria, idx * 8 + 5).rgb,
			texelFetch(tria, idx * 8 + 6).rgb, texelFetch(tria, idx * 8 + 7).r > 0);
		judge = intersectT(ray, trian);
		if (judge.isIntersect && (judge.distanceI < minDistance || minDistance < 0)) {
			inter = judge;
			minDistance = judge.distanceI;
		}
	}
	return inter;
}
vec3 lightResult(Ray ray, Inter p, int iterationTime) {
	Inter lightInter;
	vec3 ws;
	vec3 emit = 8.0f*vec3(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) + 15.6f * 
		vec3(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4f *vec3(0.737f + 0.642f, 0.737f + 0.159f, 0.737f);
	vec3 lightColor = vec3(0, 0, 0);
	Inter test;
	Triangle tria1;
	Ray raylight;
	lightInter = sampleLight(p, iterationTime);
	ws = normalize(lightInter.position - p.position);
	raylight = Ray(p.position, ws, 1 / ws, sign(ws));
	test = normMethod(raylight);
	if (test.distanceI >= lightInter.distanceI - 0.00001 || test.isLight) {
		lightColor = emit * eval(ws, p.normal, p.color)
			*dot(normalize(-ws), normalize(lightInter.normal)*(-1)
				*dot(normalize(-ws), normalize(p.normal)))
			/ lightInter.pdf / pow(lightInter.distanceI, 2);
	}
	return lightColor;
}
vec3 sampleRay(vec3 n, vec3 dir, int iterationTime) {
	//sample 
	float x_1 = random(iterationTime, 3),
		x_2 = random(iterationTime, 4);
	float z = abs(1.0f - x_1);
	float r = sqrt(1.0f - z * z), phi = 2 * 3.1415926535897932* x_2;
	vec3 a = vec3(r*cos(phi), r*sin(phi), z);
	//reference:https://000ddd00dd0d.github.io/2019/04/07/Spherical-Sampling/

	//transform
	vec3 vup;
	if (abs(n.x) > 0.9)
		vup = vec3(0.0, 1.0, 0.0);
	else
		vup = vec3(1.0, 0.0, 0.0);
	vec3 t = normalize(cross(vup, n));
	vec3 s = cross(t, n);
	return normalize(a.x * t + a.y * s + a.z * n);
	//reference:https://blog.csdn.net/qq_35312463/article/details/117150378
}
Inter shade(Ray ray) {
	Inter inter;
	inter = normMethod(ray);
	return inter;
}
float pdf_m(vec3 wi, vec3 wo, vec3 N) {
	// uniform sample probability 1 / (2 * PI)
	if (dot(wo, N) >= 0.0f)
		return 0.5f / 3.141592653589732;
	else
		return 0.0f;
}
void main() {
	Ray ray;
	Ray rayTmp;
	Inter inter;
	Inter interTmp; 
	vec3 dirTemp;
	int colorIndex = 0;
	vec3 lightColor = vec3(0);
	vec3 nolightColor = vec3(0);
	vec3 color = vec3(0.0f);
	vec3 allColor = vec3(0.0f);
	bool endRandom = false;
	float russiaRate = 0.8;


	rayTmp = Ray(origin, direction, 1 / direction, sign(direction));

	inter = shade(rayTmp);
	int iterationTime = -1;
	if (inter.isIntersect) 
	{
		do {
			iterationTime += 1;
			ray = rayTmp;
			if (inter.isLight) {
				colorBuffer[colorIndex + 1] = 8.0f * vec3(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) 
					+ 15.6f * vec3(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4f *vec3(0.737f + 0.642f, 0.737f + 0.159f, 0.737f);
				colorBuffer[colorIndex] = vec3(0.0f);
				colorIndex += 2;
				break;
			}

			lightColor = lightResult(ray, inter, iterationTime);
			colorBuffer[colorIndex + 1] = lightColor;
			colorBuffer[colorIndex] = vec3(0.0f);
			colorIndex += 2;
			if (random(iterationTime, 0) > russiaRate) {
				break;
			}
			dirTemp = sampleRay(inter.normal, inter.position, iterationTime);
			rayTmp = Ray(inter.position, dirTemp, 1 / dirTemp, sign(dirTemp));
			interTmp = shade(rayTmp);
			if (!interTmp.isLight) {
				nolightColor = eval(rayTmp.dir, inter.normal, inter.color)
					*dot(normalize(rayTmp.dir), normalize(inter.normal))
					/ pdf_m(ray.dir, rayTmp.dir, inter.normal) / russiaRate;
			}
			colorBuffer[colorIndex - 2] = nolightColor;
			inter = interTmp;
		} while (iterationTime < 30 && inter.isIntersect);
		for (int i = colorIndex; i >= 2; i -= 2) {
			color = colorBuffer[i - 1] + colorBuffer[i - 2] * color;
		}
	}
	if (true) {
		FragColor = vec4(color, 1.0f);
	}
	else {
		FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

}