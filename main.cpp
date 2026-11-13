// ============================================================================
//  Spellculator
//  An educational action game: dodge slime fireballs and defeat slimes by
//  solving math problems. Built with C++ and raylib.
//
//  Author: Hania Qaisar
//  Course: CS-110 Fundamentals of Computer Programming
// ============================================================================

#include <raylib.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <utility>

using namespace std;

// ----------------------------------------------------------------------------
//  Constants
// ----------------------------------------------------------------------------
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

const int ENEMY_COUNT = 3;
const int MAX_FIREBALLS = 5;
const int MAX_HEALTH = 3;

const int WIZARD_FRAMES = 11;  // wizard sprite sheet: 11 frames in one row
const int SLIME_FRAMES = 12;   // slime sprite sheet: 4 columns x 3 rows
const int SLIME_COLS = 4;
const int SLIME_ROWS = 3;

const float QUESTION_TIME = 10.0f;  // seconds allowed per math question
const float FIRE_RATE = 2.0f;       // seconds between enemy volleys
const float WORLD_LIMIT = 760.0f;   // max x/y for wizard and slimes

// Offsets used for hit detection (sprite centres)
const float PLAYER_CENTER_OFFSET = 40.0f;
const float ENEMY_CENTER_OFFSET = 30.0f;

const Color arcanePurple = { 45, 25, 80, 255 };
const Color wizardGold = { 255, 215, 0, 255 };
const Color spiritLavender = { 180, 150, 230, 255 };
const Color dullForest = { 40, 120, 40, 255 };

// ----------------------------------------------------------------------------
//  Game state and data structures
// ----------------------------------------------------------------------------
enum GameState { MENU, INSTRUCTIONS, CREDITS, PLAYING, QUESTION, VICTORY, GAME_OVER, SETTINGS };

struct Player {
    Vector2 position;
    int health;
    int speed;
    int score;
    int currentFrame;
    float frameTimer;
};

struct Enemy {
    Vector2 position;
    int speed;
    bool active;
    int currentFrame;
    float frameTimer;
};

struct Question {
    int num1;
    int num2;
    char op;     // '+', '-', '*', '/', or '?' for "X + num1 = num2"
    int answer;
};

struct Spell {
    Vector2 position;  // centre of the spell
    bool active;
    int targetEnemyIndex;
    float speed;
};

struct Fireball {
    Vector2 position;  // centre of the fireball
    Vector2 speed;
    bool active;
    float radius;
};

// ----------------------------------------------------------------------------
//  Function declarations
// ----------------------------------------------------------------------------
void InitializePlayer(Player& player);
void ResetEnemies(Enemy enemies[], int level);
void ResetFireballs(Fireball fireballs[]);
void SpawnFireball(Fireball fireballs[], const Enemy& enemy, const Player& player);
void UpdateSpell(Spell& spell, Enemy enemies[]);
Question GenerateQuestion(int level);

bool DrawButtonWithPop(Rectangle rect, const char* text, Sound buttonSound);
void DrawMenu(const Texture2D& background, Sound clickSound, GameState& state);
void DrawGame(const Player& player, const Enemy enemies[], const Texture2D& wizardTex,
              const Texture2D& slimeTex, const Texture2D& background,
              const Spell& spell, const Fireball fireballs[]);
void DrawQuestion(const Question& q, const string& input, float timeRemaining,
                  float feedbackTimer, bool lastAnswerCorrect);
void DrawVictory(const Texture2D& background, int level);
void DrawGameOver(const Texture2D& background, const Player& player);
void DrawInstructions(const Texture2D& background);
void DrawCredits(const Texture2D& background);
void DrawSettings(const Texture2D& background, float musicVol, float sfxVol);

