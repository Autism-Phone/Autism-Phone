![image](https://github.com/user-attachments/assets/c0b0c966-1900-4ad0-937f-25f075c74d52)

# Nie działa? Przeczytaj!
## Błąd na 99% dotyczy tego paskudnego cpp
- Ten syf był kompilowany do jsa i WebAssembly emscriptenem 4.0.4
- Najpewniej było to robione na Macu z ARMem, więc nie ma prawa zadziałać na innych procesorach
- Zapraszam do pobrania [tutaj](https://emscripten.org/docs/getting_started/downloads.html)
- Trzeba to samemu skompilować z (jak narazie) tymi flagami `emcc "plik-do-kompilacji".cpp -o "plik do kompilacji".js -sEXPORTED_FUNCTIONS=_main,_on_mouse_move -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sSTACK_SIZE=1000000`
- Kiedyś zrobię jakiegoś makea do tego
- Kiedyś...

## Ewentualnie mamy error typu PEBKAC
- Wywaliło CORSy
- Proszę odpalić to na serwerze
- Jeżeli dalej nie działa to poproszę o zgłoszenie issue
