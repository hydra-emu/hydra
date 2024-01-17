#pragma once

#include "corewrapper.hxx"
#include <glad/glad.h>
#include <hydra/core.hxx>
#include <string>
#include <thread>

struct Bot
{
    Bot(std::shared_ptr<hydra::EmulatorWrapper> emulator, GLuint fbo, const std::string& token,
        int frames_pressed, int frames_released);
    ~Bot();
    void runFrame();
    void pushFrame(void* data, hydra::Size size);
    static void videoCallback(Bot* bot, void* data, hydra::Size size);

private:
    void start();
    std::shared_ptr<hydra::EmulatorWrapper> emulator;
    GLuint fbo;
    std::string token;
    int frames_pressed;
    int frames_released;
    hydra::Size native_size;
    std::vector<std::vector<uint8_t>> frames;
};

int bot_main();