#include "Api.h"

using namespace emscripten;
using json = nlohmann::json;

Api::Api() {
    
}

void Api::create_game() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onCreationSuccess;
    attr.onerror = Api::onErrorDefault;

    emscripten_fetch(&attr, "/create-game");
}

void Api::join_game(const std::string &inviteCode, const std::string &playerName) {
    val::global("localStorage").call<void>("setItem", std::string("playerName"), playerName);
        
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onJoinSuccess;
    attr.onerror = Api::onErrorDefault;

    json j = {
        {"invite_code", inviteCode},
        {"name", playerName}
    };
    requestDataBuffer = j.dump();
    attr.requestData = requestDataBuffer.c_str();
    attr.requestDataSize = requestDataBuffer.size();

    emscripten_fetch(&attr, "/join-game");
}

void Api::start_game() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onCreationSuccess;
    attr.onerror = Api::onErrorDefault;

    std::string url = "/start-game/" + gameId;

    emscripten_fetch(&attr, url.c_str());
}

void Api::round_init(GameType gameType) {
    val gameIdVal = val::global("localStorage").call<val>("getItem", std::string("gameId"));
    val playerIdVal = val::global("localStorage").call<val>("getItem", std::string("playerId"));

    gameId = gameIdVal.isNull() || gameIdVal.isUndefined() 
        ? "" 
        : gameIdVal.as<std::string>();

    playerId = playerIdVal.isNull() || playerIdVal.isUndefined()
        ? ""
        : playerIdVal.as<std::string>();

    if (gameId.empty() || playerId.empty()) {
        std::cerr << "Game ID or Player ID not found in localStorage." << std::endl;
        return;
    }

    fetchGameState();

    json json = json::parse(json_string);

    std::string round_type = json["round"]["type"].get<std::string>();

    GameType round_type_enum;

    if (round_type == "drawing") {
        round_type_enum = GameType::DRAWING;
    } else if (round_type == "writing") {
        round_type_enum = GameType::PROMPTING;
    } else {
        std::cerr << "Unknown round type: " << round_type << std::endl;
        return;
    }

    if (round_type_enum != gameType) {
        std::cerr << "Game type mismatch: expected " << gameType << ", got " << round_type_enum << std::endl;
        return;
    }

    gameURL = getCurrentURL();
}

double Api::fetch_time() {
    fetchGameState();
    json json = json::parse(json_string);

    double time_left = json["round"]["time_left"].get<double>();

    return time_left;
}

void Api::submit(Color* pixelBuffer) {
    std::string encodedData = encodeImageData(pixelBuffer);
    
    json j = {
        {"player_id", playerId},
        {"content", {
            {"drawing", encodedData}
        }}
    };

    requestDataBuffer = j.dump();

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onSubmitSuccess;
    attr.onerror = Api::onErrorDefault;

    attr.requestData = requestDataBuffer.c_str();
    attr.requestDataSize = requestDataBuffer.size();

    emscripten_fetch(&attr, "/submit");
}

void Api::submit(const std::string &text) {
    json j = {
        {"player_id", playerId},
        {"content", {
            {"text", text}
        }}
    };
    
    requestDataBuffer = j.dump();

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onSubmitSuccess;
    attr.onerror = Api::onErrorDefault;

    attr.requestData = requestDataBuffer.c_str();
    attr.requestDataSize = requestDataBuffer.size();

    emscripten_fetch(&attr, "/submit");
}

void Api::onErrorDefault(emscripten_fetch_t *fetch) {
    printf("Fetch failed. HTTP status: %d\n", fetch->status);
    printf("Error message: %s\n", fetch->data ? fetch->data : "No error message");
    emscripten_fetch_close(fetch);
}

void Api::onSuccessDefault(emscripten_fetch_t *fetch) {
    printf("Fetch succeeded! HTTP status: %d\n", fetch->status);
    emscripten_fetch_close(fetch);
}

void Api::onJoinSuccess(emscripten_fetch_t *fetch) {
    printf("Fetch succeeded! HTTP status: %d\n", fetch->status);

    Api* self = static_cast<Api*>(fetch->userData);

    self->json_string = std::string((char*)fetch->data, fetch->numBytes);

    emscripten_fetch_close(fetch);

    json json = json::parse(self->json_string);
    self->playerId = json["player_id"].get<std::string>();
    self->gameId = json["game_id"].get<std::string>();

    val::global("localStorage").call<void>("setItem", std::string("playerId"), self->playerId);
    val::global("localStorage").call<void>("setItem", std::string("gameId"), self->gameId);

    EM_ASM({
        window.location.href = "/static/lobby.html";
    });
}

void Api::onCreationSuccess(emscripten_fetch_t *fetch) {
    printf("Fetch succeeded! HTTP status: %d\n", fetch->status);

    Api* self = static_cast<Api*>(fetch->userData);

    self->json_string = std::string((char*)fetch->data, fetch->numBytes);

    printf("JSON string: %s\n", self->json_string.c_str());

    emscripten_fetch_close(fetch);

    json json = json::parse(self->json_string);
    self->inviteCode = json["invite_code"].get<std::string>();
    self->gameId = json["game_id"].get<std::string>();

    printf("Game ID: %s\n", self->gameId.c_str());
    printf("Invite Code: %s\n", self->inviteCode.c_str());

    self->join_game(self->inviteCode, "joe");
}

void Api::onGetStateSuccess(emscripten_fetch_t *fetch) {
    printf("Fetch succeeded! HTTP status: %d\n", fetch->status);

    Api* self = static_cast<Api*>(fetch->userData);

    self->json_string = std::string((char*)fetch->data, fetch->numBytes);

    emscripten_fetch_close(fetch);
}

void Api::fetchGameState() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onGetStateSuccess;
    attr.onerror = Api::onErrorDefault;

    std::string url = "/game-state/" + gameId + "?playerId=" + playerId;

    emscripten_fetch(&attr, url.c_str());
}

void Api::onSubmitSuccess(emscripten_fetch_t *fetch) {
    printf("Submit succeeded! HTTP status: %d\n", fetch->status);
    emscripten_fetch_close(fetch);
}

std::string Api::encodeImageData(const Color *pixelBuffer) {
    size_t bufferSize = 600'000;
    unsigned const char* data = reinterpret_cast<unsigned const char*>(pixelBuffer);
    size_t dataSize = bufferSize * sizeof(Color);

    return base64_encode(data, dataSize);
}

std::string Api::getCurrentURL() {
    char* url = (char*)EM_ASM_PTR({
        const str = window.location.href;
        const lengthBytes = lengthBytesUTF8(str) + 1;
        const stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(str, stringOnWasmHeap, lengthBytes);
        return stringOnWasmHeap;
    });

    std::string urlStr(url);
    free(url); 
    return urlStr;
}
