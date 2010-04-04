#include "Mesh.h"

#include <sstream>
#include <vector>
#include <iostream>
#include <stdexcept>



/*
TODO:

Some sort of reference counting for materials (at present they leak)
*/



Material::Material()
{
	ambient = Color(0.3, 0.0, 0.0);
	diffuse = Color(0.5, 0.5, 1.0);
	specular = Color(1.0, 1.0, 1.0);
	shininess = 30.0;
}

Material::Material(const Color& a, const Color& d, const Color& s, double sh)
{
	ambient = a;
	diffuse = d;
	specular = s;
	shininess = sh;
}

map<string, Material*> Material::from_mtlfile(ifstream &file_s)
{
	Color amb, diff, spec;
	double sh;
	string name;
	bool in_mtl = false;
	
	map<string, Material*> mtls;
	
	while (true)
	{
		string line;
		getline(file_s, line);
		if (file_s.fail()) break;
		stringstream line_ss(line);
		
		string keyword;
		line_ss >> keyword;
		if (line_ss.fail()) continue;
		
		if (keyword == "newmtl")
		{
			if (in_mtl)
			{
				mtls[name] = new Material(amb, diff, spec, sh);
				in_mtl = false;
			}
			
			amb = Color(0.5, 0.5, 0.5);
			diff = Color(0.5, 0.5, 0.5);
			spec = Color(0, 0, 0);
			sh = 100.0;
			
			line_ss >> name;
			if (line_ss.fail())
				throw logic_error("parse error: newmtl is missing name");
			
			in_mtl = true;
		}
		else if (keyword == "Ka")
		{
			double c[3];
			line_ss >> c[0] >> c[1] >> c[2];
			if (line_ss.fail())
				throw logic_error("parse error: Ka has bad fields");
			amb = Color(c[0], c[1], c[2]);
		}
		else if (keyword == "Kd")
		{
			double c[3];
			line_ss >> c[0] >> c[1] >> c[2];
			if (line_ss.fail())
				throw logic_error("parse error: Kd has bad fields");
			diff = Color(c[0], c[1], c[2]);
		}
		else if (keyword == "Ks")
		{
			double c[3];
			line_ss >> c[0] >> c[1] >> c[2];
			if (line_ss.fail())
				throw logic_error("parse error: Ks has bad fields");
			spec = Color(c[0], c[1], c[2]);
		}
		else if (keyword == "Ns")
		{
			line_ss >> sh;
			if (line_ss.fail())
				throw logic_error("parse error: Ns has bad field");
		}
	}
	
	if (in_mtl)
	{
		mtls[name] = new Material(amb, diff, spec, sh);
		in_mtl = false;
	}
	
	return mtls;
}



Vertex::Vertex()
{
}

Vertex::Vertex(const Point3& _p, const Vec3& _n, const Point2& _t)
{
	point = _p;
	normal = _n;
	texcoord = _t;
}



Face::Face(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Material* m)
{
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;
	material = m;
}

void Face::autocompute_normals()
{
	Vec3 n = cross(
		vertices[0].point - vertices[1].point,
		vertices[0].point - vertices[2].point)
		.normalize();
	vertices[0].normal = n;
	vertices[1].normal = n;
	vertices[2].normal = n;
}



Mesh::Mesh()
{
}

Mesh Mesh::from_objfile(ifstream& file_s)
{
	const int buffer_size = 1000;
	char cwd[buffer_size];
	getcwd(cwd, buffer_size);
	return Mesh::from_objfile(file_s, cwd);
}

