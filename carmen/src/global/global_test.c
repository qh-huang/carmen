#include <carmen/carmen.h>

void test_carmen_int_random(void)
{
  int index;
  int bins[100];
  int rand;

  memset(bins, 0, 100*sizeof(int));

  for (index = 0; index < 10000; index++) {
    rand = carmen_int_random(100);
    bins[rand]++;
  }

  for (index = 0; index < 100; index++) {
    if (fabs(bins[index] - 100) > 30)
      carmen_warn("Failed: %d %d\n", index, bins[index]);
  }
    
}

int main(int argc __attribute__ ((unused)), 
	 char *argv[] __attribute__ ((unused))) 
{
  test_carmen_int_random();
  
  return 0;
}
