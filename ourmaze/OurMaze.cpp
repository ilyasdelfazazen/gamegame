#include "raylib.h"
#include <vector>
#include <stack>
#include <cstdlib>
#include <ctime>
#include <algorithm>
// Constants for window size
const int screenWidth = 800;
const int screenHeight = 600;

//  The variable that controls the state of the game (main list, difficulty selection, or game)
enum GameState { MENU, DIFFICULTY_SELECTION, GAME, NOMBRE_SELECTION, HELP, DEVS };
GameState currentState = MENU;

bool isMultiplayer = false; // Default is single-player mode
// Declare pictures
Texture2D HelpBut;
Texture2D ExitBut;
Texture2D DevsBut;
Texture2D BackBut;
Texture2D startbut;
Texture2D hardbut;
Texture2D easybut;
Texture2D normalbut;
Texture2D restart;
Texture2D scorebut;
Texture2D PLAYERIMAGE;
Texture2D goalTexture;
Texture2D background;
int mazeCols = 20; // Default maze columns
int mazeRows = 15; // Default maze rows
int cellSize = 40; // Default cell size
Sound buttonSound; // Sounds declaration
Sound winnerSound;
Sound bgSound;
Sound hardSound;
Sound PantherSound;
// Cell structure for maze generation
struct Cell {
    int x, y;
    bool visited = false;
    bool walls[4] = { true, true, true, true }; // top, right, bottom, left
};

// Maze class
class Maze {
public:

    Color wallColor; // Variable for change of wall color
    Maze() : wallColor(GREEN) { // default color initialization
        GenerateMaze();
    }
    void GenerateMaze() {
        grid.clear();
        grid.resize(mazeCols * mazeRows);

        // Initialize grid
        for (int y = 0; y < mazeRows; ++y) {
            for (int x = 0; x < mazeCols; ++x) {
                grid[y * mazeCols + x] = { x, y };
            }
        }

        // DFS maze generation
        std::stack<Cell*> stack;
        Cell* current = &grid[0];
        current->visited = true;

        while (true) {
            Cell* next = GetUnvisitedNeighbor(*current);
            if (next) {
                stack.push(current);
                RemoveWalls(*current, *next);
                current = next;
                current->visited = true;
            }
            else if (!stack.empty()) {
                current = stack.top();
                stack.pop();
            }
            else {
                break;
            }
        }
    }

    void DrawMaze() const {

        for (const Cell& cell : grid) {
            int x = cell.x * cellSize;
            int y = cell.y * cellSize;

            if (cell.walls[0]) DrawLine(x, y, x + cellSize, y, wallColor);           // Top
            if (cell.walls[1]) DrawLine(x + cellSize, y, x + cellSize, y + cellSize, wallColor); // Right
            if (cell.walls[2]) DrawLine(x, y + cellSize, x + cellSize, y + cellSize, wallColor); // Bottom
            if (cell.walls[3]) DrawLine(x, y, x, y + cellSize, wallColor);           // Left
        }
    }

    bool IsWall(int x, int y, int dir) const {
        int index = y * mazeCols + x;
        if (index < 0 || index >= (int)grid.size()) return true;
        return grid[index].walls[dir];
    }

private:
    std::vector<Cell> grid;

    Cell* GetUnvisitedNeighbor(const Cell& cell) {
        std::vector<Cell*> neighbors;

        int dx[] = { 0, 1, 0, -1 };
        int dy[] = { -1, 0, 1, 0 };

        for (int i = 0; i < 4; ++i) {
            int nx = cell.x + dx[i];
            int ny = cell.y + dy[i];

            if (nx >= 0 && nx < mazeCols && ny >= 0 && ny < mazeRows) {
                Cell* neighbor = &grid[ny * mazeCols + nx];
                if (!neighbor->visited) {
                    neighbors.push_back(neighbor);
                }
            }
        }

        if (!neighbors.empty()) {
            return neighbors[std::rand() % neighbors.size()];
        }
        return nullptr;
    }

    void RemoveWalls(Cell& a, Cell& b) {
        int dx = b.x - a.x;
        int dy = b.y - a.y;

        if (dx == 1) { a.walls[1] = false; b.walls[3] = false; }
        if (dx == -1) { a.walls[3] = false; b.walls[1] = false; }
        if (dy == 1) { a.walls[2] = false; b.walls[0] = false; }
        if (dy == -1) { a.walls[0] = false; b.walls[2] = false; }
    }
};

