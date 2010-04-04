#include <iostream>
using namespace std;



#ifndef GEOMETRY_H
#define GEOMETRY_H



struct Vec2;
struct Point2;
struct Vec3;
struct Point3;
struct Matrix4;



struct Vec2
{
	double x,y;
	
	Vec2();
	Vec2(double, double);
	Vec2(const Vec2&);
	Vec2(const Vec3&);
	
	double magnitude();
	Vec2 normalize();
};

Vec2 operator+(const Vec2&, const Vec2&);
Vec2 operator-(const Vec2&, const Vec2&);
Vec2 operator-(const Vec2&);
Vec2 operator*(const Vec2&, double);
Vec2 operator*(double, const Vec2&);
Vec2 operator/(const Vec2&, double);
bool operator==(const Vec2&, const Vec2&);
ostream& operator<<(ostream&, const Vec2&);

double dot(const Vec2&, const Vec2&);
double cross(const Vec2&, const Vec2&);



struct Point2
{
	double x,y;
	
	Point2();
	Point2(double, double);
	Point2(const Point2&);
	Point2(const Point3&);
};

Point2 operator+(const Point2&, const Vec2&);
Point2 operator+(const Vec2&, const Point2&);
Point2 operator-(const Point2&, const Vec2&);
Vec2 operator-(const Point2&, const Point2&);
bool operator==(const Point2&, const Point2&);
ostream& operator<<(ostream&, const Point2&);



struct Vec3
{
	double x,y,z;
	
	Vec3();
	Vec3(double, double, double);
	Vec3(const Vec3&);
	
	double magnitude();
	Vec3 normalize();
};

Vec3 operator+(const Vec3&, const Vec3&);
Vec3 operator-(const Vec3&, const Vec3&);
Vec3 operator-(const Vec3&);
Vec3 operator*(const Vec3&, double);
Vec3 operator*(double, const Vec3&);
Vec3 operator/(const Vec3&, double);
ostream& operator<<(ostream&, const Vec3&);

double dot(const Vec3&, const Vec3&);
Vec3 cross(const Vec3&, const Vec3&);



struct Point3
{
	double x,y,z;
	
	Point3();
	Point3(double, double, double);
	Point3(const Point3&);
};

Point3 operator+(const Point3&, const Vec3&);
Point3 operator+(const Vec3&, const Point3&);
Point3 operator-(const Point3&, const Vec3&);
Vec3 operator-(const Point3&, const Point3&);
ostream& operator<<(ostream&, const Point3&);



struct Matrix4
{
	double e[4][4];
	
	Matrix4(double(&)[4][4]);
	Matrix4(
		double, double, double, double,
		double, double, double, double,
		double, double, double, double,
		double, double, double, double);
	Matrix4(const Matrix4&);
	
	Matrix4 transpose();
	
	static const Matrix4 identity;
	static Matrix4 translation(const Vec3&);
	static Matrix4 rotation(double, const Vec3&);
	static Matrix4 scaling(const Vec3&);
};

Matrix4 operator+(const Matrix4&, const Matrix4&);
Matrix4 operator-(const Matrix4&, const Matrix4&);
Matrix4 operator-(const Matrix4&);
Matrix4 operator*(const Matrix4&, const Matrix4&);
Matrix4 operator*(const Matrix4&, double);
Matrix4 operator*(double, const Matrix4&);
Vec3 operator*(const Matrix4&, const Vec3&);
Point3 operator*(const Matrix4&, const Point3&);



#endif
