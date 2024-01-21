#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

#define PI 3.1415926
#define INF 114514.0
#define SIZE_BVHNODE 4
#define SIZE_TRIANGLE 12
#define RussianRoulette 0.8
#define EPSILON 0.00001

struct Camera {
	vec3 camPos;
	vec3 front;
	vec3 right;
	vec3 up;
	float halfH;
	float halfW;
	vec3 leftbottom;
	int LoopNum;
};

struct BVHNode {
	int left;           // 左子树
	int right;          // 右子树
	int n;              // 包含三角形数目
	int index;          // 三角形索引
	vec3 AA, BB;        // 碰撞盒
};

struct Triangle {
	vec3 p1, p2, p3;    // 顶点坐标
	vec3 n1, n2, n3;    // 顶点法线
};

struct Material {
	vec3 emissive;          // 作为光源时的发光颜色
	vec3 baseColor;
	float subsurface;
	float metallic;
	float specular;
	float specularTint;
	float roughness;
	float anisotropic;
	float sheen;
	float sheenTint;
	float clearcoat;
	float clearcoatGloss;
	float IOR;
	int transmission;
};

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct HitResult {
	bool isHit;             // 是否命中
	bool isInside;          // 是否从内部命中
	float distance;         // 与交点的距离
	vec3 hitPoint;          // 光线命中点
	vec3 normal;            // 命中点法线
	vec3 viewDir;           // 击中该点的光线的方向
	Material material;      // 命中点的表面材质
};

uniform int screenWidth;
uniform int screenHeight;

uniform Camera camera;
uniform float randOrigin;

uniform samplerBuffer triangles;
uniform int nTriangles;
uniform samplerBuffer nodes;
uniform int nNodes;

uniform sampler2D historyTexture;

uint wseed;
float rand(void);

HitResult hitBVH(Ray ray);
BVHNode getBVHNode(int i);
HitResult hitArray(Ray ray, int l, int r);
Triangle getTriangle(int i);
HitResult hitTriangle(Triangle triangle, Ray ray);
Material getMaterial(int i);
vec3 shade(HitResult hit_obj, vec3 wo);
vec3 sample_(vec3 wo, vec3 N, int type);
vec3 toWorld(vec3 v, vec3 N);
float pdf_(vec3 wo, vec3 wi, vec3 N, int type);
vec3 eval_(vec3 wi, vec3 wo, vec3 N, Material material);
float DistributionGGX(vec3 N, vec3 H, float a);
float GeometrySmith(float NdotV, float NdotL, float k);
float GeometrySchlickGGX(float NdotV, float k);
vec3 lerp(vec3 a, vec3 b, float t);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main() {
	wseed = uint(randOrigin * float(6.95857) * (TexCoords.x * TexCoords.y));

	vec3 hist = texture(historyTexture, TexCoords).rgb;

	Ray cameraRay;
	cameraRay.origin = camera.camPos;
	cameraRay.direction = normalize(camera.leftbottom + (TexCoords.x * 2.0 * camera.halfW) * camera.right + (TexCoords.y * 2.0 * camera.halfH) * camera.up);

	HitResult firstHit = hitBVH(cameraRay);

	vec3 curColor;
	if(firstHit.isHit) {
		// curColor = firstHit.material.baseColor;
		curColor += shade(firstHit, -cameraRay.direction);
	}else{
		curColor = vec3(0.0, 0.0, 0.0);
	}
	// vec3 curColor = shader(cameraRay);

	curColor = (1.0 / float(camera.LoopNum))*curColor + (float(camera.LoopNum - 1) / float(camera.LoopNum)) * hist;
	FragColor = vec4(curColor, 1.0);

}

// ************ ��������� ************** //
float randcore(uint seed) {
	seed = (seed ^ uint(61)) ^ (seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> uint(4));
	seed *= uint(0x27d4eb2d);
	wseed = seed ^ (seed >> uint(15));
	return float(wseed) * (1.0 / 4294967296.0);
}
float rand() {
	return randcore(wseed);
}

