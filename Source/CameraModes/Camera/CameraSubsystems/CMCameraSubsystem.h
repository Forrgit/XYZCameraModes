#pragma once

#include "CoreMinimal.h"

#include "CMCameraSubsystem.generated.h"

class AActor;
class APawn;
class APlayerController;
class UCMSpringArmComponent;

UCLASS(Blueprintable, BlueprintType, Abstract)
class UCMCameraSubsystem : public UObject
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime);
	
	void SetOwningSpringArm(UCMSpringArmComponent* SpringArm);
	UCMSpringArmComponent* GetOwningSpringArm() const;

	AActor* GetOwningActor() const;
	APawn* GetOwningPawn() const;

	APlayerController* GetOwningController() const;
	
private:
	UPROPERTY()
	UCMSpringArmComponent* OwningSpringArmComponent;
};

