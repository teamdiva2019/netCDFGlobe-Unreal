// Fill out your copyright notice in the Description page of Project Settings.

#include "NetCDFGlobe.h"
#include "Data.h"
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <iostream>
#include <string>


/* This is the name of the data file we will read. */
#define FILE_NAME "C:\\Users\\Erin\\Documents\\Unreal Projects\\NetCDFGlobe\\mslp.2002.nc"
/* We are reading 2D data, a 144 x 73 grid, over 1460 timesteps. */
#define DIMS 3
#define TIME 1460
#define LAT 73
#define LONG 144


int getIndex(int x, int y, int z) {
	return x + y * LAT + z * LAT * LONG;
}

// Sets default values
AData::AData()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/* There will be netCDF IDs for the file and variable. */
	int ncid, mslpvarid;
	/* Vectors used when reading in a varialbe */
	size_t start[DIMS], count[DIMS];
	/* Arrays to read data in to */
	short *mslp_in = (short *)malloc(TIME*LAT*LONG*sizeof(short));
	short temp_in[LAT][LONG];
	/* Loop indexes, and error handling. */
	int i, j, k, index, retval;
	/* Open the file. NC_NOWRITE tells netCDF we want read-only access to the file.*/
	retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));
	/* Get the varid of the mean sea level pressure data variable, based on its name */
	retval = nc_inq_varid(ncid, "mslp", &mslpvarid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Get varialbe"));
	/* Read the data. */
	/* Set vectors for reading data */
	count[0] = 1;
	count[1] = LAT;
	count[2] = LONG;
	start[1] = 0;
	start[2] = 0;
	/* Read in data one timestep at a time */
	for (i = 0; i < TIME; i++) {
		start[0] = i;
		/* Copy current timestep to a temp array */
		retval = nc_get_vara_short(ncid, mslpvarid, start, count, &temp_in[0][0]);
		if (retval)
			UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Read variable"));
		/* Copy each individual value in this record to the main 3D array and print it to the logger */
		for (j = 0; j < LAT; j++) {
			for (k = 0; k < LONG; k++) {
				index = getIndex(k,j,i);
				mslp_in[index] = temp_in[j][k];
				UE_LOG(LogTemp, Log, TEXT(" %d "), mslp_in[index]);
			}
		}
	}

	/* Close the file and free all resources. */
	retval = nc_close(ncid);
	if (retval) {
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("SUCCESS"));
	}

	free(mslp_in);

}

// Called when the game starts or when spawned
void AData::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AData::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

