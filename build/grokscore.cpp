#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>
using namespace sf;

const int rows = 25;
const int cols = 40;

int grid[rows][cols] = {0};
int ts = 18; // tile size

struct Enemy {
    int x, y, dx, dy;
    Enemy() {
        x = y = 300;
        dx = 4 - rand() % 8;
        dy = 4 - rand() % 8;
    }
    void move() {
        x += dx;
        if (x < ts || x >= (cols-1)*ts || y < ts || y >= (rows-1)*ts || grid[y/ts][x/ts] == 1) {
            dx = -dx;
            x += dx;
        }
        y += dy;
        if (y < ts || y >= (rows-1)*ts || x < ts || x >= (cols-1)*ts || grid[y/ts][x/ts] == 1) {
            dy = -dy;
            y += dy;
        }
    }
};

void drop(int y, int x, int& filled_count) {
    if (y < 0 || y >= rows || x < 0 || x >= cols || grid[y][x] != 0) return;
    grid[y][x] = -1;
    filled_count++;
    drop(y-1, x, filled_count);
    drop(y+1, x, filled_count);
    drop(y, x-1, filled_count);
    drop(y, x+1, filled_count);
}

// Load top 5 scores from scores.txt
void load_scores(int scores[], int& score_count) {
    std::ifstream file("scores.txt");
    score_count = 0;
    while (score_count < 5 && file >> scores[score_count]) {
        score_count++;
    }
    file.close();
    // Sort scores (highest first)
    for (int i = 0; i < score_count - 1; i++)
        for (int j = 0; j < score_count - 1 - i; j++)
            if (scores[j] < scores[j+1]) {
                int temp = scores[j];
                scores[j] = scores[j+1];
                scores[j+1] = temp;
            }
}

// Save new score if it's in top 5
void save_score(int scores[], int& score_count, int new_score) {
    if (score_count < 5) {
        scores[score_count++] = new_score;
    } else if (new_score > scores[4]) {
        scores[4] = new_score;
    }
    // Sort scores
    for (int i = 0; i < score_count - 1; i++)
        for (int j = 0; j < score_count - 1 - i; j++)
            if (scores[j] < scores[j+1]) {
                int temp = scores[j];
                scores[j] = scores[j+1];
                scores[j+1] = temp;
            }
    std::ofstream file("scores.txt");
    for (int i = 0; i < score_count; i++)
        file << scores[i] << "\n";
    file.close();
}

// Check if score is in top 5
int is_high_score(int scores[], int score_count, int score) {
    return score_count < 5 || score > scores[score_count-1];
}

// Reset game to start
void reset_game(int& x, int& y, int& dx, int& dy, int& score, int level, int& enemyCount, Enemy a[], int& game) {
    // Clear grid, set borders
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            grid[i][j] = (i == 0 || j == 0 || i == rows-1 || j == cols-1) ? 1 : 0;
    x = 10; y = 0; // Player starts at (10,0)
    dx = dy = 0; // No movement
    score = 0; // Reset score
    enemyCount = 3 + level; // Enemies based on level
    for (int i = 0; i < enemyCount; i++) {
        a[i] = Enemy(); // Create enemy
        a[i].dx = (4 + level) - rand() % (8 + level); // Faster for higher levels
        a[i].dy = (4 + level) - rand() % (8 + level);
    }
    game = 1; // Game active
}

// Draw Start Menu
void draw_start_menu(RenderWindow& window, Font& font, int selected_level, Text& text) {
    // Title
    text.setFont(font);
    text.setCharacterSize(40);
    text.setString("Xonix Game");
    text.setPosition(cols*ts/2 - 100, 50); // Center at top
    text.setFillColor(Color::White);
    window.draw(text);

    // Start Game button
    text.setCharacterSize(30);
    text.setString("Start Game");
    text.setPosition(cols*ts/2 - 50, 150); // Below title
    text.setFillColor(Color::Green);
    window.draw(text);

    // Select Level button
    char level_str[20];
    snprintf(level_str, sizeof(level_str), "Level: %d", selected_level);
    text.setString(level_str);
    text.setPosition(cols*ts/2 - 60, 200); // Below Start Game
    text.setFillColor(Color::Yellow);
    window.draw(text);

    // Scoreboard button
    text.setString("Scoreboard");
    text.setPosition(cols*ts/2 - 50, 250); // Below Level
    text.setFillColor(Color::Cyan);
    window.draw(text);
}