// 光线和三角形求交
HitResult hitTriangle(Triangle triangle, Ray ray) {
	HitResult res;
	res.distance = INF;
	res.isHit = false;
	res.isInside = false;

	vec3 p1 = triangle.p1;
	vec3 p2 = triangle.p2;
	vec3 p3 = triangle.p3;

	vec3 S = ray.origin;    // 射线起点
	vec3 d = ray.direction;     // 射线方向
	vec3 N = normalize(cross(p2-p1, p3-p1));    // 法向量

	// 从三角形背后（模型内部）击中
	if (dot(N, d) > 0.0f) {
		res.isInside = true;
	}

	// 如果视线和三角形平行
	if (abs(dot(N, d)) < 0.0001f) return res;

	// 距离
	float t = (dot(N, p1) - dot(S, N)) / dot(d, N);
	if (t < 0.0005f) return res;    // 如果三角形在光线背面

	// 交点计算
	vec3 P = S + d * t;

	// 判断交点是否在三角形中
	vec3 c1 = cross(p2 - p1, P - p1);
	vec3 c2 = cross(p3 - p2, P - p2);
	vec3 c3 = cross(p1 - p3, P - p3);
	bool r1 = (dot(c1, N) > 0 && dot(c2, N) > 0 && dot(c3, N) > 0);
	bool r2 = (dot(c1, N) < 0 && dot(c2, N) < 0 && dot(c3, N) < 0);

	// 命中，封装返回结果
	if (r1 || r2) {
		res.isHit = true;
		res.hitPoint = P;
		res.distance = t;
		res.normal = N;
		res.viewDir = d;
	}

	return res;
}
// 获取第 i 下标的三角形
Triangle getTriangle(int i) {
	int offset = i * SIZE_TRIANGLE;
	Triangle t;

	// 顶点坐标
	t.p1 = texelFetch(triangles, offset + 0).xyz;
	t.p2 = texelFetch(triangles, offset + 1).xyz;
	t.p3 = texelFetch(triangles, offset + 2).xyz;

	// 法线
	t.n1 = texelFetch(triangles, offset + 3).xyz;
	t.n2 = texelFetch(triangles, offset + 4).xyz;
	t.n3 = texelFetch(triangles, offset + 5).xyz;

	return t;
}

// 获取第 i 下标的三角形的材质
Material getMaterial(int i) {
	Material m;

	int offset = i * SIZE_TRIANGLE;
	vec3 param1 = texelFetch(triangles, offset + 8).xyz;
	vec3 param2 = texelFetch(triangles, offset + 9).xyz;
	vec3 param3 = texelFetch(triangles, offset + 10).xyz;
	vec3 param4 = texelFetch(triangles, offset + 11).xyz;

	m.emissive = texelFetch(triangles, offset + 6).xyz;
	m.baseColor = texelFetch(triangles, offset + 7).xyz;
	m.subsurface = param1.x;
	m.metallic = param1.y;
	m.specular = param1.z;
	m.specularTint = param2.x;
	m.roughness = param2.y;
	m.anisotropic = param2.z;
	m.sheen = param3.x;
	m.sheenTint = param3.y;
	m.clearcoat = param3.z;
	m.clearcoatGloss = param4.x;
	m.IOR = param4.y;
	m.transmission = int(param4.z);

	return m;
}

// 获取第 i 下标的 BVHNode 对象
BVHNode getBVHNode(int i) {
	BVHNode node;

	// 左右子树
	int offset = i * SIZE_BVHNODE;
	ivec3 childs = ivec3(texelFetch(nodes, offset + 0).xyz);
	ivec3 leafInfo = ivec3(texelFetch(nodes, offset + 1).xyz);
	node.left = int(childs.x);
	node.right = int(childs.y);

	node.n = int(leafInfo.x);
	node.index = int(leafInfo.y);

	// 包围盒
	node.AA = texelFetch(nodes, offset + 2).xyz;
	node.BB = texelFetch(nodes, offset + 3).xyz;

	return node;
}

HitResult hitArray(Ray ray, int l, int r) {
	HitResult res;
	res.isHit = false;
	res.distance = INF;
	for(int i=l; i<=r; i++) {
		Triangle triangle = getTriangle(i);
		HitResult r = hitTriangle(triangle, ray);
		if(r.isHit && r.distance < res.distance) {
			res = r;
			res.material = getMaterial(i);
		}
	}
	return res;
}

