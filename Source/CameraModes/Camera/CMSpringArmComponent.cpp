#include "CMSpringArmComponent.h"

#include "CMCameraMode.h"
#include "DrawDebugHelpers.h"
#include "CameraModes/CMPlayerController.h"
#include "CameraSubsystems/CMCameraSubsystem_Transform.h"
#include "UObject/StrongObjectPtr.h"

UCMSpringArmComponent::UCMSpringArmComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	
	bAutoActivate = true;
	//bTickInEditor = true;
}

void UCMSpringArmComponent::SetCameraMode(FGameplayTag CameraModeTag)
{
	const auto foundCameraMode = Algo::FindByPredicate(CameraModes, [CameraModeTag](UCMCameraMode* CameraMode)
	{
		return CameraMode != nullptr && CameraMode->CameraModeTag == CameraModeTag;
	});

	if(foundCameraMode != nullptr)
	{
		SetCameraMode(*foundCameraMode);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Camera mode don't found! Tag: %s"), *CameraModeTag.ToString());
	}
	
	// for(const auto cameraMode : CameraModes)
	// {
	// 	if(cameraMode != nullptr && cameraMode->CameraModeTag == CameraModeTag)
	// 	{
	// 		SetCameraMode(cameraMode);
	// 		break;
	// 	}
	// }
}

void UCMSpringArmComponent::SetCameraMode(UCMCameraMode* NewCameraMode)
{
	if(NewCameraMode != nullptr && CurrentCameraMode != NewCameraMode)
	{
		CurrentCameraMode = NewCameraMode;

		TArray<UCMCameraSubsystem*> newCameraSubsystems;
		for(const auto subsystemTemplate : CurrentCameraMode->CameraSubsystems)
		{
			if(subsystemTemplate != nullptr)
			{
				const auto subsystem = DuplicateObject<UCMCameraSubsystem>(subsystemTemplate, this);
				//const auto subsystem = NewObject<UCMCameraSubsystem>(this, subsystemTemplate);
				subsystem->SetOwningSpringArm(this);
				subsystem->OnEnterToCameraMode();
				newCameraSubsystems.Add(subsystem);
			}
		}

		for(const auto subsystem : CameraSubsystems)
		{
			if(subsystem != nullptr)
			{
				subsystem->ConditionalBeginDestroy();
			}
		}
		CameraSubsystems = newCameraSubsystems;
	}
}

UCMCameraMode* UCMSpringArmComponent::GetCurrentCameraMode() const
{
	static TStrongObjectPtr<UCMCameraMode> defaultCameraMode; 
	if(!defaultCameraMode.IsValid())
	{
		defaultCameraMode.Reset(NewObject<UCMCameraMode>());
	}
	
	return CurrentCameraMode != nullptr ? CurrentCameraMode : defaultCameraMode.Get();
}

const TArray<UCMCameraSubsystem*>& UCMSpringArmComponent::GetCameraSubsystems() const
{
	return CameraSubsystems;
}

FRotator UCMSpringArmComponent::GetPlayerRotationInput() const
{
	return PlayerRotationInput;
}

APlayerController* UCMSpringArmComponent::GetOwningController() const
{
	const auto owningPawn = GetOwner<APawn>();
	return owningPawn != nullptr ? Cast<APlayerController>(owningPawn->Controller): nullptr;
}

void UCMSpringArmComponent::OnControllerRotationInput(FRotator InPlayerInput)
{
	PlayerRotationInput = InPlayerInput;
}

void UCMSpringArmComponent::OnRegister()
{
	Super::OnRegister();

	// Set initial location (without lag).
	//UpdateDesiredArmLocation(false, false, false, 0.f);
}

void UCMSpringArmComponent::BeginPlay()
{
	Super::BeginPlay();

	if(const auto playerController = Cast<ACMPlayerController>(GetOwningController()))
	{
		playerController->OnRotationInputTickDelegate.AddUObject(this, &UCMSpringArmComponent::OnControllerRotationInput);
	}
	
	SetCameraMode(InitialCameraModeTag);
}

void UCMSpringArmComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(const auto playerController = GetOwningController())
	{
		for(const auto subsystem : CameraSubsystems)
		{
			if(subsystem != nullptr)
			{
				subsystem->Tick(DeltaTime);
			}
		}
	}
	
	UpdateChildTransforms();
}

FTransform UCMSpringArmComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	const auto transformSubsystem = GetCameraSubsystem<UCMCameraSubsystem_Transform>();
	if(transformSubsystem == nullptr)
	{
		return GetOwner()->GetTransform();
	}
	return transformSubsystem->GetSocketTransform(InSocketName, TransformSpace);
}

bool UCMSpringArmComponent::HasAnySockets() const
{
	return false;
	//return true;
}

void UCMSpringArmComponent::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	//new (OutSockets) FComponentSocketDescription(SocketName, EComponentSocketType::Socket);
}

