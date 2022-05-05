#include "CMCameraSubsystem_Transform.h"

#include "GameFramework/Pawn.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CameraModes/Camera/CMSpringArmComponent.h"

UCMCameraSubsystem_Transform::UCMCameraSubsystem_Transform()
{
	Settings = CreateDefaultSubobject<UCMCameraModeSubsystem_TransformSettings>("Settings");
}

void UCMCameraSubsystem_Transform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CurrentSocketOffset = FMath::VInterpConstantTo(CurrentSocketOffset, Settings->SocketOffset, DeltaTime, Settings->SocketOffsetSpeed);
	CurrentTargetOffset = FMath::VInterpConstantTo(CurrentTargetOffset, Settings->TargetOffset, DeltaTime, Settings->TargetOffsetSpeed);
	CurrentTargetArmLenght = FMath::FInterpConstantTo(CurrentTargetArmLenght, Settings->TargetArmLength, DeltaTime, Settings->TargetArmLengthSpeed);

	if(const auto cameraManager = GetCameraManager())
	{
		cameraManager->ViewPitchMax = FMath::FInterpConstantTo(cameraManager->ViewPitchMax, Settings->ViewPitchMax, DeltaTime, Settings->ViewMinMaxSpeed);
		cameraManager->ViewPitchMin = FMath::FInterpConstantTo(cameraManager->ViewPitchMin, Settings->ViewPitchMin, DeltaTime, Settings->ViewMinMaxSpeed);
	}
	
	if(FMath::Abs(GetOwningSpringArm()->GetPlayerRotationInput().Pitch) < Settings->MinPlayerInputToStopDesiredViewPitch
		&& GetOwningActor()->GetVelocity().SizeSquared() >= Settings->MinVelocityToActivateDesiredViewPitch * Settings->MinVelocityToActivateDesiredViewPitch)
	{
		const auto playerController = GetOwningController();
		if(GetWorld()->GetTimeSeconds() > TimeBlockedDesiredView + Settings->MinTimeToActivateDesiredViewPitch)
		{
			const auto currentControlRotation = playerController->GetControlRotation();

			auto resultControlRotation = currentControlRotation;
			resultControlRotation.Pitch = Settings->DesiredViewPitch;
			resultControlRotation = FMath::RInterpConstantTo(currentControlRotation, resultControlRotation, DeltaTime, Settings->ViewMinMaxSpeed);

			playerController->SetControlRotation(resultControlRotation);
		}
	}
	else
	{
		TimeBlockedDesiredView = GetWorld()->GetTimeSeconds();
	}
	
	UpdateDesiredArmLocation(Settings->bDoCollisionTest, Settings->bEnableCameraLag, Settings->bEnableCameraRotationLag, DeltaTime);
	
}

void UCMCameraSubsystem_Transform::OnEnterToCameraMode(const FCMCameraSubsystemContext& Context)
{
	Super::OnEnterToCameraMode(Context);

	if(!Context.bWithInterpolation)
	{
		CurrentSocketOffset = Settings->SocketOffset;
		CurrentTargetOffset = Settings->TargetOffset;
		CurrentTargetArmLenght = Settings->TargetArmLength;
	}
}

void UCMCameraSubsystem_Transform::SetSubsystemSettings(UCMCameraModeSubsystem_BaseSettings* NewSettings)
{
	Settings = Cast<UCMCameraModeSubsystem_TransformSettings>(NewSettings);
}

UCMCameraModeSubsystem_BaseSettings* UCMCameraSubsystem_Transform::GetSubsystemSettings() const
{
	return Settings;
}

FRotator UCMCameraSubsystem_Transform::GetDesiredRotation() const
{
	return GetCameraRotation();
}

FRotator UCMCameraSubsystem_Transform::GetTargetRotation() const
{
	FRotator DesiredRot = GetDesiredRotation();

	if (Settings->bUsePawnControlRotation)
	{
		if (APawn* OwningPawn = GetOwningPawn())
		{
			const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
			if (DesiredRot != PawnViewRotation)
			{
				DesiredRot = PawnViewRotation;
			}
		}
	}

	// If inheriting rotation, check options for which components to inherit
	if (!GetOwningSpringArm()->IsUsingAbsoluteRotation())
	{
		const FRotator LocalRelativeRotation = GetSocketTransform(NAME_None, ERelativeTransformSpace::RTS_Component).Rotator();
		if (!Settings->bInheritPitch)
		{
			DesiredRot.Pitch = LocalRelativeRotation.Pitch;
		}

		if (!Settings->bInheritYaw)
		{
			DesiredRot.Yaw = LocalRelativeRotation.Yaw;
		}

		if (!Settings->bInheritRoll)
		{
			DesiredRot.Roll = LocalRelativeRotation.Roll;
		}
	}

	return DesiredRot;
}

FVector UCMCameraSubsystem_Transform::GetCameraLocation() const
{
	return GetCameraTransform().GetLocation();
}

