// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "DebuffNiagaraComponent.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()
public:
	
	// Constructor for this class
	UDebuffNiagaraComponent();
	
	/** The gameplay tag associated with this debuff effect, used to monitor debuff application/removal on the owning actor */
	UPROPERTY(VisibleAnywhere)
	FGameplayTag DebuffTag;

protected:
	/** Called when the component is first initialized */
	virtual void BeginPlay() override;
	
	/** Callback function triggered when the debuff tag count changes on the owning actor's ASC */
	void DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	/** Callback function invoked when the owning actor dies, deactivates the Niagara effect */
	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor);
};
