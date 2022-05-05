#include "CMCameraSubsystem_FOV.h"

UCMCameraSubsystem_FOV::UCMCameraSubsystem_FOV()
{
	Settings = CreateDefaultSubobject<UCMCameraModeSubsystem_FOVSettings>("Settings");
}

void UCMCameraSubsystem_FOV::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(const auto cameraManager = GetCameraManager())
	{
		const float newFOV = FMath::FInterpConstantTo(cameraManager->GetFOVAngle(), Settings->FOV, DeltaTime, Settings->FOVSpeed);
		SetFOV(newFOV);
	}
}

void UCMCameraSubsystem_FOV::OnEnterToCameraMode(const FCMCameraSubsystemContext& Context)
{
	Super::OnEnterToCameraMode(Context);

	if(!Context.bWithInterpolation)
	{
		SetFOV(Settings->FOV);
	}
}

void UCMCameraSubsystem_FOV::SetSubsystemSettings(UCMCameraModeSubsystem_BaseSettings* NewSettings)
{
	Settings = Cast<UCMCameraModeSubsystem_FOVSettings>(NewSettings);
}

UCMCameraModeSubsystem_BaseSettings* UCMCameraSubsystem_FOV::GetSubsystemSettings() const
{
	return Settings;
}

void UCMCameraSubsystem_FOV::SetFOV(float NewFOV)
{
	if(const auto cameraManager = GetCameraManager())
	{
		cameraManager->SetFOV(NewFOV);
	}
}