// ----------------------------------------------------------------------------
//  Main
// ----------------------------------------------------------------------------
int main() {
    srand((unsigned int)time(0));
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Spellculator");
    SetTargetFPS(60);
    InitAudioDevice();

    // Make "assets/..." resolve next to the executable, wherever the game is launched from
    ChangeDirectory(GetApplicationDirectory());

    // --- Audio ---
    Music bgm = LoadMusicStream("assets/bg_music.mp3");
    Sound hoverSound = LoadSound("assets/button.ogg");
    bgm.looping = true;
    PlayMusicStream(bgm);
    float musicVolume = 0.5f;
    float sfxVolume = 0.5f;

    // --- Sprites ---
    Texture2D wizardTexture = LoadTexture("assets/wizard.png");
    Texture2D slimeTexture = LoadTexture("assets/slime.png");

    // --- Backgrounds (shared where screens look the same) ---
    Texture2D menuBackground = LoadTexture("assets/menu_bg.png");
    Texture2D playingBackground = LoadTexture("assets/playing_bg.png");
    Texture2D gameoverBackground = LoadTexture("assets/gameover_bg.png");  // also used for victory
    Texture2D infoBackground = LoadTexture("assets/instructions_bg.png");  // instructions, credits, settings

    // --- Game objects ---
    GameState gamestate = MENU;
    int level = 1;

    Player player;
    InitializePlayer(player);

    Enemy enemies[ENEMY_COUNT];
    ResetEnemies(enemies, level);
    int enemyIndex = -1;  // enemy currently being fought (-1 = none)

    Question currentQuestion = { 0, 0, '+', 0 };

    Spell wizardSpell;
    wizardSpell.active = false;
    wizardSpell.targetEnemyIndex = -1;
    wizardSpell.speed = 10.0f;
    wizardSpell.position = { 0, 0 };

    Fireball fireballs[MAX_FIREBALLS];
    ResetFireballs(fireballs);
    float fireTimer = 0.0f;

    string input = "";
    float timeRemaining = QUESTION_TIME;
    float feedbackTimer = 0.0f;
    bool lastAnswerCorrect = false;

    // ------------------------------------------------------------------------
    //  Game loop
    // ------------------------------------------------------------------------
    while (!WindowShouldClose()) {
        UpdateMusicStream(bgm);
        SetMusicVolume(bgm, musicVolume);
        SetSoundVolume(hoverSound, sfxVolume);
        float deltaTime = GetFrameTime();

        // ---------------------------- UPDATE ------------------------------
        switch (gamestate) {

        case MENU:
            // Buttons are handled inside DrawMenu().
            break;

        case PLAYING: {
            // Wizard animation
            player.frameTimer += deltaTime;
            if (player.frameTimer >= 0.1f) {
                player.frameTimer = 0.0f;
                player.currentFrame = (player.currentFrame + 1) % WIZARD_FRAMES;
            }

            // Wizard movement (clamped to the screen)
            if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed;
            if (IsKeyDown(KEY_LEFT))  player.position.x -= player.speed;
            if (IsKeyDown(KEY_UP))    player.position.y -= player.speed;
            if (IsKeyDown(KEY_DOWN))  player.position.y += player.speed;
            if (player.position.x < 0) player.position.x = 0;
            if (player.position.x > WORLD_LIMIT) player.position.x = WORLD_LIMIT;
            if (player.position.y < 0) player.position.y = 0;
            if (player.position.y > WORLD_LIMIT) player.position.y = WORLD_LIMIT;

            // Enemies: animation, movement, and collision with the wizard
            for (int i = 0; i < ENEMY_COUNT; i++) {
                if (!enemies[i].active) continue;

                enemies[i].frameTimer += deltaTime;
                if (enemies[i].frameTimer >= 0.15f) {
                    enemies[i].frameTimer = 0.0f;
                    enemies[i].currentFrame = (enemies[i].currentFrame + 1) % SLIME_FRAMES;
                }

                // Patrol left/right, bouncing off the edges
                enemies[i].position.x += enemies[i].speed;
                if (enemies[i].position.x < 0) {
                    enemies[i].position.x = 0;
                    enemies[i].speed = abs(enemies[i].speed);
                }
                else if (enemies[i].position.x > WORLD_LIMIT) {
                    enemies[i].position.x = WORLD_LIMIT;
                    enemies[i].speed = -abs(enemies[i].speed);
                }

                Rectangle playerRec = { player.position.x + 15, player.position.y + 15, 40, 40 };
                Rectangle enemyRec = { enemies[i].position.x + 10, enemies[i].position.y + 10, 40, 40 };

                // Touching a slime starts a math battle
                if (!wizardSpell.active && CheckCollisionRecs(playerRec, enemyRec)) {
                    enemyIndex = i;
                    currentQuestion = GenerateQuestion(level);
                    input.clear();
                    timeRemaining = QUESTION_TIME;
                    feedbackTimer = 0.0f;
                    player.position.x -= 50;  // knockback
                    gamestate = QUESTION;
                    break;
                }
            }
            if (gamestate == QUESTION) break;  // battle started, skip the rest of this frame

            // Spell flying toward the defeated slime
            if (wizardSpell.active) UpdateSpell(wizardSpell, enemies);

            // Enemy fireballs: every active slime fires one per volley
            fireTimer += deltaTime;
            if (fireTimer >= FIRE_RATE) {
                fireTimer = 0.0f;
                for (int i = 0; i < ENEMY_COUNT; i++) {
                    if (enemies[i].active) SpawnFireball(fireballs, enemies[i], player);
                }
            }

            Vector2 playerCenter = { player.position.x + PLAYER_CENTER_OFFSET,
                                     player.position.y + PLAYER_CENTER_OFFSET };
            for (int i = 0; i < MAX_FIREBALLS; i++) {
                if (!fireballs[i].active) continue;

                fireballs[i].position.x += fireballs[i].speed.x;
                fireballs[i].position.y += fireballs[i].speed.y;

                if (CheckCollisionCircles(fireballs[i].position, fireballs[i].radius, playerCenter, 25)) {
                    player.health--;
                    fireballs[i].active = false;
                    if (player.health <= 0) {
                        gamestate = GAME_OVER;
                        break;
                    }
                }
                else if (fireballs[i].position.x < 0 || fireballs[i].position.x > SCREEN_WIDTH ||
                         fireballs[i].position.y < 0 || fireballs[i].position.y > SCREEN_HEIGHT) {
                    fireballs[i].active = false;
                }
            }

            // Level cleared once every slime is gone
            bool allDefeated = true;
            for (int i = 0; i < ENEMY_COUNT; i++) {
                if (enemies[i].active) allDefeated = false;
            }
            if (gamestate == PLAYING && allDefeated) gamestate = VICTORY;
            break;
        }

        case QUESTION: {
            // Showing "CORRECT!" / "WRONG!" feedback
            if (feedbackTimer > 0.0f) {
                feedbackTimer -= deltaTime;
                if (feedbackTimer <= 0.0f) {
                    gamestate = (player.health <= 0) ? GAME_OVER : PLAYING;
                }
                break;
            }

            // Countdown
            timeRemaining -= deltaTime;
            if (timeRemaining <= 0.0f) {
                player.health--;
                lastAnswerCorrect = false;
                feedbackTimer = 2.0f;
                break;
            }

            // Typing (digits only, max 5)
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= '0' && key <= '9' && input.length() < 5) input.push_back((char)key);
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !input.empty()) input.pop_back();

            // Submit answer
            if (IsKeyPressed(KEY_ENTER) && !input.empty()) {
                int userAnswer = stoi(input);
                if (userAnswer == currentQuestion.answer) {
                    player.score += 10;
                    lastAnswerCorrect = true;

                    // Launch the spell from the wizard's centre toward the slime
                    wizardSpell.active = true;
                    wizardSpell.targetEnemyIndex = enemyIndex;
                    wizardSpell.position = { player.position.x + PLAYER_CENTER_OFFSET,
                                             player.position.y + PLAYER_CENTER_OFFSET };
                }
                else {
                    player.health--;
                    lastAnswerCorrect = false;
                }
                feedbackTimer = 1.5f;
            }
            break;
        }

        case VICTORY:
            if (IsKeyPressed(KEY_ENTER)) {
                level++;
                ResetFireballs(fireballs);
                ResetEnemies(enemies, level);
                wizardSpell.active = false;
                player.position = { 100, 300 };
                gamestate = PLAYING;
            }
            break;

        case GAME_OVER:
            if (IsKeyPressed(KEY_R)) {
                InitializePlayer(player);
                level = 1;
                ResetFireballs(fireballs);
                ResetEnemies(enemies, level);
                wizardSpell.active = false;
                gamestate = MENU;
            }
            break;

        case INSTRUCTIONS:
        case CREDITS:
            if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ENTER)) gamestate = MENU;
            break;

        case SETTINGS:
            if (IsKeyPressed(KEY_RIGHT)) musicVolume += 0.1f;
            if (IsKeyPressed(KEY_LEFT))  musicVolume -= 0.1f;
            if (IsKeyPressed(KEY_UP))    sfxVolume += 0.1f;
            if (IsKeyPressed(KEY_DOWN))  sfxVolume -= 0.1f;
            if (musicVolume > 1.0f) musicVolume = 1.0f;
            if (musicVolume < 0.0f) musicVolume = 0.0f;
            if (sfxVolume > 1.0f) sfxVolume = 1.0f;
            if (sfxVolume < 0.0f) sfxVolume = 0.0f;
            if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ENTER)) gamestate = MENU;
            break;
        }

        // ----------------------------- DRAW -------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (gamestate) {
        case MENU:         DrawMenu(menuBackground, hoverSound, gamestate); break;
        case PLAYING:      DrawGame(player, enemies, wizardTexture, slimeTexture, playingBackground, wizardSpell, fireballs); break;
        case INSTRUCTIONS: DrawInstructions(infoBackground); break;
        case CREDITS:      DrawCredits(infoBackground); break;
        case QUESTION:     DrawQuestion(currentQuestion, input, timeRemaining, feedbackTimer, lastAnswerCorrect); break;
        case VICTORY:      DrawVictory(gameoverBackground, level); break;
        case GAME_OVER:    DrawGameOver(gameoverBackground, player); break;
        case SETTINGS:     DrawSettings(infoBackground, musicVolume, sfxVolume); break;
        }

        EndDrawing();
    }

    // --- Cleanup ---
    UnloadTexture(wizardTexture);
    UnloadTexture(slimeTexture);
    UnloadTexture(menuBackground);
    UnloadTexture(playingBackground);
    UnloadTexture(gameoverBackground);
    UnloadTexture(infoBackground);
    UnloadMusicStream(bgm);
    UnloadSound(hoverSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

// ============================================================================
//  Game logic helpers
// ============================================================================

void InitializePlayer(Player& player) {
    player.position = { 100, 267 };
    player.speed = 5;
    player.health = MAX_HEALTH;
    player.score = 0;
    player.currentFrame = 0;
    player.frameTimer = 0.0f;
}

// Places the slimes for a level. Higher levels make them faster.
void ResetEnemies(Enemy enemies[], int level) {
    for (int i = 0; i < ENEMY_COUNT; i++) {
        enemies[i].position = { 200.0f + i * 200.0f,
                                level == 1 ? 300.0f : (float)GetRandomValue(100, 700) };
        int speed = 1 + level + i;
        enemies[i].speed = (speed > 10) ? 10 : speed;  // cap so it stays playable
        enemies[i].active = true;
        enemies[i].currentFrame = 0;
        enemies[i].frameTimer = 0.0f;
    }
}

void ResetFireballs(Fireball fireballs[]) {
    for (int i = 0; i < MAX_FIREBALLS; i++) {
        fireballs[i].active = false;
        fireballs[i].position = { 0, 0 };
        fireballs[i].speed = { 0, 0 };
        fireballs[i].radius = 10.0f;
    }
}

// Fires one fireball from the enemy toward the wizard (uses the first free slot).
void SpawnFireball(Fireball fireballs[], const Enemy& enemy, const Player& player) {
    for (int j = 0; j < MAX_FIREBALLS; j++) {
        if (fireballs[j].active) continue;

        Vector2 start = { enemy.position.x + ENEMY_CENTER_OFFSET, enemy.position.y + ENEMY_CENTER_OFFSET };
        Vector2 target = { player.position.x + PLAYER_CENTER_OFFSET, player.position.y + PLAYER_CENTER_OFFSET };
        float dx = target.x - start.x;
        float dy = target.y - start.y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < 1.0f) return;  // avoids dividing by zero when overlapping

        fireballs[j].active = true;
        fireballs[j].position = start;
        fireballs[j].speed = { (dx / dist) * 4.0f, (dy / dist) * 4.0f };
        fireballs[j].radius = 10.0f;
        return;
    }
}

