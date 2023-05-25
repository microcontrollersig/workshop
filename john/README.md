
# John's projects

## State machine for curtains

```mermaid
stateDiagram-v2
    direction TB
    [*] --> IDLE
    IDLE --> START_OPENING: Open Button pressed,debounced
    IDLE --> START_CLOSING: Close Button pressed and debounced
    CLOSING --> IDLE: either Close Limit hit or Close button pressed
    OPENING --> IDLE: either Open Limit hit or Open Button pressed
START_OPENING --> OPENING: Opening commenced
    
      START_CLOSING --> CLOSING: Closing commenced
  
```

```mermaid

stateDiagram-v2
    [*] --> IDLE
    
    so: START_OPENING
    sc: START_CLOSING
    
    state fork_state <<fork>>
      IDLE --> fork_state
      fork_state --> so
      fork_state --> sc
      so --> OPENING 
      sc --> CLOSING

      state join_state <<join>>
      OPENING --> join_state
      CLOSING --> join_state
      join_state --> IDLE 
```

