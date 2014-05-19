#ifndef SPINNER_H
#define SPINNER_H

class spinner
{
  
public:
  
  // Constructor / Destructor
  spinner(int max);
  ~spinner();
  
  // Mutator
  void tick(int count = 1);
  
protected:
  
  int cur;
  int max;
  int width;
  int old_pos;
  
};

#endif
