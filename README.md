# Xonix-Game-
This is the Xonix game in whiich 1 or 2 players have to fill the tiles by tackling enemies

  
#Overall Project Workflow
The development of the Xonix game,  our programming Fundamentals coarse  was executed through a structured, iterative workflow using C++ Programming Language  and the SFML library. The process spanned from  requirement analysis, design, implementation, testing, and documentation to deliver a feature rich game. Required features included single and two-player modes, interactive start and end menus, varied difficulty levels with dynamic enemy counts, a movement counter, enhanced enemy speed and movement patterns, a scoring system with power-up rewards, a file handling based scoreboard, and specific two-player interaction rules. The workflow for each feature is detailed below, highlighting the development approach, challenges faced, and resolutions achieved through hit-and-trial methods.

#Start And End Menu 
The end menu was designed to display final scores, highlight new high scores, and offer options to restart, return to the main menu, or exit, using mouse-based selection for user interaction. Our  team focused on integrating score display with the scoreboard system and implementing click detection. A major challenge that we faced was with inaccurate click detection, as initial hitbox calculations misregistered inputs, causing missed selections. Through hit-and-trial, the team iteratively refined bounding box coordinates, testing clicks across various screen regions to ensure precision. Synchronizing high score highlights required multiple tests to prevent text formatting errors, resulting in a responsive and polished interface.
The start menu was designed to provide an intuitive interface for players to start the game, select difficulty levels, or view the scoreboard, featuring a gradient background and arrow key navigation. Our team focused on creating a visually appealing layout and robust navigation logic. A major challenge was handling rapid key presses, which caused the selection cursor to skip options unpredictably, frustrating users. Through hit-and-trial, the team implemented a debouncing mechanism, testing various delay intervals to ensure smooth and acc


#Movement Counter Workflow
The movement counter tracked player moves, defined as directional changes while constructing tiles, to enhance gameplay feedback. The team initialized a counter variable, incrementing it when players altered their movement direction. Early versions overcounted moves due to continuous key press detection, inflating the counter during sustained inputs. Through hit-and-trial, the team refined the logic to increment only on direction changes, testing various conditions for accuracy. Displaying the counter using SFML posed alignment issues, resolved by iteratively adjusting text coordinates and font sizes to achieve a centered, clear presentation.

Scoring and Reward System Workflow
The scoring system awarded 1 point per captured tile, with bonuses for capturing over 10 tiles (double points) and further rewards after multiple bonuses (threshold reduced to 5 tiles after 3 bonuses, quadruple points after 5). Power-ups were granted at scores of 50, 70, 100, 130, and every 30 points thereafter. A difficulty was preventing incorrect bonus point calculations during large captures, as early logic applied multipliers multiple times. Through trial and error, the team isolated bonus calculations within the capture loop, testing edge cases to ensure accurate scoring and reliable power-up distribution.

#Two-Player Mode
The two-player mode enabled simultaneous play on a shared game board, with Player 1 using WASD keys and Player 2 using arrow keys, each with individual scores and power-ups. The team extended single-player mechanics to handle dual inputs and implemented collision rules, such as a constructing player dying when hitting another’s tiles. Initial collision checks were overly aggressive, causing unintended deaths during minor overlaps. The team experimented with refined boundary conditions, conducting multiple playtests to ensure fair rules, penalizing only the constructing player, thus balancing competitive gameplay.

#Difficulty and Enemy Count 
The game supported four difficulty modes—Easy (2 enemies), Medium (4 enemies), Hard (6 enemies), and Continuous (starting with 2 enemies, adding 2 every 20 seconds)—to cater to varying skill levels. The team configured enemy counts during game initialization and used a timer for Continuous mode additions. Balancing Continuous mode was challenging, as frequent enemy spawns overwhelmed players. The team tested different spawn intervals, settling on 20 seconds after iterative playtests. Preventing enemy overlap at spawn required adjusting randomization logic, achieved through repeated trials for seamless integration.

#Enemy Speed and Movement 
Enemy dynamics were enhanced by increasing speed every 20 seconds and switching half the enemies to a zig-zag pattern after 30 seconds, using dedicated functions. Early zig-zag patterns were unstable, causing enemies to jitter or bypass tiles, disrupting gameplay. Through hit-and-trial, the team introduced step-based tracking to stabilize the pattern, testing various step counts for smoothness. Speed increments risked making the game unplayable, so the team iteratively tested modest increases, validated through playtesting to maintain a challenging yet fair experience.

#Scoreboard 
The file-based scoreboard stored the top 5 scores and their times, updating automatically for new high scores. The team developed functions to manage file input/output, but initial sorting attempts misaligned scores and times, leading to display errors. Through trial and error, the team implemented a stable sorting mechanism, testing file operations to ensure data integrity. Displaying scores on-screen required careful text alignment to avoid overlaps, achieved through iterative coordinate adjustments, resulting in a clear and professional presentation.

#GAME   DEVELOPMENT   APPROACH 

We approached the game feature by feature , First , We developed the scoring and reward system to award points for tiles and power-ups at score milestones. We also tackled the scoreboard to store and display top scores with times. The start and end menus were created for game navigation, difficulty selection, and score display with restart options. We implemented enemy speed and movement, adding timed speed increases and zig-zag patterns. Additionally, we built two-player mode rules for shared gameplay with collision mechanics. We addressed difficulty and enemy count, introducing modes like Continuous with increasing enemies. Finally, we incorporated the movement counter to track player directional changes, ensuring a cohesive game experience.



