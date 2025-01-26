#include "ImGui/imconfig.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imstb_rectpack.h"
#include "ImGui/imstb_textedit.h"
#include "ImGui/imstb_truetype.h"

namespace MyUI {
	class ChatMessage {
	public:
		char sender[256];
		char message[1024];
		ChatMessage(const char* defaultSender = "Alice") {
			strncpy_s(sender, defaultSender, sizeof(sender) - 1);
			
		}
	};

	

	// key parameter
	static char inputs[1024]; // input text
	static char inputSender[256]; // input text
	static std::vector<ChatMessage> chatMessages; // store all chat message

	static char privateInputs[1024]; // input text
	static char privateInputSender[256]; // input text
	static std::vector<ChatMessage> privateChatMessages; // store all chat message

	class ChatBox {
	public:
		virtual void init(std::vector<ChatMessage> chatMessages) {
			// the display chat box
			ImGui::BeginChild("ChatBox", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Text("System: Welcome to Public Chatroom..");
			for (const auto& message : chatMessages) {
				ImGui::TextWrapped("%s: %s", message.sender, message.message); // Display each message
			}

			// Auto-scroll to the bottom when new messages are added
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
				ImGui::SetScrollHereY(1.0f);
			}
			// the limitation
			if (chatMessages.size() > 1) {
				ImGui::TextColored(ImColor(255, 0, 0, 0), "System: The message will be deleted!!!");
				if (chatMessages.size() > 105)
					chatMessages.erase(chatMessages.begin());
			}

			ImGui::EndChild();
		}
	};

	class privateChatBox :public ChatBox {
	public:
		void init(std::vector<ChatMessage> chatMessages) override {
			// Add some notification
			
			// display chat box
			ImGui::BeginChild("ChatBox", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
			//const auto& firstMessage = chatMessages.front(); // extract first vector
			//ImGui::Text("This is a private chat window with %s", firstMessage.sender);
			ImGui::Text("Connecting....");
			ImGui::Text("Connected! Lets chat!");
			
			for (const auto& message : chatMessages) {
				ImGui::TextWrapped("%s: %s", message.sender, message.message); // Display each message
			}

			// Auto-scroll to the bottom when new messages are added
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
				ImGui::SetScrollHereY(1.0f);
			}
			// the limitation
			if (chatMessages.size() > 1) {
				ImGui::TextColored(ImColor(255, 0, 0, 0), "System: The message will be deleted!!!");
				if (chatMessages.size() > 105)
					chatMessages.erase(chatMessages.begin());
			}

			ImGui::EndChild();
		}
	};

	// keys
	void SendMessages(char* inputs, char *inputsSender, std::vector<ChatMessage>* chatMessages) {
		//! idea of network: Send newMessage to server, and in server, run pushback to add info
		// avoid send empty message
		if (inputs[0] != '\0') {
			//! Logic to send the message
			ChatMessage newMessage;

			// add content into new message
			// newMessage.message = inputs;
			strncpy_s(newMessage.message, inputs,sizeof(newMessage.message)-1);
			strncpy_s(newMessage.sender, inputsSender,sizeof(newMessage.sender)-1);

			chatMessages->push_back(newMessage);

			// Clear the input buffer
			memset(inputs, 0, sizeof(inputs));
		}
	}

	void SystemMessage() {
		static char SysInputs[512] = ""; // input text
		static char SysSender[10] = "system"; // input text
		ChatMessage newMessage;

		// add content into new message
		// newMessage.message = inputs;
		strncpy_s(newMessage.message, SysInputs, sizeof(newMessage.message) - 1);
		strncpy_s(newMessage.sender, SysSender, sizeof(newMessage.sender) - 1);

		chatMessages.push_back(newMessage);
	}

	void DeleteMessage(char* inputs) {
		if (inputs[0] != '\0') {
			// Clear the input buffer
			memset(inputs, 0, sizeof(inputs));
		}
	}

	
	
	bool RenderUI() {
		static bool publicChat = true;
		static bool privateChat = false;
		static bool gameChat = false;
		ChatBox publicChatBox;
		ImGui::ShowDemoWindow();

		if (publicChat) {

			if (ImGui::Begin("Public Chatroom", &publicChat)) {

				// network section to control?
				// button to private chat
				ImGui::SetCursorPos(ImVec2(20, 40));
				if (ImGui::Button("User1", ImVec2(60, 30))) {
					privateChat = true;	
				}

				ImGui::SetCursorPos(ImVec2(20, 90));
				if (ImGui::Button("User2", ImVec2(60, 30))) {
					privateChat = true;
				}

				ImGui::SetCursorPos(ImVec2(20, 140));
				if (ImGui::Button("User3", ImVec2(60, 30))) {
					privateChat = true;
				}

				ImGui::SetCursorPos(ImVec2(20, 300));
				if (ImGui::Button("????", ImVec2(20, 20))) {
					gameChat = true;
				}
				
			}

			// public chat room
			ImGui::SetCursorPos(ImVec2(100, 320));
			ImGui::SetNextItemWidth(350.f);
			ImGui::InputTextWithHint(" ", "Enter text here", inputs, IM_ARRAYSIZE(inputs));
			// ImGui::InputText("Text message here", inputs, sizeof(inputs));
			ImGui::SetCursorPos(ImVec2(480, 320));
			if (ImGui::Button("Send", ImVec2(50, 25))) {
				SendMessages(inputs,inputSender,&chatMessages);
			}
			ImGui::SetCursorPos(ImVec2(540, 320));
			if (ImGui::Button("Cancel", ImVec2(50, 25))) {
				DeleteMessage(inputs);
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Enter,false)) {
				SendMessages(inputs, inputSender, &chatMessages);
			}
			ImGui::SetCursorPos(ImVec2(100, 20));
			ImGui::SetNextItemWidth(220.f);
			publicChatBox.init(chatMessages);
			ImGui::End();
		}
		
		
	
		// sub windows of public, private chat
		if (privateChat) {
			if (ImGui::Begin("Private Chat with ...", &privateChat)) {
				privateChatBox privateChat;
				privateChat.init(privateChatMessages);

				ImGui::SetCursorPos(ImVec2(20, 350));
				ImGui::SetNextItemWidth(350.f);
				ImGui::InputTextWithHint(" ", "Enter text here", privateInputs, IM_ARRAYSIZE(inputs));
				// logic function....
				ImGui::SetCursorPos(ImVec2(400, 350));
				if (ImGui::Button("Send", ImVec2(50, 25))) {
					SendMessages(privateInputs,privateInputSender, &privateChatMessages);
				}
				ImGui::SetCursorPos(ImVec2(460, 350));
				if (ImGui::Button("Cancel", ImVec2(50, 25))) {
					DeleteMessage(privateInputs);
				}
				if (ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
					SendMessages(privateInputs, privateInputSender, &privateChatMessages);
				}
			}
			ImGui::End();
		}
		
		if (gameChat) {
			if (ImGui::Begin("....c..h....t....w...i.....h....Ru...K.ia", &gameChat)) {
			
			}
			ImGui::End();
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