// Moves the spell straight toward its target and destroys the slime on contact.
void UpdateSpell(Spell& spell, Enemy enemies[]) {
    if (spell.targetEnemyIndex < 0 || spell.targetEnemyIndex >= ENEMY_COUNT) {
        spell.active = false;
        return;
    }

    Enemy& target = enemies[spell.targetEnemyIndex];
    Vector2 targetCenter = { target.position.x + ENEMY_CENTER_OFFSET, target.position.y + ENEMY_CENTER_OFFSET };
    float dx = targetCenter.x - spell.position.x;
    float dy = targetCenter.y - spell.position.y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist <= 30.0f) {  // hit
        target.active = false;
        spell.active = false;
        spell.targetEnemyIndex = -1;
    }
    else {
        spell.position.x += (dx / dist) * spell.speed;
        spell.position.y += (dy / dist) * spell.speed;
    }
}

// Difficulty by level:
//   1-2 : addition / subtraction
//   3   : simple algebra (X + a = b)
//   4+  : multiplication, division, and algebra mixed
Question GenerateQuestion(int level) {
    Question q = { 0, 0, '+', 0 };

    if (level <= 2) {
        q.op = (rand() % 2 == 0) ? '+' : '-';
        q.num1 = rand() % 20 + 5;
        q.num2 = rand() % 20 + 5;
        if (q.op == '-' && q.num1 < q.num2) swap(q.num1, q.num2);  // no negative answers
        q.answer = (q.op == '+') ? q.num1 + q.num2 : q.num1 - q.num2;
    }
    else if (level == 3) {
        q.op = '?';  // X + num1 = num2
        int hiddenX = rand() % 15 + 2;
        q.num1 = rand() % 20 + 5;
        q.num2 = hiddenX + q.num1;
        q.answer = hiddenX;
    }
    else {
        int choice = rand() % 3;
        if (choice == 0) {
            q.op = '*';
            q.num1 = rand() % (level + 8) + 2;  // +2 so we never multiply by 0 or 1
            q.num2 = rand() % 10 + 2;
            q.answer = q.num1 * q.num2;
        }
        else if (choice == 1) {
            q.op = '/';
            q.num2 = rand() % 10 + 2;
            int quotient = rand() % 12 + 2;
            q.num1 = q.num2 * quotient;  // guarantees a whole-number answer
            q.answer = quotient;
        }
        else {
            q.op = '?';
            int hiddenX = rand() % 30 + 10;
            q.num1 = rand() % 30 + 5;
            q.num2 = hiddenX + q.num1;
            q.answer = hiddenX;
        }
    }
    return q;
}

