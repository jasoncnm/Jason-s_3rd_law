## Project Architecture Redesign Plan

# Goals
1. Decouple the Engine and Gameplay systems
2. Design an event-driven architecture

# Motivation
Currently, the project is separated into platform code and game code, and there is no clear separation between the engine system and the gameplay system.
This approach was fine at the beginning, as the game architecture is relatively simple. Now that there are a lot more gameplay systems being introduced, 
It becomes very hard to add new features and debug when they are heavily coupled like this. So this is why I want to have a clear separation between the engine systems,
and I think event-driven architecture is a good way to achieve this result.