// 和 aabb 盒子求交，没有交点则返回 -1
float hitAABB(Ray r, vec3 AA, vec3 BB) {
	vec3 invdir = 1.0 / r.direction;

	vec3 f = (BB - r.origin) * invdir;
	vec3 n = (AA - r.origin) * invdir;

	vec3 tmax = max(f, n);
	vec3 tmin = min(f, n);

	float t1 = min(tmax.x, min(tmax.y, tmax.z));
	float t0 = max(tmin.x, max(tmin.y, tmin.z));

	return (t1 >= t0 && (t1 >= 0.0)) ? ((t0 >= 0.0) ? (t0) : (t1)) : (-1);
}

// 遍历 BVH 求交
HitResult hitBVH(Ray ray) {
	HitResult res;
	res.isHit = false;
	res.distance = INF;

	// 栈
	int stack[256];
	int sp = 0;

	stack[sp++] = 0;
	while(sp > 0) {
		int top = stack[--sp];
		BVHNode node = getBVHNode(top);

		float d = hitAABB(ray, node.AA, node.BB);
		if(d < 0.0f) {
			res.isHit = false;
			break;
		}

		// 是叶子节点，遍历三角形，求最近交点
		if(node.n > 0) {
			int L = node.index;
			int R = node.index + node.n - 1;
			HitResult r = hitArray(ray, L, R);
			if(r.isHit && r.distance < res.distance) res = r;
			continue;
		}

		// 和左右盒子 AABB 求交
		float d1 = -INF; // 左盒子距离
		float d2 = -INF; // 右盒子距离
		if(node.left > 0) {
			BVHNode leftNode = getBVHNode(node.left);
			d1 = hitAABB(ray, leftNode.AA, leftNode.BB);
		}
		if(node.right > 0) {
			BVHNode rightNode = getBVHNode(node.right);
			d2 = hitAABB(ray, rightNode.AA, rightNode.BB);
		}

		// 在最近的盒子中搜索
		if(d1 >= 0 && d2 >= 0) {
			if(d1 < d2) { // d1 < d2, 左边先
				stack[sp++] = node.right;
				stack[sp++] = node.left;
			} else {    // d2 < d1, 右边先
				stack[sp++] = node.left;
				stack[sp++] = node.right;
			}
		} else if(d1 >= 0) {   // 仅命中左边
			stack[sp++] = node.left;
		} else if(d2 >= 0) {   // 仅命中右边
			stack[sp++] = node.right;
		}
	}

	return res;
}

vec3 toWorld(vec3 v, vec3 N) {
	vec3 helper = vec3(1, 0, 0);
	if(abs(N.x)>0.999) helper = vec3(0, 0, 1);
	vec3 tangent = normalize(cross(N, helper));
	vec3 bitangent = normalize(cross(N, tangent));
	return v.x * tangent + v.y * bitangent + v.z * N;
}

vec3 sample_(vec3 wo, vec3 N, Material material){
	vec3 dir;
	switch(material.transmission) {
		case 0: {
			float z = rand();
			float r = max(0, sqrt(1.0 - z*z));
			float phi = 2.0 * PI * rand();
			vec3 localRay = vec3(r * cos(phi), r * sin(phi), z);

			// 转换到场景中的坐标系
			dir = toWorld(localRay, N);
			break;
		}
		case 1: {
			// 随机一个 ε 和 φ
			float r0 = rand();
			float r1 = rand();
			float a2 = material.roughness * material.roughness;
			float phi = 2 * PI * r1;
			float theta = cos(sqrt((1 - r0) / (r0 * (a2 - 1) + 1)));
			// 单位向量半径就直接 1 了，转换为直角坐标系只需要用到 r*sinθ，所以这里直接乘上去了
			float r = sin(theta);
			dir = reflect(-wo, toWorld(vec3(r * cos(phi), r * sin(phi), cos(theta)), N));
			break;
		}
	}
	return dir;
}

