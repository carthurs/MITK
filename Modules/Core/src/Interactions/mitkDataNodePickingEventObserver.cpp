#include "mitkDataNodePickingEventObserver.h"

#include "mitkMousePressEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseReleaseEvent.h"

#include "mitkVtkPropRenderer.h"

namespace mitk {

DataNodePickingEventObserver::DataNodePickingEventObserver()
{
}

DataNodePickingEventObserver::~DataNodePickingEventObserver()
{
}

void DataNodePickingEventObserver::HandlePickOneEvent(InteractionEvent* interactionEvent)
{
    SingleNodePickEvent.Send(GetPickedDataNode(interactionEvent));
}

void DataNodePickingEventObserver::HandlePickAddEvent(InteractionEvent* interactionEvent)
{
    AddNodePickEvent.Send(GetPickedDataNode(interactionEvent));
}

void DataNodePickingEventObserver::HandlePickToggleEvent(InteractionEvent* interactionEvent)
{
    ToggleNodePickEvent.Send(GetPickedDataNode(interactionEvent));
}

mitk::DataNode* DataNodePickingEventObserver::GetPickedDataNode(InteractionEvent* interactionEvent)
{
    auto releaseEvent = static_cast<const mitk::MouseReleaseEvent*>(interactionEvent);

    auto propRenderer = static_cast<mitk::VtkPropRenderer*>(interactionEvent->GetSender());

    mitk::Point3D p3d;
    return propRenderer->PickObject(releaseEvent->GetPointerPositionOnScreen(), p3d);
}

}


