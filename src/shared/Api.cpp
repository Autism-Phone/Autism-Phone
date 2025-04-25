#include "Api.h"

using namespace emscripten;

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

    std::string jsonPayload = "{\"invite_code\":\"" + inviteCode + "\",\"player_name\":\"" + playerName + "\"}";
    attr.requestData = jsonPayload.c_str();
    attr.requestDataSize = jsonPayload.size();

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

    gameURL = getCurrentURL();
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
    std::string encodedData = encodeImageData(pixelBuffer);
    std::string jsonPayload = "{\"player_id\":\"" + playerId + "\",\"content\":{\"drawing\":\"" + encodedData + "\"}}";

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onSubmitSuccess;
    attr.onerror = Api::onErrorDefault;

    attr.requestData = jsonPayload.c_str();
    attr.requestDataSize = jsonPayload.size();

    emscripten_fetch(&attr, "/submit");
}

void Api::submit(const std::string &text) {
    std::string jsonPayload = "{\"player_id\":\"" + playerId + "\",\"content\":{\"text\":\"" + text + "\"}}";

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = Api::onSubmitSuccess;
    attr.onerror = Api::onErrorDefault;

    attr.requestData = jsonPayload.c_str();
    attr.requestDataSize = jsonPayload.size();

    emscripten_fetch(&attr, "/submit");
}

void Api::onErrorDefault(emscripten_fetch_t *fetch) {
    printf("Fetch failed. HTTP status: %d\n", fetch->status);
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

    EM_ASM({
        try {
            const json = JSON.parse(UTF8ToString($0));
            if (json.player_id && json.game_id) {
                stringToUTF8(json.game_id, $1, 64);
                stringToUTF8(json.player_id, $1, 64);
            } else {
                console.error("player_id not found in JSON.");
            }
        } catch (e) {
            console.error("Failed to parse JSON:", e);
        }
    }, self->json_string.c_str(), self->playerId.data());

    val::global("localStorage").call<void>("setItem", std::string("playerId"), self->playerId);
    val::global("localStorage").call<void>("setItem", std::string("gameId"), self->gameId);
}

void Api::onCreationSuccess(emscripten_fetch_t *fetch) {
    printf("Fetch succeeded! HTTP status: %d\n", fetch->status);

    Api* self = static_cast<Api*>(fetch->userData);

    self->json_string = std::string((char*)fetch->data, fetch->numBytes);

    emscripten_fetch_close(fetch);

    EM_ASM({
        try {
            const json = JSON.parse(UTF8ToString($0));
            if (json.game_id && json.invite_code) {
                stringToUTF8(json.game_id, $1, 64);
                stringToUTF8(json.invite_code, $2, 64);
            } else {
                console.error("game_id or invite_code not found in JSON.");
            }
        } catch (e) {
            console.error("Failed to parse JSON:", e);
        }
    }, self->json_string.c_str(), self->gameId.data(), self->playerId.data());

    val::global("localStorage").call<void>("setItem", std::string("gameId"), self->gameId);
    val::global("localStorage").call<void>("setItem", std::string("inviteCode"), self->inviteCode);
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
