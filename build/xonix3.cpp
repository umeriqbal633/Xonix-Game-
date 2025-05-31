#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>
#include <sstream>
#include <iomanip>
using namespace sf;

const int rows = 25;
const int cols = 40;

int grid[rows][cols] = {0};
int ts = 18; // tile size

struct Enemy {
    int x, y;
    float dx, dy;
    float speed; // Base speed multiplier
    int moveType; // 0 = linear, 1 = zig-zag
    int patternStep; // For tracking steps in zig-zag

    Enemy() {
        x = y = 300;
        dx = 1.0f * (4 - rand() % 8);
        dy = 1.0f * (4 - rand() % 8);
        speed = 1.0f; // Initial speed
        moveType = 0; // Start with linear
        patternStep = 0; // Initial step for zig-zag
    }

    // Linear movement - straight line with bouncing
    void moveLinear() {
        x += dx * speed;
        if (x < ts || x >= (cols-1)*ts || grid[y/ts][x/ts] == 1) {
            dx = -dx;
            x += dx * speed;
        }
        y += dy * speed;
        if (y < ts || y >= (rows-1)*ts || grid[y/ts][x/ts] == 1) {
            dy = -dy;
            y += dy * speed;
        }
    }

    // Zig-zag movement - alternates between diagonal and horizontal, roaming everywhere
    void moveZigZag() {
        patternStep++;
        if (patternStep >= 40) {
            patternStep = 0; // Reset after 40 steps (20 per phase)
            // Randomize direction at cycle start for broader movement
            dx = (rand() % 2 == 0 ? 1.0f : -1.0f);
            dy = (rand() % 2 == 0 ? 1.0f : -1.0f);
        }
        static int direction = 3; // Larger step size for range
        if (patternStep == 20 || patternStep == 0) {
            // Reset direction at phase change
            direction = (patternStep == 0) ? 3 : -3;
        }

        // Calculate next position
        int nextX = x;
        int nextY = y;
        if (patternStep < 20) {
            // Diagonal movement
            nextX += direction * speed * dx;
            nextY += direction * speed * dy;
        } else {
            // Horizontal movement
            nextX += direction * speed * dx;
            nextY += 0;
        }

        // Check if next position is valid
        bool validMove = true;
        if (nextX < ts || nextX >= (cols-1)*ts || nextY < ts || nextY >= (rows-1)*ts) {
            validMove = false;
        } else if (grid[nextY/ts][nextX/ts] == 1) {
            validMove = false;
        }

        if (validMove) {
            // Move if within bounds and not hitting filled tile
            x = nextX;
            y = nextY;
            // Update dx, dy to reflect current direction
            if (patternStep < 20) {
                dx = (direction > 0) ? dx : -dx;
                dy = (direction > 0) ? dy : -dy;
            } else {
                dx = (direction > 0) ? dx : -dx;
                dy = 0.0f;
            }
        } else {
            // Reverse zig-zag direction and stay in place
            direction = -direction;
            // Update dx, dy to reflect reversed direction
            if (patternStep < 20) {
                dx = -dx;
                dy = -dy;
            } else {
                dx = -dx;
                dy = 0.0f;
            }
            // Clamp to prevent exiting grid
            if (x < ts) x = ts;
            if (x >= (cols-1)*ts) x = (cols-1)*ts - 1;
            if (y < ts) y = ts;
            if (y >= (rows-1)*ts) y = (rows-1)*ts - 1;
        }
    }

