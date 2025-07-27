#include <aetherium/state/callbacks.hpp>
#include <aetherium/state/event.hpp>
#include <aetherium/state/exceptions.hpp>
#include <aetherium/state/state.hpp>

#include <iostream>
#include <thread>
#include <chrono>

enum class DoorState {
    CLOSED,
    OPEN,
    LOCKED,
    CLOSING,
    OPENING
};

enum class DoorEvent {
    OPEN_REQUEST,
    CLOSE_REQUEST,
    LOCK_REQUEST,
    UNLOCK_REQUEST,
    DOOR_CLOSED,
    DOOR_OPENED
};

struct DoorContext {
    int unlock_attempts = 0;
    bool alarm_active = false;
};

int main() {
    using namespace Aetherium::State;
    using DoorStateMachine = StateMachine<
        DoorState,
        DoorEvent
    >;

    DoorStateMachine door_state_mach("Smart Door FSM");
    std::cout << "--- State Machine Example: Smart Door ---"
        << std::endl;

    auto closed = door_state_mach.add_state(
        DoorState::CLOSED,
        "Closed"
    );
    closed->set_entry_action(
        [](const State<DoorState>& state) {
            std::cout << "  [Action] Door is now "
                << state.get_name()
                << "."
                << std::endl;
        }
    );

    auto open = door_state_mach.add_state(
        DoorState::OPEN,
        "Open"
    );
    open->set_entry_action(
        [](const State<DoorState>& state){
            std::cout << "  [Action] Door is now "
                << state.get_name()
                << "."
                << std::endl;
        }
    );

    auto locked = door_state_mach.add_state(
        DoorState::LOCKED,
        "Locked"
    );
    locked->set_entry_action(
        [](const State<DoorState>& state){
            std::cout << "  [Action] Door is now "
                << state.get_name()
                << ". Alarm arming."
                << std::endl;
        }
    );

    auto closing = door_state_mach.add_state(
        DoorState::CLOSING,
        "Closing"
    );
    closing->set_entry_action(
        [](const State<DoorState>& state){
            std::cout << "  [Action] Door is "
                << state.get_name()
                << "..."
                << std::endl;
        }
    );

    auto opening = door_state_mach.add_state(
        DoorState::OPENING,
        "Opening"
    );
    opening->set_entry_action(
        [](const State<DoorState>& state) {
            std::cout << "  [Action] Door is "
                << state.get_name()
                << "..."
                << std::endl;
        }
    );

    auto open_request = door_state_mach.add_event(
        DoorEvent::OPEN_REQUEST,
        "Open Request"
    );
    auto close_request = door_state_mach.add_event(
        DoorEvent::CLOSE_REQUEST,
        "Close Request"
    );
    auto lock_request = door_state_mach.add_event(
        DoorEvent::LOCK_REQUEST,
        "Lock Request"
    );
    auto unlock_request = door_state_mach.add_event(
        DoorEvent::UNLOCK_REQUEST,
        "Unlock Request"
    );
    auto door_closed = door_state_mach.add_event(
        DoorEvent::DOOR_CLOSED,
        "Door Closed Event"
    );
    auto door_opened = door_state_mach.add_event(
        DoorEvent::DOOR_OPENED,
        "Door Opened Event"
    );

    auto closed_to_opening = door_state_mach.add_transition(
        DoorState::CLOSED,
        DoorState::OPENING,
        DoorEvent::OPEN_REQUEST
    );
    closed_to_opening->set_action(
        [](
            const auto& from,
            const auto& to,
            const auto& event
        ) {
            std::cout << "    [Transition Action] Initiating door opening mechanism."
                << std::endl;
        }
    );

    auto closed_to_locked = door_state_mach.add_transition(
        DoorState::CLOSED,
        DoorState::LOCKED,
        DoorEvent::LOCK_REQUEST
    );
    closed_to_locked->set_action(
        [](
            const auto& from,
            const auto& to,
            const auto& event
        ) {
            std::cout << "    [Transition Action] Engaging lock."
                << std::endl;
        }
    );

    door_state_mach.add_transition(
        DoorState::OPENING,
        DoorState::OPEN,
        DoorEvent::DOOR_OPENED
    );

    door_state_mach.add_transition(
        DoorState::OPEN,
        DoorState::CLOSING,
        DoorEvent::CLOSE_REQUEST
    );

    door_state_mach.add_transition(
        DoorState::CLOSING,
        DoorState::CLOSED,
        DoorEvent::DOOR_CLOSED
    );

    auto locked_to_closed = door_state_mach.add_transition(
        DoorState::LOCKED,
        DoorState::CLOSED,
        DoorEvent::UNLOCK_REQUEST
    );
    
    DoorContext door_ctx;
    locked_to_closed->set_guard_condition(
        [&door_ctx](
            const State<DoorState>& from,
            const State<DoorState>& to,
            const StateEvent<DoorEvent>& event
        ) {
            door_ctx.unlock_attempts++;
            if(door_ctx.unlock_attempts >= 3) {
                std::cout << "    [Guard] Too many unlock attempts! Alarm triggered."
                    << std::endl;

                door_ctx.alarm_active = true;
                return false;
            }

            std::cout << "    [Guard] Checking unlock credentials... (Attempt "
                << door_ctx.unlock_attempts
                << ")"
                << std::endl;

            return true;
        }
    );

    locked_to_closed->set_action(
        [](
            const auto& from,
            const auto& to,
            const auto& event
        ) {
            std::cout << "    [Transition Action] Disengaging lock."
                << std::endl;
        }
    );

    try {
        door_state_mach.set_initial_state(
            DoorState::CLOSED
        );
        door_state_mach.start();
    }
    catch(const StateConfigurationException& e) {
        std::cerr << "FSM Configuration Error: "
            << e.what()
            << std::endl;
        return 1;
    }
    catch(const std::logic_error& e) {
        std::cerr << "FSM Logic Error: "
            << e.what()
            << std::endl;
        return 1;
    }

    std::cout << "--- Processing Events ---"
        << std::endl;
    std::cout << "\nAttempting to open from CLOSED..."
        << std::endl;

    door_state_mach.process_event(DoorEvent::OPEN_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nSimulating door opened..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::DOOR_OPENED);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nAttempting to lock from OPEN (should not transition)..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::LOCK_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nAttempting to close from OPEN..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::CLOSE_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nSimulating door closed..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::DOOR_CLOSED);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nAttempting to lock from CLOSED..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::LOCK_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nAttempting to unlock from LOCKED (1st attempt)..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::UNLOCK_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nAttempting to lock from CLOSED again..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::LOCK_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nAttempting to unlock from LOCKED (2nd attempt)..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::UNLOCK_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nAttempting to lock from CLOSED again..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::LOCK_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nAttempting to unlock from LOCKED (3rd attempt - should fail and trigger alarm)..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::UNLOCK_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;
    std::cout << "Alarm active: "
        << (door_ctx.alarm_active ? "YES" : "NO")
        << std::endl;

    std::cout << "\nAttempting to process an unknown event..."
        << std::endl;
    try {
        door_state_mach.process_event(
            static_cast<DoorEvent>(999)
        );
    }
    catch(const UnknownEventException& e) {
        std::cerr << "Caught Expected Error: "
            << e.what()
            << std::endl;
    }

    door_state_mach.stop();
    door_state_mach.set_initial_state(DoorState::OPEN);
    door_state_mach.start();

    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\nTriggering global lock event from OPEN state..."
        << std::endl;
    door_state_mach.process_event(DoorEvent::LOCK_REQUEST);
    std::cout << "Current State: "
        << door_state_mach.get_current_state()
            ->get_name()
        << std::endl;

    std::cout << "\n--- Simulation Complete ---"
        << std::endl;

    door_state_mach.stop();
    try {
        door_state_mach.clear();
    }
    catch(const std::logic_error& e) {
        std::cerr << "Error clearing state machine: "
            << e.what()
            << std::endl;
    }

    return 0;
}
