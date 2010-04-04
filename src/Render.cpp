#include "Render.h"

#include <iostream>
#include <vector>
#include <math.h>


/*
TODO:

Add methods to the Image class for blitting images. Same with the Array2D<T> class?

Make Image only another specialization (or subclass) of Array2D?
*/



void render_core(
	const Mesh& mesh,
	const Matrix4& transform,
	Array2D<double>& depth_buffer,
	Array2D<Vec3>& normal_buffer,
	Array2D<const Material*>& material_buffer,
	CullMode cullmode = CULL_NONE);

void supersample(
	const Mesh& mesh,
	const Matrix4& transform,
	Array2D<double>& depth_buffer,
	Array2D<Vec3>& normal_buffer,
	Array2D<const Material*>& material_buffer,
	int ssf,
	CullMode cullmode = CULL_NONE);

Color light_fragment(
	const Vec3& loc,
	const Vec3& normal,
	const Material& mat,
	const list<SunLight>& lights);

void outline_discontinuities(
	Image& canvas,
	Array2D<double> &depth_buffer,
	Array2D<Vec3> &normal_buffer,
	Array2D<const Material*> &material_buffer);

void outline_material_bounds(
	Image& canvas,
	Array2D<double> &depth_buffer,
	Array2D<Vec3> &normal_buffer,
	Array2D<const Material*> &material_buffer);

vector< pair<Point2,Vec3> > rasterize_triangle(Point2 p1, Point2 p2, Point2 p3);



const Vec3 eye(0,0,1);



void render(
	const Mesh& mesh,
	const Matrix4& transform,
	const list<SunLight>& lights,
	Image& canvas,
	CullMode cullmode)
{
	Array2D<double> depth_buffer(canvas.width, canvas.height);
	Array2D<Vec3> normal_buffer(canvas.width, canvas.height);
	Array2D<const Material*> material_buffer(canvas.width, canvas.height);
	
	supersample(mesh, transform, depth_buffer, normal_buffer, material_buffer, 3, cullmode);
	
	for (int x=0; x<canvas.width; x++) for (int y=0; y<canvas.height; y++)
	{
		if (material_buffer(x,y))
		{
			canvas(x,y) = light_fragment(
				Vec3(x,y,depth_buffer(x,y)),
				normal_buffer(x,y),
				*material_buffer(x,y),
				lights);
		}
	}
	
	outline_material_bounds(canvas, depth_buffer, normal_buffer, material_buffer);
}

void outline_discontinuities(
	Image& canvas,
	Array2D<double> &depth_buffer,
	Array2D<Vec3> &normal_buffer,
	Array2D<const Material*> &material_buffer)
{
	for (int x=0; x<canvas.width; x++) for (int y=0; y<canvas.height; y++)
	{
		const int xoffs[4] = {1, -1, 0, 0};
		const int yoffs[4] = {0, 0, 1, -1};
		
		double diff = 0;
		for (int offi=0; offi<4; offi++)
		{
			int x2 = x+xoffs[offi], y2 = y+yoffs[offi];
			if (x2<0 || x2>=canvas.width || y2<0 || y2>=canvas.height) continue;
			
			if (isfinite(depth_buffer(x2,y2)) && isfinite(depth_buffer(x,y)))
			{
				Vec3 n = normal_buffer(x,y);
				double expected_depth =
					depth_buffer(x,y) -
					xoffs[offi]*n.x/n.z -
					yoffs[offi]*n.y/n.z;
				if (expected_depth > depth_buffer(x2,y2))
					diff += expected_depth - depth_buffer(x2,y2);
			}
			else if (!isfinite(depth_buffer(x,y)) && isfinite(depth_buffer(x2,y2)) )
			{
				diff = INFINITY;
			}
		}
		
		double adjust = diff / 10;
		canvas(x,y).r -= adjust;
		canvas(x,y).g -= adjust;
		canvas(x,y).b -= adjust;
	}
}



void outline_material_bounds(
	Image& canvas,
	Array2D<double> &depth_buffer,
	Array2D<Vec3> &normal_buffer,
	Array2D<const Material*> &material_buffer)
{
	for (int x=0; x<canvas.width; x++) for (int y=0; y<canvas.height; y++)
	{
		const int xoffs[4] = {1, -1, 0, 0};
		const int yoffs[4] = {0, 0, 1, -1};
		
		bool change = false;
		for (int offi=0; offi<4; offi++)
		{
			int x2 = x+xoffs[offi], y2 = y+yoffs[offi];
			if (x2<0 || x2>=canvas.width || y2<0 || y2>=canvas.height) continue;
			
			if (material_buffer(x,y) != material_buffer(x2,y2))
			{
				if (depth_buffer(x,y) > depth_buffer(x2,y2))
				{
					change = true;
					break;
				}
			}
		}
		
		if (change)
		{
			canvas(x,y).r = 0;
			canvas(x,y).g = 0;
			canvas(x,y).b = 0;
		}
	}
}



