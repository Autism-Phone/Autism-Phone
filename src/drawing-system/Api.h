#include <string>

#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/fetch.h>

class Api {
public:
    Api(GameType gameType);

    double fetch_time();
    void submit(Color* pixelBuffer);

private:
    std::string gameId, playerId;
    std::string json_string;
    GameType gameType;

    static void onGetStateSuccess(emscripten_fetch_t* fetch);
    static void onGetStateError(emscripten_fetch_t* fetch);
    void fetchGameState();
};