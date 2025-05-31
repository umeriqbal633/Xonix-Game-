#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace sf;

const int rows = 25;
const int cols = 40;

int grid[rows][cols] = {0};
int ts = 18;

struct Enemy {
    int x, y;
    float dx, dy;
    float speed;
    int Move_Type;
    int patternStep;

    Enemy() {
        x = y = 300;
        dx = 1.0f * (4 - rand() % 8);
        dy = 1.0f * (4 - rand() % 8);
        speed = 1.0f;
        Move_Type = 0;
        patternStep = 0;
    }

    // Feature 4: Implements linear enemy movement pattern
    void Move_Linear() {
        x += dx * speed;
        if (x < ts || x >= (cols-1)*ts || grid[y/ts][x/ts] == 1) {
            dx = -dx;
            x += dx * speed;
        }
        y += dy * speed;
        if (y < ts || y >= (rows-1)*ts || grid[y/ts][x/ts] == 1) {
            dy = -dy;
            y += dy * speed;  }}
        
    

    // Feature 4: Implements zig-zag enemy movement pattern
    void Move_Zigzag() {
        patternStep++;
        if (patternStep >= 40) {
            patternStep = 0;
            dx = (rand() % 2 == 0 ? 1.0f : -1.0f);
            dy = (rand() % 2 == 0 ? 1.0f : -1.0f);
        }
        static int direction = 3;
        if (patternStep == 20 || patternStep == 0) {
            direction = (patternStep == 0) ? 3 : -3;
        }

        int nextX = x;
        int nextY = y;
        if (patternStep < 20) {
            nextX += direction * speed * dx;
            nextY += direction * speed * dy;
        } else {
            nextX += direction * speed * dx;
            nextY += 0;
        }

        bool validMove = true;
        if (nextX < ts || nextX >= (cols-1)*ts || nextY < ts || nextY >= (rows-1)*ts) {
            validMove = false;
        } else if (grid[nextY/ts][nextX/ts] == 1) {
            validMove = false;
        }

        if (validMove) {
            x = nextX;
            y = nextY;
            if (patternStep < 20) {
                dx = (direction > 0) ? dx : -dx;
                dy = (direction > 0) ? dy : -dy;
            } else {
                dx = (direction > 0) ? dx : -dx;
                dy = 0.0f;
            }
        } else {
            direction = -direction;
            if (patternStep < 20) {
                dx = -dx;
                dy = -dy;
            } else {
                dx = -dx;
                dy = 0.0f;
            }
            if (x < ts) x = ts;
            if (x >= (cols-1)*ts) x = (cols-1)*ts - 1;
            if (y < ts) y = ts;
            if (y >= (rows-1)*ts) y = (rows-1)*ts - 1;  } }
        
    

    // Feature 4: Selects enemy movement pattern ( zig-zag)
    void move() {
        switch (Move_Type) {
            case 0: Move_Linear(); break;
            case 1: Move_Zigzag(); break;
            default: Move_Linear(); break;  } }
        
    
};

// Feature 2: Recursively marks areas containing enemies to determine capturable regions
void drop(int y, int x, int& Filled_Count) {
    if (y < 0 || y >= rows || x < 0 || x >= cols || grid[y][x] != 0) return;
    grid[y][x] = -1;
    Filled_Count++;
    drop(y-1, x, Filled_Count);
    drop(y+1, x, Filled_Count);
    drop(y, x-1, Filled_Count);
    drop(y, x+1, Filled_Count);
}

struct ScoreEntry {
    int Score;
    float Time;
};

// Feature 6: Loads and sorts high scores from file
void Loading_Scores(ScoreEntry scores[], int& Score_count) {
    std::ifstream file("scores.txt");
    Score_count = 0;
    int Score;
    float Time;
    while (Score_count < 5 && file >> Score >> Time) {
        scores[Score_count].Score = Score;
        scores[Score_count].Time = Time;
        Score_count++;
    }
    file.close();
    for (int i = 0; i < Score_count - 1; i++)
        for (int j = 0; j < Score_count - 1 - i; j++)
            if (scores[j].Score < scores[j+1].Score) {
                ScoreEntry temp = scores[j];
                scores[j] = scores[j+1];
                scores[j+1] = temp;
            }
}

// Feature 6: Saves new high score to file if it ranks in top 5
void Saving_Score(ScoreEntry scores[], int& Score_count, int new_Score, float Game_Time) {
    if (Score_count < 5) {
        scores[Score_count].Score = new_Score;
        scores[Score_count].Time = Game_Time;
        Score_count++;
    } else if (new_Score > scores[4].Score) {
        scores[4].Score = new_Score;
        scores[4].Time = Game_Time;
    }
    for (int i = 0; i < Score_count - 1; i++)
        for (int j = 0; j < Score_count - 1 - i; j++)
            if (scores[j].Score < scores[j+1].Score) {
                ScoreEntry temp = scores[j];
                scores[j] = scores[j+1];
                scores[j+1] = temp;
            }
    std::ofstream file("scores.txt");
    for (int i = 0; i < Score_count; i++)
        file << scores[i].Score << " " << std::fixed << std::setprecision(1) << scores[i].Time << "\n";
    file.close();
}

// Feature 6: Checks if a score qualifies as a high score
int High_Score(ScoreEntry scores[], int Score_count, int Score) {
    return Score_count < 5 || Score > scores[Score_count-1].Score;
}

// Feature 6: Clears the high score file
void Clear_Scores() {
    std::ofstream file("scores.txt", std::ios::trunc);
    file.close();
}





