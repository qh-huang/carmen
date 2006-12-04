#ifndef CARMEN_CPP_GENERICPOINT_H
#define CARMEN_CPP_GENERICPOINT_H

#include <math.h>

template <class T>
class point2d {
 public:
  inline point2d(){};
  inline point2d(T _x, T _y):x(_x),y(_y){};
  inline virtual ~point2d(){};
  T x, y;
};

template <class T>
inline point2d<T> operator+(const point2d<T>& p1, const point2d<T>& p2){
  return point2d<T>(p1.x+p2.x, p1.y+p2.y);
}

template <class T>
inline point2d<T> operator - (const point2d<T> & p1, const point2d<T> & p2){
  return point2d<T>(p1.x-p2.x, p1.y-p2.y);
}

template <class T>
inline point2d<T> operator * (const point2d<T>& p, const T& v){
  return point2d<T>(p.x*v, p.y*v);
}

template <class T>
inline point2d<T> operator * (const T& v, const point2d<T>& p){
  return point2d<T>(p.x*v, p.y*v);
}

template <class T>
inline T operator * (const point2d<T>& p1, const point2d<T>& p2){
  return p1.x*p2.x+p1.y*p2.y;
}


template <class T>
inline bool operator==(const point2d<T>& p1, const point2d<T>& p2){
  return (p1.x == p2.x && p1.y == p2.y);
}

template <class T>
inline bool operator!=(const point2d<T>& p1, const point2d<T>& p2){
  return !(p1 == p2);
}

////////////////////////////////////////////////////////////////////////////////////////

template <class T, class A>
class orientedpoint2d: public point2d<T>{
 public:
  inline orientedpoint2d(){}
  inline orientedpoint2d(const point2d<T>& p);
  inline orientedpoint2d(T x, T y, A _theta): point2d<T>(x,y), theta(_theta){}
  inline virtual ~orientedpoint2d(){}

  inline orientedpoint2d<T,A> rotate(A alpha){
    T s=sin(alpha), c=cos(alpha);
    A a=alpha+theta;
    a=atan2(sin(a),cos(a));
    return orientedpoint2d( c*this->x-s*this->y,
			    s*this->x+c*this->y, 
			    a);
  }
  
  inline point2d<T> toPoint2d() const {
    return point2d<T>(this->x,this->y);
  }


  A theta;
};


template <class T, class A>
orientedpoint2d<T,A>::orientedpoint2d(const point2d<T>& p){
  this->x=p.x;
  this->y=p.y;
  this->theta=0.;
}


template <class T, class A>
orientedpoint2d<T,A> operator+(const orientedpoint2d<T,A>& p1, const orientedpoint2d<T,A>& p2){
  return orientedpoint2d<T,A>(p1.x+p2.x, p1.y+p2.y, p1.theta+p2.theta);
}

template <class T, class A>
bool operator==(const orientedpoint2d<T,A>& p1, const orientedpoint2d<T,A>& p2){
  return (p1.x == p2.x && p1.y == p2.y &&  p1.theta == p2.theta);
}

template <class T, class A>
orientedpoint2d<T,A> operator - (const orientedpoint2d<T,A> & p1, const orientedpoint2d<T,A> & p2){
  return orientedpoint2d<T,A>(p1.x-p2.x, p1.y-p2.y, p1.theta-p2.theta);
}

template <class T, class A>
orientedpoint2d<T,A> operator * (const orientedpoint2d<T,A>& p, const T& v){
  return orientedpoint2d<T,A>(p.x*v, p.y*v, p.theta*v);
}


template <class T, class A>
orientedpoint2d<T,A> operator * (const T& v, const orientedpoint2d<T,A>& p){
  return orientedpoint2d<T,A>(p.x*v, p.y*v, p.theta*v);
}

