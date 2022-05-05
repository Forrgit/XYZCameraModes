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
		FCMCameraSubsystemContext subsystemContext;
		subsystemContext.bWithInterpolation = CurrentCameraMode != nullptr;
		
		CurrentCameraMode = NewCameraMode;
		
		for(const auto subsystemTemplate : CurrentCameraMode->CameraSubsystems)
		{
			if(subsystemTemplate != nullptr)
			{
				UCMCameraSubsystem* subsystem = nullptr;
				
				if(const auto existSubsystem = GetCameraSubsystem(subsystemTemplate->GetClass()))
				{
					subsystem = existSubsystem;
					subsystem->SetSubsystemSettings(subsystemTemplate->GetSubsystemSettings());
				}
				else
				{
					subsystem = DuplicateObject<UCMCameraSubsystem>(subsystemTemplate, this);
					subsystem->SetOwningSpringArm(this);
					
					CameraSubsystems.Add(subsystem);
				}

				subsystem->OnEnterToCameraMode(subsystemContext);
			}
		}
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

UCMCameraSubsystem* UCMSpringArmComponent::GetCameraSubsystem(TSubclassOf<UCMCameraSubsystem> SubsystemClass) const
{
	if(SubsystemClass != nullptr)
	{
		for(const auto subsystem : CameraSubsystems)
		{
			if(subsystem != nullptr)
			{
				if(subsystem->GetClass()->IsChildOf(SubsystemClass))
				{
					return subsystem;
				}
			}
		}	
	}
	
	return nullptr;
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
			if(subsystem != nullptr && subsystem->GetSubsystemSettings() != nullptr)
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

FVector UCMSpringArmComponent::GetCameraLocation() const
{
	return GetSocketTransform(NAME_None, ERelativeTransformSpace::RTS_World).GetLocation();
}

FRotator UCMSpringArmComponent::GetCameraRotation() const
{
	return GetSocketTransform(NAME_None, ERelativeTransformSpace::RTS_World).Rotator();
}

