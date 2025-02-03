#pragma once

#include"Network.h"


#include "ImGui/imconfig.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imstb_rectpack.h"
#include "ImGui/imstb_textedit.h"
#include "ImGui/imstb_truetype.h"

#include<vector>
#include"Audio.h"
#include <mutex>
// #include"Network.h"

namespace MyUI {
    // Our global network instance
    Network net;
    std::mutex messageMutex;

    // ChatMessage class to hold each message
    class ChatMessage {
    public:
        char sender[256];
        char message[1024];
        ChatMessage(const char* defaultSender = "Alice") {
            strncpy_s(sender, defaultSender, sizeof(sender) - 1);
            sender[sizeof(sender) - 1] = '\0';
            message[0] = '\0';
        }
    };

    // The current online user list received from the server.
    static std::vector<std::string> currentUser = { };
    std::string userSelected;  // user currently selected for private chat

    // Input buffers for public chat.
    static char inputs[1024];        // public message input
    char inputSender[128];             // sender name for public messages
    static std::vector<ChatMessage> chatMessages; // public chat history

    // Input buffers for private chat.
    static char privateInputs[1024]; // private message input
    static char privateInputSender[256]; // sender for private chat (unused ¨C sender is known)
    static std::vector<ChatMessage> privateChatMessages; // private chat history

    // Registration UI flags and global username storage.
    static bool popupwin = true;
    static bool nameValid = false;
    static char globalUsername[128] = "";
    // Function to parse messages: remove header
    std::pair<std::string, std::string> ParseBroadcastMessage(const std::string& msg) {
        // Expected format: "BROADCAST|<sender>|<content>"
        const std::string prefix = "BROADCAST|";
        if (msg.find(prefix) == 0) {
            std::string temp = msg.substr(prefix.size());
            size_t sep = temp.find('|');
            if (sep != std::string::npos) {
                std::string sender = temp.substr(0, sep);
                std::string content = temp.substr(sep + 1);
                return { sender, content };
            }
        }
        return { "System", msg };
    }

    // ChatBox for displaying public messages.
    class ChatBox {
    public:
        virtual void init(const std::vector<ChatMessage>& messages) {
            ImGui::BeginChild("ChatBox", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("System: Welcome to Public Chatroom..");
            for (const auto& message : messages) {
                ImGui::TextWrapped("%s: %s", message.sender, message.message);
            }
            // Auto-scroll to the bottom when new messages are added.
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
            // If there are too many messages, warn and remove the oldest.
            if (messages.size() > 1) {
                ImGui::TextColored(ImColor(255, 0, 0, 255), "System: The message will be deleted!!!");
            }
            ImGui::EndChild();
        }
    };

