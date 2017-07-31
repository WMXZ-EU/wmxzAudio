//Copyright 2017 by Walter Zimmer
// Version 22-06-17
//
#ifndef LOGGER_H
#define LOGGER_H

/*********************** system ******************************************/
#define INF ((uint32_t) (-1))

typedef struct
{
  uint32_t rtc;
  uint32_t t0;
  uint32_t nch;
  uint32_t fsamp;
  uint32_t fsize;
  uint32_t nsamp;
  uint32_t hsize;
  uint32_t nclst;
  uint32_t fill[128-8];
} header_s;

// function prototypes
void logger_init(header_s * header);
void logger_write(uint8_t *data, uint16_t nbuf);
uint32_t logger_save(void);

#endif

