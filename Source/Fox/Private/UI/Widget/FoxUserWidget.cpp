// Copyright TryingToMakeGames


#include "UI/Widget/FoxUserWidget.h"

void UFoxUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