    // Main move function to select movement type
    void move() {
        switch (moveType) {
            case 0: moveLinear(); break;
            case 1: moveZigZag(); break;
            default: moveLinear(); break;
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

void load_scores(int scores[], int& score_count) {
    std::ifstream file("scores.txt");
    score_count = 0;
    while (score_count < 5 && file >> scores[score_count]) {
        score_count++;
    }
    file.close();
    for (int i = 0; i < score_count - 1; i++)
        for (int j = 0; j < score_count - 1 - i; j++)
            if (scores[j] < scores[j+1]) {
                int temp = scores[j];
                scores[j] = scores[j+1];
                scores[j+1] = temp;
            }
}

void save_score(int scores[], int& score_count, int new_score) {
    if (score_count < 5) {
        scores[score_count++] = new_score;
    } else if (new_score > scores[4]) {
        scores[4] = new_score;
    }
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

int is_high_score(int scores[], int score_count, int score) {
    return score_count < 5 || score > scores[score_count-1];
}

void reset_game(int& x, int& y, int& dx, int& dy, int& prevDx, int& prevDy, int& score, int& moveCounter, int level, int& enemyCount, Enemy a[], int& game) {
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            grid[i][j] = (i == 0 || j == 0 || i == rows-1 || j == cols-1) ? 1 : 0;
    x = 10; y = 0;
    dx = dy = 0;
    prevDx = prevDy = 0;
    score = 0;
    moveCounter = 0;
    enemyCount = 3 + level;
    for (int i = 0; i < enemyCount; i++) {
        a[i] = Enemy();
    }
    game = 1;
}

void draw_start_menu(RenderWindow& window, Font& font, int selected_level, Text& text) {
    text.setFont(font);
    text.setCharacterSize(40);
    text.setString("Xonix Game");
    text.setPosition(cols*ts/2 - 100, 50);
    text.setFillColor(Color::White);
    window.draw(text);

    text.setCharacterSize(30);
    text.setString("Start Game");
    text.setPosition(cols*ts/2 - 50, 150);
    text.setFillColor(Color::Green);
    window.draw(text);

    char level_str[20];
    snprintf(level_str, sizeof(level_str), "Level: %d", selected_level);
    text.setString(level_str);
    text.setPosition(cols*ts/2 - 60, 200);
    text.setFillColor(Color::Yellow);
    window.draw(text);

    text.setString("Scoreboard");
    text.setPosition(cols*ts/2 - 50, 250);
    text.setFillColor(Color::Cyan);
    window.draw(text);
}

void handle_start_clicks(Event& e, RenderWindow& window, int& state, int& selected_level, int& level, int& x, int& y, int& dx, int& dy, int& prevDx, int& prevDy, int& score, int& moveCounter, int& enemyCount, Enemy a[], int& game, Text& text, Clock& gameClock) {
    if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
        Vector2i mouse = Mouse::getPosition(window);
        text.setString("Start Game");
        text.setPosition(cols*ts/2 - 50, 150);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            level = selected_level;
            reset_game(x, y, dx, dy, prevDx, prevDy, score, moveCounter, level, enemyCount, a, game);
            state = 1;
            gameClock.restart();
        }
        char level_str[20];
        snprintf(level_str, sizeof(level_str), "Level: %d", selected_level);
        text.setString(level_str);
        text.setPosition(cols*ts/2 - 60, 200);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            selected_level = (selected_level % 5) + 1;
        }
        text.setString("Scoreboard");
        text.setPosition(cols*ts/2 - 50, 250);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            state = 3;
        }
    }
}

void draw_end_menu(RenderWindow& window, Font& font, int score, int scores[], int score_count, Text& text) {
    text.setFont(font);
    text.setCharacterSize(40);
    text.setString("Game Over!");
    text.setPosition(cols*ts/2 - 100, 50);
    text.setFillColor(Color::Red);
    window.draw(text);

    char score_str[100];
    snprintf(score_str, sizeof(score_str), "Score: %d%s", score, is_high_score(scores, score_count, score) ? " (High Score!)" : "");
    text.setCharacterSize(30);
    text.setString(score_str);
    text.setPosition(cols*ts/2 - 100, 100);
    text.setFillColor(Color::White);
    window.draw(text);

    text.setString("Restart");
    text.setPosition(cols*ts/2 - 50, 200);
    text.setFillColor(Color::Green);
    window.draw(text);

    text.setString("Main Menu");
    text.setPosition(cols*ts/2 - 60, 250);
    text.setFillColor(Color::Yellow);
    window.draw(text);

    text.setString("Exit");
    text.setPosition(cols*ts/2 - 30, 300);
    text.setFillColor(Color::Red);
    window.draw(text);
}

void handle_end_clicks(Event& e, RenderWindow& window, int& state, int& x, int& y, int& dx, int& dy, int& prevDx, int& prevDy, int& score, int& moveCounter, int level, int& enemyCount, Enemy a[], int& game, Text& text, Clock& gameClock) {
    if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
        Vector2i mouse = Mouse::getPosition(window);
        text.setString("Restart");
        text.setPosition(cols*ts/2 - 50, 200);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            reset_game(x, y, dx, dy, prevDx, prevDy, score, moveCounter, level, enemyCount, a, game);
            state = 1;
            gameClock.restart();
        }
        text.setString("Main Menu");
        text.setPosition(cols*ts/2 - 60, 250);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            state = 0;
        }
        text.setString("Exit");
        text.setPosition(cols*ts/2 - 30, 300);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            window.close();
        }
    }
}

