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
    Api(GameType gameType);

    double fetch_time();
    void submit(Color* pixelBuffer);
    void submit(const std::string& text);

private:
    std::string gameId, playerId;
    std::string json_string;
    std::string gameURL;
    GameType gameType;

    static void onGetStateSuccess(emscripten_fetch_t* fetch);
    static void onGetStateError(emscripten_fetch_t* fetch);
    void fetchGameState();

    static void onSubmitSuccess(emscripten_fetch_t* fetch);
    static void onSubmitError(emscripten_fetch_t* fetch);

    std::string encodeImageData(const Color* pixelBuffer);

    std::string getCurrentURL();
};