float DistributionGGX(vec3 N, vec3 H, float a){
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.f);
	float NdotH2 = NdotH*NdotH;
	float nom    = a2;
	float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
	denom        = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float NdotV, float NdotL, float k)
{
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float pdf_(vec3 wo, vec3 wi, vec3 N, Material material){
	float pdf;
	float cosalpha_i_N = dot(wi, N);
	switch(material.transmission) {
		case 0:{
			if (cosalpha_i_N > EPSILON)
				pdf =  0.5 / PI;
			else
				pdf =  0.0f;
			break;
		}
		case 1: {
			if (cosalpha_i_N > EPSILON) {
				vec3 h = normalize(wo + wi);
				pdf = DistributionGGX(N, h, material.roughness) * dot(N, h) / (4.f * dot(wo, h));
			}else{
				pdf =  0.0f;
			}
			break;
		}
	}
	return pdf;
}

vec3 lerp(vec3 a, vec3 b, float t){
	return a * (1 - t) + b * t;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0){
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 eval_(vec3 wi, vec3 wo, vec3 N, Material material) {
	vec3 f_r;
	switch(material.transmission) {
		case 0:{
			float cosalpha = dot(N, wo);
			if(cosalpha > EPSILON) {
				f_r = material.baseColor / PI;
			}else {
				f_r = vec3(0.0f);
			}
			break;
		}
		case 1: {
			// cosθ是入射光和法线的夹角，也就是光源方向和法线方向的夹角
			float cosTheta = dot(N, wo);
			if(cosTheta > EPSILON) {
				vec3 V = wi;
				vec3 L = wo;
				vec3 H = normalize(V + L);
				float NdotV = max(dot(N, V), EPSILON);
				float NdotL = cosTheta;
				// 直接光照情况下的 k 公式
				float k = (material.roughness + 1.f) * (material.roughness + 1.f) / 8.f;
				float D = DistributionGGX(N, H, material.roughness);
				float G = GeometrySmith(NdotV, NdotL, k);

				vec3 F0 = vec3(0.04f);
				F0 = lerp(F0, material.baseColor, material.metallic);
				vec3 F = fresnelSchlick(dot(H, V), F0);
				// float F;
				// fresnel(-V, N, ior, F);
				vec3 fs = D * G * F / (4.f * NdotV  * NdotL);

				// std::cout << fs <<std::endl;
				// 菲涅尔项就是 ks， kd = 1-ks;
				vec3 fr =  material.baseColor / PI;

				// return (Vector3f(1.0f) - F0) * fr + F0 * fs;
				f_r = (vec3(1.0f) - F) * (1 - material.metallic) * fr + fs;
			}else {
				f_r = vec3(0.f);
			}
			break;
		}
	}
	return f_r;
}

vec3 shade(HitResult hit_obj, vec3 wo){

	vec3 Lo = vec3(1);
	int flag = 1;
	for (int i = 0; i < 5;i++) {
		if (flag == 0) break;

		switch(hit_obj.material.transmission) {
			case -1: {
				Lo *= hit_obj.material.emissive;
				flag = 0;
				break;
			}
			case 0: {
				vec3 dir_next = sample_(wo, hit_obj.normal, hit_obj.material);
				float pdf = pdf_(wo, dir_next, hit_obj.normal, hit_obj.material);
				if(pdf > EPSILON) {
					Ray ray; ray.origin = hit_obj.hitPoint; ray.direction = dir_next;
					HitResult nextObj = hitBVH(ray);

					if(nextObj.isHit) {
						vec3 f_r = eval_(dir_next, wo, hit_obj.normal, hit_obj.material);
						float cos = max(0.0f, dot(dir_next, hit_obj.normal));

						Lo *= f_r * cos / pdf;

						hit_obj = nextObj;
						wo = -dir_next;
					}else {
						Lo = vec3(0.0f);
						flag = 0;
					}
				}else {
					Lo = vec3(0.0f);
					flag = 0;
				}
				break;
			}
			case 1:{
				vec3 dir_next = sample_(wo, hit_obj.normal, hit_obj.material);
				float pdf = pdf_(wo, dir_next, hit_obj.normal, hit_obj.material);
				if(pdf > EPSILON) {
					Ray ray; ray.origin = hit_obj.hitPoint; ray.direction = dir_next;
					HitResult nextObj = hitBVH(ray);

					if(nextObj.isHit) {
						vec3 f_r = eval_(dir_next, wo, hit_obj.normal, hit_obj.material);
						float cos = max(0.0f, dot(dir_next, hit_obj.normal));

						Lo *= f_r * cos / pdf;

						hit_obj = nextObj;
						wo = -dir_next;
					}else {
						Lo = vec3(0.0f);
						flag = 0;
					}
				}else {
					Lo = vec3(0.0f);
					flag = 0;
				}
				break;
			}
		}
	}

	return Lo;
}