// Handle Start Menu clicks
void handle_start_clicks(Event& e, RenderWindow& window, int& state, int& selected_level, int& level, int& x, int& y, int& dx, int& dy, int& score, int& enemyCount, Enemy a[], int& game, Text& text) {
    if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
        Vector2i mouse = Mouse::getPosition(window); // Get mouse position in window
        // Start Game
        text.setString("Start Game");
        text.setPosition(cols*ts/2 - 50, 150);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            level = selected_level; // Set game level
            reset_game(x, y, dx, dy, score, level, enemyCount, a, game); // Start game
            state = 1; // Switch to playing
        }
        // Select Level
        char level_str[20];
        snprintf(level_str, sizeof(level_str), "Level: %d", selected_level);
        text.setString(level_str);
        text.setPosition(cols*ts/2 - 60, 200);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            selected_level = (selected_level % 5) + 1; // Cycle 1-5
        }
        // Scoreboard
        text.setString("Scoreboard");
        text.setPosition(cols*ts/2 - 50, 250);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            state = 3; // Switch to scoreboard
        }
    }
}

// Draw End Menu
void draw_end_menu(RenderWindow& window, Font& font, int score, int scores[], int score_count, Text& text) {
    // Title
    text.setFont(font);
    text.setCharacterSize(40);
    text.setString("Game Over!");
    text.setPosition(cols*ts/2 - 100, 50);
    text.setFillColor(Color::Red);
    window.draw(text);

    // Score
    char score_str[100];
    snprintf(score_str, sizeof(score_str), "Score: %d%s", score, is_high_score(scores, score_count, score) ? " (High Score!)" : "");
    text.setCharacterSize(30);
    text.setString(score_str);
    text.setPosition(cols*ts/2 - 100, 100);
    text.setFillColor(Color::White);
    window.draw(text);

    // Restart button
    text.setString("Restart");
    text.setPosition(cols*ts/2 - 50, 200);
    text.setFillColor(Color::Green);
    window.draw(text);

    // Main Menu button
    text.setString("Main Menu");
    text.setPosition(cols*ts/2 - 60, 250);
    text.setFillColor(Color::Yellow);
    window.draw(text);

    // Exit button
    text.setString("Exit");
    text.setPosition(cols*ts/2 - 30, 300);
    text.setFillColor(Color::Red);
    window.draw(text);
}

// Handle End Menu clicks
void handle_end_clicks(Event& e, RenderWindow& window, int& state, int& x, int& y, int& dx, int& dy, int& score, int level, int& enemyCount, Enemy a[], int& game, Text& text) {
    if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
        Vector2i mouse = Mouse::getPosition(window);
        // Restart
        text.setString("Restart");
        text.setPosition(cols*ts/2 - 50, 200);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            reset_game(x, y, dx, dy, score, level, enemyCount, a, game);
            state = 1;
        }
        // Main Menu
        text.setString("Main Menu");
        text.setPosition(cols*ts/2 - 60, 250);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            state = 0;
        }
        // Exit
        text.setString("Exit");
        text.setPosition(cols*ts/2 - 30, 300);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            window.close();
        }
    }
}

// Draw Scoreboard
void draw_scoreboard(RenderWindow& window, Font& font, int scores[], int score_count, Text& text) {
    // Title
    text.setFont(font);
    text.setCharacterSize(40);
    text.setString("High Scores");
    text.setPosition(cols*ts/2 - 100, 50);
    text.setFillColor(Color::White);
    window.draw(text);

    // Scores
    char s[200] = "";
    for (int i = 0; i < score_count; i++) {
        char line[50];
        snprintf(line, sizeof(line), "%d. %d\n", i+1, scores[i]);
        strcat(s, line);
    }
    if (score_count == 0) strcat(s, "No scores yet");
    text.setCharacterSize(30);
    text.setString(s);
    text.setPosition(cols*ts/2 - 100, 100);
    text.setFillColor(Color::White);
    window.draw(text);

    // Back button
    text.setString("Back");
    text.setPosition(cols*ts/2 - 30, 300);
    text.setFillColor(Color::Red);
    window.draw(text);
}

// Handle Scoreboard clicks
void handle_scoreboard_clicks(Event& e, RenderWindow& window, int& state, Text& text) {
    if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
        Vector2i mouse = Mouse::getPosition(window);
        text.setString("Back");
        text.setPosition(cols*ts/2 - 30, 300);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            state = 0; // Back to menu
        }
    }
}

