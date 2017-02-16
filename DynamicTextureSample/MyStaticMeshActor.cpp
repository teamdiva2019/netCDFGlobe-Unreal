// Fill out your copyright notice in the Description page of Project Settings.

// This class has been modified from the example by unreal engine forum user JR4815 in his response to the following thread:
// https://forums.unrealengine.com/showthread.php?84856-Drawing-on-textures-in-real-time

#include "DynamicTextureSample.h"
#include "MyStaticMeshActor.h"
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <iostream>
#include <string>


#define RED 2
#define GREEN 1
#define BLUE 0
#define ALPHA 3

#define ALPHA_CHECK 200

/* This is the name of the data file we will read. */
#define FILE_NAME "C:\\Users\\Erin\\Documents\\Unreal Projects\\DynamicTextureSample\\mslp.2002.nc"
/* There are 3 dimensions (lat, lon, time) and we will update the texture every .1 seconds */
#define DIMS 3
#define UPDATEINTERVAL 0.1


short *mslp_in;
int currz = 0, min = 16000, max = -40000;
size_t lat, lon, time;


int getIndex(int x, int y, int z) {
	return x + y * lat + z * lat * lon;
}

void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture && Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
							);
					}
				}
				if (bFreeData)
				{
					FMemory::Free(RegionData->Regions);
					FMemory::Free(RegionData->SrcData);
				}
				delete RegionData;
			});
	}
}


AMyStaticMeshActor::AMyStaticMeshActor(const class FObjectInitializer& PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	// ID variables and error handling
	int ncid, latid, lonid, timeid, retval;
	/* Open the file. NC_NOWRITE tells netCDF we want read-only access to the file*/
	retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));
	// Get the dimension names
	retval = nc_inq_dimid(ncid, "lat", &latid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: latid"));
	retval = nc_inq_dimid(ncid, "lon", &lonid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: lonid"));
	retval = nc_inq_dimid(ncid, "time", &timeid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: timeid"));
	// Get the dimension lengths; these are used for initialization later on
	retval = nc_inq_dimlen(ncid, latid, &lat);
	UE_LOG(LogTemp, Error, TEXT("LAT: %d"), lat);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: latdim"));
	retval = nc_inq_dimlen(ncid, lonid, &lon);
	UE_LOG(LogTemp, Error, TEXT("LON: %d"), lon);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: londim"));
	retval = nc_inq_dimlen(ncid, timeid, &time);
	UE_LOG(LogTemp, Error, TEXT("TIME: %d"), time);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: timedim"));
	// Close file for now
	retval = nc_close(ncid);
	if (retval) {
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
	}
	// TODO: Texture breaks if lon > lat, so we have to manually set it to read in half of the data for now
	lon = 73;

	mDynamicColors = nullptr;
	mUpdateTextureRegion = nullptr;

	
}

void AMyStaticMeshActor::BeginPlay()
{
	/* There will be netCDF IDs for the file and variables */
	int ncid, mslpvarid;
	/* Vectors used when reading in a varialbe */
	size_t start[DIMS], count[DIMS];
	/* Loop index and error handling */
	int retval;
	/* Open the file. NC_NOWRITE tells netCDF we want read-only access to the file*/
	retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));
	// Allocate space for array to read in
	mslp_in = (short *)malloc(time*lat*lon * sizeof(short));
	// Get the varid of the mean sea level pressure data variable, based on its name
	retval = nc_inq_varid(ncid, "mslp", &mslpvarid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Get variable"));

	/* Read the data */
	/* Set vectors for reading data */
	start[0] = 0;
	start[1] = 0;
	start[2] = 0;
	count[0] = time;
	count[1] = lat;
	count[2] = lon;

	// Read in the data
	retval = nc_get_vara_short(ncid, mslpvarid, start, count, mslp_in);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Read variable"));

	/* Close the file and free all resources. */
	retval = nc_close(ncid);
	// Temp max and min values because linear scaling isn't very pretty
	min = -11500;
	max = -6500;
	if (retval) {
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("SUCCESS"));
		UE_LOG(LogTemp, Log, TEXT("MIN: %d"), min);
		UE_LOG(LogTemp, Log, TEXT("MAX: %d"), max);
	}


	Super::BeginPlay();
}

void AMyStaticMeshActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete[] mDynamicColors; mDynamicColors = nullptr;
	delete mUpdateTextureRegion; mUpdateTextureRegion = nullptr;

	free(mslp_in);

	Super::EndPlay(EndPlayReason);
}

void AMyStaticMeshActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetupTexture();
}

