/* Tyler Niles */
/* April 2010 */

/* float2uchar.c
 * Converts a raw IEEE Floating point Image 
 * to a raw 8-bit unsigned Char Image.
 * Image Values between 0.0-1.0 expected.
 * Grayscale only. */

/* WARNING!!!
 * This Program only does float to uchar
 * (as the name indicates). However, the structure
 * is in place to convert it to a multi-format
 * IN and OUT image data type converter. */

/* Use the command
 *	gcc -Wall -g -o test test.c
 * to compile the program */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#define nextargi (--argc,atoi(*++argv))
#define nextargf (--argc,atof(*++argv))
#define nextargs (--argc,*++argv)

// keys for image data type
enum
{
	NOIMAGE,
	FLOATIMAGE,
	UCHARIMAGE
};

typedef float FPIXEL;		// define pixel datatype
typedef unsigned char  UCPIXEL;		//

typedef struct{
	FPIXEL   *imgf;		// float ptr to image elements
	UCPIXEL  *imguc;	// uchar ptr to image elements
	int   xmax;
	int   ymax;
	int   zmax;
	int  elems;	// sizeof(__PIXEL)*xmax*ymax*zmax
} IMAGE;


/* ***************************************************
 * Function   : copyimage
 * Arguments  : IMAGE *copy, IMAGE *to_copy, unsigned char image_to_copy_datatype
 * Returns    : 0 for success, 0xDEAD for failed
 * Description: Allocate mem and copy image attributes
 * **************************************************/
int copyimage(IMAGE *eve, IMAGE *adam, unsigned char imagetype)
{
  eve->xmax  = adam->xmax;
  eve->ymax  = adam->ymax;
  eve->zmax  = adam->zmax;
  eve->elems = adam->elems;

  // make switch(imagetype)
  if(imagetype == FLOATIMAGE)
  {
    eve->imgf = malloc((adam->xmax)*(adam->ymax)*(adam->zmax)*sizeof(FPIXEL));
    if(!(eve->imgf)) exit(0xDEAD);  // memory allocation failed? Exit immediately
  }
  else if(imagetype == UCHARIMAGE)
  {
    eve->imguc = malloc((adam->xmax)*(adam->ymax)*(adam->zmax)*sizeof(UCPIXEL));
    if(!(eve->imguc)) exit(0xDEAD);  // memory allocation failed? Exit immediately
  }

  return 0;
}

/* ***************************************************
 * Function   : removeimage
 * Arguments  : IMAGE *I
 * Returns    : 0 for success
 * Description: Frees memory dynamically allocated
 * 		for an IMAGE ptr.
 * **************************************************/
int removeimage(IMAGE *I, unsigned char imagetype)
{
  // make switch(imagetype)
  if(imagetype == FLOATIMAGE){       free(I->imgf);  }
  else if(imagetype == UCHARIMAGE){  free(I->imguc); }

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

	data->imgf = malloc((data->xmax)*(data->ymax)*(data->zmax)*sizeof(FPIXEL));	// allocate memory
	if(!data->imgf) return -1;		// exit if operation failed

	F = fopen(fname, "rb");			// open file for read

	data->elems = fread(data->imgf, sizeof(FPIXEL), (data->xmax)*(data->ymax)*(data->zmax), F);
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

	data->elems = fwrite(data->imguc, sizeof(UCPIXEL), (data->xmax)*(data->ymax)*(data->zmax), F);
	if(data->elems!=(data->xmax)*(data->ymax)*(data->zmax))
		return -1;			// size mismatch error

	return 0;				// success
}

/* ***************************************************
 * Function   : float2uchar
 * Arguments  : IMAGE *target, IMAGE *data
 * Returns    : 0 for success
 * Description: Converts IEEE Grayscale Float (img
 * 		vals 0-1) to 8-bit uchar Grayscale.
 * **************************************************/
int float2uchar(IMAGE *target, IMAGE *data)
{
  int i;
  //int maxval = 1;	 // maxval of image elements
  float scaler = 255/1;  // conversion factor 
  printf("   float2uchar conversion...\n");

  for(i=0; i<data->elems; i++)
  {
	  //target->imguc[i] = (unsigned char) (data->imgf[i] * scaler);
	  target->imguc[i] = (data->imgf[i] * scaler);
  }

  return 0;
}


/**********************************************************/
/************************** MAIN **************************/
/**********************************************************/

int main(int argc, char **argv)	// argv are the command-line arguments
{
 
	char infname[512], outfname[512];	// file names for input and output
	unsigned char imagetypein, imagetypeout;
	IMAGE *data;
	IMAGE *target;

	if(argc<5)		// too few command-line arguments?
	{
		printf("Command-line usage:\n");
		printf("   %s (inf) (outf) (x) (y)\n",argv[0]);
		printf("   (inf)  is the input file name\n");
		printf("   (outf) is the output file name\n");
		printf("   (x), (y), are the image dimensions\n");
		exit(0);
	}

	// Allocate local memory for struct pointers
	data = malloc(sizeof(IMAGE));

	// Handle Command Line args
	strcpy(infname,nextargs);	// read input file name
	strcpy(outfname,nextargs);	// read output file name
	data->xmax = nextargi;		// Read image dimensions
	data->ymax = nextargi; 
	data->zmax = 1;

	// params set image data types in and out
	imagetypein  = FLOATIMAGE;
	imagetypeout = UCHARIMAGE;

	// Read Image into Mem
	printf("Reading image %s with dimensions %d, %d, %d\n",infname,data->xmax,data->ymax,data->zmax);
	if(read_raw(infname, data)){  printf("Error reading file\n");  exit (1);  }

	// Set target params, Allocate local memory
  	target = malloc(sizeof(IMAGE));
	if(copyimage(target, data, imagetypeout)){  fprintf(stderr,"Could not Create Image: target\n");  exit(-1);  }

	/* Image Processing calls here */
	printf("   Converting Data Types...\n");
	//switch case with command line parameters to specify data type conversions
	if(float2uchar(target, data)){ printf("Error Converting\n");  exit(3); }

	// Write Image to File
	printf("Writing processed image %s with dimensions %d, %d, %d\n",outfname,target->xmax,target->ymax,target->zmax);
	if(write_raw(outfname, target)){  printf("Error writing file\n");  exit (4);  }

	// Free All Memory
	removeimage(data,imagetypein);
	removeimage(target,imagetypeout);
	printf("Program completed successfully\n");
	exit (0);

}
