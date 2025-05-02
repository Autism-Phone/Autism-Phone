#include "Api.h"

using namespace emscripten;
using json = nlohmann::json;

EMSCRIPTEN_WEBSOCKET_T websocket;

bool onWebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) {

    Api* self = static_cast<Api*>(userData);

    std::string msg(reinterpret_cast<const char*>(websocketEvent->data), websocketEvent->numBytes);

    msg.pop_back();

    if (msg == "start_game") {
        std::cout << "Game started! Redirecting to game page..." << std::endl;
        EM_ASM({
            window.location.href = "/static/writing-page.html";
        });
    } else if (msg == "next_round?text") {
        std::cout << "Next round started! Redirecting to game page..." << std::endl;
        EM_ASM({
            window.location.href = "/static/writing-page.html";
        });
    } else if (msg == "next_round?drawing") {
        std::cout << "Next round started! Redirecting to game page..." << std::endl;
        EM_ASM({
            window.location.href = "/static/drawing-page.html";
        });
    } else if (msg == "end_game") {
        std::cout << "Game over! Redirecting to lobby..." << std::endl;
        EM_ASM({
            window.location.href = "/static/home.html";
        });
    } else {
        std::cout << "Received WebSocket message: {" << msg << '}' << std::endl;
    }
    
    return true;
}

Api::Api() {
    
}

Api::~Api() {
    if (websocket > 0) {
        disconnectWebSocket();
    }
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
    val::global("sessionStorage").call<void>("setItem", std::string("playerName"), playerName);
        
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

    attr.onsuccess = Api::onStartSuccess;
    attr.onerror = Api::onErrorDefault;

    gameId = val::global("sessionStorage").call<val>("getItem", std::string("gameId")).as<std::string>();

    std::string url = "/start-game/" + gameId;

    emscripten_fetch(&attr, url.c_str());
}

void Api::round_init() {
    val gameIdVal = val::global("sessionStorage").call<val>("getItem", std::string("gameId"));
    val playerIdVal = val::global("sessionStorage").call<val>("getItem", std::string("playerId"));

    gameId = gameIdVal.isNull() || gameIdVal.isUndefined() 
        ? "" 
        : gameIdVal.as<std::string>();

    playerId = playerIdVal.isNull() || playerIdVal.isUndefined()
        ? ""
        : playerIdVal.as<std::string>();

    if (gameId.empty() || playerId.empty()) {
        std::cerr << "Game ID or Player ID not found in sessionStorage." << std::endl;
        return;
    }

    connect_websocket();
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

    val::global("sessionStorage").call<void>("setItem", std::string("playerId"), self->playerId);
    val::global("sessionStorage").call<void>("setItem", std::string("gameId"), self->gameId);

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

    val::global("sessionStorage").call<void>("setItem", std::string("inviteCode"), self->inviteCode);

    printf("Game ID: %s\n", self->gameId.c_str());
    printf("Invite Code: %s\n", self->inviteCode.c_str());

    self->join_game(self->inviteCode, self->playerName);
}

void Api::onGetStateSuccess(emscripten_fetch_t *fetch) {
    printf("Fetch succeeded! HTTP status: %d\n", fetch->status);

    Api* self = static_cast<Api*>(fetch->userData);

    self->json_string = std::string((char*)fetch->data, fetch->numBytes);

    json json = json::parse(self->json_string);

    

    emscripten_fetch_close(fetch);
}

void Api::fetchGameState() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onGetStateSuccess;
    attr.onerror = Api::onErrorDefault;

    std::string url = "/game-state/" + gameId + "?player_id=" + playerId;
    std::cout << "Fetching game state from: " << url << std::endl;

    emscripten_fetch(&attr, url.c_str());
}

void Api::onSubmitSuccess(emscripten_fetch_t *fetch) {
    printf("Submit succeeded! HTTP status: %d\n", fetch->status);
    emscripten_fetch_close(fetch);
}

void Api::onStartSuccess(emscripten_fetch_t *fetch) {

}

void Api::connect_websocket() {
    if (!emscripten_websocket_is_supported()) {
        std::cerr << "WebSockets are not supported in this environment." << std::endl;
        return;
    }

    std::string playerId = val::global("sessionStorage").call<val>("getItem", std::string("playerId")).as<std::string>();
    std::string gameId = val::global("sessionStorage").call<val>("getItem", std::string("gameId")).as<std::string>();
    std::string url = "ws://127.0.0.1:2137/ws?game_id=" + gameId + "&player_id=" + playerId;

    EmscriptenWebSocketCreateAttributes attr = {
        url.c_str(),
        NULL,
        EM_TRUE
    };

    websocket = emscripten_websocket_new(&attr);

    if (websocket <= 0) {
        std::cerr << "Failed to create WebSocket." << std::endl;
        return;
    }

    emscripten_websocket_set_onmessage_callback(websocket, this, onWebSocketMessage);
    std::cout << "WebSocket connected to server with game_id: " << gameId << " and player_id: " << playerId << std::endl;
}

void Api::disconnectWebSocket() {
    if (websocket > 0) {
        emscripten_websocket_close(websocket, 1000, "Client disconnecting");
        emscripten_websocket_delete(websocket);
        std::cout << "WebSocket disconnected." << std::endl;
    }
}

std::string Api::encodeImageData(const Color *pixelBuffer) {
    size_t bufferSize = 600'000;
    unsigned const char* data = reinterpret_cast<unsigned const char*>(pixelBuffer);
    size_t dataSize = bufferSize * sizeof(Color);

    return base64_encode(data, dataSize);
}