FRotator UCMCameraSubsystem_Transform::GetCameraRotation() const
{
	return GetCameraTransform().Rotator();
}

FTransform UCMCameraSubsystem_Transform::GetCameraTransform() const
{
	return GetSocketTransform(NAME_None, ERelativeTransformSpace::RTS_World);
}

void UCMCameraSubsystem_Transform::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
	FRotator DesiredRot = GetTargetRotation();

	// Apply 'lag' to rotation if desired
	if(bDoRotationLag)
	{
		if (Settings->bUseCameraLagSubstepping && DeltaTime > Settings->CameraLagMaxTimeStep && Settings->CameraRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(Settings->CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(LerpTarget), LerpAmount, Settings->CameraRotationLagSpeed));
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(DesiredRot), DeltaTime, Settings->CameraRotationLagSpeed));
		}
	}
	PreviousDesiredRot = DesiredRot;

	// Get the spring arm 'origin', the target we want to look at
	FVector ArmOrigin = GetOwningSpringArm()->GetComponentLocation() + CurrentTargetOffset;
	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = ArmOrigin;
	if (bDoLocationLag)
	{
		if (Settings->bUseCameraLagSubstepping && DeltaTime > Settings->CameraLagMaxTimeStep && Settings->CameraLagSpeed > 0.f)
		{
			const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
			FVector LerpTarget = PreviousDesiredLoc;

			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(Settings->CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, Settings->CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, Settings->CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (Settings->CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(Settings->CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(Settings->CameraLagMaxDistance);
				bClampedDist = true;
			}
		}		

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (Settings->bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), ArmOrigin, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);

			const FVector ToOrigin = ArmOrigin - DesiredLoc;
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc, DesiredLoc + ToOrigin * 0.5f, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc + ToOrigin * 0.5f, ArmOrigin,  7.5f, bClampedDist ? FColor::Red : FColor::Green);
		}
#endif
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	// Now offset camera position back along our rotation
	DesiredLoc -= DesiredRot.Vector() * CurrentTargetArmLenght;
	// Add socket offset in local space
	DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(CurrentSocketOffset);

	// Do a sweep to ensure we are not penetrating the world
	FVector ResultLoc;
	if (bDoTrace && (Settings->TargetArmLength != 0.0f))
	{
		bIsCameraFixed = true;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwningActor());

		FHitResult Result;
		GetWorld()->SweepSingleByChannel(Result, ArmOrigin, DesiredLoc, FQuat::Identity, Settings->ProbeChannel, FCollisionShape::MakeSphere(Settings->ProbeSize), QueryParams);
		
		UnfixedCameraPosition = DesiredLoc;

		ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);

		if (ResultLoc == DesiredLoc) 
		{	
			bIsCameraFixed = false;
		}
	}
	else
	{
		ResultLoc = DesiredLoc;
		bIsCameraFixed = false;
		UnfixedCameraPosition = ResultLoc;
	}

	// Form a transform for new world transform for camera
	FTransform WorldCamTM(DesiredRot, ResultLoc);
	// Convert to relative to component
	//FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetOwningSpringArm()->GetComponentTransform() GetComponentTransform());
	FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetOwningSpringArm()->GetComponentTransform());

	// Update socket location/rotation
	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();
}

FVector UCMCameraSubsystem_Transform::BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime)
{
	return bHitSomething ? TraceHitLocation : DesiredArmLocation;
}

// void UCMCameraSubsystem_Transform::OnRegister()
// {
// 	Super::OnRegister();
//
// 	// enforce reasonable limits to avoid potential div-by-zero
// 	CameraLagMaxTimeStep = FMath::Max(CameraLagMaxTimeStep, 1.f / 200.f);
// 	CameraLagSpeed = FMath::Max(CameraLagSpeed, 0.f);
// }

FTransform UCMCameraSubsystem_Transform::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	FTransform RelativeTransform(RelativeSocketRotation, RelativeSocketLocation);

	switch(TransformSpace)
	{
		case RTS_World:
		{
			return RelativeTransform * GetOwningSpringArm()->GetComponentTransform();
			//return RelativeTransform * GetOwningSpringArm()->GetComponentTransform()GetComponentTransform();
			break;
		}
		case RTS_Actor:
		{
			if( const AActor* Actor = GetOwningActor() )
			{
				//FTransform SocketTransform = RelativeTransform * GetComponentTransform();
				FTransform SocketTransform = RelativeTransform * GetOwningSpringArm()->GetComponentTransform();
				return SocketTransform.GetRelativeTransform(Actor->GetTransform());
			}
			break;
		}
		case RTS_Component:
		{
			return RelativeTransform;
		}
	}
	return RelativeTransform;
}

FVector UCMCameraSubsystem_Transform::GetUnfixedCameraPosition() const
{
	return UnfixedCameraPosition;
}

bool UCMCameraSubsystem_Transform::IsCollisionFixApplied() const
{
	return bIsCameraFixed;
}