// Player class
class Player {
public:
    Player() : posX(0), posY(0) {}
    Texture2D PLAYERIMAGE = LoadTexture("assets/PLAYER.png");
    Texture2D goalTexture = LoadTexture("assets/CHEESE_IM.png");
    void SetPosition(int x, int y) {
        posX = x;
        posY = y;
    }

    void Update(const Maze& maze, bool isPlayer2 = false, float deltaTime = 0.016f) {
        static float moveCooldown = 0.1f; // Minimum time (in seconds) between moves
        static float timeSinceLastMove = 0.0f;

        // Update the time since the last move
        timeSinceLastMove += deltaTime;

        // If not enough time has passed, do nothing
        if (timeSinceLastMove < moveCooldown) {
            return;
        }
        int newX = posX;
        int newY = posY;

        if (isPlayer2) { // Controls for player 2
            PLAYERIMAGE = LoadTexture("assets/oo_PLAYER.png");
            if (IsKeyDown(KEY_W) && !maze.IsWall(posX, posY, 0)) newY--;
            if (IsKeyDown(KEY_D) && !maze.IsWall(posX, posY, 1)) newX++;
            if (IsKeyDown(KEY_S) && !maze.IsWall(posX, posY, 2)) newY++;
            if (IsKeyDown(KEY_A) && !maze.IsWall(posX, posY, 3)) newX--;
        }
        else { // Default controls
            if (IsKeyDown(KEY_UP) && !maze.IsWall(posX, posY, 0)) newY--;
            if (IsKeyDown(KEY_RIGHT) && !maze.IsWall(posX, posY, 1)) newX++;
            if (IsKeyDown(KEY_DOWN) && !maze.IsWall(posX, posY, 2)) newY++;
            if (IsKeyDown(KEY_LEFT) && !maze.IsWall(posX, posY, 3)) newX--;
        }

        if (newX >= 0 && newX < mazeCols && newY >= 0 && newY < mazeRows) {
            posX = newX;
            posY = newY;
            timeSinceLastMove = 0.0f;
        }
    }

    void Draw() const {
        DrawTexture(PLAYERIMAGE, posX * cellSize, posY * cellSize, WHITE);
    }

    void DrawGoal() const {
        int goalX = (mazeCols - 1) * cellSize + cellSize / 2;
        int goalY = (mazeRows - 1) * cellSize + cellSize / 2;
        DrawTexture(goalTexture, goalX - goalTexture.width / 2, goalY - goalTexture.height / 2, WHITE);

    }

    bool HasReachedGoal() const {
        return posX == mazeCols - 1 && posY == mazeRows - 1;
    }

private:
    int posX, posY;

};
//Level class
class Level {
public:
    int mazeCols;
    int mazeRows;
    int cellSize;
    int difficulty;
    Color wallColor;

    Level(int diff) : difficulty(diff), mazeCols(0), mazeRows(0), cellSize(0), wallColor(GREEN) {
        ConfigureLevel();
    }

    void ConfigureLevel() {
        // Set maze dimensions and color based on difficulty
        switch (difficulty) {
        case 0: // Easy
            mazeCols = 12;
            mazeRows = 8;
            cellSize = 67;
            wallColor = GREEN;
            break;
        case 1: // Medium
            mazeCols = 20;
            mazeRows = 13;
            cellSize = 40;
            wallColor = YELLOW;
            break;
        case 2: // Hard
            mazeCols = 25;
            mazeRows = 17;
            cellSize = 32;
            wallColor = RED;
            break;
        }
    }

    void ApplySettings(Game& game) {
        // Update the game settings with the current level's configuration
        game.UpdateMazeWallColor(wallColor); // Set the maze wall color
        game = Game();                      // Reset the game for the level
        game.UpdateMazeWallColor(wallColor); // Reapply the wall color
    }

    void Run(Game& game) {
        ApplySettings(game);
        game.Run();
    }
};

// Game class
class Game {
public:
    Game() : level(1), gameWon(false), timer(0), winnerSoundPlayed(false),

