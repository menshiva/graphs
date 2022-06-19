#pragma once

#include "NiagaraComponent.h"
#include "Laser.generated.h"

UCLASS()
class GRAPHS_API ULaser final : public UNiagaraComponent {
	GENERATED_BODY()
public:
	explicit ULaser(const FObjectInitializer &ObjectInitializer);
	void Init(const FLinearColor &Color, float Length);

	const FVector &GetEndPoint() const;

	void SetVisibility(bool IsVisible);
	FORCEINLINE void SetColor(const FLinearColor &NewColor);
	FORCEINLINE void SetLength(float NewLength);

	void Update(const FVector &StartPos, const FVector &LaserDir);
private:
	FVector m_EndPoint;
	float m_Length;
};
