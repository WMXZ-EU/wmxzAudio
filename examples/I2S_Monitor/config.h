//Copyright 2017 by Walter Zimmer
// Version 22-07-17
//
#ifndef CONFIG_H
#define CONFIG_H

/****************** Custom ************************************************/
#define DO_DEBUG
//#undef DO_DEBUG

#define F_SAMP 100000

#define FMT "0_%05u.dat"    // defines filename

#define MXFN 100         // maximal number of files


//#define MAX_BLOCK_COUNT 1000  // number of BUFFSIZE writes to file (defines file size)

/*
  for nd=128+3
      ndb=nd*16;
      
      fprintf('   fs     nd     i1     i2      j1    j2      k     i3    blk    st\n')
      fprintf('--------------------------------------------------------------------\n')
      mn = 60;
      clst=128*1024;
      rec=32*1024;
  
      for fs=[48,50,60,70,75,80,100,125]
  
          ix = find(mod(512+(1:1000000)*ndb,clst)==0,2); % number of acq to fill up cluster
          i1=ix(1);
          i2=diff(ix);
  
          jx= (512+ix*ndb)/clst;
          j1=jx(1);
          j2=diff(jx);
  
          k=find((512+(i1+(1:10000)*i2)*(ndb+3*16))/(fs*1000*16)>60,1);
  
          i3=(i1+k*i2);
          nx=(512+i3*(ndb-3*16));
  
          nbl=(512+i3*ndb)/rec;
          dt=nx/(fs*1000*16);
  
          fprintf('%5d, ',[fs,nd,i1,i2,j1,j2,k,i3,nbl]), fprintf(' %.1f\n',dt);
      end
      fprintf('\n');
  end
 * 
 * 
   fs     nd     i1     i2      j1    j2      k     i3    blk    st
--------------------------------------------------------------------
   48,   131,  6816,  8192,   109,   131,     2, 23200,  1484,  61.9
   50,   131,  6816,  8192,   109,   131,     2, 23200,  1484,  59.4
   60,   131,  6816,  8192,   109,   131,     3, 31392,  2008,  67.0
   70,   131,  6816,  8192,   109,   131,     3, 31392,  2008,  57.4
   75,   131,  6816,  8192,   109,   131,     4, 39584,  2532,  67.6
   80,   131,  6816,  8192,   109,   131,     4, 39584,  2532,  63.3
  100,   131,  6816,  8192,   109,   131,     5, 47776,  3056,  61.2
  125,   131,  6816,  8192,   109,   131,     7, 64160,  4104,  65.7
 *
 */

// for 1 minute data per file  
// N_K = ceil((60*F_SAMP - 6816*128)/(128*8*1024)
// approximate:
//  #define N_K 				((60*F_SAMP)>>20)	// number of super clusters per file
//  #define MAX_BLOCK_COUNT	(4*(109+N_K*131))	// for (128 +3)record size

#define MAX_BLOCK_COUNT (10*131) // about 26 seconds
#endif

