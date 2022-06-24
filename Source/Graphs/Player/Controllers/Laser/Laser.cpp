#include "Laser.h"
#include <NiagaraDataInterfaceArrayFunctionLibrary.h>

ULaser::ULaser(const FObjectInitializer &ObjectInitializer) : UNiagaraComponent(ObjectInitializer) {
	UNiagaraComponent::SetComponentTickEnabled(false);
	PrimaryComponentTick.bCanEverTick = false;
}

void ULaser::Init(const FLinearColor &Color, const float Length) {
	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/Graphs/VFX/LaserTrace"));
	SetAsset(LaserAsset.Object);
	SetColor(Color);
	SetLength(Length);
}

const FVector &ULaser::GetEndPoint() const {
	return m_EndPoint;
}

void ULaser::SetVisibility(const bool IsVisible) {
	IsVisible ? Activate() : Deactivate();
	UNiagaraComponent::SetVisibility(IsVisible);
}

void ULaser::SetColor(const FLinearColor &NewColor) {
	SetColorParameter("User.CustomColor", NewColor);
}

void ULaser::SetLength(const float NewLength) {
	m_Length = NewLength;
}

void ULaser::Update(const FVector &StartPos, const FVector &LaserDir) {
	if (IsActive()) {
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
			this,
			"User.PointArray", 0,
			StartPos, false
		);
		m_EndPoint = StartPos + LaserDir * m_Length;
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
			this,
			"User.PointArray", 1,
			m_EndPoint, false
		);
	}
}
