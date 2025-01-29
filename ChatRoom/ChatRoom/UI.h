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
	static std::vector<std::string> currentUser = {"Alice","Miku","Luna"}; // should get from server
	std::string userSelected;

	static char inputs[1024]; // input text
	char inputSender[128]; // input text
	static std::vector<ChatMessage> chatMessages; // store all chat message

	static char privateInputs[1024]; // input text
	static char privateInputSender[256]; // input text
	static std::vector<ChatMessage> privateChatMessages; // store all chat message

	static bool popupwin = true;
	static bool nameValid = false;
	static char inputName[128];

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
			ImGui::BeginChild(("This is Private chat with "+ userSelected).c_str(), ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
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

	void Register() {

	}
	
	bool RenderUI(Audio sfxSys) {
		/*if (popupwin) {
			ImGui::OpenPopup("Please set your name");
			ImVec2 centre = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(centre, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			
			ImGui::InputTextWithHint(" ", "Enter your name", inputName, IM_ARRAYSIZE(inputName));
			if (ImGui::Button("х╥хо", ImVec2(80, 25))) {
				if (strlen(inputName) > 0) {
					nameValid = true;
					popupwin = false;
					strncpy_s(inputSender, inputName, sizeof(inputSender) - 1);
				}

			}
			ImGui::EndPopup();

		}*/
		static bool publicChat = true;
		static bool privateChat = false;
		static bool gameChat = false;
		ChatBox publicChatBox;
		
		ImGui::ShowDemoWindow();

		if (publicChat) {

			if (ImGui::Begin("Public Chatroom", &publicChat)) {
				sfxSys.init();
				sfxSys.playMusic1();
				// network section to control?
				// button to private chat

				ImGui::BeginChild("user list",ImVec2(100,500),true);

				for (const auto& user : currentUser) {
					if (ImGui::Selectable(user.c_str(), userSelected == user.c_str(), NULL, ImVec2(50, 15))) {
						userSelected = user;
						privateChat = true;
					}
					
					// notice when mouse hover
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Click to start a private chat", user.c_str());
					}
				}

				ImGui::EndChild();

				
			}
			// copy contents of userSelected to inputsender
			//! THIS IS ABSOLUTELY WRONG TO USE USERSE;ECTED NAME AS SENDER NAME, BUT IT TESTS SOMETHING.
			//strncpy_s(inputSender, userSelected.c_str(), sizeof(inputSender) - 1);
			////inputSender[sizeof(inputSender) - 1] = '\0'; 
			
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
			if (ImGui::Begin(("Private chat with "+ userSelected).c_str(), &privateChat)) {
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
				// INVOKE GAME LOGIC
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
