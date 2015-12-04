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
    RAW IMAGE requirements:
        Grayscale LUT
        8-bit unsigned (uchar, values 0-255)
        Note the grayscale makes the 3D effect viable, or at least needs to be represented as 
        a 3D object. Better "3D Modeling" in the native image of course results in a better
        stereo effect.

Use uchar02Float to convert to a heightmap (another *.raw image file).

Use autostereogram to convert to a SIRDS (another *.raw image file).

Use float02uchar to data convert for libtiff final step (another *.raw image file).

Use raw2tiff (from libtiff) with switches -w,-l,-c to convert to a .tif image for display.
    -w      width (x) of image in pixels
    -l      length (y) of image in pixels
    -c      compression scheme (none, packbits (default), jpg)
    Note: requires libtiff-tools ('sudo apt-get install libtiff-tools')

####
Suggested preprocessing for 3D model image with arbitrary colormap:
#convert -separate foo.jpg fooG.jpg | convert -combine fooG* -separate foo.png
-or-
#convert foo.jpg -set colorspace Gray -separate -average foo.png
-or with hist equalization-
#convert foo.jpg -set colorspace Gray -separate -average -equalize foo.png
####
afrik.jpg       # 1024x768
afrik_8.raw     # 1024x1024
#./uchar02float afrik_8.raw afrikFloat.raw 1024 1024
#./autostereogram afrikFloat.raw afrikSird.raw 1024 1024 
#./float02uchar afrikSird.raw afrikView.raw 1024 1024
#raw2tiff -w 1024 -l 1024 afrikView.raw output.tif
####


/////////////////////////////////////////////////
Working with imageJ:
    sudo apt-get install default-jdk maven
    export JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64/
        # NOTE: path is `locate tools.jar` but omit the lib/tools.jar
    Run 'mvn' from top level repo to build with maven
    Run the app with 'mvn -Pexec'


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
