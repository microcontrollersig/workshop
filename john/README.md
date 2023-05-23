
# John's projects

## State machine for curtains

```mermaid
stateDiagram
    direction LR
    [*] --> IDLE
    IDLE --> OPEN_UNTIL_LIMIT_HIT: Open button pressed,debounced
    IDLE --> CLOSE_UNTIL_LIMIT_HIT: Close button pressed,debounced
    OPENING --> IDLE: Open button pressed again
    CLOSING --> IDLE: Close button pressed again
    OPENING --> OPENED : Open limit switch hit
    OPENED --> CLOSING: Close button pressed
    state OPEN_UNTIL_LIMIT_HIT {
      direction LR
      START_OPENING --> OPENING: Opening commenced
    }
    state CLOSE_UNTIL_LIMIT_HIT {
      direction LR
      START_CLOSING --> CLOSING: Closing commenced
    }    
```