Color light_fragment(
	const Vec3& loc,
	const Vec3& normal,
	const Material& mat,
	const list<SunLight>& lights)
{
	Color color;
					
	// Ambient lighting
	color = color + mat.ambient;
	
	for (list<SunLight>::const_iterator it = lights.begin(); it != lights.end(); it++)
	{
		const SunLight &light = *it;
		
		double bias = 0.2;
		double alignment = (dot(light.direction, normal) + bias) / (1 + bias);
		if (alignment > 0)
		{
			// Make shading granular
			
			if (alignment < 0.6) alignment = 0.4;
			else alignment = 0.8;
			
			// Diffuse lighting
			color = color + alignment * mat.diffuse * light.color;
			
			Vec3 reflection = (2 * alignment * normal - light.direction).normalize();
			double eye_alignment = - dot(eye, reflection);
			if (eye_alignment > 0)
			{
				// Specular lighting
				color = color +
					pow(eye_alignment, mat.shininess) *
					mat.specular *
					light.color;
			}
		}
	}
		
	return color;
}



void supersample(
	const Mesh& mesh,
	const Matrix4& transform,
	Array2D<double>& depth_buffer,
	Array2D<Vec3>& normal_buffer,
	Array2D<const Material*>& material_buffer,
	int ssf,
	CullMode cullmode)
{
	int width = depth_buffer.width, height = depth_buffer.height;
	
	Array2D<double> depth_ss_buffer(width*ssf, height*ssf);
	Array2D<Vec3> normal_ss_buffer(width*ssf, height*ssf);
	Array2D<const Material*> material_ss_buffer(width*ssf, height*ssf);
	
	Matrix4 transform2 = Matrix4::scaling(Vec3(ssf,ssf,ssf)) * transform;
	render_core(mesh, transform2, depth_ss_buffer, normal_ss_buffer, material_ss_buffer, cullmode);
	
	depth_buffer.clear(INFINITY);
	normal_buffer.clear(Vec3(0,0,0));
	material_buffer.clear(NULL);
	
	for (int x=0; x<width; x++) for (int y=0; y<height; y++)
	{
		int best_count = 0;
		for (int xo=0; xo<ssf; xo++) for (int yo=0; yo<ssf; yo++)
		{
			const Material* this_material = material_ss_buffer(x*ssf+xo,y*ssf+yo);
			int count = 0;
			for (int xo2=0; xo2<ssf; xo2++) for (int yo2=0; yo2<ssf; yo2++)
				if (this_material == material_ss_buffer(x*ssf+xo2,y*ssf+yo2)) count++;
			if (count > best_count)
			{
				material_buffer(x,y) = this_material;
				best_count = count;
			}
		}
		
		if (material_buffer(x,y))
		{
			depth_buffer(x,y) = 0;
			normal_buffer(x,y) = Vec3(0,0,0);
			for (int xo=0; xo<ssf; xo++) for (int yo=0; yo<ssf; yo++)
			{
				if (material_ss_buffer(x*ssf+xo, y*ssf+yo) == material_buffer(x,y))
				{
					depth_buffer(x,y) += depth_ss_buffer(x*ssf+xo, y*ssf+yo) / ssf;
					normal_buffer(x,y) = normal_buffer(x,y) + normal_ss_buffer(x*ssf+xo, y*ssf+yo);
				}
			}
			depth_buffer(x,y) /= best_count;
			normal_buffer(x,y) = normal_buffer(x,y).normalize();
		}
	}
}



void render_core(
	const Mesh& mesh,
	const Matrix4& transform,
	Array2D<double> &depth_buffer,
	Array2D<Vec3> &normal_buffer,
	Array2D<const Material*> &material_buffer,
	CullMode cullmode)
{
	int width = depth_buffer.width, height = depth_buffer.height;
		
	depth_buffer.clear(INFINITY);
	normal_buffer.clear(Vec3(0,0,0));
	material_buffer.clear(NULL);
		
	for (list<Face>::const_iterator it = mesh.faces.begin(); it != mesh.faces.end(); it++)
	{
		const Face &face = *it;
		
		// if (face.material.ambient.r<face.material.ambient.b+0.2) continue;
		
		const Vertex &p1 = face.vertices[0];
		const Vertex &p2 = face.vertices[1];
		const Vertex &p3 = face.vertices[2];
				
		Point3 p1_t = transform * p1.point;
		Point3 p2_t = transform * p2.point;
		Point3 p3_t = transform * p3.point;
		
		/* Back-face culling */
		switch(cullmode)
		{
		case CULL_FRONT:
			if (dot(cross(p2_t-p1_t, p3_t-p1_t), eye)>0) continue;
			break;
		case CULL_BACK:
			if (dot(cross(p2_t-p1_t, p3_t-p1_t), eye)<0) continue;
			break;
		case CULL_NONE: break;
		}
				
		Vec3 n1 = p1.normal;
		if (dot(eye, transform*n1)<0) n1 = -n1;
		Vec3 n2 = p2.normal;
		if (dot(eye, transform*n2)<0) n2 = -n2;
		Vec3 n3 = p3.normal;
		if (dot(eye, transform*n3)<0) n3 = -n3;
		
		vector< pair<Point2,Vec3> > raster_pixels = rasterize_triangle(p1_t, p2_t, p3_t);
		
		for (vector< pair<Point2,Vec3> >::iterator it = raster_pixels.begin();
			it != raster_pixels.end();
			it++)
		{
			Point2 location = (*it).first;
			int x = location.x, y = location.y;
			Vec3 affinities = (*it).second;
			
			// Skip pixels outside of the canvas
			if (x<0 || y<0 || x>=width || y>=height) continue;
			
			double depth = 
				p1_t.z * affinities.x +
				p2_t.z * affinities.y +
				p3_t.z * affinities.z;
			double pdepth = depth_buffer(x,y);
			
			if (depth <= pdepth)
			{
				depth_buffer(x,y) = depth;
				
				Vec3 normal = n1*affinities.x + n2*affinities.y + n3*affinities.z;
				normal_buffer(x,y) = (transform * normal).normalize();
				
				material_buffer(x,y) = face.material;
			}
		}
	}
}