    // ChatBox for private messages.
    class privateChatBox : public ChatBox {
    public:
        void init(const std::vector<ChatMessage>& messages) override {
            std::string winTitle = "Private Chat with " + userSelected;
            ImGui::BeginChild(winTitle.c_str(), ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("Connecting...");
            ImGui::Text("Connected! Let's chat!");
            for (const auto& message : messages) {
                ImGui::TextWrapped("%s: %s", message.sender, message.message);
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
            if (messages.size() > 1) {
                ImGui::TextColored(ImColor(255, 0, 0, 255), "System: The message will be deleted after closing!!!");
            }
            ImGui::EndChild();
        }
    };

    // Utility: Trim leading/trailing whitespace.
    std::string Trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    // Send a public chat message.
    void SendMessages(char* inputs, char* inputsSender, std::vector<ChatMessage>* messages) {
        if (inputs[0] != '\0') {
            // Format the message using the new protocol:
            // "BROADCAST|<sender>|<message>\n"
            std::string msg = "BROADCAST|" + std::string(inputsSender) + "|" + std::string(inputs) + "\n";
            net.Send(msg);
            // Clear the input buffer.
            memset(inputs, 0, sizeof(inputs));
        }
    }

    // Send a system message locally.
    void SystemMessage() {
        static char SysInputs[512] = "";
        static char SysSender[10] = "System";
        ChatMessage newMessage;
        strncpy_s(newMessage.message, SysInputs, sizeof(newMessage.message) - 1);
        strncpy_s(newMessage.sender, SysSender, sizeof(newMessage.sender) - 1);
        chatMessages.push_back(newMessage);
    }

    // Clear an input buffer.
    void DeleteMessage(char* inputs) {
        if (inputs[0] != '\0') {
            memset(inputs, 0, sizeof(inputs));
        }
    }

    // Placeholder functions for sound effects.
    void playsoundS(Audio& sfxSys) {
        sfxSys.playMusicSend();
    }

    void playsoundPR(Audio& sfxSys) {
        sfxSys.playMusicPRec();
    }

    void playsoundR(Audio& sfxSys) {
        sfxSys.playMusicRec();
    }

    // Render the UI and process received messages.
    bool RenderUI(Audio& sfxSys, bool& isplaying) {
        // Retrieve any new messages from the network.
        auto received = net.GetMessages();
        std::lock_guard<std::mutex> lock(messageMutex);

        for (auto& msg : received) {
            // Check for updated user list.
            if (msg.find("USERLIST|") == 0) {
                std::string list = msg.substr(9); // remove "USERLIST|"
                list = Trim(list);
                currentUser.clear();
                size_t pos = 0;
                while ((pos = list.find(',')) != std::string::npos) {
                    std::string user = list.substr(0, pos);
                    currentUser.push_back(Trim(user));
                    list.erase(0, pos + 1);
                }
                if (!list.empty()) {
                    currentUser.push_back(Trim(list));
                }
                continue; // do not add to chatMessages
            }
            // Process user join notifications.
            else if (msg.find("USERJOIN|") == 0) {
                // e.g. "USERJOIN|Tom"
                std::istringstream iss(msg);
                std::string cmd, newUser;
                getline(iss, cmd, '|');  // "USERJOIN"
                getline(iss, newUser);
                if (std::find(currentUser.begin(), currentUser.end(), newUser) == currentUser.end())
                    currentUser.push_back(newUser);
                // Optionally log the event.
                ChatMessage cm("System");
                std::string sysMsg = newUser + " has joined.";
                strncpy_s(cm.message, sysMsg.c_str(), sizeof(cm.message) - 1);
                chatMessages.push_back(cm);
                continue;
            }
            // Process user left notifications.
            else if (msg.find("USERLEFT|") == 0) {
                // e.g. "USERLEFT|Bob"
                std::istringstream iss(msg);
                std::string cmd, user;
                getline(iss, cmd, '|');  // "USERLEFT"
                getline(iss, user);
                auto it = std::find(currentUser.begin(), currentUser.end(), user);
                if (it != currentUser.end())
                    currentUser.erase(it);
                ChatMessage cm("System");
                std::string sysMsg = user + " has left.";
                strncpy_s(cm.message, sysMsg.c_str(), sizeof(cm.message) - 1);
                chatMessages.push_back(cm);
                continue;
            }
            // Process private messages.
            else if (msg.find("PRIVATE|") == 0) {
                // Expected format when received: "PRIVATE|<fromUser>|<message>"
                size_t firstSep = msg.find('|', 11);
                if (firstSep != std::string::npos) {
                    std::string fromUser = msg.substr(11, firstSep - 11);
                    std::string messageContent = msg.substr(firstSep + 1);
                    ChatMessage cm;
                    strncpy_s(cm.sender, fromUser.c_str(), sizeof(cm.sender) - 1);
                    strncpy_s(cm.message, messageContent.c_str(), sizeof(cm.message) - 1);
                    privateChatMessages.push_back(cm);
                    playsoundPR(sfxSys);
                }
                continue;
            }
            // Process public broadcast messages.
            else if (msg.find("BROADCAST|") == 0) {
                auto [remoteSender, content] = ParseBroadcastMessage(msg);
                ChatMessage cm;

                if (remoteSender != inputSender)
                    playsoundR(sfxSys);
                strncpy_s(cm.sender, remoteSender.c_str(), sizeof(cm.sender) - 1);
                strncpy_s(cm.message, content.c_str(), sizeof(cm.message) - 1);
                chatMessages.push_back(cm);
                continue;
            }
            // For any unknown message types, treat as a system message.
            else {
                ChatMessage cm("System");
                strncpy_s(cm.message, msg.c_str(), sizeof(cm.message) - 1);
                chatMessages.push_back(cm);
            }
        }

        // Render connection status window.
        ImGui::Begin("Connecting status");
        if (net.isConnected) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Connected!");
        }
        else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Disconnected! Your message is local only");
        }
        ImGui::End();

