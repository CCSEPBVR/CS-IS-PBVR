
#include <dSFMT.h>

int main(int argc, char** argv)
{
  dsfmt_t dsfmt;
  long long ll;
  double v;

  // set random seed
  //
  dsfmt_init_gen_rand(&dsfmt, 1);


  // get random value
  //
  printf("10,000,000,000 loop\n");

  for(ll = 0LL ; ll < 10000000000LL; ++ll)
    v = dsfmt_genrand_close1_open2(&dsfmt);

  printf("  last value = %f\n", v);

  return 0;
}
