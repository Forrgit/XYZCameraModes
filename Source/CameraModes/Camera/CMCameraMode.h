#pragma once

#include "Engine/DataAsset.h"

#include "CMCameraMode.generated.h"

class UCMCameraSubsystem;

UCLASS()
class UCMCameraMode : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UCMCameraSubsystem>> CameraSubsystems;
};