// Feature 1, 2: Initializes or resets game state, including grid, players, enemies, and difficulty-based enemy count
void Reset_Game(int& x1, int& Y1_Position, int& Dx_1, int& Dy_1, int& Prev_Dx_1, int& Prev_Dy_1, int& x2, int& y2Pos, int& Dx_2, int& Dy_2, int& Prev_Dx_2, int& Prev_Dy_2, int& Score_1, int& Score_2, int& Move_Counter, int Level, int& enemyCount, Enemy a[], int& game, int Game_Mode, bool& Player_1_Alive, bool& Player_2_Alive, int& Bonus_Count_1, int& Bonus_Count_2, int& Bonus_Minimum_1, int& Bonus_Minimum_2, int& Power_Up_Count_1, int& Power_Up_Count_2, bool& Enemies_Stopped, float& Freeze_Timer, int& Last_Power_Up_Score_1, int& Last_Power_Up_Score_2) {
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            grid[i][j] = (i == 0 || j == 0 || i == rows-1 || j == cols-1) ? 1 : 0;
    x1 = 10; Y1_Position = 0;
    x2 = cols-2; y2Pos = rows-2;
    Dx_1 = Dy_1 = Dx_2 = Dy_2 = 0;
    Prev_Dx_1 = Prev_Dy_1 = Prev_Dx_2 = Prev_Dy_2 = 0;
    Score_1 = Score_2 = 0;
    Move_Counter = 0;
    switch (Level) {
        case 1: enemyCount = 2; break;
        case 2: enemyCount = 4; break;
        case 3: enemyCount = 6; break;
        case 4: enemyCount = 2; break;
        default: enemyCount = 2; break;
    }
    for (int i = 0; i < enemyCount; i++) {
        a[i] = Enemy();
    }
    game = 1;
    Player_1_Alive = Player_2_Alive = true;
    Bonus_Count_1 = Bonus_Count_2 = 0;
    Bonus_Minimum_1 = Bonus_Minimum_2 = 10;
    Power_Up_Count_1 = Power_Up_Count_2 = 0;
    Enemies_Stopped = false;
    Freeze_Timer = 0.0f;
    Last_Power_Up_Score_1 = Last_Power_Up_Score_2 = 0;
}




// Feature 1: Renders the start menu with options for starting game, selecting level/mode, and viewing scoreboard
void Draw_Start_Meanu(RenderWindow& window, Font& font, int Selected_Level, int Selected_Mode, Text& text, int Select_Option, float Frame_Time) {
    VertexArray gradient(Quads, 4);
    gradient[0].position = Vector2f(0, 0);
    gradient[1].position = Vector2f(cols*ts, 0);
    gradient[2].position = Vector2f(cols*ts, rows*ts);
    gradient[3].position = Vector2f(0, rows*ts);
    gradient[0].color = Color(50, 50, 150);
    gradient[1].color = Color(150, 50, 150);
    gradient[2].color = Color(50, 150, 150);
    gradient[3].color = Color(150, 150, 50);
    window.draw(gradient);

    text.setFont(font);
    text.setCharacterSize(50);
    text.setStyle(Text::Bold);
    text.setString("Xonix Game");
    text.setPosition(cols*ts/2 - 120, 30);
    text.setFillColor(Color::White);
    Text shadow = text;
    shadow.setFillColor(Color(0, 0, 0, 100));
    shadow.setPosition(text.getPosition().x + 2, text.getPosition().y + 2);
    window.draw(shadow);
    window.draw(text);

    const char* options[] = {"Start Game", "", "Level", "Scoreboard"};
    const char* mode_names[] = {"Single Player", "Two Player"};
    const char* level_names[] = {"Easy", "Medium", "Hard", "Continuous"};
    Color colors[] = {Color::Green, Color::Magenta, Color::Yellow, Color::Cyan};
    float y_positions[] = {150, 220, 290, 360};
    float scale = 1.0f;

    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 1: text.setString(mode_names[Selected_Mode - 1]); break;
            case 2: text.setString(level_names[Selected_Level - 1]); break;
            default: text.setString(options[i]); break;
        }
        
        
        
        text.setCharacterSize(35);
        text.setStyle(Text::Bold);
        float x_pos = cols*ts/2 - text.getLocalBounds().width/2 - 10;
        text.setPosition(x_pos, y_positions[i]);
        
        if (i == Select_Option) {
            scale = 1.1f + 0.05f * sin(Frame_Time * 5);
            text.setScale(scale, scale);
            text.setFillColor(colors[i]);
            shadow = text;
            shadow.setFillColor(Color(0, 0, 0, 150));
            shadow.setPosition(x_pos + 2, y_positions[i] + 2);
            window.draw(shadow);
        } else {
            text.setScale(1.0f, 1.0f);
            Color dim_color = colors[i];
            dim_color.a = 180;
            text.setFillColor(dim_color);  }
        
        window.draw(text);  }
    

    text.setString("Use Up/Down to navigate, Space to select");
    text.setCharacterSize(20);
    text.setStyle(Text::Regular);
    text.setPosition(cols*ts/2 - text.getLocalBounds().width/2, rows*ts - 30);
    text.setFillColor(Color::White);
    text.setScale(1.0f, 1.0f);
    window.draw(text); }





// Feature 1: Processes keyboard input for navigating and selecting start menu options
void Handle_Start_Input(Event& e, int& State, int& Selected_Level, int& Selected_Mode, int& Level, int& Game_Mode, int& x1, int& Y1_Position, int& Dx_1, int& Dy_1, int& Prev_Dx_1, int& Prev_Dy_1, int& x2, int& y2Pos, int& Dx_2, int& Dy_2, int& Prev_Dx_2, int& Prev_Dy_2, int& Score_1, int& Score_2, int& Move_Counter, int& enemyCount, Enemy a[], int& game, bool& Player_1_Alive, bool& Player_2_Alive, Clock& Game_Clock, int& Bonus_Count_1, int& Bonus_Count_2, int& Bonus_Minimum_1, int& Bonus_Minimum_2, int& Power_Up_Count_1, int& Power_Up_Count_2, bool& Enemies_Stopped, float& Freeze_Timer, int& Last_Power_Up_Score_1, int& Last_Power_Up_Score_2, int& Select_Option) {
    if (e.type == Event::KeyPressed) {
        switch (e.key.code) {
            case Keyboard::Up: {
                Select_Option = (Select_Option - 1 + 4) % 4;
                break;
            }
            case Keyboard::Down: {
                Select_Option = (Select_Option + 1) % 4;
                break;
            }
            case Keyboard::Space: {
                switch (Select_Option) {
                    case 0: {
                        Level = Selected_Level;
                        Game_Mode = Selected_Mode;
                        Reset_Game(x1, Y1_Position, Dx_1, Dy_1, Prev_Dx_1, Prev_Dy_1, x2, y2Pos, Dx_2, Dy_2, Prev_Dx_2, Prev_Dy_2, Score_1, Score_2, Move_Counter, Level, enemyCount, a, game, Game_Mode, Player_1_Alive, Player_2_Alive, Bonus_Count_1, Bonus_Count_2, Bonus_Minimum_1, Bonus_Minimum_2, Power_Up_Count_1, Power_Up_Count_2, Enemies_Stopped, Freeze_Timer, Last_Power_Up_Score_1, Last_Power_Up_Score_2);
                        State = 1;
                        Game_Clock.restart();
                        break;
                    }
                    case 1: {
                        Selected_Mode = (Selected_Mode % 2) + 1;
                        break;
                    }
                    case 2: {
                        Selected_Level = (Selected_Level % 4) + 1;
                        break;
                    }
                    case 3: {
                        State = 3;
                        break;
                    }
                }
                break;  } } } }
            

