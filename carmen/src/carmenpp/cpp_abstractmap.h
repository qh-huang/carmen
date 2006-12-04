#ifndef CARMEN_CPP_ABSTRACT_MAP_H
#define CARMEN_CPP_ABSTRACT_MAP_H

#include <carmen/carmen.h>
#include "cpp_point.h"


class MapConfig {
 public:
  inline MapConfig();
  virtual ~MapConfig() {};
  inline MapConfig(int sizeX, int sizeY,  double res, Point offset);
  inline MapConfig(const MapConfig& src);
  inline bool isValid() const ;

  int m_sizeX;
  int m_sizeY;
  double m_res;
  Point m_offset;
};

inline bool operator==(const MapConfig& cfg1, const MapConfig& cfg2)  {
  if (cfg1.m_sizeX  == cfg2.m_sizeX && 
      cfg1.m_sizeY  == cfg2.m_sizeY && 
      cfg1.m_res    == cfg2.m_res && 
      cfg1.m_offset == cfg2.m_offset)
    return true;
  else
    return false;
};

inline bool operator!=(const MapConfig& cfg1, const MapConfig& cfg2)  {
  return !(cfg1 == cfg2);
};

///////////////////////////////////////////////////////////////////////////

template <class CELL>
class AbstractMap {
 public:
  AbstractMap();
  AbstractMap(const AbstractMap& x);
  AbstractMap(const MapConfig& cfg);
  virtual ~AbstractMap();  

  // Vitual function to be defined in subclass
  virtual void init(const MapConfig& cfg) = 0;
  virtual const CELL& defaultCell() const = 0;

  virtual CELL& cell(const IntPoint& p) = 0;  
  virtual CELL& cell(const IntPoint& p) const = 0;

  // non-virtual function for the map handling
  void init(int sizeX, int sizeY, double res=1.0, Point offset = Point(0.0, 0.0) ); 
  void init(double xfrom, double xto, double yfrom, double yto, double res);
  void initIfSmaller(int sizeX, int sizeY, double res=1.0, Point offset = Point(0.0, 0.0) ); 

  const MapConfig& getConfig() const;

  inline  IntPoint getMin() const;
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