// ============================================================================
//  Drawing / UI
// ============================================================================

// Draws a button that "pops" on hover. Returns true when it is clicked.
bool DrawButtonWithPop(Rectangle rect, const char* text, Sound buttonSound) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    bool clicked = false;

    static const char* lastHoveredButton = nullptr;
    float scale = 1.0f;

    if (hover) {
        if (lastHoveredButton != text) {  // play the sound once per hover
            PlaySound(buttonSound);
            lastHoveredButton = text;
        }
        scale = 1.1f;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) scale = 0.9f;
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) clicked = true;
    }
    else if (lastHoveredButton == text) {
        lastHoveredButton = nullptr;
    }

    Rectangle pop = {
        rect.x - (rect.width * scale - rect.width) * 0.5f,
        rect.y - (rect.height * scale - rect.height) * 0.5f,
        rect.width * scale,
        rect.height * scale
    };

    DrawRectangleRec(pop, hover ? spiritLavender : arcanePurple);
    DrawRectangleLinesEx(pop, 3, hover ? WHITE : wizardGold);

    int fontSize = (int)(25 * scale);
    int textWidth = MeasureText(text, fontSize);
    DrawText(text,
             (int)(pop.x + (pop.width - textWidth) * 0.5f),
             (int)(pop.y + (pop.height - fontSize) * 0.5f),
             fontSize, hover ? YELLOW : WHITE);

    return clicked;
}

