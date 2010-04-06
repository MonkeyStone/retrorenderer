#include "Image.h"

#include <math.h>
#include <inttypes.h>



Color::Color() { r = 0; g = 0; b = 0; }
Color::Color(const Color& c) { r = c.r; g = c.g; b = c.b; }
Color::Color(double _r, double _g, double _b) { r = _r; g = _g; b = _b; }

Color Color::clamp() { return Color(r>1?1:r<0?0:r, g>1?1:g<0?0:g, b>1?1:b<0?0:b); }

Color mix(const Color& a, const Color& b, double f)
	{ return Color(a.r*(1-f)+b.r*f, a.g*(1-f)+b.g*f, a.b*(1-f)+b.b*f); }
Color operator+(const Color& a, const Color& b) { return Color(a.r+b.r, a.g+b.g, a.b+b.b); }
Color operator-(const Color& a, const Color& b) { return Color(a.r-b.r, a.g-b.g, a.b-b.b); }
Color operator*(const Color& a, const Color& b) { return Color(a.r*b.r, a.g*b.g, a.b*b.b); }
Color operator*(const Color& a, double f) { return Color(a.r*f, a.g*f, a.b*f); }
Color operator*(double f, const Color& a) { return a*f; }



Image::Image(int _width, int _height)
{
	width = _width;
	height = _height;
	pixels = new Color[width*height];
}

Image::Image(Image& src)
{
	width = src.width;
	height = src.height;
	pixels = new Color[width*height];
	
	for (int x=0; x<width; x++) for (int y=0; y<height; y++)
		pixels[x+y*width] = src.pixels[x+y*width];
}

Image::~Image()
{
	delete[] pixels;
}
	
Color& Image::operator()(int x, int y)
{
	return pixels[x+y*width];
}

Image& Image::operator=(Image& src)
{
	delete[] pixels;
	
	width = src.width;
	height = src.height;
	pixels = new Color[width*height];
	
	for (int x=0; x<width; x++) for (int y=0; y<height; y++)
		pixels[x+y*width] = src.pixels[x+y*width];
	
	return *this;
}

void Image::write_TGA(ofstream &s)
{
	uint8_t id_length = 0; // No id field
	s.write((const char*)&id_length, 1);
	
	uint8_t colormap_type = 0; // No color map
	s.write((const char*)&colormap_type, 1);
	
	uint8_t image_type = 2; // Uncompressed, true-color image
	s.write((const char*)&image_type, 1);
	
	uint8_t color_map_info[5] = {0, 0, 0, 0, 24}; // Unused
	s.write((const char*)&color_map_info, 5);
	
	uint16_t xorigin=0, yorigin=0, _width=width, _height=height;
	uint8_t bpp = 32, descriptor = 0x00;
	s.write((const char*)&xorigin, 2);
	s.write((const char*)&yorigin, 2);
	s.write((const char*)&_width, 2);
	s.write((const char*)&_height, 2);
	s.write((const char*)&bpp, 1);
	s.write((const char*)&descriptor, 1);
	
	for (int y=0; y<height; y++) for (int x=0; x<width; x++)
	{
		Color c = (*this)(x,y).clamp();
		uint8_t pixel[4] = {c.b*255, c.g*255, c.r*255, 255};
		s.write((const char*)&pixel, 4);
	}
}

void Image::clear(const Color &c)
{
	for (int x=0; x<width; x++) for (int y=0; y<height; y++)
		(*this)(x,y) = c;
}
