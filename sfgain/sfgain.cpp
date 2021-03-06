// Alter gain of a soundfile and create a new sf
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_AMPFAC, ARG_NARGS};

int main(int argc, char** argv){

  PSF_PROPS props;
  long framesread, totalread;
  // Init all resources
  int ifd = -1, ofd = -1, i;
  int error = 0;
  psf_format outformat = PSF_FMT_UNKNOWN;
  PSF_CHPEAK* peaks = NULL;
  float* frame = NULL;
  float ampfac;

  printf("SFGAIN: change level of a soundfile\n");

  if(argc < ARG_NARGS){
    printf("Insufficient arguments.\n" 
        "usage:\n\t"
        "sf2float infile outfile ampface\n"
        "\twhere ampfac must be >0\n");
    return 1;
  }
  // Start up portsf
  if(psf_init()){
    printf("unable to start portsf\n");
    return 1;
  }

  ifd = psf_sndOpen(argv[ARG_INFILE], &props, 0);
  if(ifd < 0){
    printf("error: unable to open infile %s\n",
        argv[ARG_INFILE]);
    return 1;
  }

  // Tell user if source file is already floats
  if(props.samptype == PSF_SAMP_IEEE_FLOAT){
    printf("Info: infile is already in floats format.\n");
  }

  props.samptype = PSF_SAMP_IEEE_FLOAT;

  // Check outfile extension is one we know
  outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
  if(outformat == PSF_FMT_UNKNOWN){
    printf("outfile name %s has unknown format.\n"
        "Use any of .wav, .aiff, .aif, .afc, .aifc\n",
        argv[ARG_OUTFILE]);
    error++;
    goto exit;
  }

  props.format = outformat;

  ofd = psf_sndCreate(argv[2], &props, 0, 0, PSF_CREATE_RDWR);
  if(ofd < 0){
    printf("Error: unable to create outfile %s\n", argv[ARG_OUTFILE]);
    error++;
    goto exit;
  }

  ampfac = (float) atof(argv[ARG_AMPFAC]);
  if(ampfac <= 0.0){
    printf("Error: ampfac must be greater than 0.\n");
    return 1;
  }

  // Allocate space for one sample frame
  frame = (float*) malloc(props.chans * sizeof(float));
  if(frame == NULL){
    puts("No memory!\n");
    error++;
    goto exit;
  }

  // And allocate space for PEAK info
  peaks = (PSF_CHPEAK*) malloc(props.chans * sizeof(PSF_CHPEAK));
  if(peaks == NULL){
    puts("No memory!\n");
    error++;
    goto exit;
  }
  printf("copying....");

  // Single-frame loop to do copy, report any errors
  framesread = psf_sndReadFloatFrames(ifd, frame, 1);
  totalread = 0;
  while(framesread == 1){
    totalread++;
    for(i = 0; i < props.chans; i++){
      frame[i] *= ampfac;
      if(psf_sndWriteFloatFrames(ofd, frame, 1) != 1){
        printf("error writing to outfile\n");
        error++;
        break;
      }
    }
    // Do any processing here
    framesread = psf_sndReadFloatFrames(ifd, frame, 1);
  }

  if(framesread < 0){
    printf("Error reading infile. Outfile is incomplete.\n");
    error++;
  }else{
    printf("Done. %ld sample frames copied to %s\n",
            totalread, argv[ARG_OUTFILE]);
  }
  // Report peak values to user
  if(psf_sndReadPeaks(ofd, peaks, NULL) > 0){
    double peaktime;
    printf("PEAK information:\n");
    for(i = 0; i < props.chans; i++){
      peaktime = (double) peaks[i].pos / props.srate;
      printf("CH %lf:\t%.4f at %.4f secs\n",
          i+1.0, peaks[i].val, peaktime);
    }
  }
exit:
  if(ifd >= 0)
    psf_sndClose(ifd);
  if(ofd >= 0)
    psf_sndClose(ofd);
  if(frame)
    free(frame);
  if(peaks)
    free(peaks);
  psf_finish();
  return error;
}
