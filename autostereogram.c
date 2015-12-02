/* Tyler Niles */
/* April 2010 */

/* autostereogram.c */
/* Generates Single Image Random-Dot Stereograms given a Z(x,y)
 * height map of a 3d object.       */
/* Only accepts Binary or Raw files */

// INPUT : Raw image file representing the height map of a 3d Image, IEEE Float (0.0-2.0)
// OUTPUT: Raw image file SIRDS, IEEE Float (0.0-1.0)

/* Incorporates the SIRDS algorithm from
 * Thimbleby et. al, Algorithms for SIRDS, 1994. */

/* Use the command
 *	gcc -Wall -g -o test autostereogram.c
 * to compile the program */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#define nextargi (--argc,atoi(*++argv))		// Handle Command Line
#define nextargf (--argc,atof(*++argv))
#define nextargs (--argc,*++argv)

/* Autostereogram defines&macros */
#define round(X) (int)((X)+0.5) /* Often need to round rather than truncate */
#define DPI	 72		/* Output device has 72 pixels per inch */
#define E	 round(2.5*DPI)	/* Eye separation is assumed to be 2.5 in */
#define mu 	 (1/3.0)	/* Depth of field (fraction of viewing distance) */
/* Stereo separation corresponding to position Z */
#define separation(Zpix) round( (1-mu*(Zpix))*E / (2-mu*(Zpix)) )
#define far separation(0) 	/* ... and corresponding to far plane, Zpix=0 */

#if 1
typedef float PIXEL;		// define pixel datatype
#else
typedef char PIXEL;		//
#endif

typedef struct{
	PIXEL *img;		// pointer to image elements
	int   xmax;
	int   ymax;
	int   zmax;
	int  elems;	// sizeof(PIXEL)*xmax*ymax*zmax
} IMAGE;


/* ***************************************************
 * Function   : copyimage
 * Arguments  : IMAGE *copy_to, IMAGE *to_copy
 * Returns    : 0 for success, 0xDEAD for failed
 * Description: Allocate mem and copy image attributes
 * **************************************************/
int copyimage(IMAGE *eve, IMAGE *adam)
{
  eve->xmax  = adam->xmax;
  eve->ymax  = adam->ymax;
  eve->zmax  = adam->zmax;
  eve->elems = adam->elems;
  eve->img   = malloc((adam->xmax)*(adam->ymax)*(adam->zmax)*sizeof(PIXEL));
  if(!(eve->img)) 
    exit(0xDEAD);	// memory allocation failed? Exit immediately
  return 0;
}

/* ***************************************************
 * Function   : removeimage
 * Arguments  : IMAGE *I
 * Returns    : 0 for success
 * Description: Frees memory dynamically allocated
 * 		for an IMAGE ptr.
 * **************************************************/
int removeimage(IMAGE *I)
{
  free(I->img);
  free(I);
  return 0;
}

/* ***************************************************
 * Function   : read_raw
 * Arguments  : const char *fname, IMAGE *data
 * Returns    : 0 for success
 * Description: reads into memory a raw image file
 * **************************************************/
int read_raw(const char *fname, IMAGE *data)
{
	FILE *F;				// input file handle
	data->img = malloc( (data->xmax)*(data->ymax)*(data->zmax)*sizeof(PIXEL) );	// allocate memory
	if(!data->img) return -1;		// exit if operation failed
	F = fopen(fname, "rb");			// open file for read
	data->elems = fread(data->img, sizeof(PIXEL), (data->xmax)*(data->ymax)*(data->zmax), F);
	if(data->elems!=(data->xmax)*(data->ymax)*(data->zmax))
		return -1;			// size mismatch error
	return 0;				// success
}

/* ***************************************************
 * Function   : write_raw
 * Arguments  : const char *fname, IMAGE *data
 * Returns    : 0 for success
 * Description: writes a raw image in memory to file
 * **************************************************/
int write_raw(const char *fname, IMAGE *data)
{
	FILE *F;				// output file handle
	F = fopen(fname, "wb");			// open file for write
	data->elems = fwrite(data->img, sizeof(PIXEL), (data->xmax)*(data->ymax)*(data->zmax), F);
	if(data->elems!=(data->xmax)*(data->ymax)*(data->zmax))
		return -1;			// size mismatch error
	return 0;				// success
}

/* ****************************************************
 * Function   : heightmap
 * Arguments  : IMAGE *map = input image or height map
 * 		(to be modified).
 * Returns    : 0 for success
 * Description: Scale values to 0<=x<=1 to prepare for
 * 		the function DrawAutoStereogram
 * ***************************************************/
int heightmap(IMAGE *map)
{
  int i;
  double temp;
  double maxval;

  /* Find maximum value in input image */
  maxval = 0;
  for(i=0; i<map->elems; i++)
  {
	  temp = *((map->img) + i);
	  if(maxval < temp)
		  maxval = temp;
  }

  if(!maxval) return -1;  /* completely black image */

  //printf("maxval is %f\n",maxval);
  maxval = ceil(maxval);
  //printf("adjusted maxval is %f\n",maxval);

  /* Scale down image by maximum value */
  if(maxval > 1)
  {
    for(i=0; i<map->elems; i++)
    {
	*((map->img) + i) = *((map->img) + i) / maxval;
	//printf("img[%d] = %f\n",i,*((map->img) + i));
    }
  }

  return 0;
}