// Feature 1, 7: Displays game over screen with final scores, high score indication, and options (Restart, Main Menu, Exit); shows winner in two-player mode


void Draw_End_Menu(RenderWindow& window, Font& font, int Score_1, int Score_2, int Game_Mode, ScoreEntry scores[], int Score_count, Text& text) {
    VertexArray gradient(Quads, 4);
    gradient[0].position = Vector2f(0, 0);
    gradient[1].position = Vector2f(cols*ts, 0);
    gradient[2].position = Vector2f(cols*ts, rows*ts);
    gradient[3].position = Vector2f(0, rows*ts);
    gradient[0].color = Color(50, 50, 150);
    gradient[1].color = Color(150, 50, 150);
    gradient[2].color = Color(50, 150, 150);
    gradient[3].color = Color(150, 150, 50);
    window.draw(gradient);

    text.setFont(font);
    text.setCharacterSize(50);
    text.setStyle(Text::Bold);
    text.setString("Game Over!");
    text.setPosition(cols*ts/2 - 120, 30);
    text.setFillColor(Color::Red);
    Text shadow = text;
    shadow.setFillColor(Color(0, 0, 0, 100));
    shadow.setPosition(text.getPosition().x + 2, text.getPosition().y + 2);
    window.draw(shadow);
    window.draw(text);

    char score_str[200];
    if (Game_Mode == 1) {
        snprintf(score_str, sizeof(score_str), "Score: %d%s", Score_1, High_Score(scores, Score_count, Score_1) ? " (High Score!)" : "");
    } else {
        snprintf(score_str, sizeof(score_str), "Player 1: %d\nPlayer 2: %d\nWinner: %s",
                 Score_1, Score_2, (Score_1 > Score_2) ? "Player 1" : (Score_2 > Score_1) ? "Player 2" : "Tie");
    }
    text.setCharacterSize(30);
    text.setStyle(Text::Regular);
    text.setString(score_str);
    text.setPosition(cols*ts/2 - 100, 100);
    text.setFillColor(Color::White);
    window.draw(text);

    text.setCharacterSize(35);
    text.setStyle(Text::Bold);
    text.setString("Restart");
    text.setPosition(cols*ts/2 - 50, 200);
    text.setFillColor(Color::Green);
    window.draw(text);

    text.setString("Main Menu");
    text.setPosition(cols*ts/2 - 60, 260);
    text.setFillColor(Color::Yellow);
    window.draw(text);

    text.setString("Exit");
    text.setPosition(cols*ts/2 - 30, 320);
    text.setFillColor(Color::Red);
    window.draw(text);

    text.setString("Click to select");
    text.setCharacterSize(20);
    text.setStyle(Text::Regular);
    text.setPosition(cols*ts/2 - text.getLocalBounds().width/2, rows*ts - 30);
    text.setFillColor(Color::White);
    window.draw(text);
}

// Feature 1: Handles mouse clicks for selecting end menu options (Restart, Main Menu, Exit)
void Handle_End_Clicks(Event& e, RenderWindow& window, int& State, int& x1, int& Y1_Position, int& Dx_1, int& Dy_1, int& Prev_Dx_1, int& Prev_Dy_1, int& x2, int& y2Pos, int& Dx_2, int& Dy_2, int& Prev_Dx_2, int& Prev_Dy_2, int& Score_1, int& Score_2, int Level, int Game_Mode, int& Move_Counter, int& enemyCount, Enemy a[], int& game, bool& Player_1_Alive, bool& Player_2_Alive, Text& text, Clock& Game_Clock, int& Bonus_Count_1, int& Bonus_Count_2, int& Bonus_Minimum_1, int& Bonus_Minimum_2, int& Power_Up_Count_1, int& Power_Up_Count_2, bool& Enemies_Stopped, float& Freeze_Timer, int& Last_Power_Up_Score_1, int& Last_Power_Up_Score_2) {
    if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
        Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));

        if (text.getFont()) {
            text.setFont(*text.getFont());
        }

        text.setCharacterSize(35);
        text.setStyle(Text::Bold);
        text.setString("Restart");
        text.setPosition(cols*ts/2 - 50, 200);
        FloatRect Restart_Bounds = text.getGlobalBounds();
        if (Restart_Bounds.contains(mouse)) {
            Reset_Game(x1, Y1_Position, Dx_1, Dy_1, Prev_Dx_1, Prev_Dy_1, x2, y2Pos, Dx_2, Dy_2, Prev_Dx_2, Prev_Dy_2, Score_1, Score_2, Move_Counter, Level, enemyCount, a, game, Game_Mode, Player_1_Alive, Player_2_Alive, Bonus_Count_1, Bonus_Count_2, Bonus_Minimum_1, Bonus_Minimum_2, Power_Up_Count_1, Power_Up_Count_2, Enemies_Stopped, Freeze_Timer, Last_Power_Up_Score_1, Last_Power_Up_Score_2);
            State = 1;
            Game_Clock.restart();
        }

        text.setString("Main Menu");
        text.setPosition(cols*ts/2 - 60, 260);
        FloatRect Menu_Bounds = text.getGlobalBounds();
        if (Menu_Bounds.contains(mouse)) {
            State = 0;
        }

        text.setString("Exit");
        text.setPosition(cols*ts/2 - 30, 320);
        FloatRect Exit_Bounds = text.getGlobalBounds();
        if (Exit_Bounds.contains(mouse)) {
            window.close();                                      }}}
        
        

