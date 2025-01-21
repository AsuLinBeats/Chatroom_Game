#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include<chrono>
// This part make me passionate about making a text adventure game, so This part is game logic for that.
// The game will occupy one dialogue, which is not connected to network.
// I try to add story on it, feel free play it! If you are bored with waiting, just change wait time to 0
struct Dialogue
{
	std::string npcText;
	std::vector<std::string> playerChoice;
	std::vector<int> nextDialogue;
	int waitingTime; // in second
};

struct GameState {
	int playerAffinity = 0; // decide the end of game
	int currentState;
	bool isWaiting;
	std::unordered_map<int, Dialogue> dialogueMap;
	std::chrono::steady_clock::time_point waitStartTime; // time starting waiting
};

void GameInit() {
	GameState game;
	game.currentState = 0;

	Dialogue node0 = {
		"Hello? Hello?..."
	};

	Dialogue node1 = {
	"Anyone there???",
	 {"Yeah!!", "(Keep Silent)"},
	 {}
	};

	Dialogue node2 = {
		"Ahhh, "
	};

}