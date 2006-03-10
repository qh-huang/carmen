/* File : pyCarmen.i */

%module(directors="1") pyCarmen
%{
#include "pyCarmenMessages.h"
%}

//%typemap(memberin) float * {
 // int i;
 //int size = PyList_Size($input);
//
//  float $1[size];
//  	
//  for (i = 0; i < size; i++) {
//      $1[i] = $input[i];
//  }
//}

//// Map a Python sequence into any sized C double array
%typemap(memberin, python) float* {
  int i, my_len;
  if (!PySequence_Check($input)) {
      PyErr_SetString(PyExc_TypeError,"Expecting a sequence");
      return NULL;
  }

  my_len = PyObject_Length($input);
  float temp[my_len];
  for (i =0; i < my_len; i++) {
      PyObject *o = PySequence_GetItem($input,i);
      if (!PyFloat_Check(o)) {
         PyErr_SetString(PyExc_ValueError,"Expecting a sequence of floats");
         return NULL;
      }
      temp[i] = PyFloat_AsDouble(o);
  }
  $1 = &temp[0];
}

//%typemap(out) float* {
//  int i, size;
//  size = sizeof($1);
//  $result = PyList_New(size);
//
//  for  for (i=0; i < size; i++){
//
//    PyObject *o = PyFloat_FromDouble((double)$1[i]);
//    PyList_SetItem($result, i, o);
//  }
//}

//%typemap(out) float** {
//  int i, size1;
//  size1 = sizeof($1);///sizeof(float*);
//
//  printf("in typemap\n");
//  $result = PyList_New(size1);
//  for (i=0; i < size1; i++){
//
//    int j, size2;
//    carmen_warn("in typemap3\n");
//    size2 = sizeof($1[i]);
//    carmen_warn("size2 %d\n", size2);
//
//    PyObject *tmp = PyList_New(size2);
//    carmen_warn("in typemap4\n");
//    for(j=0; j < size2; j++){
//       carmen_warn("in typemap5\n");
//       PyObject *o = PyFloat_FromDouble((double)$1[i][j]);
//       carmen_warn("in typemap6\n");
//       PyList_SetItem(tmp, j, o);
//       carmen_warn("in typemap7\n");
//    } 
//    carmen_warn("in typemap8\n");
//    PyList_SetItem($result, i, tmp);
//  }
//  printf("in typemapend\n");
//}

//%typemap(out) float** {
//  int i, size1;
//  size1 = sizeof($1);///sizeof(float*);
//
//  printf("in typemap\n");
//  $result = PyList_New(size1);
//  for (i=0; i < size1; i++){
//
//    int j, size2;
//    carmen_warn("in typemap3\n");
//    size2 = sizeof($1[i]);
//    carmen_warn("size2 %d\n", size2);
//
//    PyObject *tmp = PyList_New(size2);
//    carmen_warn("in typemap4\n");
//    for(j=0; j < size2; j++){
//       carmen_warn("in typemap5\n");
//       PyObject *o = PyFloat_FromDouble((double)$1[i][j]);
//       carmen_warn("in typemap6\n");
//       PyList_SetItem(tmp, j, o);
//       carmen_warn("in typemap7\n");
//    } 
//    carmen_warn("in typemap8\n");
//    PyList_SetItem($result, i, tmp);
//  }
//  printf("in typemapend\n");
//}


// Map a Python sequence into any sized C double array
%typemap(in, python) float* {
  int i, my_len;
  if (!PySequence_Check($input)) {
      PyErr_SetString(PyExc_TypeError,"Expecting a sequence");
      return NULL;
  }

  my_len = PyObject_Length($input);
  float temp[my_len];
  for (i =0; i < my_len; i++) {
      PyObject *o = PySequence_GetItem($input,i);
      if (!PyFloat_Check(o)) {
         PyErr_SetString(PyExc_ValueError,"Expecting a sequence of floats");
         return NULL;
      }
      temp[i] = PyFloat_AsDouble(o);
  }
  $1 = &temp[0];
}



//%typemap(memberout) float [ANY] {
//  int i;
//  $result = PyList_New($1_dim0);
//  for (i = 0; i < $1_dim0; i++) {
//    PyObject *o = PyFloat_FromDouble((double) $1[i]);
//    PyList_SetItem($result,i,o);
//  }
//}


%include "std_string.i"
%include "typemaps.i"
%include "carrays.i"
%array_class(double, doubleArray);
%array_class(float, floatArray);

/* turn on director wrapping Callback */
%feature("director") MessageHandler;
#define __attribute__(x)
%include "carmen.h"
%include "global.h"
%include "map.h"

%include "pyCarmen.h"
%include "pyCarmenMessages.h"




%include arm_messages.h
%include base_messages.h
%include camera_messages.h
%include gps_nmea_messages.h
%include laser_messages.h
%include localize_messages.h
%include logger_messages.h
%include playback_messages.h
%include map_messages.h
%include navigator_messages.h
%include param_messages.h
%include robot_messages.h
%include simulator_messages.h
//%include hokuyo_messages.h
//%include pantilt_messages.h
//%include proccontrol_messages.h
//segway_messages.h


%include arm_interface.h
%include base_interface.h
%include camera_interface.h
%include gps_nmea_interface.h
%include laser_interface.h
%include localize_interface.h
%include playback_interface.h
%include map_interface.h
%include navigator_interface.h
%include param_interface.h
%include robot_interface.h
%include simulator_interface.h

//%include planner_interface.h
//%include hokuyo_interface.h
//%include pantilt_interface.h
//%include proccontrol_interface.h
//%include segway_interface.h



// This tells SWIG to treat char ** as a special case
%typemap(in) char ** {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
	$1[i] = PyString_AsString(PyList_GetItem($input,i));
      else {
	PyErr_SetString(PyExc_TypeError,"list must contain strings");
	free($1);
	return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

// This cleans up the char ** array we malloc'd before the function call
%typemap(freearg) char ** {
  free((char *) $1);
}



%include "ipc_wrapper.h"
