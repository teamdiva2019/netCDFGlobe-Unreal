# netCDFGlobe-Unreal

1. In visual studio, go to properties and VC++ directories. Set include to the include directory in the location you installed your netCDF library.
2. Create a new folder called Thirdparty in your Unreal projecthe .zip directory. Create a folder inside that called netCDF.
  Copy the bin, include, lib, and share folders from the installation location here.
3. Add necessary code from the build.cs file (uploaded here). This specifies include directories and copies the necessary binaries (the .dll files)
to your project folder.
4. Download the .zip file to a new Unreal Project folder and open the project in Unreal engine to make sure you have no errors. You should see an cube running the Game of Life (example by JR4815: https://forums.unrealengine.com/showthread.php?84856-Drawing-on-textures-in-real-time). Once you have this working, replace the build.cs, the .h, and the .cpp file with the ones included here. Change the filename in the .cpp file to your location for the file. You should see a visualization of mean sea level pressure.
