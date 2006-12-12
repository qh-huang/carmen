#ifndef CARMEN_CPP_ABSTRACT_MAP_H
#define CARMEN_CPP_ABSTRACT_MAP_H

#include <carmen/carmen.h>
#include <carmen/cpp_point.h>
#include <carmen/cpp_mapconfig.h>

template <class CELL>
class AbstractMap {
 public:
  AbstractMap();
  AbstractMap(const AbstractMap& x);
  AbstractMap(const MapConfig& cfg);
  virtual ~AbstractMap();  


  virtual CELL& getCell(int x, int y) = 0;
  virtual CELL& getCell(int x, int y) const = 0;
  

  // Vitual function to be defined in subclass
  virtual void init(const MapConfig& cfg) = 0;
  virtual const CELL& defaultCell() const = 0;

  // non-virtual function for the map handling
  void init(int sizeX, int sizeY, double res=1.0, Point offset = Point(0.0, 0.0) ); 
  void init(double xfrom, double xto, double yfrom, double yto, double res);
  void initIfSmaller(int sizeX, int sizeY, double res=1.0, Point offset = Point(0.0, 0.0) ); 

  const MapConfig& getConfig() const;

  inline IntPoint getMin() const;
  inline IntPoint getMax() const;
  inline int getMapSizeX() const;
  inline int getMapSizeY() const;

  inline void setOffset(const Point& p);
  inline Point getOffset() const;

  inline void setResolution(double res);
  inline double getResolution() const;
  
  inline Point world2map_double(const Point& p) const;
  inline Point map2world(const Point& p) const;
  inline IntPoint world2map(const Point& p) const;
  inline Point map2world(const IntPoint& p) const;
  inline Point map2world(int x, int y) const;
  inline IntPoint world2map(double x, double y) const;
  
  inline bool isInside(const Point& p) const;
  inline bool isInside(const IntPoint& p) const;
  inline bool isInside(int x, int y) const;
  inline bool isInside(double x, double y) const;
  
  inline IntPoint minInside(const IntPoint& p) const;
  inline IntPoint maxInside(const IntPoint& p) const;
  inline IntPoint putInside(const IntPoint& p) const;

  inline CELL& cell(const IntPoint& p);  
  inline CELL& cell(const IntPoint& p) const;

  inline CELL& cell(int x, int y);
  inline CELL& cell(int x, int y) const;

  inline CELL& cell(double x, double y);
  inline CELL& cell(double x, double y) const;
  
  inline CELL& cell(const Point& p) const ;
  inline CELL& cell(const Point& p);

  void copy(const AbstractMap<CELL>& src, const IntPoint& relative_offset);
  void moveMap(int dx, int dy);

  void resetCells();
  void resetCells(const CELL& val);
  void resetCells(const CELL& val, const IntPoint& from, const IntPoint& to);
 public:
  MapConfig m_cfg;
};

#include "cpp_abstractmap.hxx"

#endif

