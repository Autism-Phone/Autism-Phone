#include "Api.h"

using namespace emscripten;

Api::Api(GameType gameType) : gameType(gameType) {
    gameId = val::global("localStorage").call<std::string>("getItem", "gameId");
    playerId = val::global("localStorage").call<std::string>("getItem", "playerId");

    if (gameId.empty() || playerId.empty()) {
        std::cerr << "Game ID or Player ID not found in localStorage." << std::endl;
        return;
    }

    fetchGameState();

    char round_type[64] = {0};

    EM_ASM({
        try {
            const json = JSON.parse(UTF8ToString($0));
            const type = json.round?.type || "";
            stringToUTF8(type, $1, 64);
        } catch (e) {
            console.error("Failed to parse JSON or get round type:", e);
        }
    }, json_string.c_str(), round_type);

    GameType round_type_enum;

    if (strcmp(round_type, "drawing") == 0) {
        round_type_enum = GameType::DRAWING;
    } else if (strcmp(round_type, "prompting") == 0) {
        round_type_enum = GameType::PROMPTING;
    } else {
        std::cerr << "Unknown round type: " << round_type << std::endl;
        return;
    }

    if (round_type_enum != gameType) {
        std::cerr << "Game type mismatch: expected " << gameType << ", got " << round_type_enum << std::endl;
        return;
    }
}   

double Api::fetch_time() {
    fetchGameState();
    double time_left = EM_ASM_DOUBLE({
        try {
            var jsonString = UTF8ToString($0);
            var parsed = JSON.parse(jsonString);
            if (parsed.round && typeof parsed.round.time_left !== "undefined") {
                return parsed.round.time_left;
            }
        }
        catch (e) {
            console.error("JSON parse failed:", e);
        }
        return -1.0;
    }, json_string.c_str());

    return time_left;
}

void Api::submit(Color* pixelBuffer) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onSuccess;
    attr.onerror = Api::onError;

    std::string url = "https://example.com/api/submit"; // Replace with your actual URL
    attr.url = url.c_str();

    std::string data = "{\"gameId\":\"" + gameId + "\",\"playerId\":\"" + playerId + "\"}";
    attr.requestData = data.c_str();
    attr.requestDataSize = data.size();

    emscripten_fetch(&attr);
    
}

void Api::onGetStateSuccess(emscripten_fetch_t *fetch) {
    printf("Fetch succeeded! HTTP status: %d\n", fetch->status);

    Api* self = static_cast<Api*>(fetch->userData);

    self->json_string = std::string((char*)fetch->data, fetch->numBytes);

    emscripten_fetch_close(fetch);
}

void Api::onGetStateError(emscripten_fetch_t *fetch) {
    printf("Fetch failed. HTTP status: %d\n", fetch->status);
    emscripten_fetch_close(fetch);
}

void Api::fetchGameState() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onGetStateSuccess;
    attr.onerror = Api::onGetStateError;
}