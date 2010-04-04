#include "Geometry.h"

#include "math.h"



Vec2::Vec2() { x = 0; y = 0; }
Vec2::Vec2(double _x, double _y) { x = _x; y = _y; }
Vec2::Vec2(const Vec2 &v) { x = v.x; y = v.y; }
Vec2::Vec2(const Vec3 &v) { x = v.x; y = v.y; }
	
double Vec2::magnitude() { return sqrt(x*x + y*y); }
Vec2 Vec2::normalize() { return (*this)/magnitude(); }

Vec2 operator+(const Vec2& a, const Vec2& b) { return Vec2(a.x+b.x, a.y+b.y); }
Vec2 operator-(const Vec2& a, const Vec2& b) { return Vec2(a.x-b.x, a.y-b.y); }
Vec2 operator-(const Vec2& a) { return Vec2(-a.x, -a.y); }
Vec2 operator*(const Vec2& a, double f) { return Vec2(a.x*f, a.y*f); }
Vec2 operator*(double f, const Vec2& a) { return a*f; }
Vec2 operator/(const Vec2& a, double f) { return a*(1/f); }
bool operator==(const Vec2& a, const Vec2& b) { return a.x==b.x && a.y==b.y; }
ostream& operator<<(ostream& s, const Vec2& v)
{
	s << "<" << v.x << "," << v.y << ">";
	return s;
}

double dot(const Vec2& a, const Vec2& b) { return a.x*b.x + a.y*b.y; }
double cross(const Vec2& a, const Vec2& b) { return a.x*b.y - a.y*b.x; }



Point2::Point2() { x = 0; y = 0; }
Point2::Point2(double _x, double _y) { x = _x; y = _y; }
Point2::Point2(const Point2& p) { x = p.x; y = p.y; }
Point2::Point2(const Point3& p) { x = p.x; y = p.y; }

Point2 operator+(const Point2& p, const Vec2& v) { return Point2(p.x+v.x, p.y+v.y); }
Point2 operator+(const Vec2& v, const Point2& p) { return p+v; }
Point2 operator-(const Point2& p, const Vec2& v) { return Point2(p.x-v.x, p.y-v.y); }
Vec2 operator-(const Point2& p1, const Point2& p2) { return Vec2(p1.x-p2.x, p1.y-p2.y); }
bool operator==(const Point2& p1, const Point2& p2) { return p1.x==p2.x && p1.y==p2.y; }
ostream& operator<<(ostream& s, const Point2& p)
{
	s << "(" << p.x << "," << p.y << ")";
	return s;
}



Vec3::Vec3() { x = 0; y = 0; }
Vec3::Vec3(double _x, double _y, double _z) { x = _x; y = _y; z = _z; }
Vec3::Vec3(const Vec3& v) { x = v.x; y = v.y; z = v.z; }

double Vec3::magnitude() { return sqrt(x*x + y*y + z*z); }
Vec3 Vec3::normalize() { return (*this)/magnitude(); }

Vec3 operator+(const Vec3& a, const Vec3& b) { return Vec3(a.x+b.x, a.y+b.y, a.z+b.z); }
Vec3 operator-(const Vec3& a, const Vec3& b) { return Vec3(a.x-b.x, a.y-b.y, a.z-b.z); }
Vec3 operator-(const Vec3& a) { return Vec3(-a.x, -a.y, -a.z); }
Vec3 operator*(const Vec3& a, double f) { return Vec3(a.x*f, a.y*f, a.z*f); }
Vec3 operator*(double f, const Vec3& a) { return a*f; }
Vec3 operator/(const Vec3& a, double f) { return a*(1/f); }
ostream& operator<<(ostream& s, const Vec3& v)
{
	s << "<" << v.x << "," << v.y << "," << v.z << ">";
	return s;
}