/* ****************************************************
 * Function   : DrawAutoStereogram
 * Arguments  : IMAGE *target = output image
 * 		IMAGE *Zimage = has (float *) depth map
 * Returns    : 0 for success
 * Description: Draw an AutoStereogram!
 * ***************************************************/
int DrawAutoStereogram(IMAGE *target, IMAGE *Zimage)
{
  /* Object's depth is Z(x,y) (between 0 and 1) */
  int x,y;		/* Coordinates of the current point */

  int maxX = Zimage->xmax;
  int maxY = Zimage->ymax;

  int *pix;		/* Color of this pixel */
  int *same;		/* Points to a pixel to the right ... */
    			/* ... that is constrained to be this color */
  int s;		/* Stereo separation at this (x,y) point */
  int left, right;	/* X-values corresponding to left and right eyes */

  int visible;		/* First, perform hidden-surface removal */
  int t=1;		/* We will check the points (x-t,y) and (x+t,y) */
  float zt;		/* Z-coord of ray at these two points */

  int l;		// temp var for left/right adjustments
  PIXEL Zval;		// temp var for *(Zimage->img) pixels
  
  pix  = malloc(maxX * sizeof(int));
  same = malloc(maxX * sizeof(int));

  for(y=0; y < maxY; y++)  /* Convert each scan line independently */
  {
    for(x=0; x < maxX; x++)
      same[x] = x;	/* Each pixel is initially linked with itself */

    for(x=0; x < maxX ; x++)
    {
      // Originally indexed Z[x][y]
      Zval = *((Zimage->img) + (x + (Zimage->xmax)*y));
      s = separation( Zval );
      left = x - s/2;			/* Pixels at left and right ... */
      right = left + s;			/* ... must be the same ... */
      if(0 <= left && right < maxX)	/* ... or must they? */
      {
	t=1;				/* grrr... haha... RESET t */
        do
        {
          zt = Zval + 2*(2 - mu*Zval)*t/(mu*E);
          visible = ( *((Zimage->img) + (x-t + (Zimage->xmax)*y)) < zt) && \
		    ( *((Zimage->img) + (x+t + (Zimage->xmax)*y)) < zt);  /* False if obscured */
          t++;
        } while(visible && zt < 1); /* Done - hidden-surface removal */
      
        if(visible)		/* So record the fact that pixels at */
        {
          l = same[left];	/* ... left and right are the same */
        
	  while(l != left && l != right)
            if(l < right)	/* But first, juggle the pointers ... */
	    {
              left = l; 	/* ... until either same[left]=left */
              l = same[left]; 	/* ... or same[left]=right */
            }
            else
	    {
              same[left] = right;
              left = right;
              l = same[left];
              right = l;
            }

          same[left] = right;  /* This is where we actually record it */
        }
      }
    }

    for(x=maxX-1 ; x>=0 ; x--) 	/* Now set the pixels on this scan line */
    {
      if(same[x] == x)
	      pix[x] = random()&1;	/* Free choice; do it randomly */
      else
	      pix[x] = pix[same[x]];	/* Constrained choice; obey constraint */
      *((target->img) + (x + (target->xmax)*y)) = pix[x];	// Record Pixval 
    }
  }

  //DrawCircle(maxX/2-far/2, maxY*19/20);  // Draw convergence dots at far plane,
  //DrawCircle(maxX/2+far/2, maxY*19/20);  // near the bottom of the screen.

  return 0;

}	// end drawautostereogram


/**********************************************************/
/************************** MAIN **************************/
/**********************************************************/

int main(int argc, char **argv)	// argv are the command-line arguments
{
 
	char infname[512], outfname[512];	// file names for input and output
	IMAGE *data;
	IMAGE *target;

	if(argc<5)		// too few command-line arguments?
	{
		printf("Command-line usage:\n");
		printf("   %s (inf) (outf) (x) (y)\n",argv[0]);
		printf("   (inf)  is the input file name\n");
		printf("   (outf) is the output file name\n");
		printf("   (x), (y) are the image dimensions\n");
		//printf("   (x), (y), (z) are the image dimensions\n");
		exit(0);
	}

	// Allocate local memory for struct pointers
	data = malloc(sizeof(IMAGE));

	// Handle Command Line args
	strcpy(infname,nextargs);	// read input file name
	strcpy(outfname,nextargs);	// read output file name
	data->xmax = nextargi;		// Read image dimensions
	data->ymax = nextargi; 
	data->zmax = 1;	//nextargi;

	// Read Image, Allocate Img mem
	printf("Reading image %s with dimensions %d, %d, %d\n",infname,data->xmax,data->ymax,data->zmax);
	if(read_raw(infname, data)){ printf("Error reading file\n");  exit (1);	}

	// Set target params, Allocate local memory
  	target = malloc(sizeof(IMAGE));
	if(copyimage(target, data)){ fprintf(stderr,"Could not Create Image: target\n");  exit(-1); }

	/* Image Processing calls here */
	printf("   Drawing AutoStereogram...\n");
	if(heightmap(data)){ printf("Error Making Height Map\n");  exit(3); }
	if(DrawAutoStereogram(target, data)){ printf("Error Generating SIRDS\n");  exit(3); }

	// Write Image
	printf("Writing processed image %s with dimensions %d, %d, %d\n",outfname,target->xmax,target->ymax,target->zmax);
	if(write_raw(outfname, target)){ printf("Error writing file\n");  exit (4); }

	// Free All Memory
	removeimage(data);
	removeimage(target);
	printf("Program completed successfully\n");
	exit (0);

}

