#pragma once

#include <string>
#include <cstring>
#include <sstream>

#include "Utils.h"
#include "base64.h"

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/fetch.h>

class Api {
public:
    Api();

    void create_game();
    void join_game(const std::string& inviteCode, const std::string& playerName);
    void start_game();
    void round_init(GameType gameType);
    double fetch_time();
    void submit(Color* pixelBuffer);
    void submit(const std::string& text);

private:
    std::string gameId, playerId, inviteCode;
    std::string json_string;
    std::string gameURL;
    GameType gameType;

    static void onErrorDefault(emscripten_fetch_t* fetch);
    static void onSuccessDefault(emscripten_fetch_t* fetch);
    
    static void onJoinSuccess(emscripten_fetch_t* fetch);

    static void onCreationSuccess(emscripten_fetch_t* fetch);

    static void onGetStateSuccess(emscripten_fetch_t* fetch);
    void fetchGameState();

    static void onSubmitSuccess(emscripten_fetch_t* fetch);

    std::string encodeImageData(const Color* pixelBuffer);

    std::string getCurrentURL();
};