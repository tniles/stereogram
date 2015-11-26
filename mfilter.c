/* Tyler Niles */
/* March 2010 */

/* mfilter.c - based on program template from Chapter 14.6 */
/* Formatted for Filtering by Neighborhood */
/* Only accepts Binary or Raw files */

/* Use the command

	cc -Wall -g -o test test.c

	to compile the program */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define nextargi (--argc,atoi(*++argv))
#define nextargf (--argc,atof(*++argv))
#define nextargs (--argc,*++argv)

#define HOODSIZE 9		// kernel sizen (MUST be odd for median filters)

typedef float PIXEL;		// define pixel datatype

typedef struct{
	PIXEL  *img;		// pointer to image elements
	int   xmax;
	int   ymax;
	int   zmax;
	int  elems;	// sizeof(PIXEL)*xmax*ymax*zmax
} IMAGE;


/* Functions to read and write a raw file */

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

int write_raw(const char *fname, IMAGE *data)
{
	FILE *F;				// output file handle
	F = fopen(fname, "wb");			// open file for write
	data->elems = fwrite(data->img, sizeof(PIXEL), (data->xmax)*(data->ymax)*(data->zmax), F);
	if(data->elems!=(data->xmax)*(data->ymax)*(data->zmax))
		return -1;			// size mismatch error
	return 0;				// success
}


/* Filter-Specific Functions */

int compare(const void *f1, const void *f2)
{  return (( *(PIXEL*)f1 > *(PIXEL*)f2 ) ? 1 : -1); }

PIXEL mediansort(PIXEL *array, int NUMNUMS)
{
  int index = NUMNUMS/2;
  qsort(array,NUMNUMS,sizeof(PIXEL),compare);
  if(!(NUMNUMS%2)) return ((array[index]+array[index-1])/2);
  else	return (array[index]);
}

PIXEL getelement(IMAGE *data, int x, int y)
{
  int i = x;
  int j = y;
  int offset;
  // deal with boundary pixels
  if(x < 0)		i=0;
  if(x >= (data->xmax)) i=(data->xmax)-1;
  if(y < 0)		j=0;
  if(y >= (data->ymax)) j=(data->ymax)-1;
  // calc offset, return memory element
  offset = i + (data->xmax)*j;
  return ( *((data->img)+offset) );
}

int getneighborhood(IMAGE *data, PIXEL *array, int x, int y)
{
  // get all HOODSIZE (9) elements
  int i=0;
  // left col
  *(array+i) = getelement(data, x-1, y-1);  i++;
  *(array+i) = getelement(data, x-1, y  );  i++;
  *(array+i) = getelement(data, x-1, y+1);  i++;
  // middle col
  *(array+i) = getelement(data, x  , y-1);  i++;
  *(array+i) = getelement(data, x  , y  );  i++;
  *(array+i) = getelement(data, x  , y+1);  i++;
  // right col
  *(array+i) = getelement(data, x+1, y-1);  i++;
  *(array+i) = getelement(data, x+1, y  );  i++;
  *(array+i) = getelement(data, x+1, y+1);  i++;
  return 0;
}

int putneighborhood(IMAGE *target, PIXEL pixval, int x, int y)
{
  //write pixval to target pixel
  int offset = x + (target->xmax)*y;
  *((target->img)+offset) = pixval;
  return 0;
}

int filter(IMAGE *target, IMAGE *data)
{
  int x,y;
  PIXEL   pixval;
  PIXEL   *array;
  array = malloc(HOODSIZE * sizeof(PIXEL));
  if(!array) exit(2);	// memory allocation failed? Exit immediately

  // Move through entire x,y of Image
  for(y=0; y < (data->ymax); y++)
  {
    for(x=0; x < (data->xmax); x++)
    {
      if(getneighborhood(data, array, x, y))	// read hood for pixel x,y
        printf("Failed to read neighborhood for Pixel (%d,%d)\n",x,y);
      pixval = mediansort(array, HOODSIZE);	// get median value
      if(putneighborhood(target, pixval, x, y))	// write hood for pixel x,y
        printf("Failed to write Pixel (%d,%d)\n",x,y);
    }
  }

  free(array);
  printf("Filter Done\n");
  return 0;
}




/************************** MAIN **************************/

int main(int argc, char **argv)	// argv are the command-line arguments
{
   
	char infname[512], outfname[512];	// file names for input and output
	IMAGE *data;
	IMAGE *target;

	if(argc<5)			// too few command-line arguments?
	{
		printf("Command-line usage:\n");
		printf("   %s (inf) (outf) (x) (y)\n",argv[0]);
		printf("   (inf)  is the input file name\n");
		printf("   (outf) is the output file name\n");
		printf("   (x), (y) are the image dimensions\n");
		exit(0);
	}

	// Allocate memory for struct pointers
	data   = malloc(sizeof(IMAGE));
	target = malloc(sizeof(IMAGE));

	// Handle Command Line args
	strcpy(infname,nextargs);		// read input file name
	strcpy(outfname,nextargs);		// read output file name
	data->xmax = nextargi;			// Read image dimensions
	data->ymax = nextargi; 
	data->zmax = 1;

	// Read Image
	printf("Reading image %s with dimensions %d, %d, %d\n",\
			infname,data->xmax,data->ymax,data->zmax);
	if(read_raw(infname, data)){
		printf("Error reading file\n");  exit (1);
	}

	// Set target params, Allocate memory
	target->xmax = data->xmax;
	target->ymax = data->ymax;
	target->zmax = data->zmax;
	target->img  = malloc((target->xmax)*(target->ymax)*(target->zmax)*sizeof(PIXEL));
	if(!target->img) exit(2);	// memory allocation failed? Exit immediately

	/* Image Processing calls here */
	//memcpy(target->img, data->img, (target->xmax)*(target->ymax)*(target->zmax)*sizeof(PIXEL));
	if(filter(target, data)){	// filter action
		printf("Error Filtering\n");  exit(3);
	}

	// Write Image
	printf("Writing processed image %s with dimensions %d, %d, %d\n",\
			outfname,target->xmax,target->ymax,target->zmax);
	if(write_raw(outfname, target)){
		printf("Error writing file\n");  exit (4);
	}

	// Free All Memory
	free(data->img);
	free(target->img);
	free(data);
	free(target);
	printf ("Program completed successfully\n");
	exit (0);

}

