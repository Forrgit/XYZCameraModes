#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CMCameraSubsystem.generated.h"

class AActor;
class APawn;
class APlayerController;
class APlayerCameraManager;
class UCMSpringArmComponent;

struct FCMCameraSubsystemContext
{
public:
	bool bWithInterpolation = true;
};

UCLASS(EditInlineNew, DefaultToInstanced, Abstract)
class UCMCameraModeSubsystem_BaseSettings : public UDataAsset
{
	GENERATED_BODY()
};

UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, DefaultToInstanced)
class UCMCameraSubsystem : public UObject
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime);

	virtual void OnEnterToCameraMode(const FCMCameraSubsystemContext& Context);

	virtual void SetSubsystemSettings(UCMCameraModeSubsystem_BaseSettings* NewSettings);
	virtual UCMCameraModeSubsystem_BaseSettings* GetSubsystemSettings() const;
	
	void SetOwningSpringArm(UCMSpringArmComponent* SpringArm);
	UCMSpringArmComponent* GetOwningSpringArm() const;

	AActor* GetOwningActor() const;
	APawn* GetOwningPawn() const;

	APlayerController* GetOwningController() const;

	APlayerCameraManager* GetCameraManager() const;
	
private:
	UPROPERTY()
	UCMSpringArmComponent* OwningSpringArmComponent;
};

