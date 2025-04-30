#pragma once

#include "Compile.h"

#include <string>
#include <cstring>
#include <sstream>

#include "Utils.h"
#include "base64.h"
#include "json.hpp"

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/fetch.h>
#include <emscripten/websocket.h>

class Api {
public:
    Api();
    ~Api();

    std::string playerName;

    void create_game();
    void join_game(const std::string& inviteCode, const std::string& playerName);
    void start_game();
    void round_init();
    double fetch_time();
    void submit(Color* pixelBuffer);
    void submit(const std::string& text);
    void connect_websocket();

private:
    std::string gameId, playerId, inviteCode;
    std::string json_string;
    GameType gameType;

    std::string requestDataBuffer;

    StateTypes waddayawant;

    static void onErrorDefault(emscripten_fetch_t* fetch);
    static void onSuccessDefault(emscripten_fetch_t* fetch);
    
    static void onJoinSuccess(emscripten_fetch_t* fetch);

    static void onCreationSuccess(emscripten_fetch_t* fetch);

    static void onGetStateSuccess(emscripten_fetch_t* fetch);
    void fetchGameState();

    static void onSubmitSuccess(emscripten_fetch_t* fetch);

    static void onStartSuccess(emscripten_fetch_t* fetch);

    void disconnectWebSocket();

    std::string encodeImageData(const Color* pixelBuffer);
};