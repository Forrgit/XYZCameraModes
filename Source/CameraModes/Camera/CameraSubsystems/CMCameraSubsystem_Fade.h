#pragma once

#include "CMCameraSubsystem.h"

#include "CMCameraSubsystem_Fade.generated.h"

UCLASS()
class UCMCameraModeSubsystem_FadeSettings : public UCMCameraModeSubsystem_BaseSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MaterialParameterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaterialParameterMin = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaterialParameterMax = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TraceHalfSize = FVector(1.f, 120.f, 180.f);
};

UCLASS()
class UCMCameraSubsystem_Fade : public UCMCameraSubsystem
{
	GENERATED_BODY()

	struct FFadeActorData
	{
	public:
		TWeakObjectPtr<AActor> Actor;
		float FadeProgress = 0.f;
		bool bFadeIn = true;
	};
public:
	UCMCameraSubsystem_Fade();
	
	virtual void Tick(float DeltaTime) override;

	virtual void OnEnterToCameraMode(const FCMCameraSubsystemContext& Context) override;

	virtual void SetSubsystemSettings(UCMCameraModeSubsystem_BaseSettings* NewSettings) override;
	virtual UCMCameraModeSubsystem_BaseSettings* GetSubsystemSettings() const override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	UCMCameraModeSubsystem_FadeSettings* Settings;
	
private:
	TArray<FFadeActorData> FadeActors;
};
