 

#include "buffer.h"
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <csignal>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



void exitmsg()
{
  std::cout << "** usage: prdflist infile outfile1 outfile2 ..." << std::endl;
  exit(0);
}


// this function returns o if this not a 
// valid buffer marker, or 1 for a buffer that 
// doesn't need swapping, -1 for one that does need swapping
 
int check_buffermarker ( const unsigned int bm)
{
  
  if ( bm == BUFFERMARKER || 
       bm == GZBUFFERMARKER || 
       bm == LZO1XBUFFERMARKER ||
       bm == LZO1CBUFFERMARKER ||
       bm == LZO2ABUFFERMARKER ||
       bm == BZ2BUFFERMARKER )
    {
      return 1;
    }
  
  else if ( buffer::u4swap(bm) == BUFFERMARKER || 
	    buffer::u4swap(bm) == GZBUFFERMARKER ||
	    buffer::u4swap(bm) == LZO1XBUFFERMARKER ||
	    buffer::u4swap(bm) == LZO1CBUFFERMARKER ||
	    buffer::u4swap(bm) == LZO2ABUFFERMARKER ||
	    buffer::u4swap(bm) == BZ2BUFFERMARKER )
    {
      return -1;
    }


  return 0;
}


int  main(int argc, char *argv[])
{

  unsigned int buffer[8*8192];
  
  int i;
  int fd;

  int nr_outfiles = argc-2;
  int *fdout = new int[nr_outfiles];


  fd = open(argv[1], O_RDONLY | O_LARGEFILE);

  for ( i=0; i< nr_outfiles; i++)
    {
      //      std::cout << "opening file " <<  argv[2+i] << std::endl;

      fdout[i] = open(argv[2+i], O_RDWR | O_CREAT | O_EXCL | O_LARGEFILE , 
		  S_IRWXU | S_IROTH | S_IRGRP);
      if ( fdout[i] < 0)
	{
	  std::cout << " could not open " << argv[2+i] << std::endl;
	  exit(1);
	} 

    }


  int length;
  int ip;

  int total_read = 0;

  int xc;

  int current_fdnr = 0;

  xc = read ( fd, (char *)buffer, 8192);
  //  total_read++;
  while ( xc == 8192 )
    {
 
      ip = 8192;

      int markerstatus;
      if (  (markerstatus = check_buffermarker(buffer[1]) ) )
	{

	  //	  std::cout << " new buffer " << buffer[0] << std::endl;
	  
	  if ( markerstatus == -1)
	    {
	      length = buffer::i4swap(buffer[0]);
	    }
	  else
	    {
	      length = buffer[0];
	    }


	  int nwritten = write (fdout[current_fdnr], buffer, 8192);
	  if ( nwritten < 8192)
	    {
		  std::cout << " could not write output " << total_read << std::endl;
		  exit(1);
	    }

	  while ( ip < length)
	    {
	      xc = read ( fd, (char *)buffer, 8192);
	      if ( xc < 8192 ) 
		{
		  std::cout << "end or error in read loop at rec " << total_read << std::endl;
		  exit(1);
		}
	      total_read++;
	      ip+= 8192;
	      nwritten = write (fdout[current_fdnr], buffer, 8192);
	    }

	}

      if ( ++current_fdnr >= nr_outfiles) current_fdnr = 0;

      //  std::cout << " current fdnr: " << current_fdnr << std::endl;
      xc = read ( fd, (char *)buffer, 8192);

    }
  for ( i=0; i< nr_outfiles; i++)
    {
      close (fdout[i]);
    }
  return 0;
  
}