int main() {
    srand(time(0));
    RenderWindow window(VideoMode(cols*ts, rows*ts), "Xonix Game!"); // 720x450 window
    window.setFramerateLimit(60);

    // Load textures
    Texture t1, t2, t3;
    t1.loadFromFile("images/tiles.png");
    t2.loadFromFile("images/gameover.png");
    t3.loadFromFile("images/enemy.png");

    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);

    // Load font
    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Font load failed\n";
        return -1;
    }

    Text text; // One text object for all displays
    text.setFont(font);

    // High scores
    int scores[5] = {0};
    int score_count = 0;
    load_scores(scores, score_count);

    // Game variables
    int enemyCount = 4;
    Enemy a[10]; // Up to 10 enemies
    int game = 1; // 1 = active, 0 = ended
    int x = 10, y = 0, dx = 0, dy = 0; // Player position and movement
    float timer = 0, delay = 0.07; // Movement timing
    Clock clock;
    int score = 0; // Player score
    int level = 1; // Current level
    int selected_level = 1; // Menu-selected level
    int state = 0; // 0=menu, 1=playing, 2=game over, 3=scoreboard

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();
            // Return to menu with Escape
            if (state == 1 && e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                state = 0;
                save_score(scores, score_count, score); // Save score
                game = 0;
            }
            // Handle menu clicks
            if (state == 0) handle_start_clicks(e, window, state, selected_level, level, x, y, dx, dy, score, enemyCount, a, game, text);
            if (state == 2) handle_end_clicks(e, window, state, x, y, dx, dy, score, level, enemyCount, a, game, text);
            if (state == 3) handle_scoreboard_clicks(e, window, state, text);
        }

        if (state == 1 && game) {
            // Player movement
            if (Keyboard::isKeyPressed(Keyboard::Left)) { dx = -1; dy = 0; }
            if (Keyboard::isKeyPressed(Keyboard::Right)) { dx = 1; dy = 0; }
            if (Keyboard::isKeyPressed(Keyboard::Up)) { dx = 0; dy = -1; }
            if (Keyboard::isKeyPressed(Keyboard::Down)) { dx = 0; dy = 1; }

            // Update player position
            float time = clock.getElapsedTime().asSeconds();
            clock.restart();
            timer += time;
            if (timer > delay) {
                int new_x = x + dx;
                int new_y = y + dy;
                if (new_x >= 0 && new_x < cols && new_y >= 0 && new_y < rows) {
                    if (grid[new_y][new_x] == 0) {
                        x = new_x;
                        y = new_y;
                        grid[y][x] = 2; // Draw trail
                    } else if (grid[new_y][new_x] == 1) {
                        x = new_x;
                        y = new_y;
                        dx = dy = 0;
                        int filled_count = 0;
                        for (int i = 0; i < enemyCount; i++)
                            drop(a[i].y/ts, a[i].x/ts, filled_count); // Fill areas
                        for (int i = 0; i < rows; i++)
                            for (int j = 0; j < cols; j++)
                                if (grid[i][j] == -1) grid[i][j] = 0;
                                else if (grid[i][j] != 1) grid[i][j] = 1;
                        score += filled_count; // Add to score
                    }
                }
                timer = 0;
            }

            // Move enemies
            for (int i = 0; i < enemyCount; i++) a[i].move();

            // Check collisions
            for (int i = 0; i < enemyCount; i++) {
                int ex = a[i].x / ts;
                int ey = a[i].y / ts;
                if (grid[ey][ex] == 2 || (ex == x && ey == y)) {
                    save_score(scores, score_count, score); // Save score
                    state = 2; // Game over
                    game = 0;
                    break;
                }
            }
        }

        window.clear();
        if (state == 0) {
            draw_start_menu(window, font, selected_level, text);
        } else if (state == 1) {
            // Draw grid
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++) {
                    if (grid[i][j] == 0) continue;
                    if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts));
                    if (grid[i][j] == 2) sTile.setTextureRect(IntRect(54, 0, ts, ts));
                    sTile.setPosition(j*ts, i*ts);
                    window.draw(sTile);
                }
            // Draw player
            sTile.setTextureRect(IntRect(36, 0, ts, ts));
            sTile.setPosition(x*ts, y*ts);
            window.draw(sTile);
            // Draw enemies
            sEnemy.rotate(10);
            for (int i = 0; i < enemyCount; i++) {
                sEnemy.setPosition(a[i].x, a[i].y);
                window.draw(sEnemy);
            }
            if (!game) window.draw(sGameover);
        } else if (state == 2) {
            draw_end_menu(window, font, score, scores, score_count, text);
        } else if (state == 3) {
            draw_scoreboard(window, font, scores, score_count, text);
        }
        window.display();
    }

    return 0;
}
