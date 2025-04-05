


#include "IYapHandleReactor.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UYapHandleReactor : public UInterface
{
    GENERATED_BODY()
};

class IYapHandleReactor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent)
    void K2_OnSpeakingEnds();
    
    UFUNCTION(BlueprintImplementableEvent)
    void K2_OnHandleInvalidated();

    YAP_API virtual void OnSpeakingEnds()
    {
        K2_OnSpeakingEnds();
    }
    
    YAP_API virtual void OnHandleInvalidated()
    {
        K2_OnHandleInvalidated();
    }
};