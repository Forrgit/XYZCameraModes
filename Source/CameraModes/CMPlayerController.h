#pragma once

#include "GameFramework/PlayerController.h"

#include "CMPlayerController.generated.h"

UCLASS()
class ACMPlayerController : public APlayerController
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRotationInputTickDelegate, FRotator /*RotationInput*/);
	
	GENERATED_BODY()
public:
	virtual void ProcessPlayerInput(const float DeltaTime, const bool bGamePaused) override;
	
public:
	FOnRotationInputTickDelegate OnRotationInputTickDelegate;
};

