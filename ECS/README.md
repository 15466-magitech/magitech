# ECS Documentation
## **Entity**
A container of components corresponding to objects and elements of the game. 
Classes for game objects (e.g., Player) should include a function to create an entity
representing the data that the main game loop needs.

## **Components**
Define a new component type for any functionality that is shared between game objects 
of different classes. Keep track of component definitions and 
any implementation notes below.

### EventHandler
Add to any entities that handle user inputs (e.g., Player and Terminal). 
After attaching the EventHandler component, make sure to update its handle_event() function appropriately.
Edit the main loop to check if the event is handled across any of the entities with this component.

## **Systems**