void DrawMenu(const Texture2D& background, Sound clickSound, GameState& state) {
    DrawTexture(background, 0, 0, WHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.4f));
    DrawText("SPELLCULATOR", (SCREEN_WIDTH - MeasureText("SPELLCULATOR", 60)) / 2, 100, 60, wizardGold);

    if (DrawButtonWithPop({ 250, 290, 300, 60 }, "START GAME", clickSound))   state = PLAYING;
    if (DrawButtonWithPop({ 250, 370, 300, 60 }, "INSTRUCTIONS", clickSound)) state = INSTRUCTIONS;
    if (DrawButtonWithPop({ 250, 450, 300, 60 }, "CREDITS", clickSound))      state = CREDITS;
    if (DrawButtonWithPop({ 250, 530, 300, 60 }, "AUDIO SETTINGS", clickSound)) state = SETTINGS;
}

void DrawGame(const Player& player, const Enemy enemies[], const Texture2D& wizardTex,
              const Texture2D& slimeTex, const Texture2D& background,
              const Spell& spell, const Fireball fireballs[]) {
    DrawTexture(background, 0, 0, WHITE);
    Vector2 origin = { 0, 0 };

    // Wizard
    int wizardFrameWidth = wizardTex.width / WIZARD_FRAMES;
    int wizardFrameHeight = wizardTex.height;
    Rectangle wizardSource = { (float)(player.currentFrame * wizardFrameWidth), 0,
                               (float)wizardFrameWidth, (float)wizardFrameHeight };
    Rectangle wizardDest = { player.position.x, player.position.y,
                             wizardFrameWidth * 2.0f, wizardFrameHeight * 2.0f };
    DrawTexturePro(wizardTex, wizardSource, wizardDest, origin, 0.0f, WHITE);

    // Slimes
    int slimeFrameWidth = slimeTex.width / SLIME_COLS;
    int slimeFrameHeight = slimeTex.height / SLIME_ROWS;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (!enemies[i].active) continue;
        int frameX = enemies[i].currentFrame % SLIME_COLS;
        int frameY = enemies[i].currentFrame / SLIME_COLS;
        Rectangle slimeSource = { (float)(frameX * slimeFrameWidth), (float)(frameY * slimeFrameHeight),
                                  (float)slimeFrameWidth, (float)slimeFrameHeight };
        Rectangle slimeDest = { enemies[i].position.x, enemies[i].position.y,
                                slimeFrameWidth * 2.5f, slimeFrameHeight * 2.5f };
        DrawTexturePro(slimeTex, slimeSource, slimeDest, origin, 0.0f, WHITE);
    }

    // Wizard's spell
    if (spell.active) DrawCircleV(spell.position, 10, SKYBLUE);

    // Enemy fireballs
    for (int i = 0; i < MAX_FIREBALLS; i++) {
        if (!fireballs[i].active) continue;
        DrawCircleGradient((int)fireballs[i].position.x, (int)fireballs[i].position.y,
                           fireballs[i].radius + 5, ORANGE, Fade(RED, 0.0f));
        DrawCircleV(fireballs[i].position, fireballs[i].radius, RED);
    }

    // HUD: health bar (48 px per heart) and score
    DrawRectangle(20, 20, 150, 25, BLACK);
    DrawRectangle(22, 22, player.health * 48, 21, RED);
    DrawText("LIFE", 75, 23, 18, WHITE);
    DrawText(TextFormat("Score: %d", player.score), 20, 50, 20, GREEN);
}

