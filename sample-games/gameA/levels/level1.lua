return {
    meta = {
        name = "Test Level",
        ambient_color = {0.1, 0.1, 0.1},
    },

    entities = {
        {
            id = "player",
            type = "SpawnPoint",
            position = {0, 0, 0},
            scripts = {
                    "player_movement.lua"
            }
        },
        {
            id = "crate1",
            prefab = "crate",
            position = {2, 0, 3}
        }
    },

    ui = {
        {
            id = "start_button",
            type = "Button",
            text = "Start Game",
            position = {50, 20},
            size = {120, 40},
            anchor = "top_left",
            on_click = function()
                print("Game started!")
            end
        },

        {
            id = "label_fps",
            type = "Label",
            text = "FPS: 60",
            position = {10, 10},
            font_size = 14
        }
    }
}
