#include "GraphProvider.h"

AGraphProvider::AGraphProvider() {
	PrimaryActorTick.bCanEverTick = false;
}

void AGraphProvider::MarkDirty() {
	while (!CommandQueue.IsEmpty()) {
		CommandQueue.Peek()->Get()->Execute(*this);
		CommandQueue.Pop();
	}
}
