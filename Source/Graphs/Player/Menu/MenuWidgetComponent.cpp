#include "MenuWidgetComponent.h"
#include "MenuWidget.h"

UMenuWidgetComponent::UMenuWidgetComponent(const FObjectInitializer &ObjectInitializer) : UWidgetComponent(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;

	const ConstructorHelpers::FClassFinder<UUserWidget> MenuAsset(TEXT("/Game/Graphs/UI/Blueprints/Menu"));
	SetWidgetClass(MenuAsset.Class);
	SetDrawAtDesiredSize(true);
	SetPivot({0.0f, 0.5f});
	SetRelativeLocationAndRotation(
		FVector(0.0f, 5.0f, 0.0f),
		FRotator(75.0f, 15.0f, 185.0f)
	);
	SetRelativeScale3D(FVector(0.03f));
	SetGenerateOverlapEvents(false);
	CanCharacterStepUpOn = ECB_No;
	UPrimitiveComponent::SetCollisionProfileName("VRUI");

	const ConstructorHelpers::FObjectFinder<UStaticMesh> CursorShape(TEXT("/Engine/BasicShapes/Plane"));
	const ConstructorHelpers::FObjectFinder<UMaterial> CursorMaterial(TEXT("/Game/Graphs/Materials/CursorMaterial"));
	Cursor = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "Cursor");
	Cursor->SetStaticMesh(CursorShape.Object);
	Cursor->SetMaterial(0, CursorMaterial.Object);
	Cursor->SetGenerateOverlapEvents(false);
	Cursor->SetCollisionProfileName("NoCollision");
	Cursor->SetCastShadow(false);
	Cursor->SetRelativeScale3D(FVector(0.3f));
	Cursor->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	Cursor->SetVisibility(false);
	Cursor->SetupAttachment(this);
}

void UMenuWidgetComponent::SetVisble(const bool Visible) {
	const auto menuWidget = Cast<UMenuWidget>(GetWidget());
	if (Visible) {
		menuWidget->SetRenderOpacity(0.0f);
		SetVisibility(Visible);
		menuWidget->PlayShowHideAnimation(EUMGSequencePlayMode::Forward, [] {});
	}
	else {
		menuWidget->PlayShowHideAnimation(
			EUMGSequencePlayMode::Reverse,
			[&] {
				Cursor->SetVisibility(false);
				SetVisibility(false);
			}
		);
	}
}
