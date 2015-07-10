#ifndef DataNodePickingEventObserver_h
#define DataNodePickingEventObserver_h

#include <MitkCoreExports.h>
#include "mitkPickingEventObserver.h"

namespace mitk
{
    /**
    * \class DataNodePickingEventObserver
    */
    
    class MITKCORE_EXPORT DataNodePickingEventObserver: public PickingEventObserver
    {
    public:
        DataNodePickingEventObserver();
        virtual ~DataNodePickingEventObserver();

        mitk::Message1<mitk::DataNode*> SingleNodePickEvent;
        mitk::Message1<mitk::DataNode*> AddNodePickEvent;
        mitk::Message1<mitk::DataNode*> ToggleNodePickEvent;
    protected:
        virtual void HandlePickOneEvent(InteractionEvent* interactionEvent);
        virtual void HandlePickAddEvent(InteractionEvent* interactionEvent);
        virtual void HandlePickToggleEvent(InteractionEvent* interactionEvent);

        mitk::DataNode* GetPickedDataNode(InteractionEvent* interactionEvent);
    };

} /* namespace mitk */

#endif /* DataNodePickingEventObserver_h */