void AMyStaticMeshActor::SetupTexture()
{
	if (mDynamicColors) delete[] mDynamicColors;
	if (mUpdateTextureRegion) delete mUpdateTextureRegion;

	int32 w, h;
	w = lon;
	h = w;

	mDynamicMaterials.Empty();
	mDynamicMaterials.Add(GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0));
	mDynamicTexture = UTexture2D::CreateTransient(w, h);
	mDynamicTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	mDynamicTexture->SRGB = 0;
	mDynamicTexture->Filter = TextureFilter::TF_Nearest;
	mDynamicTexture->AddToRoot();
	mDynamicTexture->UpdateResource();

	mUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, w, h);

	mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);

	mDataSize = w * h * 4;
	mDataSqrtSize = w * 4;
	mArraySize = w * h;
	mArrayRowSize = w;

	mDynamicColors = new uint8[mDataSize];

	memset(mDynamicColors, 0, mDataSize);
}

void AMyStaticMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float t = GetWorld()->GetTimeSeconds();

	// Only update the texture every update interval
	if (t - lastTick > UPDATEINTERVAL)
	{
		lastTick = t;

		// Go through each pixel and set RGB values
		for (uint32 i = 0; i < mArraySize; ++i) {
			// derive x and y values from pixel number
			uint32 x = i % mArrayRowSize;
			uint32 y = i / mArrayRowSize;
			// If out of x range, make it black
			if (x >= lon) {
				mDynamicColors[i * 4 + RED] = 0;
				mDynamicColors[i * 4 + GREEN] = 0;
				mDynamicColors[i * 4 + BLUE] = 0;
			}
			// If out of y range, make it black
			else if(y >= lat){
					mDynamicColors[i * 4 + RED] = 0;
					mDynamicColors[i * 4 + GREEN] = 0;
					mDynamicColors[i * 4 + BLUE] = 0;
			}
			// If in range, determine color using linear scaling algorithm
			else {
				short curr = mslp_in[getIndex(x, y, currz)];
				curr = (curr < 0) ? curr * -1 : curr;
				// Color scaling algo from:
				//http://stackoverflow.com/questions/2374959/algorithm-to-convert-any-positive-integer-to-an-rgb-value by Martin Beckett
				// Get the midpoint of your data values
				float mid = ((max*-1) + (min*-1)) / 2;
				//Get the range of your data values
				short range = (max*-1) - (min*-1);
				// The red value is a fraction between -1 and 1 where 0 is the midpoint
				float redvalue = 2 * (curr - mid) / range;
				// If the red value is positive, the value is > midpoint
				if (redvalue > 0 && redvalue < 1) {
					// red value is a fraction of the max value, 255 and green is the remaining fraction
					mDynamicColors[i * 4 + RED] = redvalue * 255;
					mDynamicColors[i * 4 + GREEN] = 255 - (redvalue * 255);
					mDynamicColors[i * 4 + BLUE] = 0;
				}
				// If the red value is negative, the value is < midpoint
				else if (redvalue < 0 && redvalue > -1) {
					// inverse of red value becomes the new blue value, and the remainig fraction is green
					mDynamicColors[i * 4 + RED] = 0;
					mDynamicColors[i * 4 + GREEN] = 255 - (redvalue * -1 * 255);
					mDynamicColors[i * 4 + BLUE] = redvalue * -1 * 255;
				}
				// Positive but off the scale
				else if (redvalue > 0) {
					mDynamicColors[i * 4 + RED] = 255;
					mDynamicColors[i * 4 + GREEN] = 0;
					mDynamicColors[i * 4 + BLUE] = 0;
				}
				// Negative but off the scale
				else {
					mDynamicColors[i * 4 + RED] = 0;
					mDynamicColors[i * 4 + GREEN] = 0;
					mDynamicColors[i * 4 + BLUE] = 255;
				}
			}
			// For debugging
			/*if (y%2 == 0) {
				mDynamicColors[i * 4 + RED] = 0;
				mDynamicColors[i * 4 + GREEN] = 0;
				mDynamicColors[i * 4 + BLUE] = 0;
			}*/
		}

		// Once each pixel has had its RGB channels updated, update the texture and increment the timestep
		UpdateTexture();
		currz++;
		// Loop timestep if animation is over
		if (currz >= time) {
			currz = 0;
		}
		// For debugging
		//currz = 0;
	}
}

void AMyStaticMeshActor::UpdateTexture()
{
	UpdateTextureRegions(mDynamicTexture, 0, 1, mUpdateTextureRegion, mDataSqrtSize, (uint32)4, mDynamicColors, false);
	mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);
}
