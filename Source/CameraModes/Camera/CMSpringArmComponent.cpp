#include "CMSpringArmComponent.h"

#include "CMCameraMode.h"
#include "DrawDebugHelpers.h"
#include "CameraSubsystems/CMCameraSubsystem_Transform.h"
#include "UObject/StrongObjectPtr.h"

UCMSpringArmComponent::UCMSpringArmComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	
	bAutoActivate = true;
	//bTickInEditor = true;
}

void UCMSpringArmComponent::SetCameraMode(UCMCameraMode* NewCameraMode)
{
	if(NewCameraMode != nullptr && CurrentCameraMode != NewCameraMode)
	{
		for(const auto subsystem : CameraSubsystems)
		{
			if(subsystem != nullptr)
			{
				subsystem->ConditionalBeginDestroy();
			}
		}
		CameraSubsystems.Empty();
		
		CurrentCameraMode = NewCameraMode;

		for(const auto subsystemClass : CurrentCameraMode->CameraSubsystems)
		{
			if(subsystemClass != nullptr)
			{
				const auto subsystem = NewObject<UCMCameraSubsystem>(this, subsystemClass);
				subsystem->SetOwningSpringArm(this);
				CameraSubsystems.Add(subsystem);
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

void UCMSpringArmComponent::OnRegister()
{
	Super::OnRegister();

	// Set initial location (without lag).
	//UpdateDesiredArmLocation(false, false, false, 0.f);
}

void UCMSpringArmComponent::BeginPlay()
{
	Super::BeginPlay();

	SetCameraMode(InitialCameraMode);
}

void UCMSpringArmComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for(const auto subsystem : CameraSubsystems)
	{
		if(subsystem != nullptr)
		{
			subsystem->Tick(DeltaTime);
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
