#include "Geometry.h"
#include "Render.h"
#include "Image.h"
#include "Mesh.h"

#include <stdio.h>
#include <math.h>



int main(int argc, char *argv[])
{
	string obj_path;
	string mtl_search_dir;
	string output_path = "render.tga";
	int img_width;
	int img_height;
	double size_factor;
	double pitch = 0;
	double yaw = 0;
	Vec3 light_angle(1,-2,0);
	Color light_color(1,1,1);
	CullMode cullmode = CULL_NONE;
	bool autocompute_normals = false;
	
	if (argc<5)
	{
		cout << "need at least 4 arguments: model, width, height, scale factor" << endl;
		exit(1);
	}
	
	for (int i=0; i<argc; i++)
	{
		char *arg = argv[i];
		if (i==0)
		{
			continue;
		}
		else if (i==1)
		{
			obj_path = arg;
			
			size_t last_slash_pos = obj_path.find_last_of('/');
			if (last_slash_pos == string::npos) mtl_search_dir = "";
			else mtl_search_dir = obj_path.substr(0, last_slash_pos+1);
		}
		else if (i==2)
		{
			img_width = atoi(arg);
			if (img_width <= 0) { cout << "bad image width" << endl; exit(1); }
		}
		else if (i==3) 
		{
			img_height = atoi(arg);
			if (img_height <= 0) { cout << "bad image height" << endl; exit(1); }
		}
		else if (i==4)
		{
			size_factor = atof(arg);
			if (size_factor == 0) { cout << "bad size factor" << endl; exit(1); }
		}
		else if (string(arg) == "-o" || string(arg) == "--output")
		{
			i++;
			if (i >= argc) { cout << "--output needs an argument" << endl; exit(1); }
			output_path = argv[i];
		}
		else if (string(arg) == "--pitch")
		{
			i++;
			if (i >= argc) { cout << "--pitch needs an argument" << endl; exit(1); }
			pitch = atof(argv[i])*M_PI/180;
		}
		else if (string(arg) == "--yaw")
		{
			i++;
			if (i >= argc) { cout << "--yaw needs an argument" << endl; exit(1); }
			yaw = atof(argv[i])*M_PI/180;
		}
		else if (string(arg) == "--lightangle")
		{
			if (i+3 >= argc) { cout << "--lightangle needs three argumnts" << endl; exit(1); }
			light_angle.x = atof(argv[i+1]);
			light_angle.y = atof(argv[i+2]);
			light_angle.z = atof(argv[i+3]);
			i += 3;
		}
		else if (string(arg) == "--lightcolor")
		{
			if (i+3 >= argc) { cout << "--lightcolor needs three argumnts" << endl; exit(1); }
			light_color.r = atof(argv[i+1]);
			light_color.g = atof(argv[i+2]);
			light_color.b = atof(argv[i+3]);
			i += 3;
		}
		else if (string(arg) == "--autocompute-normals")
		{
			autocompute_normals = true;
		}
		else if (string(arg) == "--cull")
		{
			i++;
			if (i >= argc) { cout << "--cull needs an argument" << endl; exit(1); }
			if (string(argv[i]) == "front") cullmode = CULL_FRONT;
			else if (string(argv[i]) == "back") cullmode = CULL_BACK;
			else if (string(argv[i]) == "none") cullmode = CULL_NONE;
			else { cout << "--cull expects 'front', 'back', or 'none'" << endl; exit(1); }
		}
		else
		{
			cout << "do not recognize "+string(arg) << endl;
			exit(1);
		}
	}
	
	ifstream model_file(obj_path.c_str(), ios_base::in);
	Mesh model = Mesh::from_objfile(model_file, mtl_search_dir);
	model_file.close();
	
	if (autocompute_normals)
	{
		for (list<Face>::iterator it = model.faces.begin();
			it != model.faces.end(); it++)
		{
			Face &f = *it;
			f.autocompute_normals();
		}
	}

	int num_images = 8;

	Image canvas(img_width*num_images, img_height);
	canvas.clear(Color(0.5,0.5,0.5));
	
	for (int i=0; i<num_images; i++)
	{
		Matrix4 transform = Matrix4::identity;
		
		// Center image in canvas
		transform = transform * Matrix4::translation(Vec3(img_width/2+img_width*i,img_height/2,0));
		
		// Scale image by desired scaling factor
		double sf = size_factor * img_height;
		transform = transform * Matrix4::scaling(Vec3(sf,sf,sf));
		
		// Pitch image
		transform = transform * Matrix4::rotation(pitch, Vec3(1,0,0));
		
		// Yaw image
		transform = transform * Matrix4::rotation(yaw + i*2*M_PI/num_images, Vec3(0,1,0));
		
		list<SunLight> lights;
		lights.push_back(SunLight(light_angle, light_color));
		
		render(model, transform, lights, canvas, cullmode);
	}
	
	ofstream output_file(output_path.c_str(), ios_base::out);
	canvas.write_TGA(output_file);
	output_file.close();
}