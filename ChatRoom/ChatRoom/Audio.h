#pragma once
#define _USE_MATH_DEFINES
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
public:
	void init() {
		FMOD::System_Create(&system);
		system->init(512, FMOD_INIT_NORMAL, NULL);
	}
	FMOD_RESULT F_CALLBACK sineCallback(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int
		inchannels, int* outchannels)
	{
		static float phase = 0.0f;
		const float frequency = 5000.0f;
		const float sampleRate = 48000.0f;
		for (unsigned int i = 0; i < length; i++) {
			float sample = sinf(phase);
			phase += 2.0f * M_PI * frequency / sampleRate;
			if (phase >= 2.0f * M_PI) {
				phase -= 2.0f * M_PI;
			}
			// PUT THIS INTO CHANNELS
			for (int j = 0; j < *outchannels; j++) {
				outbuffer[i * (*outchannels) + j] = sample;
			}
		}
		return FMOD_OK;
	}

	void playMusic() {
		FMOD::Sound* sound = NULL;
		FMOD::Channel* channel = NULL;
		float volume = 1.0f;
		channel->setVolume(volume);
		system->set3DSettings(10.f, 10.f, 10.f);
		system->createSound("music.mp3", FMOD_DEFAULT, NULL, &sound);
		system->playSound(sound, NULL, false, &channel);
	}

	void playMultiMusic() {
		FMOD::Sound* sound1 = NULL;
		FMOD::Channel* channel1 = NULL;
		system->createSound("music.mp3", FMOD_DEFAULT, NULL, &sound1);
		sound1->setMode(FMOD_LOOP_NORMAL);
		system->playSound(sound1, NULL, false, &channel1);
		FMOD::Sound* sound2 = NULL;
		FMOD::Channel* channel2 = NULL;
		system->createSound("machinegun.mp3", FMOD_DEFAULT, NULL, &sound2);
		sound2->setMode(FMOD_LOOP_OFF);
		while (true) {
			bool isPlaying = false;
			channel2->isPlaying(&isPlaying);
			if (isPlaying == false) {
				system->playSound(sound2, NULL, false, &channel2);
			}
			system->update();
		}
	}

	void Spatial() {
		FMOD::Sound* sound = NULL;
		FMOD::Channel* channel = NULL;
		system->createSound("machinegun.mp3", FMOD_DEFAULT, NULL, &sound);
		sound->setMode(FMOD_LOOP_OFF);

		// set player position
		FMOD_VECTOR playerPos = { 0.f,0.f,0.f };
		FMOD_VECTOR playerVel = { 0.f,0.f,0.f };
		FMOD_VECTOR playerForward = { 0.f,0.f,1.f };
		FMOD_VECTOR playerUp = { 0.f,1.f,0.f };
		system->set3DListenerAttributes(0, &playerPos, &playerVel, &playerForward, &playerUp);
		// set sound position
		FMOD_VECTOR soundPos = { 10.f,10.f,20.f };
		FMOD_VECTOR soundVel = { 0.f,0.f,0.f };
		// add rotate sound
		float angle = 0.f;
		const float radius = 5.f;
		const float speed = 0.05f;


		system->playSound(sound, NULL, true, &channel);
		channel->set3DAttributes(&soundPos, &soundVel);
		channel->setPaused(false);
		while (true) {
			soundPos.x = radius * cos(angle);
			soundPos.z = radius * sin(angle);
			channel->set3DAttributes(&soundPos, &soundVel);
			angle += speed;
			if (angle > 2.f * M_PI) {
				angle -= 2.f * M_PI;
			}
			system->update();
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			if (_kbhit()) {
				break;
			}
		}
	}

	void RIR() {

	}

	void Stop() {
		std::cout << "Press any key to quit" << std::endl;
		while (true)
		{
			std::cin.get();
			break;
		}
	}

};