void draw_scoreboard(RenderWindow& window, Font& font, int scores[], int score_count, Text& text) {
    text.setFont(font);
    text.setCharacterSize(40);
    text.setString("High Scores");
    text.setPosition(cols*ts/2 - 100, 50);
    text.setFillColor(Color::White);
    window.draw(text);

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

    text.setString("Back");
    text.setPosition(cols*ts/2 - 30, 300);
    text.setFillColor(Color::Red);
    window.draw(text);
}

void handle_scoreboard_clicks(Event& e, RenderWindow& window, int& state, Text& text) {
    if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
        Vector2i mouse = Mouse::getPosition(window);
        text.setString("Back");
        text.setPosition(cols*ts/2 - 30, 300);
        if (text.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y))) {
            state = 0;
        }
    }
}

void draw_move_counter(RenderWindow& window, Font& font, int moveCounter, bool game_active) {
    Text move_text;
    move_text.setFont(font);
    move_text.setCharacterSize(22);
    move_text.setStyle(Text::Bold);
    
    std::stringstream ss;
    ss << "Moves: " << moveCounter;
    
    move_text.setString(ss.str());
    move_text.setPosition(20, 50);
    
    if (game_active) {
        Text shadow = move_text;
        shadow.setFillColor(Color(0, 0, 0, 200));
        for (int offset = 1; offset <= 1; offset++) {
            shadow.setPosition(move_text.getPosition().x + offset, move_text.getPosition().y);
            window.draw(shadow);
            shadow.setPosition(move_text.getPosition().x - offset, move_text.getPosition().y);
            window.draw(shadow);
            shadow.setPosition(move_text.getPosition().x, move_text.getPosition().y + offset);
            window.draw(shadow);
            shadow.setPosition(move_text.getPosition().x, move_text.getPosition().y - offset);
            window.draw(shadow);
        }
        move_text.setFillColor(Color::White);
    } else {
        move_text.setFillColor(Color::Yellow);
    }
    
    window.draw(move_text);
}

void draw_timer(RenderWindow& window, Font& font, float game_time) {
    Text timer_text;
    timer_text.setFont(font);
    timer_text.setCharacterSize(22);
    timer_text.setString("Time: " + std::to_string((int)game_time));
    timer_text.setPosition(20, 20);
    timer_text.setFillColor(Color::White);
    window.draw(timer_text);
}

