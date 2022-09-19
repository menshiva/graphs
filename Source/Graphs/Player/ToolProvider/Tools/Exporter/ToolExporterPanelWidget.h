#pragma once

#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolExporterPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolExporterPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	enum class PanelType : uint8_t {
		NONE,
		LOADING,
		SUCCESS,
		ERROR
	};

	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

	FORCEINLINE void SetPanelType(const PanelType Type) { CurrentPanelType = Type; }
	FORCEINLINE void SetMessage(const FString &Msg) { Message = Msg; }
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UWidgetSwitcher *ExporterPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *ExporterText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *ExporterConfirmButton;

	PanelType CurrentPanelType = PanelType::NONE;
	FString Message;
};