void DrawQuestion(const Question& q, const string& input, float timeRemaining,
                  float feedbackTimer, bool lastAnswerCorrect) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
    DrawRectangle(200, 100, 400, 300, LIGHTGRAY);
    DrawRectangleLines(200, 100, 400, 300, BLACK);
    DrawText("MATH CHALLENGE!", 270, 120, 25, DARKPURPLE);

    if (q.op == '?') {
        DrawText("Solve for X!", 330, 150, 20, RED);
        DrawText(TextFormat("X + %d = %d", q.num1, q.num2), 300, 180, 35, BLACK);
    }
    else {
        DrawText(TextFormat("%d %c %d = ?", q.num1, q.op, q.num2), 300, 180, 35, BLACK);
    }

    // Answer box
    DrawRectangle(300, 240, 200, 50, WHITE);
    DrawRectangleLines(300, 240, 200, 50, BLACK);
    DrawText(input.c_str(), 320, 250, 30, BLUE);

    // Timer turns red in the last 5 seconds
    Color timerColor = (timeRemaining > 5.0f) ? GREEN : RED;
    DrawText(TextFormat("Time: %.1f", timeRemaining > 0 ? timeRemaining : 0.0f), 330, 320, 25, timerColor);
    DrawText("Press ENTER to submit", 270, 360, 18, DARKGRAY);

    if (feedbackTimer > 0.0f) {
        DrawRectangle(200, 420, 400, 80, Fade(BLACK, 0.6f));
        if (lastAnswerCorrect) DrawText("CORRECT!", 320, 440, 40, GREEN);
        else                   DrawText("WRONG!", 335, 440, 40, RED);
    }
}

