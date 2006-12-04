#ifndef CARMEN_CPP_GENERIC_MAP_H
#define CARMEN_CPP_GENERIC_MAP_H

#include "cpp_point.h"
#include "cpp_abstractmap.h"

template<class CELL>
class GenericMap : public AbstractMap<CELL> {
 public:
  GenericMap();
  GenericMap(const MapConfig& cfg);
  GenericMap(const GenericMap<CELL>& src);
  virtual ~GenericMap();  

  virtual void init(const MapConfig& cfg);  
  virtual const CELL& defaultCell() const;
  
  inline CELL& cell(const IntPoint& p) ;
  inline CELL& cell(const IntPoint& p) const ;

 protected:
  CELL*  m_maplinear;
  CELL** m_map;
  CELL   m_defaultCell;
};

#include "cpp_genericmap.hxx"

#endif

