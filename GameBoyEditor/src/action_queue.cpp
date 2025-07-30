#include "action_queue.hpp"

void ActionQueue::Push(Action* action, const bool_t perform)
{
    if (IsAtEnd())
    {
        ShiftActions();
        m_QueueIndex--;
    }

    RerouteQueue();

    m_Queue[m_QueueIndex] = action;

    if (perform)
        action->Do();

    m_QueueIndex++;
}

void ActionQueue::StepForward()
{
    if (!CanGoForward())
        return;

    m_Queue[m_QueueIndex]->Do();
    m_QueueIndex++;
}

void ActionQueue::StepBack()
{
    if (!CanGoBackward())
        return;

    m_QueueIndex--;
    m_Queue[m_QueueIndex]->Undo();
}

bool_t ActionQueue::IsAtBeginning() const { return m_QueueIndex == 0; }

bool_t ActionQueue::IsAtEnd() const { return m_QueueIndex == QueueSize; }

bool_t ActionQueue::CanGoForward() const { return !IsAtEnd() && m_Queue[m_QueueIndex] != nullptr; }

bool_t ActionQueue::CanGoBackward() const { return !IsAtBeginning(); }

void ActionQueue::ShiftActions()
{
    delete m_Queue[0];

    for (size_t i = 0; i < QueueSize - 1; i++)
        m_Queue[i] = m_Queue[i + 1];
}

void ActionQueue::RerouteQueue()
{
    if (m_Queue[m_QueueIndex] == nullptr)
    {
        // Nothing to reroute
        return;
    }

    for (size_t i = m_QueueIndex; i < QueueSize; i++)
    {
        delete m_Queue[m_QueueIndex];
        m_Queue[m_QueueIndex] = nullptr;
    }
}
