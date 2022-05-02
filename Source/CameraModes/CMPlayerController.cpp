#include "CMPlayerController.h"

void ACMPlayerController::ProcessPlayerInput(const float DeltaTime, const bool bGamePaused)
{
	Super::ProcessPlayerInput(DeltaTime, bGamePaused);

	OnRotationInputTickDelegate.Broadcast(RotationInput);
}
