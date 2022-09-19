#pragma once

#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolImporterPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolImporterPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	enum class PanelType : uint8_t {
		NONE,
		LOADING,
		SUCCESS,
		ERROR
	};

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

	void SetInputFiles(TArray<FString> &InputFilesPaths) const;
	FORCEINLINE void SetPanelType(const PanelType Type) { CurrentPanelType = Type; }
	FORCEINLINE void SetMessage(const FString &Msg) { Message = Msg; }
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UWidgetSwitcher *ImporterPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UListView *ImporterList;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *ImporterRefreshButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *ImporterText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *ImporterConfirmButton;
private:
	PanelType CurrentPanelType = PanelType::NONE;
	FString Message;
};
