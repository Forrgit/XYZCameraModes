#pragma once

#include "CMCameraSubsystem.h"

#include "CMCameraSubsystem_FOV.generated.h"

UCLASS()
class UCMCameraModeSubsystem_FOVSettings : public UCMCameraModeSubsystem_BaseSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOV = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOVSpeed = 40.f;
};

UCLASS()
class UCMCameraSubsystem_FOV : public UCMCameraSubsystem
{
	GENERATED_BODY()
public:
	UCMCameraSubsystem_FOV();
	
	virtual void Tick(float DeltaTime) override;

	virtual void OnEnterToCameraMode(const FCMCameraSubsystemContext& Context) override;

	virtual void SetSubsystemSettings(UCMCameraModeSubsystem_BaseSettings* NewSettings) override;
	virtual UCMCameraModeSubsystem_BaseSettings* GetSubsystemSettings() const override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	UCMCameraModeSubsystem_FOVSettings* Settings;
	
private:
	void SetFOV(float NewFOV);
};