Mesh Mesh::from_objfile(ifstream& file_s, string dir)
{
	Mesh m;
	
	vector<Point3> points;
	vector<Vec3> normals;
	vector<Point2> texcoords;
	map<string, Material*> mtls;
	Material *current_mtl;
	
	while (true)
	{
		string line;
		getline(file_s, line);
		if (file_s.eof() || file_s.fail()) break;
		stringstream line_ss(line);
		
		string keyword;
		line_ss >> keyword;
		if (line_ss.eof() || line_ss.fail()) continue;
		
		if (keyword == "v")
		{
			double point[3];
			line_ss >> point[0] >> point[1] >> point[2];
			if (line_ss.fail())
				throw logic_error("parse error: v has bad fields");
			points.push_back(Point3(point[0], point[1], point[2]));
		}
		else if (keyword == "vn")
		{
			double normal[3];
			line_ss >> normal[0] >> normal[1] >> normal[2];
			if (line_ss.fail())
				throw logic_error("parse error: vn has bad fields");
			normals.push_back(Vec3(normal[0], normal[1], normal[2]));
		}
		else if (keyword == "vt")
		{
			double texcoord[2];
			line_ss >> texcoord[0] >> texcoord[1];
			if (line_ss.fail())
				throw logic_error("parse error: vt hs bad fields");
			texcoords.push_back(Point2(texcoord[0], texcoord[1]));
		}
		else if (keyword == "f")
		{
			vector<Vertex> vertices;
			while (true)
			{
				string vertex_str;
				line_ss >> vertex_str;
				if (line_ss.fail()) break;
				
				stringstream vertex_ss(vertex_str);
				
				// Check for a comment
				if (vertex_ss.peek() == '#') break;
				
				
				int p_ix, n_ix, t_ix;
				
				vertex_ss >> p_ix;
				if (vertex_ss.fail())
					throw logic_error("parse error: vertex has bad point index");
				
				char sep1 = vertex_ss.get();
				if (vertex_ss.fail())
				{
					n_ix = t_ix = 0;
				}
				else
				{
					if (sep1 != '/')	
						throw logic_error("parse error: vertex point index not followed by '/'");
				
					vertex_ss >> t_ix;
					if (vertex_ss.fail())
						{ t_ix = 0; vertex_ss.clear(); }
				
					char sep2 = vertex_ss.get();
					if (vertex_ss.fail())
					{
						n_ix = 0;
					}
					else
					{
						if (sep2 != '/')
							throw logic_error("parse error: vertex texcoord index not followed by '/'");
						
						vertex_ss >> n_ix;
						if (vertex_ss.fail())
							{ n_ix = 0; vertex_ss.clear(); }
					}
				}
					
				
				
				Point3 point;
				if (p_ix>0) point = points[p_ix-1];
				else if (p_ix<0) point = points[points.size()+p_ix];
				
				Vec3 normal;
				if (n_ix>0) normal = normals[n_ix-1];
				else if (n_ix<0) normal = normals[normals.size()+n_ix];
				else if (n_ix==0) normal = Vec3(0,0,0); // Parse failed (normal was omitted)
				
				Point2 texcoord;
				if (t_ix>0) texcoord = texcoords[t_ix-1];
				else if (t_ix<0) texcoord = texcoords[texcoords.size()+t_ix];
				else if (t_ix==0) texcoord = Point2(0,0); // Parse failed (texcoord was omitted)
				
				vertices.push_back(Vertex(point, normal, texcoord));
			}
			
			// Face in .obj file may have more than three vertices, but our Face class only supports
			// triangles. So here we tessellate the face. We use the naive technique and assume that
			// the face is convex.
			for (unsigned int i=1; i+1<vertices.size(); i++)
			{
				m.faces.push_back(Face(vertices[0], vertices[i], vertices[i+1], current_mtl));
			}
		}
		else if (keyword == "mtllib")
		{
			string filename;
			line_ss >> filename;
			if (line_ss.fail())
				throw logic_error("parse error: expected filename after 'mtllib'");
			
			string filepath = dir+"/"+filename;
			ifstream mtlfile(filepath.c_str(), ios_base::in);
			if (!mtlfile) throw logic_error("failed to open mtl file "+filepath);
			
			map<string, Material*> newmtls = Material::from_mtlfile(mtlfile);
			
			for (map<string, Material*>::iterator it = newmtls.begin(); it != newmtls.end(); it++)
			{
				mtls[(*it).first] = (*it).second;
			}
		}
		else if (keyword == "usemtl")
		{
			string name;
			line_ss >> name;
			if (line_ss.fail())
				throw logic_error("parse error: expected mtl name after 'usemtl'");
			if (mtls.count(name)) current_mtl = mtls[name];
			else throw logic_error("parse error: no material with specified name");
		}
	}
	
	return m;
}



SunLight::SunLight(const Vec3& _direction, const Color& _color)
{
	direction = Vec3(_direction).normalize();
	color = _color;
}
