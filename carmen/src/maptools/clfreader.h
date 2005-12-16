#ifndef CLFREADER_H
#define CLFREADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include "point.h"

#define CLF_MAX_LINE_LENGHT (100000)

namespace CLFReader{
	
using namespace std;

/////////////////////////////////////////////////////////////////

class CLFRecord{
 public:
  CLFRecord();
  virtual ~CLFRecord();
  virtual void read(istream& is)=0;
  virtual void write(ostream& os)const =0 ;
  virtual string id() const{ return "NO-TYPE"; };

  unsigned int dim;
  double time;
};

/////////////////////////////////////////////////////////////////

class CLFCommentRecord: public CLFRecord {
 public:
  CLFCommentRecord();
  virtual void read(istream& is);
  virtual void write(ostream& os) const ;
  virtual string id() const{ return "COMMENT"; };
  string text;
};


/////////////////////////////////////////////////////////////////

class CLFTruePoseRecord: public CLFRecord{
 public:
  CLFTruePoseRecord();
  void read(istream& is);
  virtual void write(ostream& os)const ;
  virtual string id() const{ return "TRUEPOS"; };

  OrientedPoint truePose;
  OrientedPoint odomPose;
};

/////////////////////////////////////////////////////////////////

class CLFOdometryRecord: public CLFRecord{
 public:
  CLFOdometryRecord();
  virtual void read(istream& is);
  virtual void write(ostream& os)const ;
  virtual string id() const{ return "ODOM"; };

  OrientedPoint pose;
  double tv, rv, acceleration;
};

/////////////////////////////////////////////////////////////////

class CLFLaserRecord: public CLFRecord{
 public:
  CLFLaserRecord(int laserid);
  virtual void read(istream& is);
  virtual void write(ostream& os)const ;
  virtual string id() const;

  vector<double> readings;
  OrientedPoint laserPose;
  OrientedPoint odomPose;
  int laserID;
};

/////////////////////////////////////////////////////////////////

class CLFRecordList: public deque<CLFRecord*>{
 public:
  CLFRecordList(bool showDebug = false);
  virtual ~CLFRecordList() {};
  istream& read(istream& is);
  virtual CLFRecord* createRecord(string recordType);
  
  bool showDebug;
};

/////////////////////////////////////////////////////////////////

} //end namespace

#endif
