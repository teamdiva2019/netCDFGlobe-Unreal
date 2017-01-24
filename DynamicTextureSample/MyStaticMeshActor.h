// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "GameOfLife/GameOfLife.h"

#include "MyStaticMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class DYNAMICTEXTURESAMPLE_API AMyStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

	AMyStaticMeshActor(const class FObjectInitializer& PCIP);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaTime) override;

	void SetupTexture();
	void UpdateTexture();

	TArray<class UMaterialInstanceDynamic*> mDynamicMaterials;
	UTexture2D *mDynamicTexture;
	FUpdateTextureRegion2D* mUpdateTextureRegion;

	uint8* mDynamicColors;
	
	uint32 mDataSize;
	uint32 mDataSqrtSize;
	uint32 mArraySize;
	uint32 mArrayRowSize;
	

	//for demostration purposes
	//GameOfLife gol;
	float lastTick;
};
