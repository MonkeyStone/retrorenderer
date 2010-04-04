#include "Geometry.h"
#include "Image.h"

#include <fstream>
#include <map>
#include <list>

using namespace std;



#ifndef MESH_H
#define MESH_H



struct Material;
struct Vertex;
struct Face;
struct Mesh;



struct Material
{
	Color ambient, diffuse, specular;
	double shininess;
	
	Material();
	Material(const Color&, const Color&, const Color&, double);
	
	static map<string, Material*> from_mtlfile(ifstream&);
};



struct Vertex
{
	Point3 point;
	Vec3 normal;
	Point2 texcoord;
	
	Vertex();
	Vertex(const Point3&, const Vec3&, const Point2&);
};



struct Face
{
	Vertex vertices[3];
	const Material* material;
	
	Face(const Vertex&, const Vertex&, const Vertex&, const Material*);
	
	void autocompute_normals();
};



struct Mesh
{
	list<Face> faces;
	
	Mesh();
	
	static Mesh from_objfile(ifstream&);
	static Mesh from_objfile(ifstream&, string dir); // dir specifies where to look for .mtl files
};



struct SunLight
{
	Vec3 direction;
	Color color;
	
	SunLight(const Vec3&, const Color&);
};



#endif