
#include <dSFMT-calc-jump.hpp>
#include <dSFMT-jump.h>
#include <dSFMT.h>
#include <NTL/GF2X.h>
#include <NTL/ZZ.h>
#include <fstream>


using namespace std;
using namespace NTL;
using namespace dsfmt;


int main(int argc, char** argv)
{
  dsfmt_t dsfmt;
  unsigned int i;

  // set random seed
  //
  dsfmt_init_gen_rand(&dsfmt, 1);


  // get random value ( rank 0 )
  //
  printf("Imitation rank:0\n");
  for(i = 0 ; i < 10; ++i)
    printf("I:%d V:%f\n", i, dsfmt_genrand_close1_open2(&dsfmt));
  

  // setup poly
  //
  GF2X poly;

  ifstream ifs("./poly.19937.txt");
  string line;
  if (ifs)
    {
      ifs >> line;
      line = "";
      ifs >> line;
    }
  stringtopoly(poly, line);


  // jump (2^1000 / 2 * 1)
  //

  ZZ z;
  z = 1;
  for(i = 0; i < 1000; ++i)
    z *= 2;

  z /= 2;   // means: # of process is 2
  z *= 1;   // means: process rank is 1

  string countStr;
  calc_jump(countStr, z, poly);

  dsfmt_init_gen_rand(&dsfmt, 1);
  dSFMT_jump(&dsfmt, countStr.c_str());


  // get random value ( rank 1 )
  //
  printf("Imitation rank:1\n");
  for(i = 0 ; i < 10; ++i)
    printf("I:%d V:%f\n", i, dsfmt_genrand_close1_open2(&dsfmt));
  
  return 0;
}
