#ifndef _POINT_H_
#define _POINT_H_
#include <assert.h>
#include <math.h>
#include <iostream>

template <class T>
struct point{
	inline point(){}
	inline point(T _x, T _y):x(_x),y(_y){}
	T x, y;
};

template <class T>
inline point<T> operator+(const point<T>& p1, const point<T>& p2){
	return point<T>(p1.x+p2.x, p1.y+p2.y);
}

template <class T>
inline point<T> operator - (const point<T> & p1, const point<T> & p2){
	return point<T>(p1.x-p2.x, p1.y-p2.y);
}

template <class T>
inline point<T> operator * (const point<T>& p, const T& v){
	return point<T>(p.x*v, p.y*v);
}

template <class T>
inline point<T> operator * (const T& v, const point<T>& p){
	return point<T>(p.x*v, p.y*v);
}

template <class T>
inline T operator * (const point<T>& p1, const point<T>& p2){
	return p1.x*p2.x+p1.y*p2.y;
}


template <class T, class A>
struct orientedpoint: public point<T>{
	inline orientedpoint(){}
	inline orientedpoint(const point<T>& p);
	inline orientedpoint(T x, T y, A _theta): point<T>(x,y), theta(_theta){}
	A theta;
};

template <class T, class A>
orientedpoint<T,A>::orientedpoint(const point<T>& p){
	this->x=p.x;
	this->y=p.y;
	this->theta=0.;
}


template <class T, class A>
orientedpoint<T,A> operator+(const orientedpoint<T,A>& p1, const orientedpoint<T,A>& p2){
	return orientedpoint<T,A>(p1.x+p2.x, p1.y+p2.y, p1.theta+p2.theta);
}

template <class T, class A>
orientedpoint<T,A> operator - (const orientedpoint<T,A> & p1, const orientedpoint<T,A> & p2){
	return orientedpoint<T,A>(p1.x-p2.x, p1.y-p2.y, p1.theta-p2.theta);
}

template <class T, class A>
orientedpoint<T,A> operator * (const orientedpoint<T,A>& p, const T& v){
	return orientedpoint<T,A>(p.x*v, p.y*v, p.theta*v);
}

template <class T, class A>
orientedpoint<T,A> operator * (const T& v, const orientedpoint<T,A>& p){
	return orientedpoint<T,A>(p.x*v, p.y*v, p.theta*v);
}

template <class T>
struct pointcomparator{
	bool operator ()(const point<T>& a, const point<T>& b) const {
		return a.x<b.x || (a.x==b.x && a.y<b.y);
	}	
};

typedef point<int> IntPoint;
typedef point<double> Point;
typedef orientedpoint<double, double> OrientedPoint;

#endif