        player(), player2() {
    }  // Timer starts at 0

    void UpdateMazeWallColor(Color newColor) {
        maze.wallColor = newColor;
    }

    void Run() {
        Reset();
        //InitWindow(screenWidth, screenHeight, "Maze Game");
        SetTargetFPS(60);

        while (!WindowShouldClose() && !exitToMenu) {
            Update();
            Draw();
        }

        CloseWindow();
    }
    void Reset() {
        if (gameWon) {
            scores.push_back(timer); // Save the score
            std::sort(scores.begin(), scores.end());
            PlaySound(bgSound);
        }

        maze.GenerateMaze();
        player = Player();
        player2 = Player(); // Reset player 2
        if (isMultiplayer) {

            player2.SetPosition(0, mazeRows - 1); // Set Player 2 to (0, mazeRows-1)
        }

        gameWon = false;
        winnerSoundPlayed = false; // Reinitialization

        timer = 0;
    }

private:
    std::vector<float> scores; // Store scores
    bool showScores = false;   // Toggle scores table
    bool exitToMenu = false; // Add a flag to indicate "Retour" button click
    bool isMusicPlaying = true; // music running by default

    Maze maze;
    Player player, player2; // Second player for multiplayer

    int level;
    bool gameWon;
    float timer;  // Timer (as seconds)
    bool winnerSoundPlayed;   // To track the playback of the sound

    void Update() {
        if (IsKeyPressed(KEY_R)) Reset();

        if (!gameWon) {
            timer += GetFrameTime();
            player.Update(maze);
            if (isMultiplayer) player2.Update(maze, true); // Update player 2 in multiplayer mode


            if (player.HasReachedGoal() || (isMultiplayer && player2.HasReachedGoal())) {
                gameWon = true;
            }

        }
        UpdateMusicControl();


        if (gameWon && !winnerSoundPlayed) { // Turn on the sound once you win
            PlaySound(winnerSound);
            StopSound(bgSound);
            StopSound(hardSound);
            StopSound(PantherSound);
            winnerSoundPlayed = true;
        }

        // Check for "Scores" button click
        Rectangle scoresButton = { screenWidth - 150, screenHeight - 40, 100, 30 };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), scoresButton)) {
            PlaySound(buttonSound);

            showScores = !showScores; // Toggle scores table
        }
        // Check for "RESET" button click
        Rectangle resetButton = { screenWidth - 580, screenHeight - 40, 100, 30 };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), resetButton)) {
            PlaySound(buttonSound);

            Reset();

        }
        // Check for "Retour" button click
        Rectangle retourButton = { screenWidth - 350, screenHeight - 40, 100, 30 };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), retourButton)) {
            PlaySound(buttonSound);
            StopSound(hardSound);
            PlaySound(bgSound);

            currentState = NOMBRE_SELECTION;
            exitToMenu = true; // Set flag to exit game

            return;
        }

    }

    void Draw() const {
        BeginDrawing();
        ClearBackground(BLACK);

        maze.DrawMaze();
        player.Draw();
        player.DrawGoal();
        if (isMultiplayer) {
            player2.Draw();
            player2.DrawGoal(); // Draw goal for player 2 // Draw goal for both players
        }

        // Display the timer at the bottom of the screen
        DrawText(TextFormat("Time: %.2f", timer), screenWidth / 2 - MeasureText(TextFormat("Time: %.2f", timer), 20) / 2, screenHeight - 30, 20, WHITE);
        // Display the "Scores" button
        Rectangle scoresButton = { screenWidth - 160, screenHeight - 40, 100, 30 };
        DrawRectangleRec(scoresButton, BLUE);

        DrawText("Scores", scoresButton.x + 10, scoresButton.y + 5, 20, WHITE);

        // Display the "RESET" button
        Rectangle resetButton = { screenWidth - 580, screenHeight - 40, 100, 30 };
        DrawRectangleRec(resetButton, BLUE);

        DrawText("Reset", resetButton.x + 10, resetButton.y + 5, 20, WHITE);

        // Display the "RETOUR" button
        Rectangle retourButton = { screenWidth - 300, screenHeight - 40, 100, 30 };
        DrawRectangleRec(retourButton, BLUE);

        DrawText("Back", retourButton.x + 10, retourButton.y + 5, 20, WHITE);

        // Display the scores table if toggled
        if (showScores) {

            DrawRectangle(screenWidth / 4, screenHeight / 4, screenWidth / 2, screenHeight / 2, DARKGRAY);
            DrawText("Scores Table", screenWidth / 2 - 60, screenHeight / 4 + 10, 20, WHITE);

            for (size_t i = 0; i < scores.size(); ++i) {
                DrawText(TextFormat("%d. %.2f", i + 1, scores[i]),
                    screenWidth / 4 + 20,
                    screenHeight / 4 + 50 + i * 20,
                    18,
                    WHITE);
            }
        }
        if (gameWon) {
            if (isMultiplayer) {
                if (player.HasReachedGoal()) {
                    DrawText("Player 1 won!", screenWidth / 2 - 150, screenHeight / 2, 40, GREEN);
                }
                else if (player2.HasReachedGoal()) {

                    DrawText("Player 2 won!", screenWidth / 2 - 150, screenHeight / 2, 40, GREEN);
                }
            }
            else {
                DrawText("You won the game!", screenWidth / 2 - 180, screenHeight / 2, 40, GREEN);
            }
        }
        DrawMusicControl();

        EndDrawing();
    }
    //Update the state of music
    void UpdateMusicControl() {
        Rectangle musicButton = { screenWidth - 760, 560, 145, 30 };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), musicButton)) {
            if (isMusicPlaying) {
                PauseSound(bgSound);// Temporarily stop music
                PauseSound(PantherSound);
                PauseSound(hardSound);
            }
            else {
                ResumeSound(buttonSound);// Resume 
            }
            isMusicPlaying = !isMusicPlaying; // Reverse the situation
            if (isMusicPlaying) {
                if (!IsSoundPlaying(bgSound)) ResumeSound(bgSound);
                if (!IsSoundPlaying(PantherSound)) ResumeSound(PantherSound);
                if (!IsSoundPlaying(hardSound)) ResumeSound(hardSound);

            }
        }
    }
    //Draw nusic control button

    void DrawMusicControl() const {
        Rectangle musicButton = { screenWidth - 760, 560, 145, 30 };
        DrawRectangleRec(musicButton, BLUE);
        DrawText(isMusicPlaying ? "Music Pause" : "Music Play", musicButton.x + 10, musicButton.y + 5, 20, WHITE);
    }

};

