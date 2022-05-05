#pragma once

#include "GameplayTagContainer.h"
#include "CameraSubsystems/CMCameraSubsystem.h"
#include "Components/SceneComponent.h"

#include "CMSpringArmComponent.generated.h"

class UCMCameraMode;
class UCMCameraSubsystem;

UCLASS(meta=(BlueprintSpawnableComponent), hideCategories=(Mobility))
class UCMSpringArmComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	UCMSpringArmComponent();
	
	// UActorComponent interface
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End of UActorComponent interface

	// USceneComponent interface
	virtual bool HasAnySockets() const override;
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;
	// End of USceneComponent interface

	FVector GetCameraLocation() const;
	FRotator GetCameraRotation() const;
	
	UFUNCTION(BlueprintCallable)
	void SetCameraMode(FGameplayTag CameraModeTag);
	
	UFUNCTION(BlueprintPure)
	UCMCameraMode* GetCurrentCameraMode() const;

	UFUNCTION(BlueprintPure)
	const TArray<UCMCameraSubsystem*>& GetCameraSubsystems() const;

	UCMCameraSubsystem* GetCameraSubsystem(TSubclassOf<UCMCameraSubsystem> SubsystemClass) const;
	
	template<typename TSubsystem>
	TSubsystem* GetCameraSubsystem() const
	{
		return Cast<TSubsystem>(GetCameraSubsystem(TSubsystem::StaticClass()));
	}

	FRotator GetPlayerRotationInput() const;
	
	APlayerController* GetOwningController() const;
	
public:
	UPROPERTY(EditAnywhere, Category="Camera Modes")
	TArray<UCMCameraMode*> CameraModes;
	
	UPROPERTY(EditAnywhere, Category="Camera Modes")
	FGameplayTag InitialCameraModeTag;

private:
	void SetCameraMode(UCMCameraMode* NewCameraMode);
	
	void OnControllerRotationInput(FRotator InPlayerInput);
	
private:
	UPROPERTY(Transient)
	UCMCameraMode* CurrentCameraMode;
	
	UPROPERTY(Transient)
	TArray<UCMCameraSubsystem*> CameraSubsystems;

	FRotator PlayerRotationInput;
};
