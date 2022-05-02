#pragma once

#include "CoreMinimal.h"

#include "CMCameraSubsystem.generated.h"

class AActor;
class APawn;
class APlayerController;
class APlayerCameraManager;
class UCMSpringArmComponent;

UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, DefaultToInstanced)
class UCMCameraSubsystem : public UObject
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime);

	virtual void OnEnterToCameraMode();
	
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

