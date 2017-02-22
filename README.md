# netCDFGlobe-Unreal

1. Download and extract the .zip file to a new Unreal Project folder and open the project in Unreal engine to make sure you have no errors. You should see an cube running the Game of Life (example by JR4815: https://forums.unrealengine.com/showthread.php?84856-Drawing-on-textures-in-real-time).

2. Create a new folder called Thirdparty in your Unreal project. Create a folder inside that called netCDF.
  Copy the bin, include, lib, and share folders from the netCDF C library installation location into here.
  
4. Replace the build.cs, the .h, and the .cpp file with the ones included here. Change the filename in the .cpp file to your location for the .nc file. You should see a visualization of mean sea level pressure.
