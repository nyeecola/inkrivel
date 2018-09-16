#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define IMGUI_IMPL_OPENGL_LOADER_GL3W
#include "GL/gl3w.h"
#include <GLFW/glfw3.h>

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_demo.cpp"
#include "imgui/imgui_impl_glfw.cpp"
#include "imgui/imgui_impl_opengl3.cpp"

static void GlfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, char **argv) {

    // IMGUI / GLFW / OpenGL
    ImVec4 clear_color;
    GLFWwindow *window;
    {
        glfwSetErrorCallback(GlfwErrorCallback);
        if (!glfwInit()) return 1;

        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

        // Create window with graphics context
        window = glfwCreateWindow(1280, 720, "InkRivel", NULL, NULL);
        if (window == NULL) return 1;

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        bool Err = gl3wInit() != 0;
        if (Err) {
            fprintf(stderr, "Failed to initialize OpenGL loader! %d\n", Err);
            return 1;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // TODO
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Setup style
        ImGui::StyleColorsDark();
        clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    }

    double LastTime = glfwGetTime();
    double DeltaTime = 0, NowTime = 0;
    double TimeSinceUpdate = 0;

    char chatBuffer[200] = {0};
    char username[50] = {0};
    char password[50] = {0};
    char buffer[100] = {0};

    bool logged = false;
    while (!glfwWindowShouldClose(window)) {
        // start of frame
        {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();

            NowTime = glfwGetTime();
            DeltaTime += (NowTime - LastTime);
            LastTime = NowTime;

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        if (!logged) {
            ImGui::SetNextWindowSize(ImVec2(0, 0));
            ImGui::Begin("Login");
            {
                ImGui::InputText("Username", username, sizeof(username));
                if (ImGui::InputText("Password", password, sizeof(password), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_Password)) {
                    logged = true;
                }
            }
            ImGui::End();
        }
        else {
            {
                ImGui::SetNextWindowSize(ImVec2(0, 0));
                ImGui::Begin("Chat");
                {
                    ImGui::BeginChild("ID", ImVec2(500, 500), true, 0);
                    //ImGui::Text("Uhu chat\nVarios escritos\n");
                    ImGui::InputTextMultiline("   Secret", chatBuffer, sizeof(chatBuffer), ImVec2(480, 480), ImGuiInputTextFlags_ReadOnly);
                    ImGui::BeginChild(ImGui::GetID("   Secret"));
                    ImGui::SetScrollHere(1);
                    ImGui::EndChild();
                    ImGui::EndChild();

                    ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
                    int r = ImGui::InputText("", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue);
                    if (r) {
                        if (strlen(buffer)) {
                            int i = 0;
                            while (chatBuffer[i]) {
                                i++;
                            }
                            if (i) chatBuffer[i] = '\n';
                            strcat(chatBuffer, buffer);
                        }
                        memset(buffer, 0, sizeof(buffer));
                        ImGui::SetKeyboardFocusHere(-1);
                    }
                    ImGui::PopItemWidth();
                }
                ImGui::End();
            }
            {
                ImGui::SetNextWindowSize(ImVec2(0, 0));
                ImGui::Begin("Play");
                {
                    ImGui::Text("%s", username);

                    static int radioPressed = 0;
                    ImGui::Separator();
                    ImGui::RadioButton("4 Players", &radioPressed, 0);
                    ImGui::RadioButton("6 Players", &radioPressed, 1);
                    ImGui::RadioButton("8 Players", &radioPressed, 2);
                    ImGui::Separator();

                    if (ImGui::Button("Play", ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) {
                    }
                }
                ImGui::End();
            }
        }

        // Rendering (end of frame)
        {
            int display_w, display_h;
            glfwMakeContextCurrent(window);
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwMakeContextCurrent(window);
            glfwSwapBuffers(window);
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
