#pragma once

#include "CMCameraSubsystem.h"

#include "CMCameraSubsystem_Transform.generated.h"

UCLASS()
class UCMCameraSubsystem_Transform : public UCMCameraSubsystem 
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

	virtual void OnEnterToCameraMode() override;
	
	/**
	* Get the target rotation we inherit, used as the base target for the boom rotation.
	* This is derived from attachment to our parent and considering the UsePawnControlRotation and absolute rotation flags.
	*/
	UFUNCTION(BlueprintCallable, Category=SpringArm)
	FRotator GetTargetRotation() const;

	UFUNCTION(BlueprintPure)
	FVector GetCameraLocation() const;
	UFUNCTION(BlueprintPure)
	FRotator GetCameraRotation() const;

	UFUNCTION(BlueprintPure)
	FTransform GetCameraTransform() const;
	
	FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const;
	
#pragma region Parameters
public:
	/** Natural length of the spring arm when there are no collisions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float TargetArmLength = 300.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float TargetArmLengthSpeed = 1.f;
	
	/** offset at end of spring arm; use this instead of the relative offset of the attached component to ensure the line trace works as desired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float SocketOffsetSpeed = 1.f;

	/** Offset at start of spring, applied in world space. Use this if you want a world-space offset from the parent component instead of the usual relative-space offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	FVector TargetOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float TargetOffsetSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=View)
	float ViewPitchMin = -40.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=View)
	float ViewPitchMax = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=View)
	float ViewMinMaxSpeed = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=View)
	float DesiredViewPitch = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=View)
	float MinTimeToActivateDesiredViewPitch = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=View)
	float MinVelocityToActivateDesiredViewPitch = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=View)
	float MinPlayerInputToStopDesiredViewPitch = 1.f;
	
	/** How big should the query probe sphere be (in unreal units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraCollision, meta=(editcondition="bDoCollisionTest"))
	float ProbeSize = 12.f;

	/** Collision channel of the query probe (defaults to ECC_Camera) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraCollision, meta=(editcondition="bDoCollisionTest"))
	TEnumAsByte<ECollisionChannel> ProbeChannel = ECollisionChannel::ECC_Camera;

	/** If true, do a collision test using ProbeChannel and ProbeSize to prevent camera clipping into level.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraCollision)
	bool bDoCollisionTest = true;

	/**
	 * If this component is placed on a pawn, should it use the view/control rotation of the pawn where possible?
	 * When disabled, the component will revert to using the stored RelativeRotation of the component.
	 * Note that this component itself does not rotate, but instead maintains its relative rotation to its parent as normal,
	 * and just repositions and rotates its children as desired by the inherited rotation settings. Use GetTargetRotation()
	 * if you want the rotation target based on all the settings (UsePawnControlRotation, InheritPitch, etc).
	 *
	 * @see GetTargetRotation(), APawn::GetViewRotation()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraSettings)
	bool bUsePawnControlRotation = false;

	/** Should we inherit pitch from parent component. Does nothing if using Absolute Rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraSettings)
	bool bInheritPitch = true;

	/** Should we inherit yaw from parent component. Does nothing if using Absolute Rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraSettings)
	bool bInheritYaw = true;

	/** Should we inherit roll from parent component. Does nothing if using Absolute Rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraSettings)
	bool bInheritRoll = true;

	/**
	 * If true, camera lags behind target position to smooth its movement.
	 * @see CameraLagSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Lag)
	bool bEnableCameraLag = false;

	/**
	 * If true, camera lags behind target rotation to smooth its movement.
	 * @see CameraRotationLagSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Lag)
	bool bEnableCameraRotationLag = false;

	/**
	 * If bUseCameraLagSubstepping is true, sub-step camera damping so that it handles fluctuating frame rates well (though this comes at a cost).
	 * @see CameraLagMaxTimeStep
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, AdvancedDisplay)
	bool bUseCameraLagSubstepping = false;

	/**
	 * If true and camera location lag is enabled, draws markers at the camera target (in green) and the lagged position (in yellow).
	 * A line is drawn between the two locations, in green normally but in red if the distance to the lag target has been clamped (by CameraLagMaxDistance).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Lag)
	bool bDrawDebugLagMarkers = false;

	/** If bEnableCameraLag is true, controls how quickly camera reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Lag, meta=(editcondition="bEnableCameraLag", ClampMin="0.0", ClampMax="1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraLagSpeed = 10.f;

	/** If bEnableCameraRotationLag is true, controls how quickly camera reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Lag, meta=(editcondition = "bEnableCameraRotationLag", ClampMin="0.0", ClampMax="1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraRotationLagSpeed = 10.f;
	
	/** Max time step used when sub-stepping camera lag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, AdvancedDisplay, meta=(editcondition = "bUseCameraLagSubstepping", ClampMin="0.005", ClampMax="0.5", UIMin = "0.005", UIMax = "0.5"))
	float CameraLagMaxTimeStep = 1.f / 60.f;

	/** Max distance the camera target may lag behind the current location. If set to zero, no max distance is enforced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Lag, meta=(editcondition="bEnableCameraLag", ClampMin="0.0", UIMin = "0.0"))
	float CameraLagMaxDistance = 0.f;
	
#pragma endregion

protected:
	FVector CurrentSocketOffset = FVector::ZeroVector;
	FVector CurrentTargetOffset = FVector::ZeroVector;

	float CurrentTargetArmLenght = 100.f;
	
	/** Get the position where the camera should be without applying the Collision Test displacement */
	UFUNCTION(BlueprintCallable, Category=CameraCollision)
	FVector GetUnfixedCameraPosition() const;

	/** Is the Collision Test displacement being applied? */
	UFUNCTION(BlueprintCallable, Category = CameraCollision)
	bool IsCollisionFixApplied() const;

	/** Returns the desired rotation for the spring arm, before the rotation constraints such as bInheritPitch etc are enforced. */
	virtual FRotator GetDesiredRotation() const;

	/** Updates the desired arm location, calling BlendLocations to do the actual blending if a trace is done */
	virtual void UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime);

	/**
	 * This function allows subclasses to blend the trace hit location with the desired arm location;
	 * by default it returns bHitSomething ? TraceHitLocation : DesiredArmLocation
	 */
	virtual FVector BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime);

protected:
	float TimeBlockedDesiredView = 0.f; 
	
	/** Temporary variables when applying Collision Test displacement to notify if its being applied and by how much */
	bool bIsCameraFixed = false;
	FVector UnfixedCameraPosition = FVector::ZeroVector;

	/** Temporary variables when using camera lag, to record previous camera position */
	FVector PreviousDesiredLoc= FVector::ZeroVector;
	FVector PreviousArmOrigin= FVector::ZeroVector;
	/** Temporary variable for lagging camera rotation, for previous rotation */
	FRotator PreviousDesiredRot = FRotator::ZeroRotator;

	/** Cached component-space socket location */
	FVector RelativeSocketLocation = FVector::ZeroVector;
	/** Cached component-space socket rotation */
	FQuat RelativeSocketRotation = FQuat::Identity;
};

