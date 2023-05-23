
# John's projects

## State machine for curtains

```mermaid
stateDiagram
    direction TB
    [*] --> IDLE
    IDLE --> OPEN_UNTIL_LIMIT_HIT: Open Button pressed,debounced
    IDLE --> CLOSE_UNTIL_LIMIT_HIT: Close Button pressed,debounced
    OPENING --> IDLE: Open Button pressed
    CLOSING --> IDLE: either Close Limit hit or Close button pressed
    OPENING --> OPENED : Open Limit switch hit
    OPENED --> CLOSING: Close Button pressed
    state OPEN_UNTIL_LIMIT_HIT {
      direction TB
      START_OPENING --> OPENING: Opening commenced
    }
    state CLOSE_UNTIL_LIMIT_HIT {
      direction TB
      START_CLOSING --> CLOSING: Closing commenced
    }    
```