// Main function with difficulty selection
int main() {
    InitWindow(screenWidth, screenHeight, "Maze Game");
    InitAudioDevice(); // Configure the audio system here

    // Loading the game icon
    Image icon = LoadImage("assets/LOGO1.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    // Loading  textures
    Texture2D background = LoadTexture("assets/HOMEPAGE.png");

    Texture2D background2 = LoadTexture("assets/BG3.png");
    Texture2D mazeName = LoadTexture("assets/MAZE_GAME.png");
    Texture2D CHOOSE = LoadTexture("assets/CHOOSE.png");

    Texture2D CHOOSE_M = LoadTexture("assets/CHOOSE_M.png");
    Texture2D MULTI = LoadTexture("assets/MULTI.png");
    Texture2D SINGLE = LoadTexture("assets/SINGLE.png");

    // Load the button texture
    normalbut = LoadTexture("assets/medumbut.png");
    startbut = LoadTexture("assets/Start_Game.png");

    HelpBut = LoadTexture("assets/HELP.png");
    ExitBut = LoadTexture("assets/EXIT.png");
    DevsBut = LoadTexture("assets/DEVS.png");
    BackBut = LoadTexture("assets/BACK.png");


    hardbut = LoadTexture("assets/hardbut.png");
    easybut = LoadTexture("assets/easybut.png");
    restart = LoadTexture("assets/easybut.png");
    scorebut = LoadTexture("assets/SCORES_BUTTON.png");

    // Loading the sound effects
    buttonSound = LoadSound("assets/buttib_click.mp3");
    bgSound = LoadSound("assets/game-music.mp3");
    winnerSound = LoadSound("assets/winner.mp3");
    hardSound = LoadSound("assets/hard.mp3");
    PantherSound = LoadSound("assets/Pink_Panther.mp3");
    PlaySound(bgSound);
    bool isGameRunning = false;
    int difficulty = 0; // 0 = easy

    Game game; // Game object

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (currentState == MENU) {
            // If we are in the menu list
            BeginDrawing();
            DrawTexture(background, 0, 0, RAYWHITE);
            DrawTexture(mazeName, screenWidth / 2 - mazeName.width / 2, 120, WHITE);

            //Drawing play button
            Rectangle playButton = { screenWidth / 2.0f - 140, screenHeight / 2.0f - 45, 344, 74 };
            DrawTexture(startbut, playButton.x, playButton.y, WHITE);
            DrawText("", playButton.x + 35, playButton.y + 10, 50, WHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), playButton)) {
                PlaySound(buttonSound);
                currentState = NOMBRE_SELECTION;
            }
            //Help button
            Rectangle helpButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 50, 200, 50 };
            DrawTexture(HelpBut, helpButton.x, helpButton.y, WHITE);
            DrawText("", helpButton.x + 50, helpButton.y + 10, 30, RAYWHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), helpButton)) {
                PlaySound(buttonSound);
                currentState = HELP;
            }
            //Devs Button
            Rectangle devsButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 125, 200, 50 }; // Adjust position and size as needed
            DrawTexture(DevsBut, devsButton.x, devsButton.y, WHITE);
            DrawText("", devsButton.x + 50, devsButton.y + 10, 30, RAYWHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), devsButton)) {
                currentState = DEVS;
            }
            // Exit button
            Rectangle exitButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 200, 200, 50 };
            DrawTexture(ExitBut, exitButton.x, exitButton.y, WHITE);

            DrawText("", exitButton.x + 50, exitButton.y + 10, 30, RAYWHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), exitButton)) {
                PlaySound(buttonSound);
                CloseWindow(); // Exit the game
                break;
            }

            EndDrawing();
        }
        else if (currentState == HELP) {
            BeginDrawing();
            DrawTexture(background, 0, 0, RAYWHITE);

            ClearBackground(RAYWHITE);

            DrawText("HOW TO PLAY", screenWidth / 2 - MeasureText("HOW TO PLAY", 50) / 2, 70, 60, RED);

            // Instructions
            DrawText("-Use arrow keys to move the player.", 30, 140, 40, WHITE);
            DrawText("-Reach the red circle to win.", 30, 200, 40, WHITE);
            DrawText("-Press 'R' to reset the game.", 30, 260, 40, WHITE);
            DrawText("-Click 'Scores' to view your best", 30, 320, 40, WHITE);
            DrawText(" times", 30, 380, 40, WHITE);
            DrawText("-In multiplayer mode, Player 2 uses", 30, 440, 40, WHITE);
            DrawText(" W/A/S/D keys.", 30, 500, 40, WHITE);

            // Back Button
            Rectangle backButton = { screenWidth / 2.0f - 100, screenHeight - 50, 200, 50 };
            DrawTexture(BackBut, backButton.x, backButton.y, WHITE);
            DrawText("", backButton.x + 20, backButton.y + 10, 20, RAYWHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), backButton)) {
                PlaySound(buttonSound);
                StopSound(PantherSound);
                currentState = MENU;
            }

            EndDrawing();
        }
        else if (currentState == DEVS) {
            BeginDrawing();

            DrawTexture(background, 0, 0, RAYWHITE);
            ClearBackground(RAYWHITE);

            DrawText("DEVELOPERS", 200, 150, 60, RED); // Title
            DrawText("El Kajdouhi Mohamed Ayman", 100, 200, 40, WHITE);
            DrawText("Akdi Fouad", 100, 250, 40, WHITE);
            DrawText("Afazaz Ilyas", 100, 300, 40, WHITE);
            DrawText("Rabih Senhaji Anas", 100, 350, 40, WHITE);
            // Back button
            Rectangle backButton = { screenWidth / 2.0f - 100, screenHeight - 100, 200, 50 };
            DrawTexture(BackBut, backButton.x, backButton.y, WHITE);
            DrawText("", backButton.x + 20, backButton.y + 10, 20, RAYWHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), backButton)) {
                currentState = MENU;
            }
            EndDrawing();
        }
        else if (currentState == NOMBRE_SELECTION) {
            BeginDrawing();

            DrawTexture(background, 0, 0, RAYWHITE);

            DrawTexture(CHOOSE_M, screenWidth / 2 - CHOOSE_M.width / 2, 125, RAYWHITE);
            // Clearing the background
            ClearBackground(RED);
            Rectangle backButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 125, 200, 50 };
            DrawTexture(BackBut, backButton.x, backButton.y, WHITE);
            DrawText("", backButton.x + 40, backButton.y + 10, 30, RAYWHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), backButton)) {
                currentState = MENU;
            }
            // Single button 
            Rectangle SINGLEButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f - 25, 200, 50 };
            // Change "PLAY" to "SINGLE PLAYER"
            DrawTexture(SINGLE, SINGLEButton.x, SINGLEButton.y, WHITE);
            Rectangle multiplayerButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 50, 200, 50 };
            DrawTexture(MULTI, multiplayerButton.x, multiplayerButton.y, WHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), multiplayerButton)) {
                PlaySound(buttonSound);
                isMultiplayer = true; // Flag for multiplayer mode
                //game.Reset();
                currentState = DIFFICULTY_SELECTION;
            }



            else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), SINGLEButton)) {
                PlaySound(buttonSound);
                isMultiplayer = false;
                game.Reset();
                currentState = DIFFICULTY_SELECTION;
            }

            EndDrawing();

        }
        else if (currentState == DIFFICULTY_SELECTION) {
            //  choosing the level of difficulty
            BeginDrawing();
            DrawTexture(background, 0, 0, RAYWHITE);
            DrawTexture(CHOOSE, screenWidth / 2 - CHOOSE.width / 2, 85, RAYWHITE);

            // Clearing background
            ClearBackground(RED);
            StopSound(PantherSound);
            Rectangle backButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 165, 200, 50 };
            DrawTexture(BackBut, backButton.x, backButton.y, WHITE);

            DrawText("", backButton.x + 40, backButton.y + 10, 30, RAYWHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), backButton)) {
                PlaySound(buttonSound);

                currentState = NOMBRE_SELECTION;
            }    // Level buttons area
            Rectangle easyButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f - 65, 200, 50 };
            Rectangle mediumButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 15, 200, 50 };
            Rectangle hardButton = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 85, 200, 50 };

            // Drawing buttons
            DrawTexture(easybut, easyButton.x, easyButton.y, WHITE);
            DrawText("", easyButton.x + 50, easyButton.y + 10, 30, DARKGRAY);

            DrawTexture(normalbut, mediumButton.x, mediumButton.y, WHITE);
            DrawText("", mediumButton.x + 50, mediumButton.y + 10, 30, DARKGRAY);

            DrawTexture(hardbut, hardButton.x, hardButton.y, WHITE);
            DrawText("", hardButton.x + 50, hardButton.y + 10, 30, DARKGRAY);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), easyButton)) {
                PlaySound(buttonSound);
                StopSound(bgSound);
                PlaySound(PantherSound);
                difficulty = 0; // easy
                currentState = GAME;
            }
            else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), mediumButton)) {
                PlaySound(buttonSound);
                StopSound(bgSound);
                PlaySound(PantherSound);

                difficulty = 1; // medium
                currentState = GAME;
            }
            else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), hardButton)) {
                PlaySound(buttonSound);
                StopSound(bgSound);
                PlaySound(hardSound);

                difficulty = 2; // hard
                currentState = GAME;
            }

            EndDrawing();
        }
        else if (currentState == GAME) {
            // Create a Level object with the selected difficulty
            Level currentLevel(difficulty);

            // Run the level
            currentLevel.Run(game);

            // Check if the user clicked "Retour" (return to menu)
            if (currentState == NOMBRE_SELECTION) {
                StopSound(PantherSound);
                PlaySound(buttonSound);
                continue;  // Restart the main loop
            }

            break; // Exit the loop when the game ends
        }


    }

    // Unloading ressources
    UnloadSound(buttonSound);
    UnloadSound(bgSound);
    UnloadSound(winnerSound);
    UnloadSound(hardSound);
    UnloadSound(PantherSound);
    // Unload the button texture before closing
    UnloadTexture(normalbut);

    UnloadTexture(background);

    CloseAudioDevice();


    CloseWindow();
    return 0;
}