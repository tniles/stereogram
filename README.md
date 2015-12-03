This was a school project and I'm hoping to revive it.


/////////////////////////////////////////////////

This is the program "chain" required to fully process a Single Image Random Dot Stereogram 
(SIRDS) to completion. 


/////////////////////////////////////////////////
12/2/2015

With a *.jpg file, do:

Convert from jpg to a raw image format (??? - I don't know how I did this. I think it was with
a special image processing suite which I no longer have). The result should be a file with 
8bit uchar data.
    *Update: Looks like this was the imageJ suite (an image processing suite written in java)

Use uchar02Float to convert to a heightmap (another *.raw image file).

Use autostereogram to convert to a SIRDS (another *.raw image file).

Use float02uchar to data convert for libtiff final step (another *.raw image file).

Use raw2tiff (from libtiff) with switches -w,-l,-c to convert to a .tif image for display.
    -w      width (x) of image in pixels
    -l      length (y) of image in pixels
    -c      compression scheme (none, packbits (default), jpg)
    Note: requires libtiff-tools ('sudo apt-get install libtiff-tools')

####
afrik_8.raw
#./uchar02float afrik_8.raw afrikFloat.raw 1024 768
#./autostereogram afrikFloat.raw afrikSird.raw 1024 768
#./float02uchar afrikSird.raw afrikView.raw 1024 768
#raw2tiff -w 1024 -l 768 afrikView.raw output.tif
####

/////////////////////////////////////////////////
*Input:

Available input formats are grayscale and either 8bit uchar or IEEE float (values 0.0-2.0).


*Conversion to heightmap:

If 8bit uchar, use uchar2float.exe to prep heightmap.


*AutoStereogram:

Run the heightmap through autostereogram.exe to make a raw file sirds.


*Display conversion:

Use rawprep.exe (src: float12uchar.c) to adjust image values from float to 8bit uchar raw 
image format.

Use raw2tiff (from libtiff) with switches -w,-l,-c to convert to a .tif image Windows can 
display.


/////////////////////////////////////////////////

decoder.exe

Input: raw file output from autostereogram.exe

Display conversion:
float2uchar.exe
raw2tiff.exe
