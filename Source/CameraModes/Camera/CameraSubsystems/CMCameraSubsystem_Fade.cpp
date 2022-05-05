#include "CMCameraSubsystem_Fade.h"

#include "CameraModes/Camera/CMSpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UCMCameraSubsystem_Fade::UCMCameraSubsystem_Fade()
{
	Settings = CreateDefaultSubobject<UCMCameraModeSubsystem_FadeSettings>("Settings");
}

void UCMCameraSubsystem_Fade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const FVector traceStart = GetOwningSpringArm()->GetCameraLocation();
	const FVector traceEnd = GetOwningActor()->GetActorLocation();

	const EDrawDebugTrace::Type debugTraceType = EDrawDebugTrace::ForOneFrame;

	TArray<FHitResult> hitResults;
	UKismetSystemLibrary::BoxTraceMulti(GetWorld(), traceStart, traceEnd, Settings->TraceHalfSize, GetOwningSpringArm()->GetCameraRotation(), UCollisionProfile::Get()->ConvertToTraceType(Settings->TraceChannel), false, {}, debugTraceType, hitResults, false);
	
	FadeActors.RemoveAll([](const FFadeActorData& FadeActorData)
	{
		return !FadeActorData.Actor.IsValid();
	});
	
	for(auto& fadeActorData : FadeActors)
	{
		fadeActorData.bFadeIn = false;
	}
	
	for(const auto& hitResult : hitResults)
	{
		if(const auto hitActor = hitResult.GetActor())
		{
			auto fadeActorData = FadeActors.FindByPredicate([hitActor](const FFadeActorData& FadeActorData)
			{
				return hitActor == FadeActorData.Actor.Get();
			});

			if(fadeActorData == nullptr)
			{
				fadeActorData = &FadeActors.AddDefaulted_GetRef();
				fadeActorData->Actor = hitActor;
				fadeActorData->FadeProgress = 0.f;
			}
		
			fadeActorData->bFadeIn = true;
		}
	}

	for(auto& fadeActorData : FadeActors)
	{
		const auto fadeTarget = fadeActorData.bFadeIn ? 1.f : 0.f;
		
		fadeActorData.FadeProgress = FMath::FInterpConstantTo(fadeActorData.FadeProgress, fadeTarget, DeltaTime, Settings->FadeSpeed);

		TArray<UMeshComponent*> meshComponents;
		fadeActorData.Actor->GetComponents(meshComponents);

		for(const auto meshComponent : meshComponents)
		{
			const float materialParameterValue = FMath::Lerp(Settings->MaterialParameterMin, Settings->MaterialParameterMax, fadeActorData.FadeProgress);
			meshComponent->SetScalarParameterValueOnMaterials(Settings->MaterialParameterName, materialParameterValue);
		}
	}
}

void UCMCameraSubsystem_Fade::OnEnterToCameraMode(const FCMCameraSubsystemContext& Context)
{
	Super::OnEnterToCameraMode(Context);

	if(!Context.bWithInterpolation)
	{
		
	}
}

void UCMCameraSubsystem_Fade::SetSubsystemSettings(UCMCameraModeSubsystem_BaseSettings* NewSettings)
{
	Settings = Cast<UCMCameraModeSubsystem_FadeSettings>(NewSettings);
}

UCMCameraModeSubsystem_BaseSettings* UCMCameraSubsystem_Fade::GetSubsystemSettings() const
{
	return Settings;
}
