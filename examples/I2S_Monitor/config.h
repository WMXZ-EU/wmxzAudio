//Copyright 2017 by Walter Zimmer
// Version 22-07-17
//
#ifndef config_h
#define config_h

/****************** Custom ************************************************/
#define DO_DEBUG
//#undef DO_DEBUG

#define F_SAMP 100000

#define FMT "0_%05u.dat"    // defines filename

#define MXFN 2           // maximal number of files 


//#define MAX_BLOCK_COUNT 1000  // number of BUFFSIZE writes to file (defines file size)

/*
  fprintf('   fs     nd     i1     i2      k     i3    blk    st\n')
  fprintf('-----------------------------------------------------\n')
  mn = 60;
  clst=128*1024;
  rec=32*1024;
  
  for fs=[48,50,60,70,75,80,100]
      nd=floor(fs*128/44.1)+3;
      ndb=nd*16;
      
      ix = find(mod(512+(1:100000)*ndb,clst)==0,2); % number of acq to fill up cluster
      i1=ix(1);
      i2=diff(ix);
  
      k=find((512+(i1+(1:10000)*i2)*ndb)/(fs*1000*16)>60,1);
  
      i3=(i1+k*i2);
      nx=(512+i3*ndb);
      
      nbl=nx/rec;
      
      dt=nx/(fs*1000*16);
      
      fprintf('%5d, ',[fs,nd,i1,i2,k,i3,nbl]), fprintf(' %.1f\n',dt);
  end
 * >>
   fs     nd     i1     i2      k     i3    blk    st
-----------------------------------------------------
   48,   142,  2192,  4096,     5, 22672,  1572,  67.1
   50,   148,   664,  2048,    10, 21144,  1528,  62.6
   60,   177,  5600,  8192,     2, 21984,  1900,  64.9
   70,   206,  2704,  4096,     5, 23184,  2332,  68.2
   75,   220,   968,  2048,    10, 21448,  2304,  62.9
   80,   235,  1952,  8192,     3, 26528,  3044,  77.9
  100,   293,  2656,  8192,     3, 27232,  3896,  79.8
 *
 */

// for 1 minute data per file
#if F_SAMP == 48000
  #define MAX_BLOCK_COUNT (1572)
  #define N_AUDIO_BUF 140
#elif F_SAMP == 50000
  #define MAX_BLOCK_COUNT (1528)
  #define N_AUDIO_BUF 146
#elif F_SAMP == 60000
  #define MAX_BLOCK_COUNT (1900)
  #define N_AUDIO_BUF 175
#elif F_SAMP == 70000
  #define MAX_BLOCK_COUNT (2332)
  #define N_AUDIO_BUF 204
#elif F_SAMP == 75000
  #define MAX_BLOCK_COUNT (2304)
  #define N_AUDIO_BUF 218
#elif F_SAMP == 80000
  #define MAX_BLOCK_COUNT (3044)
  #define N_AUDIO_BUF 233
#elif F_SAMP == 100000
  #define MAX_BLOCK_COUNT (3896)
  #define N_AUDIO_BUF 291
#elif F_SAMP == 125000
  #define MAX_BLOCK_COUNT (3896)
  #define N_AUDIO_BUF 363
#else
  #error "F_SAMP not implemented"
#endif

#undef MAX_BLOCK_COUNT
//  #define MAX_BLOCK_COUNT (3896)

  #define MAX_BLOCK_COUNT (((60*F_SAMP*16)/(128*1024))*4)

//  #define N_DATA_BUF  (4*127) // 4*16 + (4*127*16)
//  #define N_DATA_BUF  (2*126) // 4*16 + (2*126*16)
  #define N_DATA_BUF  (1*124) // 4*16 + (1*124*16)

#endif