vector< pair<Point2,Vec3> > rasterize_triangle(Point2 p1, Point2 p2, Point2 p3)
{
	vector< pair<Point2,Vec3> > pixels;

	// Bail out early if any points are shared, because this messes up the algorithm later on.
	if (p1==p2 || p2==p3 || p1==p3) return pixels;
		
	// Sort the points by Y
	Point2 p1_s = p1, p2_s = p2, p3_s = p3, temp;
	if (p1_s.y > p2_s.y) { temp = p1_s; p1_s = p2_s; p2_s = temp; }
	if (p2_s.y > p3_s.y) { temp = p2_s; p2_s = p3_s; p3_s = temp; }
	if (p1_s.y > p2_s.y) { temp = p1_s; p1_s = p2_s; p2_s = temp; }
	
	int min_y = round(p1_s.y);
	int max_y = round(p3_s.y);
	
	// Allocate tables to hold minimum and maximum x-coordinates
	double *mins = new double[max_y+1-min_y];
	double *maxes = new double[max_y+1-min_y];
	for (int y=min_y; y<=max_y; y++)
	{
		mins[y-min_y] = INFINITY;
		maxes[y-min_y] = -INFINITY;
	}
		
	// For each edge of the triangle, fill out the minimum and maximum x-coordinates
	for (int i=0; i<3; i++)
	{
		// Set a and b to the two end points of this edge
		Point2 a = i==0 ? p1_s : i==1 ? p2_s : p1_s;
		Point2 b = i==0 ? p2_s : i==1 ? p3_s : p3_s;
				
		// Skip the case where the y-coordinates are the same, because this messes the algorithm
		// up, and is meaningless anyway
		if (round(a.y) == round(b.y)) continue;
		
		double slope = (round(b.x) - round(a.x)) / (round(b.y) - round(a.y));
		
		for (int y = round(a.y); y <= round(b.y); y++)
		{			
			double x = round(a.x) + slope * (y - round(a.y));
			if (x < mins[y-min_y]) mins[y-min_y] = x;
			if (x > maxes[y-min_y]) maxes[y-min_y] = x;
		}
	}
	
	// Generate the actual points
	for (int y=min_y; y<=max_y; y++)
	{
		for (int x=round(mins[y-min_y]); x<=round(maxes[y-min_y]); x++)
		{
			Point2 p(x,y);
			
			double p2x = cross(p-p1, p3-p1) / cross(p2-p1, p3-p1);
			double p3x = cross(p-p1, p2-p1) / cross(p3-p1, p2-p1);
			
			/* Because coordinates are rounded to the nearest integer, sometimes the algorithm
			generates a pixel that is actually outside of the triangle it is supposedly a part of.
			This is okay, because the pixel will be right on the edge of the triangle in question.
			The problem is that if the triangle is very small, the error in the pixel's location may
			be many times the length of the sides of the triangle, leading to values for p1x, p2x,
			and p3x that are far out of bounds. This causes all sorts of artifacts in the final
			images. To solve the problem, we clamp the values for p1x, p2x, and p3x to within the
			expected ranges. */
			
			if (p2x<0) p2x=0;
			if (p2x>1) p2x=1;
			if (p3x<0) p3x=0;
			if (p3x>1) p3x=1;
			
			double p1x = 1.0 - p2x - p3x;
			
			if (p1x<0) { p2x += p1x/2; p3x += p1x/2; p1x=0; }
			
			pixels.push_back(pair<Point2,Vec3>(p, Vec3(p1x, p2x, p3x)));
		}
	}
	
	return pixels;
}

/*
INSERT EMBARRASSING THING IN SOURCE CODE WHICH I FORGOT TO REMOVE BEFORE MAKING CODE PUBLIC HERE
*/