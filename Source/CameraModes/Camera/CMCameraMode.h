#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "CMCameraMode.generated.h"

class UCMCameraSubsystem;

UCLASS()
class UCMCameraMode : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CameraModeTag;
	
	UPROPERTY(EditAnywhere, Instanced)
	TArray<UCMCameraSubsystem*> CameraSubsystems;
};