// Feature 6: Renders the scoreboard with high scores
void Draw_Scoreboard(RenderWindow& window, Font& font, ScoreEntry scores[], int Score_count, Text& text) {
    text.setFont(font);
    text.setCharacterSize(40);
    text.setString("High Scores");
    text.setPosition(cols*ts/2 - 100, 50);
    text.setFillColor(Color::White);
    window.draw(text);

    char s[200] = "";
    for (int i = 0; i < Score_count; i++) {
        char line[50];
        snprintf(line, sizeof(line), "%d. %d\n", i+1, scores[i].Score);
        strcat(s, line);
    }
    if (Score_count == 0) strcat(s, "No scores yet");
    text.setCharacterSize(30);
    text.setString(s);
    text.setPosition(cols*ts/2 - 100, 100);
    text.setFillColor(Color::White);
    window.draw(text);

    text.setString("Back");
    text.setPosition(cols*ts/2 - 30, 300);
    text.setFillColor(Color::Red);
    window.draw(text);

    text.setString("Click to select");
    text.setCharacterSize(20);
    text.setStyle(Text::Regular);
    text.setPosition(cols*ts/2 - text.getLocalBounds().width/2, rows*ts - 30);
    text.setFillColor(Color::White);
    window.draw(text);
}

// Feature 6: Handles mouse clicks to navigate back from the scoreboard
void Handle_Score_Board_Clicks(Event& e, RenderWindow& window, int& State, Text& text) {
    if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
        Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));

        if (text.getFont()) {
            text.setFont(*text.getFont());
        }

        text.setCharacterSize(30);
        text.setStyle(Text::Regular);
        text.setString("Back");
        text.setPosition(cols*ts/2 - 30, 300);
        FloatRect Back_Bounds = text.getGlobalBounds();
        if (Back_Bounds.contains(mouse)) {
            State = 0;  } } }
        

