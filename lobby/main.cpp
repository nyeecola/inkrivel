#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "GL/glew.h"
#include <GLFW/glfw3.h>

#include <GL/gl.h>
#include <GL/glcorearb.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_demo.cpp"
#include "imgui/imgui_impl_glfw.cpp"
#include "imgui/imgui_impl_opengl3.cpp"

#include "../chat/src/chat_client.cpp"
#include "../lib/config.hpp"

#include <curl/curl.h>

#define MAX_MSG_LEN (100 + 20)

Mix_Music *music;

static void GlfwErrorCallback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

volatile char global_message_history[1000][MAX_MSG_LEN];
volatile int global_message_history_size = 0;

volatile char global_connected_users[500][20];
volatile int global_connected_users_size = 0;

void *listenServer(void *arg) {
    for (;;) {
        // socket file descriptor
        long socket_fd = (long) arg;

        Packet p;
        int received = receivePacket(socket_fd, &p);

        // the socket is blocking, so the function must always return 1
        assert(received);

        switch (p.id) {
            case MSG_RCV_MESSAGE:
                {
                    int len_sender = strlen((const char *) p.body) + 1;
                    int len_timestamp = sizeof(uint32_t);
                    int len_message = p.size - len_sender - len_timestamp;

                    char *sender = (char *) calloc(len_sender, sizeof(*sender));
                    //uint32_t timestamp = ((uint32_t *) p.body)[len_sender+1];
                    long long timestamp = p.body[len_sender];
                    timestamp |= p.body[len_sender+1] << 8;
                    timestamp |= p.body[len_sender+2] << 16;
                    timestamp |= p.body[len_sender+3] << 24;
                    char *message = (char *) calloc(len_message, sizeof(*message));

                    memcpy(sender, p.body, len_sender);
                    memcpy(message, p.body + len_sender + len_timestamp, len_message);

                    struct tm lt;
                    localtime_r((const long*) &timestamp, &lt);
                    char formated_time[10];
                    strftime(formated_time, sizeof(formated_time), "%T", &lt);

                    sprintf((char*) global_message_history[global_message_history_size++],
                            "%s [%s]: %s\n", formated_time, sender, message);

                    free(message);
                    free(sender);
                }
                break;
            case MSG_RCV_WHISPER:
                {
                    int len_sender = strlen((const char *) p.body) + 1;
                    int len_timestamp = sizeof(uint32_t);
                    int len_message = p.size - len_sender - len_timestamp;

                    char *sender = (char *) calloc(len_sender, sizeof(*sender));
                    //uint32_t timestamp = ((uint32_t *) p.body)[len_sender+1];
                    uint32_t timestamp = p.body[len_sender];
                    timestamp |= p.body[len_sender+1] << 8;
                    timestamp |= p.body[len_sender+2] << 16;
                    timestamp |= p.body[len_sender+3] << 24;
                    char *message = (char *) calloc(len_message, sizeof(*message));

                    memcpy(sender, p.body, len_sender);
                    memcpy(message, p.body + len_sender + len_timestamp, len_message);

                    struct tm lt;
                    localtime_r((const long*) &timestamp, &lt);
                    char formated_time[10];
                    strftime(formated_time, sizeof(formated_time), "%T", &lt);

                    sprintf((char*) global_message_history[global_message_history_size++],
                            "%s from <%s>: %s\n", formated_time, sender, message);

                    free(message);
                    free(sender);
                }
                break;
            case MSG_USERLIST:
                {
                    global_connected_users_size = 0;
                    int offset = 0;
                    while (1) {
                        int len = strlen((const char *) p.body + offset) +1;
                        memcpy((void*) global_connected_users[global_connected_users_size++], p.body + offset, len);
                        offset += len;

                        if (offset >= p.size) break;
                    }

                    for (int i = 0; i < global_connected_users_size; i++) {
                        printf("%s\n", global_connected_users[i]);
                    }
                }
                break;
        }

        free(p.body);
    }

    return NULL;
}

size_t state_response(void *ptr, size_t size, size_t nmemb, void *stream){
    bool *waiting_to_play = (bool *) stream;

    char state[50];
    int waiting_players;
    int player_id;
    sscanf((const char *) ptr, "{\"state\":\"%[^\"]\",\"waiting_players\":%d,\"player_id\":%d}",
           state, &waiting_players, &player_id);

    if (!strncmp(state, "playing", 5)) {
        *waiting_to_play = false;

        Mix_HaltMusic();
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        char player_id_str[3];
        sprintf(player_id_str, "%d", player_id);
        char * const args[] = {"game_client", player_id_str, NULL};
        assert(execvp("../game_client/bin/game_client", args) >= 0);
    }

    return size*nmemb;
}