double dot(const Vec3& a, const Vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
Vec3 cross(const Vec3& a, const Vec3& b)
	{ return Vec3(a.y*b.z-b.y*a.z, a.z*b.x-b.z*a.x, a.x*b.y-b.x*a.y); }



Point3::Point3() { x = 0; y = 0; }
Point3::Point3(double _x, double _y, double _z) { x = _x; y = _y; z = _z; }
Point3::Point3(const Point3& p) { x = p.x; y = p.y; z = p.z; }

Point3 operator+(const Point3& p, const Vec3& v) { return Point3(p.x+v.x, p.y+v.y, p.z+v.z); }
Point3 operator+(const Vec3& v, const Point3& p) { return p+v; }
Point3 operator-(const Point3& p, const Vec3& v) { return Point3(p.x-v.x, p.y-v.y, p.z-v.z); }
Vec3 operator-(const Point3& p1, const Point3& p2) { return Vec3(p2.x-p1.x, p2.y-p1.y, p2.z-p1.z); }
ostream& operator<<(ostream& s, const Point3& p)
{
	s << "(" << p.x << "," << p.y << "," << p.z << ")" << endl;
	return s;
}



Matrix4::Matrix4(double (&_e)[4][4])
{
	for (int x=0; x<4; x++) for (int y=0; y<4; y++) e[x][y] = _e[x][y];
}
Matrix4::Matrix4(
	double e11, double e21, double e31, double e41,
	double e12, double e22, double e32, double e42,
	double e13, double e23, double e33, double e43,
	double e14, double e24, double e34, double e44)
{
	e[0][0] = e11; e[1][0] = e21; e[2][0] = e31; e[3][0] = e41;
	e[0][1] = e12; e[1][1] = e22; e[2][1] = e32; e[3][1] = e42;
	e[0][2] = e13; e[1][2] = e23; e[2][2] = e33; e[3][2] = e43;
	e[0][3] = e14; e[1][3] = e24; e[2][3] = e34; e[3][3] = e44;
}
Matrix4::Matrix4(const Matrix4& m)
{
	for (int x=0; x<4; x++) for (int y=0; y<4; y++) e[x][y] = m.e[x][y];
}

Matrix4 Matrix4::transpose()
{
	double e2[4][4];
	for (int x=0; x<4; x++) for (int y=0; y<4; y++)
	{
		e2[x][y] = e[y][x];
	}
	return Matrix4(e2);
}

const Matrix4 Matrix4::identity(
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);
Matrix4 Matrix4::translation(const Vec3& t)
{
	return Matrix4(
		1, 0, 0, t.x,
		0, 1, 0, t.y,
		0, 0, 1, t.z,
		0, 0, 0, 1);
}
Matrix4 Matrix4::rotation(double a, const Vec3& v)
{
	double x = v.x, y = v.y, z = v.z;
	double c = cos(a), s = sin(a), c2 = 1-c;
	return Matrix4(
		x*x*c2+c,     x*y*c2-z*s,     x*z*c2+y*s,     0,
		x*y*c2+z*s,   y*y*c2+c,       y*z*c2-x*s,     0,
		x*z*c2-y*s,   y*z*c2+x*s,     z*z*c2+c,       0,
		0,            0,              0,              1);
}
Matrix4 Matrix4::scaling(const Vec3& v)
{
	return Matrix4(
		v.x, 0, 0, 0,
		0, v.y, 0, 0,
		0, 0, v.z, 0,
		0, 0, 0, 1);
}

Matrix4 operator+(const Matrix4& a, const Matrix4& b)
{
	double e[4][4];
	for (int x=0; x<4; x++) for (int y=0; y<4; y++) e[x][y] = a.e[x][y] + b.e[x][y];
	return Matrix4(e);
}
Matrix4 operator-(const Matrix4& a, const Matrix4& b)
{
	double e[4][4];
	for (int x=0; x<4; x++) for (int y=0; y<4; y++) e[x][y] = a.e[x][y] - b.e[x][y];
	return Matrix4(e);
}
Matrix4 operator-(const Matrix4& a)
{
	double e[4][4];
	for (int x=0; x<4; x++) for (int y=0; y<4; y++) e[x][y] = -a.e[x][y];
	return Matrix4(e);
}
Matrix4 operator*(const Matrix4& a, const Matrix4& b)
{
	double e[4][4];
	for (int x=0; x<4; x++) for (int y=0; y<4; y++)
	{
		e[x][y] = 0;
		for (int z=0; z<4; z++) e[x][y] += a.e[z][y] * b.e[x][z];
	}
	return Matrix4(e);
}
Matrix4 operator*(const Matrix4& a, double f)
{
	double e[4][4];
	for (int x=0; x<4; x++) for (int y=0; y<4; y++) e[x][y] = a.e[x][y] * f;
	return Matrix4(e);
}
Matrix4 operator*(double f, const Matrix4& a)
{
	return a*f;
}
Vec3 operator*(const Matrix4& m, const Vec3& x)
{
	double x2[4] = {x.x, x.y, x.z, 0.0}, y2[4] = {0, 0, 0, 0};
	for (int x=0; x<4; x++) for (int y=0; y<4; y++) y2[y] += x2[x] * m.e[x][y];
	return Vec3(y2[0], y2[1], y2[2]);
}
Point3 operator*(const Matrix4& m, const Point3& x)
{
	double x2[4] = {x.x, x.y, x.z, 1.0}, y2[4] = {0, 0, 0, 0};
	for (int x=0; x<4; x++) for (int y=0; y<4; y++) y2[y] += x2[x] * m.e[x][y];
	return Point3(y2[0]/y2[3], y2[1]/y2[3], y2[2]/y2[3]);
}