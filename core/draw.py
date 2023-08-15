import imgui
from imgui.integrations.pygame import PygameRenderer

def draw():
    imgui.new_frame()

    if imgui.begin("Overlay", True):
        imgui.text("This is an overlay with ImGui.")
        imgui.end()

    imgui.render()
    imgui.end_frame()

if __name__ == "__main__":
    imgui.create_context()
    impl = PygameRenderer()

    while True:
        draw()