/* Tyler Niles */
/* April 2010 */

/* decoder.c */
/* Deciphers Single Image Random-Dot Stereograms.
 * Input img vals 0<=x<=1 expected. 
 * Output img is IEEE float, with 0<=x<=2 img vals. */
/* Only accepts Binary or Raw files */

// INPUT : Raw image file SIRDS
// OUTPUT: Raw image file representing the height map of a 3d Image


/* Use the command
 *	gcc -Wall -g -o test decoder.c
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
#define separation(Zpix)  round( (1-mu*(Zpix))*E / (2-mu*(Zpix)) )
#define far separation(0) 	/* ... and corresponding to far plane, Zpix=0 */
#define near separation(2.0) 	/* ... and corresponding to near plane, Zpix=2.0 */

unsigned int PTHRESH;		/* need at least this many pixels to qualify as pixel width pattern */
unsigned int STHRESHMIN;	/* near plane prediction */
unsigned int STHRESHMAX;	/* far plane prediction */

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
 * Arguments  : IMAGE *map = height map to be scaled
 * Returns    : 0 for success
 * Description: Scale values from DecipherStereo to 
 * 		0<=x<=2 for export as IEEE float.
 * ***************************************************/
int heightmap(IMAGE *map)
{
  int i;
  double scaler=2.0;

  /* Scale down image by maximum value */
  for(i=0; i<map->elems; i++)
  {
	*((map->img) + i) = *((map->img) + i) * scaler;
	//printf("img[%d] = %f\n",i,*((map->img) + i));
  }

  return 0;
}

/* ****************************************************
 * Function   : DecodeStereo
 * Arguments  : IMAGE *target = output BINARY image
 * 		IMAGE *sirds  = sirds image
 * Returns    : 0 for success
 * Description: Decipher an AutoStereogram!
 * ***************************************************/
int DecodeStereo(IMAGE *target, IMAGE *sirds)
{
  /* Object's depth is Z(x,y) (between 0 and 1) */
  int x,y;		/* Current point - MUST BE SIGNED! */

  int maxX = sirds->xmax;
  int maxY = sirds->ymax;

  int sep1, sep2;	/* Stereo separation tracked by STHRESH, calc'd thru sep2-sep1 */

  //PIXEL *pix;		/* temp array for pixel correlation */
  PIXEL zval;		/* Color of this pixel */
 
  unsigned int sthresh; 
 
  //pix  = malloc(maxX * sizeof(PIXEL));

  /* Find the Separation (AutoCorrelate) */
  //STHRESHMIN = near;		// near plane depth
  //STHRESHMAX = far;		// far plane depth

  /* While moving through all depth levels, 
   * Scan line-by-line to Find image elements and
   * Write target->img binary values (0=black, 2=white) */
for(sthresh = STHRESHMIN; sthresh < STHRESHMAX; sthresh++)
{
  zval = ((float)(2)) * ((float)((STHRESHMAX-STHRESHMIN)-(sthresh-STHRESHMIN)))/((float)(STHRESHMAX-STHRESHMIN));	// calc color value (representative of depth)
  for(y=0;y<maxY;y++){
    sep1=0;
    sep2=sep1;
    for(x=0;x<maxX;x++){
      if((x+sthresh) > maxX){	// Stay in-bounds (memory)
        // 
        break;			// line is done (close enough)
      }
      if( (*((sirds->img) + (x + maxX*y))) == ((*((sirds->img) + (sthresh + x + maxX*y)))) ){  /* PIXELS MATCH */
        sep2++;		//same, so record index
      		//check if next pixel also matches
      		//if not, delete index, start over...
      		//if so, continue until matching ceases (you've found the second image!)
      }
      else if((sep2-sep1) >= PTHRESH){	/* RUN LONG ENOUGH to be an image element? Write & move on */
        /* Grow backwards (x--) to verify element wasn't truncated */
	#if(1)
        for(x=sep1; (*((sirds->img)+(x+maxX*y)))==((*((sirds->img)+(sthresh+x+maxX*y)))); ){
	        if(--x < 0)	// Stay in-bounds (memory)
		  break;
       		else
		  sep1--;
	}
	#endif
	for(x=sep1;x<sep2;x++){
		*((target->img) + (x + maxX*y)) = zval;
	}
	sep1 = x;			// look for next element in the same y line
	sep2 = x;
      }
      else{				/* NO MATCH */
        sep1 = x;
        sep2 = x;
      }
    }
  } 
}

  // Return
  //free(pix);
  return 0;

}	/* end decodestereo */


/**********************************************************/
/************************** MAIN **************************/
/**********************************************************/

int main(int argc, char **argv)	// argv are the command-line arguments
{

	char infname[512], outfname[512];  // io file names
        int temp;	
	IMAGE *data;
	IMAGE *target;

	if(argc<5)		// too few command-line arguments?
	{
		printf("Command-line usage:\n");
		printf("   %s (inf) (outf) (x) (y) (pthresh)\n",argv[0]);
		printf("   (inf)  is the input file name\n");
		printf("   (outf) is the output file name\n");
		printf("   (x), (y) are the image dimensions\n");
		printf("   (pthresh) is an optional image element width threshold\n");
		exit(0);
	}

	// Allocate local memory for struct pointers
	data = malloc(sizeof(IMAGE));

	// Handle Command Line args
	temp = argc;
	strcpy(infname,nextargs);	// read input file name
	strcpy(outfname,nextargs);	// read output file name
	data->xmax = nextargi;		// Read image dimensions
	data->ymax = nextargi; 
	data->zmax =  1;
	if(temp>5){ PTHRESH = nextargi; }
	else	  { PTHRESH = (data->xmax)/100; }

	STHRESHMIN = near;
	STHRESHMAX = far;

	printf("Current Settings:\n   Minimum Image Element Width = %d\n   Minimum Near Plane = %d\n   Maximimum Far Plane = %d\n",PTHRESH,STHRESHMIN,STHRESHMAX);

	// Read Image, Allocate Img mem
	printf("Reading image %s with dimensions %d, %d, %d\n",infname,data->xmax,data->ymax,data->zmax);
	if(read_raw(infname, data)){ fprintf(stderr,"Error reading file\n");  exit (1);	}

	// Set target params, Allocate local memory
  	target = malloc(sizeof(IMAGE));
	if(copyimage(target, data)){ fprintf(stderr,"Could not Create Image: target\n");  exit(-1); }

	/* Image Processing calls here */
	printf("   Deciphering Stereogram...\n");
	if(DecodeStereo(target, data)){ fprintf(stderr,"Error Deciphering SIRDS\n");  exit(3); }
	if(heightmap(target)){ fprintf(stderr,"Error Generating Height Map\n");  exit(4); }

	// Write Image
	printf("Writing processed image %s with dimensions %d, %d, %d\n",outfname,target->xmax,target->ymax,target->zmax);
	if(write_raw(outfname, target)){ fprintf(stderr,"Error writing file\n");  exit(5); }

	// Free All Memory
	removeimage(data);
	removeimage(target);
	printf("Program completed successfully\n");
	exit(0);

}

