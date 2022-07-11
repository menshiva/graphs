#include "MenuWidgetComponent.h"

UMenuWidgetComponent::UMenuWidgetComponent(const FObjectInitializer &ObjectInitializer) : UWidgetComponent(ObjectInitializer) {
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

	const ConstructorHelpers::FObjectFinder<UStaticMesh> CursorShape(TEXT("/Engine/BasicShapes/Sphere"));
	const ConstructorHelpers::FObjectFinder<UMaterial> CursorMaterial(TEXT("/Game/Graphs/Materials/CursorMaterial"));
	m_Cursor = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "Cursor");
	m_Cursor->SetStaticMesh(CursorShape.Object);
	m_Cursor->SetMaterial(0, CursorMaterial.Object);
	m_Cursor->SetGenerateOverlapEvents(false);
	m_Cursor->SetCollisionProfileName("NoCollision");
	m_Cursor->SetCastShadow(false);
	m_Cursor->SetRelativeScale3D(FVector(0.3f));
	m_Cursor->SetVisibility(false);
	m_Cursor->SetupAttachment(this);
}

void UMenuWidgetComponent::SetVisble(const bool Visible) {
	if (!Visible) m_Cursor->SetVisibility(false);
	SetVisibility(Visible);
}

void UMenuWidgetComponent::SetCursorVisibility(const bool Visible) const {
	m_Cursor->SetVisibility(Visible);
}

void UMenuWidgetComponent::SetCursorLocation(const FVector &Location) const {
	m_Cursor->SetWorldLocation(Location);
}
