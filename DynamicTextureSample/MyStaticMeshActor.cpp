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
/* We are reading 2D data, a 144 x 73 grid, over 1460 timesteps. */
#define DIMS 3
#define TIME 1460
#define LAT 73
#define LONG 73
#define UPDATEINTERVAL 0.1


long *mslp_in = (long *)malloc(TIME*LAT*LONG * sizeof(long));
int currz = 0;
long min = 16000, max = -40000;


int getIndex(int x, int y, int z) {
	return x + y * LAT + z * LAT * LONG;
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

	mDynamicColors = nullptr;
	mUpdateTextureRegion = nullptr;

	/* There will be netCDF IDs for the file and variable */
	int ncid, mslpvarid;
	/* Vectors used when reading in a varialbe */
	size_t start[DIMS], count[DIMS];
	/* Arrays to read data in to */
	long temp_in[LAT][LONG];
	/* Loop indexes, and error handling */
	int i, j, k, index, retval;
	/* Open the file. NC_NOWRITE tells netCDF we want read-only access to the file*/
	retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));
	/* Get the varid of the mean sea level pressure data variable, based on its name */
	retval = nc_inq_varid(ncid, "mslp", &mslpvarid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Get varialbe"));
	/* Read the data */
	/* Set vectors for reading data */
	count[0] = 1;
	count[1] = LAT;
	count[2] = LONG;
	start[1] = 0;
	start[2] = 0;
	/* Read in data one timestep at a time */
	//for (i = 0; i < TIME; i++) {  <-- takes way too long, only read in 100 timesteps for now
	for (i = 0; i < 100; i++) {
		start[0] = i;
		/* Copy current timestep to a temp array */
		retval = nc_get_vara_long(ncid, mslpvarid, start, count, &temp_in[0][0]);
		if (retval)
			UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Read variable"));
		/* Copy each individual value in this record to the main 3D array and print it to the logger */
		for (j = 0; j < LAT; j++) {
			for (k = 0; k < LONG; k++) {
				index = getIndex(k,j,i);
				mslp_in[index] = temp_in[j][k];
				if (temp_in[j][k] < min) {
					min = temp_in[j][k];
				}
				else if (temp_in[j][k] > max) {
					max = temp_in[j][k];
				}
				UE_LOG(LogTemp, Log, TEXT(" %d "), mslp_in[index]);
			}
		}
	}

	/* Close the file and free all resources. */
	retval = nc_close(ncid);
	// Temp max and min values because linear scaling isn't very pretty
	//min = -11500;
	//max = -6500;
	if (retval) {
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("SUCCESS"));
		UE_LOG(LogTemp, Log, TEXT("MIN: %d"), min);
		UE_LOG(LogTemp, Log, TEXT("MAX: %d"), max);
	}
}

void AMyStaticMeshActor::BeginPlay()
{
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
	w = 73;
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
			if (x > 144) {
				mDynamicColors[i * 4 + RED] = 0;
				mDynamicColors[i * 4 + GREEN] = 0;
				mDynamicColors[i * 4 + BLUE] = 0;
			}
			else {
				// If out of y range, make it black
				if (y > 73) {
					mDynamicColors[i * 4 + RED] = 0;
					mDynamicColors[i * 4 + GREEN] = 0;
					mDynamicColors[i * 4 + BLUE] = 0;
				}
				// If in range, determine color using linear scaling algorithm
				else {
					long curr = mslp_in[getIndex(x, y, currz)];
					curr = (curr < 0) ? curr * -1 : curr;
					// Color scaling algo from:
					//http://stackoverflow.com/questions/2374959/algorithm-to-convert-any-positive-integer-to-an-rgb-value by Martin Beckett
					// Get the midpoint of your data values
					float mid = ((max*-1) + (min*-1)) / 2;
					//Get the range of your data values
					long range = (max*-1) - (min*-1);
					// The red value is a fraction between -1 and 1 where 0 is the midpoint
					float redvalue = 2 * (curr - mid) / range;
					// If the red value is positive, the value is > midpoint
					if (redvalue > 0) {
						// red value is a fraction of the max value, 255 and green is the remaining fraction
						mDynamicColors[i * 4 + RED] = redvalue * 255;
						mDynamicColors[i * 4 + GREEN] = 255 - (redvalue * 255);
						mDynamicColors[i * 4 + BLUE] = 0;
					}
					// If the red value is negative, the value is < midpoint
					else {
						// inverse of red value becomes the new blue value, and the remainig fraction is green
						mDynamicColors[i * 4 + RED] = 0;
						mDynamicColors[i * 4 + GREEN] = 255 - (redvalue * -1 * 255);
						mDynamicColors[i * 4 + BLUE] = redvalue * -1 * 255;
					}
				}
			}
		}

		// Once each pixel has had its RGB channels updated, update the texture and increment the timestep
		UpdateTexture();
		currz++;
		// Loop timestep if animation is over
		if (currz > 99) {
			currz = 0;
		}
	}
}

void AMyStaticMeshActor::UpdateTexture()
{
	UpdateTextureRegions(mDynamicTexture, 0, 1, mUpdateTextureRegion, mDataSqrtSize, (uint32)4, mDynamicColors, false);
	mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);
}
