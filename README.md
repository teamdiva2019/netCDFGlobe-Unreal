# netCDFGlobe-Unreal

1. In visual studio, go to properties and VC++ directories. Set include to the include directory in the location you installed your netCDF library.
2. Create a new folder called Thirdparty in your Unreal project directory. Create a folder inside that called netCDF.
  Copy the bin, include, lib, and share folders from the installation location here.
3. Add necessary code from the build.cs file (uploaded here). This specifies include directories and copies the necessary binaries (the .dll files)
to your project folder.
4. Look at my example .cpp file for an example of how I read a file with a variable of 3 dimensions
