#pragma once
#define _USE_MATH_DEFINES
#define M_PI   3.14159265358979323846264338327950288
#include <fmod.hpp>
#include <fmod_errors.h>
#include <cmath>
#include <thread>
#include <chrono>
#include <iostream>
#include <conio.h>
#include <vector>
#pragma comment(lib, "fmod_vc.lib")

class Audio {
	FMOD::System* system;
	FMOD::Sound* soundSend;
	FMOD::Sound* soundRec;
	FMOD::Sound* soundPRec;
	FMOD::Channel* channel;
	bool isplaying = false;
public:
	void init() {
		if (isplaying == false) {
			FMOD::System_Create(&system);
			system->init(512, FMOD_INIT_NORMAL, NULL);
			soundSend = NULL;
			soundRec = NULL;
			channel = NULL;
			system->createSound("Audio/notice2.wav", FMOD_DEFAULT, NULL, &soundSend);
			system->createSound("Audio/notice3.wav", FMOD_DEFAULT, NULL, &soundRec);
			system->createSound("Audio/notice4.wav", FMOD_DEFAULT, NULL, &soundPRec);
			//system->createSound("Audio/music.mp3", FMOD_DEFAULT, NULL, &sound);
		}
	}


	void playMusicSend() {
		FMOD::Channel* channel = nullptr;
		system->playSound(soundSend, 0, false, &channel);
		if (channel) {
			channel->setVolume(1.0f);
		}
	}

	void playMusicPRec() {
		FMOD::Channel* channel = nullptr;
		system->playSound(soundPRec, 0, false, &channel);
		if (channel) {
			channel->setVolume(1.0f);
		}
	}


	void playMusicRec() {
		FMOD::Channel* channel = nullptr;
		system->playSound(soundRec, 0, false, &channel);
		if (channel) {
			channel->setVolume(1.0f);
		}
	}
	void playMusicPublic() {
		FMOD::Sound* sound = NULL;
		FMOD::Channel* channel = NULL;
		float volume = 1.0f;
		channel->setVolume(volume);
		system->set3DSettings(10.f, 10.f, 10.f);
		system->createSound("Audio/notice2.wav", FMOD_DEFAULT, NULL, &sound);
		system->playSound(sound, NULL, false, &channel);
	}

	void Stop() {
		system->close();
		system->release();
	}
};