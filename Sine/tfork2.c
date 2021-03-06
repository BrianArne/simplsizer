// Alternate tuning fork generator based on expbrk.c
// Decay is always to silence regardless of duration

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.141592654)
#endif

enum { ARG_NAME, ARG_OUTFILE, ARG_DUR,
  ARG_HZ, ARG_SR, ARG_AMP, ARG_NARGS};

int main(int argc, char** argv){
  int i, sr, nsamps;
  double samp, dur, freq, srate, amp, maxsamp;
  double start, end, fac, angleincr;
  double twopi = 2.0 * M_PI;
  float fsamp;
  FILE* fp = NULL;

  if(argc != ARG_NARGS){
    printf("usage: tfork2 outfile.txt dur freq srate amp\n");
    return 1;
  }

  fp = fopen(argv[ARG_OUTFILE], "wb");
  if(fp == NULL){
    printf("Error creating output file %s\n", argv[ARG_OUTFILE]);
    return 1;
  }

  dur = atof(argv[ARG_DUR]);
  freq = atof(argv[ARG_HZ]);
  srate = atof(argv[ARG_SR]);
  amp = atof(argv[ARG_AMP]);
  nsamps = (int) (dur * srate);
  angleincr = twopi * freq / srate;

  start = 1.0;
  end = 1.0e-4;
  maxsamp = 0.0;
  fac = pow(end/start, 1.0/nsamps);

  for(i = 0; i < nsamps; i++){
    samp = amp * sin(angleincr * i);
    samp *= start;
    start *= fac;
    fsamp = (float) samp;
    fprintf(fp, "%.8lf\n", samp);
    /*
    if(fwrite(&fsamp, sizeof(float), 1, fp) != 1){
      printf("error writing data to disk\n");
      return 1;
    }
    */
    if(fabs(samp > maxsamp)){
      maxsamp = fabs(samp);
    }
  }
  fclose(fp);
  printf("done. Maximum sample value = %.8lf\n", maxsamp);
  return 0;
}
