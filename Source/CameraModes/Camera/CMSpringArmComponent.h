#pragma once

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

	UFUNCTION(BlueprintCallable)
	void SetCameraMode(UCMCameraMode* NewCameraMode);
	
	UFUNCTION(BlueprintPure)
	UCMCameraMode* GetCurrentCameraMode() const;

	UFUNCTION(BlueprintPure)
	const TArray<UCMCameraSubsystem*>& GetCameraSubsystems() const;

	template<typename TSubsystem>
	TSubsystem* GetCameraSubsystem() const
	{
		for(auto subsystem : CameraSubsystems)
		{
			if(subsystem != nullptr)
			{
				if(subsystem->GetClass()->IsChildOf(TSubsystem::StaticClass()))
				{
					return Cast<TSubsystem>(subsystem);
				}
			}
		}
		return nullptr;
	}
	
public:
	UPROPERTY(EditAnywhere, Category="Camera Modes")
	UCMCameraMode* InitialCameraMode;

private:
	UPROPERTY(Transient)
	UCMCameraMode* CurrentCameraMode;
	
	UPROPERTY(Transient)
	TArray<UCMCameraSubsystem*> CameraSubsystems;
};
