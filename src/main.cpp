#include <exception>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "app/scene.h"
#include "app/window.h"
#include "graphics/shader_program.h"

using namespace app;
using namespace gfx;
using namespace std;

constexpr int gWidth  = 1280;
constexpr int gHeight = 720;

int meshVertices = 1000;
#define GRID        0
#define OCTREE      1
#define MIN_GRID    2
#define MAX_GRID    100
#define MIN_OCTREE  5
#define MAX_OCTREE  150

int currentMode = 0;
int girdResolution = MAX_GRID;
int maxNumberPerLeaf = MIN_OCTREE;

bool regenerate = false;
bool backToOriginal = false;
bool showValence = false; 
bool wireFrame = false;
bool lighting = true;
int camPlacement = 0;
float lightPlacement = 0.5;
std::string loadedObjName = "";

// Those light colors are better suited with a thicker font than the default one + FrameBorder
// From https://github.com/procedural/gpulib/blob/master/gpulib_imgui.h
void SetupGuiTheme() {
    // cherry colors, 3 intensities
#define HI(v)   ImVec4(0.35f, 0.35f, 0.35f, v)
#define MED(v)  ImVec4(0.20, 0.20, 0.20, v)
#define LOW(v)  ImVec4(0.25, 0.25, 0.25, v)
    // backgrounds (@todo: complete with BG_MED, BG_LOW)
#define BG(v)   ImVec4(0.15, 0.15f, 0.15f, v)
    // text
#define TEXT(v) ImVec4(0.860f, 0.930f, 0.890f, v)

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.14f, 0.14f, 0.67f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.20f, 0.20f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.17f, 0.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.33f, 0.35f, 0.36f, 0.53f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.76f, 0.28f, 0.44f, 0.67f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
    colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.86f, 0.23f, 0.43f, 0.67f);
    colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.19f, 0.19f, 0.57f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.74f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


    style.WindowPadding = ImVec2(6, 4);
    style.WindowRounding = 0.0f;
    style.FramePadding = ImVec2(5, 2);
    style.FrameRounding = 3.0f;
    style.ItemSpacing = ImVec2(7, 1);
    style.ItemInnerSpacing = ImVec2(1, 1);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 6.0f;
    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 16.0f;
    style.GrabMinSize = 20.0f;
    style.GrabRounding = 2.0f;

    style.WindowTitleAlign.x = 0.50f;

    style.Colors[ImGuiCol_Border] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);
    style.FrameBorderSize = 0.0f;
    style.WindowBorderSize = 1.0f;
}

void renderGui() {
    if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove)) {
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::SetWindowSize(ImVec2(400, (float)gHeight));
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::Text(("Number of vertices : " + std::to_string(meshVertices)).c_str());
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Text("Structure : ");
        ImGui::SameLine();
        if (ImGui::RadioButton("Grid", currentMode == GRID)) currentMode = GRID;
        ImGui::SameLine();
        if (ImGui::RadioButton("Octree", currentMode == OCTREE)) currentMode = OCTREE;

        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        if (currentMode == GRID) {
            ImGui::Text("Grid resolution");
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.96f);
            //ImGui::SliderInt("", &girdResolution, MIN_GRID, MAX_GRID);
        }

        if (currentMode == OCTREE) {
            ImGui::Text("Max vertices per leaf");
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.96f);
            //ImGui::SliderInt("", &maxNumberPerLeaf, MIN_OCTREE, MAX_OCTREE);
        }
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        {
            ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.2f);
            if (ImGui::Button("Simplify", ImVec2(ImGui::GetWindowSize().x * 0.5f, 0.0f))) regenerate = true;
        }
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Checkbox("Valence", &showValence);
        ImGui::Dummy(ImVec2(0.0f, 3.0f));
        ImGui::Checkbox("Wireframe", &wireFrame);
        ImGui::Dummy(ImVec2(0.0f, 3.0f));
        ImGui::Checkbox("Lighting", &lighting);

        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::SetNextTreeNodeOpen(true);
        if (ImGui::CollapsingHeader("Rotation"))
        {
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.77f);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::SliderInt("Model", &camPlacement, 0, 360);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::SliderFloat("Light", &lightPlacement, 0, 2);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Load")) {
                if (ImGui::MenuItem("Arma")) { loadedObjName = "arma1"; }
                if (ImGui::MenuItem("Camel")) { loadedObjName = "camel"; }
                if (ImGui::MenuItem("Elephant")) { loadedObjName = "elephant"; }
                if (ImGui::MenuItem("Sphere")) { loadedObjName = "sphere"; }
                if (ImGui::MenuItem("Suzanne")) { loadedObjName = "suzanne"; }
                if (ImGui::MenuItem("Teddy")) { loadedObjName = "teddy"; }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Original")) {
                backToOriginal = true;
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenuBar();
    }
    ImGui::End();

}


int main() {

    try {
        constexpr auto kWindowDimensions = std::make_pair(gWidth, gHeight);
        constexpr auto kOpenGlVersion = std::make_pair(4, 5);
        Window window("Mesh Simplification", kWindowDimensions, kOpenGlVersion);
        Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f));
        std::string vertShader = SHADER_FOLDER + std::string("vertex.glsl");
        std::string fragShader = SHADER_FOLDER + std::string("fragment.glsl");
        ShaderProgram shader_program{ vertShader, fragShader };

        Scene scene(window, camera, shader_program);


        /**
         * Initialize ImGui
         */
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_InitForOpenGL(window.getGLFWWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 450 core");

        SetupGuiTheme();
        ImGui::GetStyle().WindowMinSize = ImVec2((float)gWidth * 0.2f, (float)gHeight);
        ImGui::GetIO().FontGlobalScale = 0.5 + float(gWidth) / (gWidth);
        ImGui::GetIO().Fonts->AddFontFromFileTTF(ASSETS_FOLDER"fonts/Roboto-Medium.ttf", 16.0f);


        for (auto previous_time = glfwGetTime(), delta_time = 0.; !window.IsClosed();) {
            window.Update();

            scene.Render(static_cast<float>(delta_time));

            const auto current_time = glfwGetTime();
            delta_time = current_time - previous_time;
            previous_time = current_time;

            // feed inputs to dear imgui, start new frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            renderGui();

            // Render dear imgui into screen
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