template <class T, class A>
orientedpoint2d<T,A> roundMinus(const orientedpoint2d<T,A>& p1,const orientedpoint2d<T,A>& p2){
  orientedpoint2d<T,A> delta=p1-p2;
  delta.theta=atan2(sin(delta.theta), cos(delta.theta));
  double s=sin(p2.theta), c=cos(p2.theta);
  return orientedpoint2d<T,A>(c*delta.x+s*delta.y, 
			    -s*delta.x+c*delta.y, delta.theta);
}

template <class T, class A>
orientedpoint2d<T,A> roundPlus(const orientedpoint2d<T,A>& p1,const orientedpoint2d<T,A>& p2){
  double s=sin(p1.theta), c=cos(p1.theta);
  return orientedpoint2d<T,A>(c*p2.x-s*p2.y,
			    s*p2.x+c*p2.y, p2.theta) + p1;
}

template <class T, class A>
point2d<T> roundPlus(const orientedpoint2d<T,A>& p1,const point2d<T>& p2){
  double s=sin(p1.theta), c=cos(p1.theta);
  return point2d<T>(c*p2.x-s*p2.y, s*p2.x+c*p2.y) + (point2d<T>) p1;
}

template <class T>
struct point2dcomparator{
  bool operator ()(const point2d<T>& a, const point2d<T>& b) const {
    return a.x<b.x || (a.x==b.x && a.y<b.y);
  }	
};

template <class T>
struct point2dradialcomparator{
  point2d<T> origin;
  bool operator ()(const point2d<T>& a, const point2d<T>& b) const {
    point2d<T> delta1=a-origin;
    point2d<T> delta2=b-origin;
    return (atan2(delta1.y,delta1.x)<atan2(delta2.y,delta2.x));
  }	
};


template <class T>
inline double euclidianDist(const point2d<T>& p1, const point2d<T>& p2){
  return hypot(p1.x-p2.x, p1.y-p2.y);
}
template <class T, class A>
inline double euclidianDist(const orientedpoint2d<T,A>& p1, const orientedpoint2d<T,A>& p2){
  return hypot(p1.x-p2.x, p1.y-p2.y);
}
template <class T, class A>
inline double euclidianDist(const orientedpoint2d<T,A>& p1, const point2d<T>& p2){
  return hypot(p1.x-p2.x, p1.y-p2.y);
}
template <class T, class A>
inline double euclidianDist(const point2d<T>& p1, const orientedpoint2d<T,A>& p2 ){
  return hypot(p1.x-p2.x, p1.y-p2.y);
}


template <class T>
inline point2d<T> max(const point2d<T>& p1, const point2d<T>& p2){
  point2d<T> p=p1;
  p.x=p.x>p2.x?p.x:p2.x;
  p.y=p.y>p2.y?p.y:p2.y;
  return p;
}

template <class T, class A>
inline point2d<T> min(const point2d<T>& p1, const orientedpoint2d<T,A>& p2){
  point2d<T> p=p1;
  p.x=p.x<p2.x?p.x:p2.x;
  p.y=p.y<p2.y?p.y:p2.y;
  return p;
}

template <class T, class F>
inline point2d<T> interpolate(const point2d<T>& p1,  
			      const F& t1, 
			      const point2d<T>& p2, 
			      const F& t2, 
			      const F& t3){
  F gain=(t3-t1)/(t2-t1);
  point2d<T> p=p1+(p2-p1)*gain;
  return p;
}

template <class T, class A, class F>
inline orientedpoint2d<T,A> interpolate(const orientedpoint2d<T,A>& p1,  
					const F& t1, 
					const orientedpoint2d<T,A>& p2, 
					const F& t2, 
					const F& t3){
  F gain=(t3-t1)/(t2-t1);
  orientedpoint2d<T,A> p;
  p.x=p1.x+(p2.x-p1.x)*gain;
  p.y=p1.y+(p2.y-p1.y)*gain;
  double  s=sin(p1.theta)+sin(p2.theta)*gain,
    c=cos(p1.theta)+cos(p2.theta)*gain;
  p.theta=atan2(s,c);
  return p;
}

#endif