size_t response(void *ptr, size_t size, size_t nmemb, void *stream){
    int *id = (int *) stream;

    sscanf((const char *) ptr, "{\"id\":%d}", id);

    return size*nmemb;
}

void setGuiStyle() {
    ImGuiStyle * style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2(6, 6);
    style->WindowRounding = 0.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.15f, 0.17f, 1.00f);
    style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.8f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.2f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.45f, 0.98f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.2f, 0.9f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.5f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.39f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.8f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.09f, 0.32f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.8f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 1.00f, 0.43f);
    style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

    ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontFromFileTTF("../assets/Ruda-Bold.ttf", 18);
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

        bool Err = glewInit() != 0;
        if (Err) {
            fprintf(stderr, "Failed to initialize OpenGL loader! %d\n", Err);
            return 1;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Setup style
        //ImGui::StyleColorsDark();
        clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        setGuiStyle();

        SDL_Init(SDL_INIT_AUDIO);
        Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    }

    CURL *curl;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    assert(curl);

    double LastTime = glfwGetTime();
    double DeltaTime = 0, NowTime = 0;
    double TimeSinceUpdate = 0;

    int socket_fd = -1;

    music = Mix_LoadMUS("../assets/lobby-bg-music.mp3");

    char username[20] = {0};
    char password[50] = {0};
    char buffer[MAX_MSG_LEN] = {0};

    char new_username[20] = {0};
    char new_password[50] = {0};
    char new_email[50] = {0};

    bool logged = false;
    int login_id = -1;

    bool waiting_to_play = false;

    bool new_account = false;
    int selectedName = -1;
    while (!glfwWindowShouldClose(window)) {
        if (global_message_history_size == 1000) {
            memcpy((void*) global_message_history, (const void*) (global_message_history + (500*MAX_MSG_LEN)), 500*MAX_MSG_LEN);
            global_message_history_size = 500;
        }
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

            if (!Mix_PlayingMusic()) {
                Mix_PlayMusic(music, -1);
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        if (!logged) {
            ImGui::SetNextWindowSize(ImVec2(0, 0));
            ImGui::Begin("Login");
            {
                ImGui::InputText("Username", username, sizeof(username));
                bool text_return = ImGui::InputText("Password", password, sizeof(password), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_Password);
                ImGui::Separator();
                bool button_return = ImGui::Button("Login", ImVec2(ImGui::GetWindowContentRegionWidth(), 0));
                if (text_return || button_return) {
                    if (strlen(username) && strlen(password)) {
                        char data[100];
                        sprintf(data, "{ \"user\": \"%s\", \"password\": \"%s\" }", username, password);

                        struct curl_slist *hs=NULL;
                        hs = curl_slist_append(hs, "Content-Type: application/json");
                        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
                        curl_easy_setopt(curl, CURLOPT_URL, LOGIN_SERVER_IP LOGIN_SERVER_PORT "/accounts/connect");
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &login_id);

                        CURLcode res = curl_easy_perform(curl);
                        if (res != CURLE_OK) printf("Failed to request login\n");

                        long http_code = 0;
                        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                        if (http_code == 200) {
                            logged = true;
                        }
                    }
                }

                if (ImGui::Button("Sign up", ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) new_account = !new_account;

                if (new_account) {
                    ImGui::SetNextWindowSize(ImVec2(0, 0));
                    ImGui::Begin("Sign Up");

                    ImGui::InputText("Username", new_username, sizeof(new_username));
                    ImGui::InputText("Password", new_password, sizeof(new_password), ImGuiInputTextFlags_Password);
                    ImGui::InputText("E-Mail", new_email, sizeof(new_email));
                    ImGui::Separator();
                    if (ImGui::Button("Create", ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) {
                        if (strlen(new_username) && strlen(new_password) && strlen(new_email)) {
                            char data[100];
                            sprintf(data, "{ \"account\": {"
                                    "\"user\":\"%s\","
                                    "\"password\": \"%s\","
                                    "\"email\": \"%s\","
                                    "\"nickname\": \"%s\" } }",
                                    new_username, new_password, new_email, new_username);

                            struct curl_slist *hs=NULL;
                            hs = curl_slist_append(hs, "Content-Type: application/json");
                            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
                            curl_easy_setopt(curl, CURLOPT_URL, LOGIN_SERVER_IP LOGIN_SERVER_PORT "/accounts.json");
                            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

                            CURLcode res = curl_easy_perform(curl);
                            if (res != CURLE_OK) printf("Failed to request signup\n");

                            long http_code = 0;
                            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                            if (http_code == 201) {
                                new_account = false;
                                memcpy(username, new_username, sizeof(username));
                            }
                        }
                    }
                    ImGui::End();
                }
            }
            ImGui::End();
        }
        else {
            {
                if (socket_fd == -1) {
                    socket_fd = createSocket();
                    chatConnect(socket_fd, username);

                    pthread_t listener;
                    pthread_create(&listener, NULL, listenServer, (void *) socket_fd);
                }

                ImGui::SetNextWindowSize(ImVec2(0, 0));
                ImGui::Begin("Chat");
                {
                    char chatBuffer[500*MAX_MSG_LEN] = {0};
                    ImGui::BeginChild("ID", ImVec2(500, 500), true, 0);
                    {
                        int end = 0;

                        for (int j = 0; j < global_message_history_size; j++) {
                            int len = strlen((const char*) global_message_history[j]);
                            memcpy(chatBuffer + end, (const void*) global_message_history[j], len);
                            end += len;
                        }
                    }
                    ImGui::InputTextMultiline("   Secret", chatBuffer, sizeof(chatBuffer), ImVec2(480, 480), ImGuiInputTextFlags_ReadOnly);
                    ImGui::BeginChild(ImGui::GetID("   Secret"));
                    ImGui::SetScrollHere(1);
                    ImGui::EndChild();
                    ImGui::EndChild();

                    ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
                    int r = ImGui::InputText("", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue);
                    if (r) {
                        if (selectedName == -1) {
                            chatSendMessage(socket_fd, buffer);
                        }
                        else {
                            chatSendWhisper(socket_fd, (const char*) global_connected_users[selectedName], buffer);

                            struct tm lt;
                            const long cur_time = time(NULL);
                            localtime_r(&cur_time, &lt);
                            char formated_time[10];
                            strftime(formated_time, sizeof(formated_time), "%T", &lt);
                            sprintf((char*) global_message_history[global_message_history_size++],
                                    "%s to <%s>: %s\n", formated_time, global_connected_users[selectedName], buffer);
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

                    static int charPick = 0;
                    ImGui::RadioButton("Rolo", &charPick, 0);
                    ImGui::RadioButton("Sniper", &charPick, 1);
                    ImGui::RadioButton("Assault", &charPick, 2);
                    ImGui::RadioButton("Bucket", &charPick, 3);
                    ImGui::Separator();

                    if (!waiting_to_play) {
                        if (ImGui::Button("Play", ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) {
                            char data[100];
                            sprintf(data, "{"
                                    "\"account_id\": \"%d\","
                                    "\"room_size\": \"%d\","
                                    "\"character_id\": \"%d\" "
                                    "}",
                                    login_id, radioPressed*2 + 4, charPick);

                            struct curl_slist *hs=NULL;
                            hs = curl_slist_append(hs, "Content-Type: application/json");
                            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
                            curl_easy_setopt(curl, CURLOPT_URL, LOGIN_SERVER_IP LOGIN_SERVER_PORT "/games/join");
                            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

                            CURLcode res = curl_easy_perform(curl);
                            if (res != CURLE_OK) printf("Failed to request join\n");

                            long http_code = 0;
                            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                            if (http_code == 200) {
                                waiting_to_play = true;
                            }
                        }
                    }
                }
                ImGui::End();
            }
            {
                ImGui::SetNextWindowSize(ImVec2(100, 0));
                ImGui::Begin("Online");
                {
#if 0
                    for (int j = 0; j < global_connected_users_size; j++) {
                        ImGui::Text("%s", global_connected_users[j]);
                    }
#else
                    //ImGui::ListBox("", &selectedName, global_connected_users, global_connected_users_size);
                    for (int j = 0; j < global_connected_users_size; j++) {
                        if (ImGui::Selectable((const char*) global_connected_users[j], selectedName == j)) {
                            if (selectedName == j) selectedName = -1;
                            else selectedName = j;
                        }
                    }
#endif
                }
                ImGui::End();
            }

            int monte_carlo = rand() % 100;
            if (waiting_to_play && monte_carlo <= 4) {
                curl = curl_easy_init();
                char url[100];
                sprintf(url, LOGIN_SERVER_IP LOGIN_SERVER_PORT "/games/state.json?account_id=%d", login_id);
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, state_response);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &waiting_to_play);

                CURLcode res = curl_easy_perform(curl);
                if (res != CURLE_OK) printf("Failed to request join\n");

                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code == 200) {
                }
            }
        }

        // Rendering (end of frame)
        {
            int display_w, display_h;
            glfwMakeContextCurrent(window);
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

            glClearColor(1, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glBegin(GL_QUADS);
            glColor3f(1, 0, 0);
            glVertex3f(-1, 1, 0);
            glColor3f(0, 1, 0);
            glVertex3f(-1, -1, 0);
            glColor3f(0, 0, 1);
            glVertex3f(1, -1, 0);
            glColor3f(0, 0, 0);
            glVertex3f(1, 1, 0);
            glEnd();

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

    if (curl) curl_easy_cleanup(curl);
    curl_global_cleanup();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
