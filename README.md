This was a school project and I'm hoping to revive it.


/////////////////////////////////////////////////

This is the program "chain" required to fully process a Single Image Random Dot Stereogram 
(SIRDS) to completion. 


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
