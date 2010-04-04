#include <fstream>

using namespace std;



#ifndef IMAGE_H
#define IMAGE_H



struct Color
{
	double r,g,b;
	
	Color();
	Color(const Color&);
	Color(double, double, double);
	
	Color clamp();
};

Color mix(const Color&, const Color&, double);
Color operator+(const Color&, const Color&);
Color operator-(const Color&, const Color&);
Color operator*(const Color&, const Color&);
Color operator*(const Color&, double);
Color operator*(double, const Color&);



struct Image
{
	int width, height;
	Color *pixels;
	
	Image(int, int);
	Image(Image&);
	~Image();
	
	Color &operator()(int, int);
	Image &operator=(Image&);
	
	void write_TGA(ofstream& outstream);
	void clear(const Color&);
};



template<typename T> struct Array2D
{
	int width, height;
	T *values;
	
	Array2D(int, int);
	Array2D(Array2D<T>&);
	~Array2D();
	
	T &operator()(int, int);
	Array2D<T> &operator=(Array2D<T>&);
	void clear(const T&);
};

template<typename T> Array2D<T>::Array2D(int _width, int _height)
{
	width = _width;
	height = _height;
	values = new T[width*height];
}

template<typename T> Array2D<T>::Array2D(Array2D<T>& src)
{
	width = src.width;
	height = src.height;
	values = new T[width*height];
	
	for (int x=0; x<width; x++) for (int y=0; y<height; y++)
		values[x+y*width] = src.values[x+y*width];
}

template<typename T> Array2D<T>::~Array2D()
{
	delete[] values;
}
	
template<typename T> T& Array2D<T>::operator()(int x, int y)
{
	return values[x+y*width];
}

template<typename T> Array2D<T>& Array2D<T>::operator=(Array2D<T>& src)
{
	delete[] values;
	
	width = src.width;
	height = src.height;
	values = new T[width*height];
	
	for (int x=0; x<width; x++) for (int y=0; y<height; y++)
		values[x+y*width] = src.values[x+y*width];
	
	return *this;
}

template<typename T> void Array2D<T>::clear(const T& val)
{
	for (int x=0; x<width; x++) for (int y=0; y<height; y++)
		(*this)(x,y) = val;
}



#endif