// Feature 3: Displays the movement counter
void Draw_Move_Counter(RenderWindow& window, Font& font, int Move_Counter, bool game_active) {
    Text move_text;
    move_text.setFont(font);
    move_text.setCharacterSize(22);
    move_text.setStyle(Text::Bold);
    
    std::stringstream ss;
    ss << "Moves: " << Move_Counter;
    
    move_text.setString(ss.str());
    move_text.setPosition(cols*ts/2 - 40, 20);
    
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

// Feature 4, 7: Displays the shared game timer
void Draw_Timer(RenderWindow& window, Font& font, float Game_Time) {
    Text timer_text;
    timer_text.setFont(font);
    timer_text.setCharacterSize(22);
    timer_text.setString("Time: " + std::to_string((int)Game_Time));
    timer_text.setPosition(cols*ts/2 - 40, 50);
    timer_text.setFillColor(Color::White);
    window.draw(timer_text);
}

// Feature 5, 7: Displays individual scores for both players
void Draw_Score(RenderWindow& window, Font& font, int Score_1, int Score_2, int Game_Mode) {
    Text score_text;
    score_text.setFont(font);
    score_text.setCharacterSize(22);
    score_text.setStyle(Text::Bold);
    
    switch (Game_Mode) {
        case 1: {
            std::stringstream ss;
            ss << "Score: " << Score_1;
            score_text.setString(ss.str());
            score_text.setPosition(cols*ts/2 - 40, 80);
            break;
        }
        case 2: {
            std::stringstream ss1;
            ss1 << "P1 Score: " << Score_1;
            score_text.setString(ss1.str());
            score_text.setPosition(20, 20);
            Text shadow = score_text;
            shadow.setFillColor(Color(0, 0, 0, 200));
            for (int offset = 1; offset <= 1; offset++) {
                shadow.setPosition(score_text.getPosition().x + offset, score_text.getPosition().y);
                window.draw(shadow);
                shadow.setPosition(score_text.getPosition().x - offset, score_text.getPosition().y);
                window.draw(shadow);
                shadow.setPosition(score_text.getPosition().x, score_text.getPosition().y + offset);
                window.draw(shadow);
                shadow.setPosition(score_text.getPosition().x, score_text.getPosition().y - offset);
                window.draw(shadow);
            }
            score_text.setFillColor(Color::White);
            window.draw(score_text);
            
            std::stringstream ss2;
            ss2 << "P2 Score: " << Score_2;
            score_text.setString(ss2.str());
            score_text.setPosition(cols*ts - 150, 20);
            break;  } }
        
    
    Text shadow = score_text;
    shadow.setFillColor(Color(0, 0, 0, 200));
    for (int offset = 1; offset <= 1; offset++) {
        shadow.setPosition(score_text.getPosition().x + offset, score_text.getPosition().y);
        window.draw(shadow);
        shadow.setPosition(score_text.getPosition().x - offset, score_text.getPosition().y);
        window.draw(shadow);
        shadow.setPosition(score_text.getPosition().x, score_text.getPosition().y + offset);
        window.draw(shadow);
        shadow.setPosition(score_text.getPosition().x, score_text.getPosition().y - offset);
        window.draw(shadow);
    }
    
    score_text.setFillColor(Color::White);
    window.draw(score_text);
}

// Feature 5, 7: Displays individual power-up counts for both players
void Draw_Power_Ups(RenderWindow& window, Font& font, int Power_Up_Count_1, int Power_Up_Count_2, int Game_Mode) {
    Text power_text;
    power_text.setFont(font);
    power_text.setCharacterSize(22);
    power_text.setStyle(Text::Bold);
    
    switch (Game_Mode) {
        case 1: {
            std::stringstream ss;
            ss << "Power-Ups: " << Power_Up_Count_1;
            power_text.setString(ss.str());
            power_text.setPosition(cols*ts/2 - 40, 110);
            break;
        }
        case 2: {
            std::stringstream ss1;
            ss1 << "P1 Power-Ups: " << Power_Up_Count_1;
            power_text.setString(ss1.str());
            power_text.setPosition(20, 50);
            Text shadow = power_text;
            shadow.setFillColor(Color(0, 0, 0, 200));
            for (int offset = 1; offset <= 1; offset++) {
                shadow.setPosition(power_text.getPosition().x + offset, power_text.getPosition().y);
                window.draw(shadow);
                shadow.setPosition(power_text.getPosition().x - offset, power_text.getPosition().y);
                window.draw(shadow);
                shadow.setPosition(power_text.getPosition().x, power_text.getPosition().y + offset);
                window.draw(shadow);
                shadow.setPosition(power_text.getPosition().x, power_text.getPosition().y - offset);
                window.draw(shadow);
            }
            power_text.setFillColor(Color::Yellow);
            window.draw(power_text);
            
            std::stringstream ss2;
            ss2 << "P2 Power-Ups: " << Power_Up_Count_2;
            power_text.setString(ss2.str());
            power_text.setPosition(cols*ts - 200, 50);
            break;  } }
       
    
    Text shadow = power_text;
    shadow.setFillColor(Color(0, 0, 0, 200));
    for (int offset = 1; offset <= 1; offset++) {
        shadow.setPosition(power_text.getPosition().x + offset, power_text.getPosition().y);
        window.draw(shadow);
        shadow.setPosition(power_text.getPosition().x - offset, power_text.getPosition().y);
        window.draw(shadow);
        shadow.setPosition(power_text.getPosition().x, power_text.getPosition().y + offset);
        window.draw(shadow);
        shadow.setPosition(power_text.getPosition().x, power_text.getPosition().y - offset);
        window.draw(shadow);
    }
    
    power_text.setFillColor(Color::Yellow);
    window.draw(power_text);
}

// Feature 1, 2, 3, 4, 5, 7: Manages game loop, player controls, enemy updates, scoring, power-ups, movement counter, difficulty (Continuous mode), and two-player mode rules (collisions, shared board)
int main() {
    srand(time(0));
    RenderWindow window(VideoMode(cols*ts, rows*ts), "Xonix Game!");
    window.setFramerateLimit(60);

    Music Background_Music;
    if (!Background_Music.openFromFile("BlackTrend_Music_Classic.ogg")) {
        std::cerr << "Failed to load BlackTrend_Music_Classic.ogg\n";
    } else {
        Background_Music.setLoop(true);
        Background_Music.play();
    }

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
    sTile.setColor(Color::White);

    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Font load failed\n";
        return -1;
    }

    Text text;
    text.setFont(font);

    ScoreEntry scores[5];
    int Score_count = 0;
    Clear_Scores();

    int enemyCount = 2;
    Enemy a[10];
    int game = 1;
    int x1 = 10, Y1_Position = 0, Dx_1 = 0, Dy_1 = 0;
    int x2 = cols-2, y2Pos = rows-2, Dx_2 = 0, Dy_2 = 0;
    int Prev_Dx_1 = 0, Prev_Dy_1 = 0, Prev_Dx_2 = 0, Prev_Dy_2 = 0;
    float timer = 0, delay = 0.07;
    Clock Game_Clock_Frame;
    Clock Game_Clock;
    int Score_1 = 0, Score_2 = 0;
    int Move_Counter = 0;
    int Level = 1;
    int Selected_Level = 1;
    int Game_Mode = 2;
    int Selected_Mode = 2;
    float Last_Speed_Increase = 0;
    float Last_Enemy_Add = 0;
    bool Patterns_Activated = false;
    int State = 0;
    bool Player_1_Alive = true, Player_2_Alive = true;
    int Bonus_Count_1 = 0, Bonus_Count_2 = 0;
    int Bonus_Minimum_1 = 10, Bonus_Minimum_2 = 10;
    int Power_Up_Count_1 = 0, Power_Up_Count_2 = 0;
    bool Enemies_Stopped = false;
    float Freeze_Timer = 0.0f;
    int Last_Power_Up_Score_1 = 0, Last_Power_Up_Score_2 = 0;
    int Select_Option = 0;

    RectangleShape Tile(Vector2f(ts, ts));
    Tile.setFillColor(Color::Black);

    RectangleShape Player_1_progess(Vector2f(ts, ts));
    Player_1_progess.setFillColor(Color::Green);

    RectangleShape Player_2_progress(Vector2f(ts, ts));
    Player_2_progress.setFillColor(Color::Blue);

    while (window.isOpen()) {
        float Frame_Time = Game_Clock_Frame.getElapsedTime().asSeconds();
        Event e;
        while (window.pollEvent(e)) {
            switch (e.type) {
                case Event::Closed: {
                    window.close();
                    break;
                }
                case Event::KeyPressed: {
                    switch (State) {
                        case 1: {
                            switch (e.key.code) {
                                case Keyboard::Escape: {
                                    State = 0;
                                    switch (Game_Mode) {
                                        case 1: {
                                            Saving_Score(scores, Score_count, Score_1, Game_Clock.getElapsedTime().asSeconds());
                                            break;
                                        }
                                        case 2: {
                                            Saving_Score(scores, Score_count, Score_1, Game_Clock.getElapsedTime().asSeconds());
                                            Saving_Score(scores, Score_count, Score_2, Game_Clock.getElapsedTime().asSeconds());
                                            break;
                                        }
                                    }
                                    game = 0;
                                    Background_Music.pause();
                                    break;
                                }
                                case Keyboard::L: {
                                    if (Power_Up_Count_1 > 0 && !Enemies_Stopped && (Game_Mode == 1 || (Game_Mode == 2 && Player_1_Alive))) {
                                        Power_Up_Count_1--;
                                        Enemies_Stopped = true;
                                        Freeze_Timer = 3.0f;
                                    }
                                    break;
                                }
                                case Keyboard::Q: {
                                    if (Game_Mode == 2 && Power_Up_Count_2 > 0 && !Enemies_Stopped && Player_2_Alive) {
                                        Power_Up_Count_2--;
                                        Enemies_Stopped = true;
                                        Freeze_Timer = 3.0f;
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                        case 0: {
                            Handle_Start_Input(e, State, Selected_Level, Selected_Mode, Level, Game_Mode, x1, Y1_Position, Dx_1, Dy_1, Prev_Dx_1, Prev_Dy_1, x2, y2Pos, Dx_2, Dy_2, Prev_Dx_2, Prev_Dy_2, Score_1, Score_2, Move_Counter, enemyCount, a, game, Player_1_Alive, Player_2_Alive, Game_Clock, Bonus_Count_1, Bonus_Count_2, Bonus_Minimum_1, Bonus_Minimum_2, Power_Up_Count_1, Power_Up_Count_2, Enemies_Stopped, Freeze_Timer, Last_Power_Up_Score_1, Last_Power_Up_Score_2, Select_Option);
                            if (State == 1) Background_Music.play();
                            break;
                        }
                    }
                    break;
                }
                case Event::MouseButtonPressed: {
                    switch (State) {
                        case 2: {
                            Handle_End_Clicks(e, window, State, x1, Y1_Position, Dx_1, Dy_1, Prev_Dx_1, Prev_Dy_1, x2, y2Pos, Dx_2, Dy_2, Prev_Dx_2, Prev_Dy_2, Score_1, Score_2, Level, Game_Mode, Move_Counter, enemyCount, a, game, Player_1_Alive, Player_2_Alive, text, Game_Clock, Bonus_Count_1, Bonus_Count_2, Bonus_Minimum_1, Bonus_Minimum_2, Power_Up_Count_1, Power_Up_Count_2, Enemies_Stopped, Freeze_Timer, Last_Power_Up_Score_1, Last_Power_Up_Score_2);
                            break;
                        }
                        case 3: {
                            Handle_Score_Board_Clicks(e, window, State, text);
                            break;
                        }
                    }
                    break;  } } }
               

        Game_Clock_Frame.restart();
        float Game_Time = Game_Clock.getElapsedTime().asSeconds();

        if (State == 1 && game) {
            if (Enemies_Stopped) {
                Freeze_Timer -= Frame_Time;
                if (Freeze_Timer <= 0.0f) {
                    Enemies_Stopped = false;
                }
            }

            if (Game_Time - Last_Speed_Increase >= 20) {
                for (int i = 0; i < enemyCount; i++) {
                    a[i].speed += 0.5f;
                }
                Last_Speed_Increase = Game_Time;
            }

            if (Level == 4 && Game_Time - Last_Enemy_Add >= 20 && enemyCount < 10) {
                enemyCount += 2;
                for (int i = enemyCount - 2; i < enemyCount; i++) {
                    a[i] = Enemy();
                }
                Last_Enemy_Add = Game_Time;
            }

            if (!Patterns_Activated && Game_Time >= 30) {
                int halfEnemies = enemyCount / 2;
                for (int i = 0; i < halfEnemies; i++) {
                    a[i].Move_Type = 1;
                }
                Patterns_Activated = true;
            }

            Dx_1 = Dy_1 = Dx_2 = Dy_2 = 0;
            switch (Game_Mode) {
                case 1: {
                    if (Keyboard::isKeyPressed(Keyboard::Left)) { Dx_1 = -1; Dy_1 = 0; }
                    else if (Keyboard::isKeyPressed(Keyboard::Right)) { Dx_1 = 1; Dy_1 = 0; }
                    else if (Keyboard::isKeyPressed(Keyboard::Up)) { Dx_1 = 0; Dy_1 = -1; }
                    else if (Keyboard::isKeyPressed(Keyboard::Down)) { Dx_1 = 0; Dy_1 = 1; }
                    break;
                }
                case 2: {
                    if (Player_1_Alive) {
                        if (Keyboard::isKeyPressed(Keyboard::Left)) { Dx_1 = -1; Dy_1 = 0; }
                        else if (Keyboard::isKeyPressed(Keyboard::Right)) { Dx_1 = 1; Dy_1 = 0; }
                        else if (Keyboard::isKeyPressed(Keyboard::Up)) { Dx_1 = 0; Dy_1 = -1; }
                        else if (Keyboard::isKeyPressed(Keyboard::Down)) { Dx_1 = 0; Dy_1 = 1; }
                    }
                    if (Player_2_Alive) {
                        if (Keyboard::isKeyPressed(Keyboard::A)) { Dx_2 = -1; Dy_2 = 0; }
                        else if (Keyboard::isKeyPressed(Keyboard::D)) { Dx_2 = 1; Dy_2 = 0; }
                        else if (Keyboard::isKeyPressed(Keyboard::W)) { Dx_2 = 0; Dy_2 = -1; }
                        else if (Keyboard::isKeyPressed(Keyboard::S)) { Dx_2 = 0; Dy_2 = 1; }
                    }
                    break;  } }
                

            timer += Frame_Time;
            if (timer > delay) {
                int old_x1 = x1, old_y1 = Y1_Position;
                int old_x2 = x2, old_y2 = y2Pos;

                int new_x1 = x1 + Dx_1;
                int new_y1 = Y1_Position + Dy_1;
                int new_x2 = x2 + Dx_2;
                int new_y2 = y2Pos + Dy_2;

                if (Game_Mode == 2 && Player_1_Alive && Player_2_Alive) {
                    bool p1_moving = (Dx_1 != 0 || Dy_1 != 0) && new_x1 >= 0 && new_x1 < cols && new_y1 >= 0 && new_y1 < rows;
                    bool p2_moving = (Dx_2 != 0 || Dy_2 != 0) && new_x2 >= 0 && new_x2 < cols && new_y2 >= 0 && new_y2 < rows;

                    if (p1_moving && p2_moving) {
                        if (new_x1 == new_x2 && new_y1 == new_y2 && grid[new_y1][new_x1] == 0) {
                            Player_1_Alive = false;
                            Player_2_Alive = false;  }}
                       

                    if (p1_moving && new_x1 == x2 && new_y1 == y2Pos) {
                        bool p1_constructing = (grid[new_y1][new_x1] == 0);
                        bool p2_not_constructing = (grid[y2Pos][x2] == 1 && Dx_2 == 0 && Dy_2 == 0);
                        if (p1_constructing && p2_not_constructing) {
                            Player_1_Alive = false;
                        } else if (p1_constructing && grid[new_y1][new_x1] == 0) {
                            Player_1_Alive = false;
                            Player_2_Alive = false; }}
                        

                    if (p2_moving && new_x2 == x1 && new_y2 == Y1_Position) {
                        bool p2_constructing = (grid[new_y2][new_x2] == 0);
                        bool p1_not_constructing = (grid[Y1_Position][x1] == 1 && Dx_1 == 0 && Dy_1 == 0);
                        if (p2_constructing && p1_not_constructing) {
                            Player_2_Alive = false;
                        } else if (p2_constructing && grid[new_y2][new_x2] == 0) {
                            Player_1_Alive = false;
                            Player_2_Alive = false;  } }}
                       

                if (Game_Mode == 2 && Player_1_Alive && (Dx_1 != 0 || Dy_1 != 0)) {
                    if (new_x1 >= 0 && new_x1 < cols && new_y1 >= 0 && new_y1 < rows) {
                        if (Player_2_Alive && grid[new_y1][new_x1] == 3) {
                            Player_1_Alive = false; }}}
                        
                if (Game_Mode == 2 && Player_2_Alive && (Dx_2 != 0 || Dy_2 != 0)) {
                    if (new_x2 >= 0 && new_x2 < cols && new_y2 >= 0 && new_y2 < rows) {
                        if (Player_1_Alive && grid[new_y2][new_x2] == 2) {
                            Player_2_Alive = false; }}}
                        

                if (Player_1_Alive && (Dx_1 != 0 || Dy_1 != 0)) {
                    if (new_x1 >= 0 && new_x1 < cols && new_y1 >= 0 && new_y1 < rows) {
                        switch (grid[new_y1][new_x1]) {
                            case 0: {
                                if ((Dx_1 != Prev_Dx_1 || Dy_1 != Prev_Dy_1) && grid[Y1_Position][x1] == 2) {
                                    Move_Counter++;
                                }
                                x1 = new_x1;
                                Y1_Position = new_y1;
                                grid[Y1_Position][x1] = 2;
                                Prev_Dx_1 = Dx_1;
                                Prev_Dy_1 = Dy_1;
                                break;
                            }
                            case 1: {
                                x1 = new_x1;
                                Y1_Position = new_y1;
                                Dx_1 = Dy_1 = 0;
                                Prev_Dx_1 = Prev_Dy_1 = 0;

                                int Filled_Count = 0;
                                for (int i = 0; i < enemyCount; i++)
                                    drop(a[i].y/ts, a[i].x/ts, Filled_Count);
                                if (Game_Mode == 2 && Player_2_Alive)
                                    drop(y2Pos, x2, Filled_Count);

                                int Captured_Count = 0;
                                for (int i = 0; i < rows; i++) {
                                    for (int j = 0; j < cols; j++) {
                                        if (grid[i][j] == -1) {
                                            grid[i][j] = 0;
                                        } else if (grid[i][j] != 1) {
                                            grid[i][j] = 1;
                                            Captured_Count++; }}}
                                        

                                int points = Captured_Count;
                                if (Captured_Count > Bonus_Minimum_1) {
                                    Bonus_Count_1++;
                                    switch (Bonus_Count_1) {
                                        case 5: {
                                            points = Captured_Count * 4;
                                            break;
                                        }
                                        default: {
                                            if (Bonus_Count_1 >= 3) {
                                                points = Captured_Count * 2;
                                                Bonus_Minimum_1 = 5;
                                            } else {
                                                points = Captured_Count * 2;
                                            }
                                            break;  }}}
                                        
                                Score_1 += points;

                                switch (true) {
                                    case true: {
                                        if (Score_1 >= 50 && Last_Power_Up_Score_1 < 50) {
                                            Power_Up_Count_1++;
                                            Last_Power_Up_Score_1 = 50;
                                        } else if (Score_1 >= 70 && Last_Power_Up_Score_1 < 70) {
                                            Power_Up_Count_1++;
                                            Last_Power_Up_Score_1 = 70;
                                        } else if (Score_1 >= 100 && Last_Power_Up_Score_1 < 100) {
                                            Power_Up_Count_1++;
                                            Last_Power_Up_Score_1 = 100;
                                        } else if (Score_1 >= 130 && Last_Power_Up_Score_1 < 130) {
                                            Power_Up_Count_1++;
                                            Last_Power_Up_Score_1 = 130;
                                        } else if (Score_1 >= Last_Power_Up_Score_1 + 30 && Last_Power_Up_Score_1 >= 130) {
                                            Power_Up_Count_1++;
                                            Last_Power_Up_Score_1 += 30;
                                        }
                                        break;
                                    }
                                }
                                break;  }}}}
                            

                if (Game_Mode == 2 && Player_2_Alive && (Dx_2 != 0 || Dy_2 != 0)) {
                    if (new_x2 >= 0 && new_x2 < cols && new_y2 >= 0 && new_y2 < rows) {
                        switch (grid[new_y2][new_x2]) {
                            case 0: {
                                if ((Dx_2 != Prev_Dx_2 || Dy_2 != Prev_Dy_2) && grid[y2Pos][x2] == 3) {
                                    Move_Counter++;
                                }
                                x2 = new_x2;
                                y2Pos = new_y2;
                                grid[y2Pos][x2] = 3;
                                Prev_Dx_2 = Dx_2;
                                Prev_Dy_2 = Dy_2;
                                break;
                            }
                            case 1: {
                                x2 = new_x2;
                                y2Pos = new_y2;
                                Dx_2 = Dy_2 = 0;
                                Prev_Dx_2 = Prev_Dy_2 = 0;

                                int Filled_Count = 0;
                                for (int i = 0; i < enemyCount; i++)
                                    drop(a[i].y/ts, a[i].x/ts, Filled_Count);
                                if (Player_1_Alive)
                                    drop(Y1_Position, x1, Filled_Count);

                                int Captured_Count = 0;
                                for (int i = 0; i < rows; i++) {
                                    for (int j = 0; j < cols; j++) {
                                        if (grid[i][j] == -1) {
                                            grid[i][j] = 0;
                                        } else if (grid[i][j] != 1) {
                                            grid[i][j] = 1;
                                            Captured_Count++;  }}}
                                        

                                int points = Captured_Count;
                                if (Captured_Count > Bonus_Minimum_2) {
                                    Bonus_Count_2++;
                                    switch (Bonus_Count_2) {
                                        case 5: {
                                            points = Captured_Count * 4;
                                            break;
                                        }
                                        default: {
                                            if (Bonus_Count_2 >= 3) {
                                                points = Captured_Count * 2;
                                                Bonus_Minimum_2 = 5;
                                            } else {
                                                points = Captured_Count * 2;
                                            }
                                            break;  }}}
                                        
                                Score_2 += points;

                                switch (true) {
                                    case true: {
                                        if (Score_2 >= 50 && Last_Power_Up_Score_2 < 50) {
                                            Power_Up_Count_2++;
                                            Last_Power_Up_Score_2 = 50;
                                        } else if (Score_2 >= 70 && Last_Power_Up_Score_2 < 70) {
                                            Power_Up_Count_2++;
                                            Last_Power_Up_Score_2 = 70;
                                        } else if (Score_2 >= 100 && Last_Power_Up_Score_2 < 100) {
                                            Power_Up_Count_2++;
                                            Last_Power_Up_Score_2 = 100;
                                        } else if (Score_2 >= 130 && Last_Power_Up_Score_2 < 130) {
                                            Power_Up_Count_2++;
                                            Last_Power_Up_Score_2 = 130;
                                        } else if (Score_2 >= Last_Power_Up_Score_2 + 30 && Last_Power_Up_Score_2 >= 130) {
                                            Power_Up_Count_2++;
                                            Last_Power_Up_Score_2 += 30;
                                        }
                                        break;
                                    }
                                }
                                break;  }}}}
                           

                timer = 0;
            }

            if (!Enemies_Stopped) {
                for (int i = 0; i < enemyCount; i++) {
                    a[i].move();
                    if (Player_1_Alive) {
                        if (a[i].x >= x1*ts && a[i].x < (x1+1)*ts && a[i].y >= Y1_Position*ts && a[i].y < (Y1_Position+1)*ts) {
                            Player_1_Alive = false;
                        }
                    }
                    if (Game_Mode == 2 && Player_2_Alive) {
                        if (a[i].x >= x2*ts && a[i].x < (x2+1)*ts && a[i].y >= y2Pos*ts && a[i].y < (y2Pos+1)*ts) {
                            Player_2_Alive = false;
                        }
                    }
                    int enemyGridX = static_cast<int>(a[i].x/ts);
                    int enemyGridY = static_cast<int>(a[i].y/ts);
                    if (enemyGridX >= 0 && enemyGridX < cols && enemyGridY >= 0 && enemyGridY < rows) {
                        switch (grid[enemyGridY][enemyGridX]) {
                            case 2: {
                                Player_1_Alive = false;
                                break;
                            }
                            case 3: {
                                Player_2_Alive = false;
                                break;  }}}}}
                            

            switch (Game_Mode) {
                case 1: {
                    if (!Player_1_Alive) {
                        game = 0;
                        State = 2;
                        Saving_Score(scores, Score_count, Score_1, Game_Clock.getElapsedTime().asSeconds());
                    }
                    break;
                }
                case 2: {
                    if (!Player_1_Alive && !Player_2_Alive) {
                        game = 0;
                        State = 2;
                        Saving_Score(scores, Score_count, Score_1, Game_Clock.getElapsedTime().asSeconds());
                        Saving_Score(scores, Score_count, Score_2, Game_Clock.getElapsedTime().asSeconds());
                    }
                    break;  }}}
                

        window.clear();

        switch (State) {
            case 0: {
                Draw_Start_Meanu(window, font, Selected_Level, Selected_Mode, text, Select_Option, Game_Time);
                break;
            }
            case 1: {
                for (int i = 0; i < rows; i++)
                    for (int j = 0; j < cols; j++) {
                        switch (grid[i][j]) {
                            case 0: {
                                Tile.setPosition(j*ts, i*ts);
                                window.draw(Tile);
                                break;
                            }
                            case 1: {
                                sTile.setTextureRect(IntRect(ts, 0, ts, ts));
                                sTile.setPosition(j*ts, i*ts);
                                window.draw(sTile);
                                break;
                            }
                            case 2:
                            case 3: {
                                sTile.setTextureRect(IntRect(2*ts, 0, ts, ts));
                                sTile.setPosition(j*ts, i*ts);
                                window.draw(sTile);
                                break;  }}}
                           

                for (int i = 0; i < enemyCount; i++) {
                    sEnemy.setPosition(a[i].x, a[i].y);
                    window.draw(sEnemy);
                }

                if (Player_1_Alive) {
                    Player_1_progess.setPosition(x1*ts, Y1_Position*ts);
                    window.draw(Player_1_progess);
                }
                if (Game_Mode == 2 && Player_2_Alive) {
                    Player_2_progress.setPosition(x2*ts, y2Pos*ts);
                    window.draw(Player_2_progress);
                }

                Draw_Move_Counter(window, font, Move_Counter, true);
                Draw_Timer(window, font, Game_Time);
                Draw_Score(window, font, Score_1, Score_2, Game_Mode);
                Draw_Power_Ups(window, font, Power_Up_Count_1, Power_Up_Count_2, Game_Mode);
                break;
            }
            case 2: {
                window.draw(sGameover);
                Draw_Move_Counter(window, font, Move_Counter, false);
                Draw_Score(window, font, Score_1, Score_2, Game_Mode);
                Draw_End_Menu(window, font, Score_1, Score_2, Game_Mode, scores, Score_count, text);
                break;
            }
            case 3: {
                Loading_Scores(scores, Score_count);
                Draw_Scoreboard(window, font, scores, Score_count, text);
                break;
            }
        }

        window.display();
    }

    return 0;
}