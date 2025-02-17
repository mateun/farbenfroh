# game-engine

Simple opengl based game engine. 
Its main features are 
- forward rendering
- simple map editor (todo)
- automatic asset loading
- sound playback
- lua scripting (todo)
- skeletal animation


### TODO

- [x] Unlit shader option
- [x] Integrate IMGUI
- [x] Draw debug grid
- [ ] Animation Blending (planned for 18.02.2025)
- [ ] Animation Montage (planned for 19.02.2025)
- [ ] Collision Detection epic (planned for 26.02.2025)





- [ ] Map editor

 


### Thougths

#### Scene concept
I am thinking if it is useful to have a container which has all the nodes which 
shall be rendered. 
This includes "controlling" nodes such as a camera and lights. And also the
"drawable" nodes which include meshes and text. 
We can keep the current imperative render style, but this is then used by the scene itself. 
The user interacts only with the higher abstract scene in this case. 