        // Registration window (only shown until the user registers).
        static bool registerWin = true;
        static char inputName[128] = "";
        if (registerWin) {
            ImGui::SetNextWindowSize(ImVec2(300, 100));
            if (ImGui::Begin("Register", &registerWin, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
                ImGui::InputTextWithHint("##name", "Enter your name", inputName, IM_ARRAYSIZE(inputName));
                if (ImGui::Button("Submit")) {
                    strncpy_s(globalUsername, inputName, sizeof(globalUsername) - 1);
                    if (strlen(inputName) > 0) {
                        // Set the sender name for public messages.
                        strncpy_s(inputSender, inputName, sizeof(inputSender) - 1);
                        registerWin = false;
                        // Connect to the server.
                        if (net.Connect("127.0.0.1", 65432)) {
                            strncpy_s(inputSender, inputName, sizeof(inputSender) - 1);
                            // Send the username to the server as registration.
                            std::string msg = std::string(inputName) + "\n";
                            net.Send(msg);
                        }
                    }
                }
                ImGui::End();
            }
            return true;
        }

        // Flags to control which chat window is active.
        static bool publicChat = true;
        static bool privateChat = false;
        static bool temp = false;

        // Public chat window.
        ChatBox publicChatBox;
        if (publicChat) {
            if (ImGui::Begin("Public Chatroom", &publicChat)) {
                ImGui::BeginChild("user list", ImVec2(100, 500), true);
                for (const auto& user : currentUser) {
                    if (ImGui::Selectable(user.c_str(), userSelected == user, 0, ImVec2(50, 15))) {
                        if (user != std::string(globalUsername)) {
                            userSelected = user;
                            privateChat = true;
                        }
                        else {
                            temp = true;
                        }
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Click to start a private chat");
                    }
                }
                ImGui::EndChild();
            }

            if (temp) {
                ImGui::SetNextWindowSize(ImVec2(300, 100));
                if (ImGui::Begin("Warning", &temp, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
                    ImGui::Text("You cannot chat with yourself!!!");
                    ImGui::End();
                }
            }
            // Public chat input.
            ImGui::SetCursorPos(ImVec2(100, 320));
            ImGui::SetNextItemWidth(350.f);
            ImGui::InputTextWithHint(" ", "Enter text here", inputs, IM_ARRAYSIZE(inputs));
            ImGui::SetCursorPos(ImVec2(480, 320));
            if (ImGui::Button("Send", ImVec2(50, 25))) {
                SendMessages(inputs, inputSender, &chatMessages);
                playsoundS(sfxSys);
            }
            ImGui::SetCursorPos(ImVec2(540, 320));
            if (ImGui::Button("Cancel", ImVec2(50, 25))) {
                DeleteMessage(inputs);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
                SendMessages(inputs, inputSender, &chatMessages);
                playsoundS(sfxSys);
            }
            ImGui::SetCursorPos(ImVec2(100, 20));
            ImGui::SetNextItemWidth(220.f);
            publicChatBox.init(chatMessages);
            ImGui::End();
        }

        // Private chat window.
        if (privateChat) {
            if (ImGui::Begin(("Private Chat with " + userSelected).c_str(), &privateChat)) {
                privateChatBox pChatBox;
                pChatBox.init(privateChatMessages);
                ImGui::SetCursorPos(ImVec2(20, 350));
                ImGui::SetNextItemWidth(350.f);
                ImGui::InputTextWithHint(" ", "Enter text here", privateInputs, IM_ARRAYSIZE(privateInputs));
                ImGui::SetCursorPos(ImVec2(400, 350));
                if (ImGui::Button("Send", ImVec2(50, 25))) {
                    // Send private message in the format:
                    // "PRIVATE|<targetUser>|<message>\n"
                    std::string pmCommand = "PRIVATE|" + userSelected + "|" + std::string(privateInputs) + "\n";
                    net.Send(pmCommand);
                    playsoundS(sfxSys);
                }
                ImGui::SetCursorPos(ImVec2(460, 350));
                if (ImGui::Button("Cancel", ImVec2(50, 25))) {
                    DeleteMessage(privateInputs);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
                    std::string pmCommand = "PRIVATE|" + userSelected + "|" + std::string(privateInputs) + "\n";
                    net.Send(pmCommand);
                }
                ImGui::End();
            }
        }


        // menu bar to control the mode, and other features
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Quit", "Alt+F4")) return false;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("New"))
            {
                if (ImGui::MenuItem("Public Chat")) {
                    //do something
                }
                if (ImGui::MenuItem("Private Chat")) {
                    // 1. New window to choose
                    // 2. private chat window
                }
                if (ImGui::MenuItem("???? Chat")) {
                    // this is Game window.
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("About"))
            {

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

    }


}

class UIconnect {

};

