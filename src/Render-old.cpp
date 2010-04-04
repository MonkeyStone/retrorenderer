#include "Render.h"

#include <iostream>
#include <vector>
#include <math.h>



vector< pair<Point2,Vec3> > rasterize_triangle(Point2 p1, Point2 p2, Point2 p3);

void render(Mesh& mesh, Matrix4& transform, list<SunLight>& lights, Image& canvas)
{
	Array2D<double> depth_buffer(canvas.width,canvas.height);
	depth_buffer.clear(INFINITY);
	
	Array2D<Vec3> normal_buffer(canvas.width,canvas.height);
	
	Vec3 eye(0,0,1);
	
	for (list<Face>::iterator it = mesh.faces.begin(); it != mesh.faces.end(); it++)
	{
		Face &face = *it;
		
		// if (face.material.ambient.r<face.material.ambient.b+0.2) continue;
		
		Vertex &p1 = face.vertices[0];
		Vertex &p2 = face.vertices[1];
		Vertex &p3 = face.vertices[2];
				
		Point3 p1_t = transform * p1.point;
		Point3 p2_t = transform * p2.point;
		Point3 p3_t = transform * p3.point;
		
		/* Back-face culling */
		if (dot(cross(p2_t-p1_t, p3_t-p1_t), eye)<0) continue;
				
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
			if (x<0 || y<0 || x>=canvas.width || y>=canvas.height) continue;
			
			double depth = 
				p1_t.z * affinities.x +
				p2_t.z * affinities.y +
				p3_t.z * affinities.z;
			double pdepth = depth_buffer(x,y);
			
			if (depth <= pdepth)
			{
				depth_buffer(x,y) = depth;
				
				Vec3 normal = n1*affinities.x + n2*affinities.y + n3*affinities.z;
				normal = normal.normalize();
				normal_buffer(x,y) = normal;
				
				Color color;
				
				// Ambient lighting
				color = color + face.material.ambient;
				
				for (list<SunLight>::iterator it = lights.begin(); it != lights.end(); it++)
				{
					SunLight &light = *it;
					
					double bias = 0.2;
					double alignment = (dot(light.direction, normal) + bias) / (1 + bias);
					if (alignment > 0)
					{
						// Make shading granular
						
						/* if (alignment < 0.6) alignment = 0.4;
						else alignment = 0.8; */
						
						
						// Diffuse lighting
						color = color + alignment * face.material.diffuse * light.color;
						
						Vec3 reflection = 2 * alignment * normal - light.direction;
						double eye_alignment = - dot(eye, (transform*reflection).normalize());
						if (eye_alignment > 0)
						{
							// Specular lighting
							color = color +
								pow(eye_alignment, face.material.shininess) *
								face.material.specular *
								light.color;
						}
					}
				}
				
				canvas(x,y) = color;
			}
		}
	}
	
	for (int x=1; x<canvas.width-1; x++) for (int y=1; y<canvas.height-1; y++)
	{
		const int xoffs[4] = {1, -1, 0, 0};
		const int yoffs[4] = {0, 0, 1, -1};
		
		double diff = 0;
		for (int offi=0; offi<4; offi++)
		{
			int x2 = x+xoffs[offi], y2 = y+yoffs[offi];
			if (isfinite(depth_buffer(x2,y2)) && isfinite(depth_buffer(x,y)))
			{
				Vec3 n = (transform * normal_buffer(x,y)).normalize();
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
		
		double adjust = isfinite(diff) ? 0 : 1;
		canvas(x,y).r -= adjust;
		canvas(x,y).g -= adjust;
		canvas(x,y).b -= adjust;
		
		// canvas(x,y).r = canvas(x,y).g = canvas(x,y).b = depth_buffer(x,y)/2000 + 0.5;
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
			
			/* Because coordinates are rounded to the nearest integer, sometimes the algorithm
			generates a pixel that is actually outside of the triangle it is supposedly a part of.
			This is okay, because the pixel will be right on the edge of the triangle in question.
			The problem is that if the triangle is very small, the error in the pixel's location may
			be many times the length of the sides of the triangle, leading to values for p1x, p2x,
			and p3x that are far out of bounds. This causes all sorts of artifacts in the final
			images. To solve the problem, we clamp the values for p1x, p2x, and p3x to within the
			expected ranges. */
			
			double p2x = cross(p-p1, p3-p1) / cross(p2-p1, p3-p1);
			double p3x = cross(p-p1, p2-p1) / cross(p3-p1, p2-p1);
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