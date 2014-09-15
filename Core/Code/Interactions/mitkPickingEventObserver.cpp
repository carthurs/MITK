#include "mitkPickingEventObserver.h"

#include "mitkMousePressEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseReleaseEvent.h"

#include "mitkVtkPropRenderer.h"

namespace mitk {

PickingEventObserver::PickingEventObserver()
    : m_startedDragging(false)
    , m_isDragging(false)
{
}

PickingEventObserver::~PickingEventObserver()
{
}

void PickingEventObserver::Notify(InteractionEvent* interactionEvent, bool isHandled)
{
    auto pressEvent = dynamic_cast<const mitk::MousePressEvent*>(interactionEvent);
    if (pressEvent != nullptr && pressEvent->GetEventButton() & InteractionEvent::LeftMouseButton && !isHandled) {
        m_startedDragging = true;
        return;
    }

    auto moveEvent = dynamic_cast<const mitk::MouseMoveEvent*>(interactionEvent);
    if (moveEvent != nullptr && m_startedDragging) {
        m_isDragging = true;
        return;
    }

    auto releaseEvent = dynamic_cast<const mitk::MouseReleaseEvent*>(interactionEvent);
    if (releaseEvent != nullptr && m_startedDragging && releaseEvent->GetEventButton() & InteractionEvent::LeftMouseButton) {
        MITK_INFO << "In mouse release, isDragging? " << m_isDragging << " isHandled? " << isHandled;

        if (!m_isDragging && !isHandled) {
            HandlePickingEvent(interactionEvent);
        }

        m_startedDragging = false;
        m_isDragging = false;
    }
}

void PickingEventObserver::HandlePickingEvent(InteractionEvent* interactionEvent)
{
    auto releaseEvent = static_cast<const mitk::MouseReleaseEvent*>(interactionEvent);

    if (releaseEvent->GetModifiers() == 0) {
        // Simple click
        MITK_INFO << "PickingEventObserver::HandlePickingEvent SingleNodePickEvent.Send()";
        HandlePickOneEvent(interactionEvent);
    }
    else if (releaseEvent->GetModifiers() & InteractionEvent::ShiftKey) {
        MITK_INFO << "PickingEventObserver::HandlePickingEvent AddNodePickEvent.Send()";
        HandlePickAddEvent(interactionEvent);
    }
    else if (releaseEvent->GetModifiers() & InteractionEvent::ControlKey) {
        MITK_INFO << "PickingEventObserver::HandlePickingEvent ToggleNodePickEvent.Send()";
        HandlePickToggleEvent(interactionEvent);
    }
}

}


