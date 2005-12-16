#ifndef DYNAMICTABLE_H
#define DYNAMICTABLE_H

#include <stdlib.h>

/** a dynnamic array of pointer to elements of type T 
  * witch grows automaticly.
  *@author Cyrill Stachniss
  */
template<class T>
class DynamicTable {
public: 
  /** O(1) */
  DynamicTable(int s=10);

  /** O(n)  */
  DynamicTable(DynamicTable<T>& tab, int desired_size=-1);

  /** O(1) if autodelete==false, else O(n) */
  ~DynamicTable();

  /** sets the autodelete variable -> see autoDelete in this class , O(1)*/
  void setAutoDelete(bool del);

  /** O(1) */
  int numberOfElements() const ;
  /** O(1), does the same as numberOfElements */
  int num() const;

  /** O(1), allocated memory (size of array) */
  int allocated() const;

  /** O(n) */
  bool contains(T* element) const;

  /** O(1) */
  T* getElement(int i);
  /** O(1) */
  T* getFirst();
  /** O(1) */
  T* getLast();
  /** O(1) */
  T** getElements();

  /** O(1) */
  const T* getElement(int i) const ;
  /** O(1) */
  const T* getFirst() const ;
  /** O(1) */
  const T* getLast() const ;
  /** O(1) */
  const T** getElements() const ;

  
  /** O(1) */
  void add(T* element);
  /** O(n + nOther) */
  void add(DynamicTable<T>& otherTab);

  /** O(1) */
  T* replace(int idx, T* elem);
  /** O(1) */
  void swap(int idx1, int idx2);


  /**  O(1), BUT DISTRUBS THE TABLE, CALL CONSOLIDATE AFTER THIS!!!!!!!!*/
  void remove_unconsolidated(int i);
  /*** O(n) **/
  void consolidate();


  /** O(1) */
  void removeLast();
  /** Destroys the order ,O(1) */
  void remove(int i);
  /** O(n) */
  void remove(T* element);
  /** O(n) */
  void removeByValue(T* element) { remove(element); }
  /** maintais the order, O(n) */
  void removeInOrder(int i);

  /** O(1) if autodelete==false, else O(n) */
  void clear();

  /** O(n) */
  void reverse();

  /** O(n + nOther) */
  DynamicTable<T>* getIntersection(DynamicTable<T>* other);


protected:
  /** Array von Zeigern auf die einzelnen fields-Elemente */
  T** fields;
  int size;
  int nextField;
		
  /** delete the elements in the table during destruction */
  bool autoDelete;
};


#include "dynamictable.hxx"

#endif
