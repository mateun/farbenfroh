# Renderer doc

The renderer has 3 main layers of abstraction: 

### Layer 1 - visible to the engine user: 
Provides an abstract scene graph and is mostly declarative. 


### Layer 2 - internal engine translation layer, implementation agnostic
Comes before any concrete implementation layer (e.g. vulkan, gl, dx). 
Translates the graph into imperative beingFrame - beginCamera() - endCamera() - endFrame()
sequences. 

### Layer 3 - internal engine, implementation specific layer
Translates into e.g. vulkan specific implementation. 