void DrawVictory(const Texture2D& background, int level) {
    DrawTexture(background, 0, 0, WHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(dullForest, 0.3f));
    DrawText("LEVEL CLEAR!", 250, 250, 45, GREEN);
    DrawText(TextFormat("You finished Level %d!", level), 270, 320, 22, DARKGRAY);
    DrawText("Press ENTER for the Next Level", 230, 400, 22, BLACK);
}

void DrawGameOver(const Texture2D& background, const Player& player) {
    DrawTexture(background, 0, 0, WHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(DARKBLUE, 0.3f));
    DrawText("GAME OVER", 280, 150, 45, RED);
    DrawText(TextFormat("Final Score: %d", player.score), 300, 220, 25, WHITE);
    DrawText("Press R to Restart", 280, 280, 22, DARKGRAY);
}

void DrawSettings(const Texture2D& background, float musicVol, float sfxVol) {
    DrawTexture(background, 0, 0, WHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(DARKBROWN, 0.2f));
    DrawText("AUDIO SETTINGS", 230, 150, 40, GOLD);

    DrawText("Music Volume (Left/Right Arrows)", 200, 280, 20, WHITE);
    DrawRectangle(200, 310, 400, 20, DARKGRAY);
    DrawRectangle(200, 310, (int)(400 * musicVol), 20, wizardGold);
    DrawText(TextFormat("%d%%", (int)roundf(musicVol * 100)), 620, 310, 20, WHITE);

    DrawText("SFX Volume (Up/Down Arrows)", 200, 400, 20, WHITE);
    DrawRectangle(200, 430, 400, 20, DARKGRAY);
    DrawRectangle(200, 430, (int)(400 * sfxVol), 20, spiritLavender);
    DrawText(TextFormat("%d%%", (int)roundf(sfxVol * 100)), 620, 430, 20, WHITE);

    DrawText("Press ENTER or BACKSPACE to return", 220, 600, 20, GRAY);
}

void DrawInstructions(const Texture2D& background) {
    DrawTexture(background, 0, 0, WHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(DARKGRAY, 0.3f));
    DrawText("HOW TO PLAY", 210, 100, 50, GOLD);
    DrawText("- Move the Wizard using ARROW KEYS", 180, 250, 20, WHITE);
    DrawText("- Avoid FIREBALLS spit by the slimes", 180, 300, 20, WHITE);
    DrawText("- Touch a Slime to trigger a MATH BATTLE", 180, 350, 20, WHITE);
    DrawText("- Type the answer and press ENTER", 180, 400, 20, WHITE);
    DrawText("- Clear all slimes to progress", 180, 450, 20, WHITE);
    DrawText("PRESS BACKSPACE / ENTER TO RETURN", 180, 680, 20, GRAY);
}

void DrawCredits(const Texture2D& background) {
    DrawTexture(background, 0, 0, WHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.3f));
    DrawText("CREDITS", 300, 120, 40, GOLD);
    DrawText("Game Design & Coding: HANIA QAISAR", 190, 300, 25, WHITE);
    DrawText("Graphics: Custom Sprites", 260, 350, 25, WHITE);
    DrawText("Powered by: C++ & Raylib", 250, 400, 25, RAYWHITE);
    DrawText("PRESS BACKSPACE / ENTER TO RETURN", 200, 680, 20, GRAY);
}
