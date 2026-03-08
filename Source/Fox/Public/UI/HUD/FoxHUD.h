// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FoxHUD.generated.h"

class USpellMenuWidgetController;
class UAttributeMenuWidgetController;
class UAttributeSet;
class UAbilitySystemComponent;
struct FWidgetControllerParams;
class UOverlayWidgetController;
class UFoxUserWidget;

/**
 * AFoxHUD - The Main HUD (Heads-Up Display) Class for the Fox Project
 * 
 * WHAT IS AHUD?
 * AHUD is Unreal Engine's base class for managing all on-screen UI elements and visual overlays that players see
 * during gameplay (health bars, minimaps, crosshairs, etc.). Think of it as the "master manager" for your game's
 * user interface. Each player has their own HUD instance that's automatically created and attached to their
 * PlayerController when they join the game.
 * 
 * WHAT DOES AFoxHUD DO?
 * This class extends AHUD to create a custom HUD system specifically for the Fox project. It manages:
 * - Creating and displaying the main overlay UI widget (the on-screen interface players interact with)
 * - Setting up the OverlayWidgetController which acts as a "middleman" between gameplay data (health, mana, etc.)
 *   and the visual UI widgets that display that data
 * - Connecting the UI system to the Gameplay Ability System (GAS) so attribute changes automatically update the UI
 */
UCLASS()
class FOX_API AFoxHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	
	UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams);
	
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	
	USpellMenuWidgetController* GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams);
	
protected:
	
	
	
private:
	
	/*
	 * OverlayWidget - The Runtime Instance of the Main UI Overlay
	 * This is the actual widget object that gets created and displayed on screen during gameplay. It shows the
	 * player's HUD elements like health bars, mana bars, and other UI components. This gets instantiated from
	 * OverlayWidgetClass when InitOverlay() is called.
	 */
	UPROPERTY()
	TObjectPtr<UFoxUserWidget> OverlayWidget;

	/*
	 * OverlayWidgetClass - The Blueprint Class Template for the Overlay Widget
	 * This is a reference to the Blueprint class (set in the editor) that defines what the overlay widget looks like
	 * and how it's structured. Think of it as the "recipe" or "template" that tells Unreal how to create the actual
	 * OverlayWidget instance. Designers set this in the editor to specify which UI design to use.
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UFoxUserWidget> OverlayWidgetClass;

	/*
	 * OverlayWidgetController - The Runtime Instance that Manages Overlay Widget Data
	 * This is the "middleman" object that sits between the gameplay systems (Gameplay Ability System) and the
	 * OverlayWidget. It listens for attribute changes (health, mana, etc.) and broadcasts those changes to the
	 * widget so the UI updates automatically. Gets created from OverlayWidgetControllerClass when needed.
	 */
	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	/*
	 * OverlayWidgetControllerClass - The Blueprint Class Template for the Overlay Widget Controller
	 * This is a reference to the C++ or Blueprint class (set in the editor) that defines the logic for managing
	 * overlay widget data. It's the "recipe" for creating the OverlayWidgetController instance. This separation
	 * allows designers to customize controller behavior without changing C++ code.
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	/*
	 * AttributeMenuWidgetController - The Runtime Instance that Manages Attribute Menu Data
	 * Similar to OverlayWidgetController, but specifically handles the attribute menu UI (character stats screen,
	 * skill trees, etc.). It manages communication between the Gameplay Ability System's attributes and the
	 * attribute menu widget, ensuring the UI displays current character stats correctly.
	 */
	UPROPERTY()
	TObjectPtr<UAttributeMenuWidgetController> AttributeMenuWidgetController;

	/*
	 * AttributeMenuWidgetControllerClass - The Blueprint Class Template for the Attribute Menu Controller
	 * This is a reference to the class (set in the editor) that serves as the template for creating
	 * AttributeMenuWidgetController instances. Allows designers to specify which controller implementation
	 * to use for managing the attribute menu without modifying C++ code.
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;
	
	/*
	 * SpellMenuWidgetController - The Runtime Instance that Manages Spell Menu Data
	 * Similar to OverlayWidgetController and AttributeMenuWidgetController, but specifically handles the spell menu UI.
	 * It manages communication between the Gameplay Ability System's
	 * abilities and the spell menu widget, ensuring the UI displays available spells and their states correctly.
	 */
	UPROPERTY()
	TObjectPtr<USpellMenuWidgetController> SpellMenuWidgetController;

	/*
	 * SpellMenuWidgetControllerClass - The Blueprint Class Template for the Spell Menu Controller
	 * This is a reference to the class (set in the editor) that serves as the template for creating
	 * SpellMenuWidgetController instances. Allows designers to specify which controller implementation
	 * to use for managing the spell menu without modifying C++ code.
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<USpellMenuWidgetController> SpellMenuWidgetControllerClass;
};