int main() {
    srand(time(0));
    RenderWindow window(VideoMode(cols*ts, rows*ts), "Xonix Game!");
    window.setFramerateLimit(60);

    Texture t1, t2, t3;
    if (!t1.loadFromFile("images/tiles.png")) {
        std::cerr << "Failed to load tiles.png\n";
        return -1;
    }
    if (!t2.loadFromFile("images/gameover.png")) {
        std::cerr << "Failed to load gameover.png\n";
        return -1;
    }
    if (!t3.loadFromFile("images/enemy.png")) {
        std::cerr << "Failed to load enemy.png\n";
        return -1;
    }

    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);

    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Font load failed\n";
        return -1;
    }

    Text text;
    text.setFont(font);

    int scores[5] = {0};
    int score_count = 0;
    load_scores(scores, score_count);

    int enemyCount = 4;
    Enemy a[10];
    int game = 1;
    int x = 10, y = 0, dx = 0, dy = 0;
    int prevDx = 0, prevDy = 0;
    float timer = 0, delay = 0.07;
    Clock clock;
    Clock gameClock;
    int score = 0;
    int moveCounter = 0;
    int level = 1;
    int selected_level = 1;
    int state = 0;
    float lastSpeedIncrease = 0; // Track time of last speed increase
    bool patternsActivated = false; // Flag for zig-zag pattern

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();
            if (state == 1 && e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                state = 0;
                save_score(scores, score_count, score);
                game = 0;
            }
            if (state == 0) handle_start_clicks(e, window, state, selected_level, level, x, y, dx, dy, prevDx, prevDy, score, moveCounter, enemyCount, a, game, text, gameClock);
            if (state == 2) handle_end_clicks(e, window, state, x, y, dx, dy, prevDx, prevDy, score, moveCounter, level, enemyCount, a, game, text, gameClock);
            if (state == 3) handle_scoreboard_clicks(e, window, state, text);
        }

        float frame_time = clock.getElapsedTime().asSeconds();
        clock.restart();
        float game_time = gameClock.getElapsedTime().asSeconds();

        if (state == 1 && game) {
            // Increase enemy speed every 20 seconds
            if (game_time - lastSpeedIncrease >= 20) {
                for (int i = 0; i < enemyCount; i++) {
                    a[i].speed += 0.5f; // Increase speed by 0.5
                }
                lastSpeedIncrease = game_time;
            }

            // Switch half of enemies to zig-zag pattern after 30 seconds
            if (!patternsActivated && game_time >= 30) {
                patternsActivated = true;
                int halfEnemies = enemyCount / 2;
                for (int i = 0; i < halfEnemies; i++) {
                    a[i].moveType = 1; // Set to zig-zag
                }
            }

            // Restore original input handling
            if (Keyboard::isKeyPressed(Keyboard::Left)) { dx = -1; dy = 0; }
            if (Keyboard::isKeyPressed(Keyboard::Right)) { dx = 1; dy = 0; }
            if (Keyboard::isKeyPressed(Keyboard::Up)) { dx = 0; dy = -1; }
            if (Keyboard::isKeyPressed(Keyboard::Down)) { dx = 0; dy = 1; }

            timer += frame_time;
            if (timer > delay) {
                int new_x = x + dx;
                int new_y = y + dy;
                if (new_x >= 0 && new_x < cols && new_y >= 0 && new_y < rows) {
                    if (grid[new_y][new_x] == 0) {
                        if ((dx != prevDx || dy != prevDy) && grid[y][x] == 2) {
                            moveCounter++;
                        }
                        x = new_x;
                        y = new_y;
                        grid[y][x] = 2;
                        prevDx = dx;
                        prevDy = dy;
                    } else if (grid[new_y][new_x] == 1) {
                        x = new_x;
                        y = new_y;
                        dx = dy = 0;
                        prevDx = prevDy = 0;
                        int filled_count = 0;
                        for (int i = 0; i < enemyCount; i++)
                            drop(a[i].y/ts, a[i].x/ts, filled_count);
                        for (int i = 0; i < rows; i++)
                            for (int j = 0; j < cols; j++)
                                if (grid[i][j] == -1) grid[i][j] = 0;
                                else if (grid[i][j] != 1) grid[i][j] = 1;
                        score += filled_count;
                    }
                }
                timer = 0;
            }

            // Move enemies
            for (int i = 0; i < enemyCount; i++) {
                a[i].move();
            }

            // Check collisions
            for (int i = 0; i < enemyCount; i++) {
                int ex = a[i].x / ts;
                int ey = a[i].y / ts;
                if (ex >= 0 && ex < cols && ey >= 0 && ey < rows) {
                    if (grid[ey][ex] == 2 || (ex == x && ey == y)) {
                        save_score(scores, score_count, score);
                        state = 2;
                        game = 0;
                        break;
                    }
                }
            }
        }

        window.clear();
        if (state == 0) {
            draw_start_menu(window, font, selected_level, text);
        } else if (state == 1) {
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++) {
                    if (grid[i][j] == 0) continue;
                    if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts));
                    if (grid[i][j] == 2) sTile.setTextureRect(IntRect(54, 0, ts, ts));
                    sTile.setPosition(j*ts, i*ts);
                    window.draw(sTile);
                }
            sTile.setTextureRect(IntRect(36, 0, ts, ts));
            sTile.setPosition(x*ts, y*ts);
            window.draw(sTile);
            for (int i = 0; i < enemyCount; i++) {
                sEnemy.setPosition(a[i].x, a[i].y);
                window.draw(sEnemy);
            }
            if (!game) window.draw(sGameover);
            
            draw_timer(window, font, game_time);
            draw_move_counter(window, font, moveCounter, game == 1);
        } else if (state == 2) {
            draw_end_menu(window, font, score, scores, score_count, text);
            draw_timer(window, font, game_time);
            draw_move_counter(window, font, moveCounter, false);
        } else if (state == 3) {
            draw_scoreboard(window, font, scores, score_count, text);
        }
        window.display();
    }

    return 0;
}
