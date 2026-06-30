#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include "External/tinyxml2.h"

#include "constants.h"
#include "structs.h"
#include "score.h"
#include "tilemap.h"
#include "player.h"
#include "enemy.h"
#include "bigE.h"
#include "menu.h"

using namespace std;
using namespace tinyxml2;

// camera
void CameraUpdate(Camera& cam, sf::View& gameView,
    float targetX, float targetY, float dt,
    int mapWidth, int mapHeight, int tileWidth, int tileHeight)
{
    float desiredX = targetX + cam.offsetX;
    float desiredY = targetY + cam.offsetY;
    cam.x += (desiredX - cam.x) * cam.smoothSpeed * dt;
    cam.y += (desiredY - cam.y) * cam.smoothSpeed * dt;

    float halfW = cam.width / 2, halfH = cam.height / 2;
    float mapPW = mapWidth * tileWidth;
    float mapPH = mapHeight * tileHeight;
    cam.x = max(halfW, min(cam.x, mapPW - halfW));
    cam.y = max(halfH, min(cam.y, mapPH - halfH));
    gameView.setCenter(cam.x, cam.y);
}

void UpdateIntro(IntroText& intro, float dt)
{
    if (intro.finished) return;
    intro.timer += dt;
    while (intro.timer >= intro.charDelay)
    {
        intro.timer -= intro.charDelay;
        intro.index++;
        if (intro.index >= (int)intro.fullText.size())
        {
            intro.index = intro.fullText.size();
            intro.finished = true;
            break;
        }
    }
    intro.visibleText = intro.fullText.substr(0, intro.index);
}

void drawIntro(sf::RenderWindow& window, sf::Text& introSfText,
    const IntroText& intro)
{
    window.setView(window.getDefaultView());
    introSfText.setString(intro.visibleText);
    window.draw(introSfText);
}

void drawInstructions(sf::RenderWindow& window,
    const sf::Sprite& instructionsSprite)
{
    window.setView(window.getDefaultView());
    window.draw(instructionsSprite);
}

// draw a centered loading screen 
static void showLoadingScreen(sf::RenderWindow& window, const sf::Font& font,
    const std::string& message = "Loading...")
{
    window.clear(sf::Color(4, 2, 12));

    // Outer glow panel
    sf::RectangleShape panel(sf::Vector2f(500.f, 160.f));
    panel.setFillColor(sf::Color(8, 4, 20, 220));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(140, 60, 255, 200));
    panel.setPosition(1920.f * 0.5f - 250.f, 1080.f * 0.5f - 80.f);
    window.draw(panel);

    sf::Text txt;
    txt.setFont(font);
    txt.setString(message);
    txt.setCharacterSize(52);
    txt.setFillColor(sf::Color(210, 170, 255));
    sf::FloatRect tb = txt.getLocalBounds();
    txt.setOrigin(tb.left + tb.width * 0.5f, tb.top + tb.height * 0.5f);
    txt.setPosition(1920.f * 0.5f, 1080.f * 0.5f);
    window.draw(txt);

    window.display();
}

int main()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktop, "Whispering Abyss", sf::Style::Fullscreen);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);

    sf::Font font;
    if (!font.loadFromFile(assetsPath + "font/arial.ttf")) cout << "error loading font\n";

    showLoadingScreen(window, font);

    GameScene currentScene = SCENE_INTRO;
    IntroText intro;
    intro.fullText =
        "Beneath the surface lies a forgotten city...\n"
        "A place where shadows wander, and silence hides a terrible truth.\n"
        "This city was not always silent.\n\n"
        "Once, it lived.\n"
        "Now, only shadows remain... and the echoes of stolen souls.\n"
        "The souls of its people have been stolen, trapped within ancient stone.\n"
        "A malevolent force binds them to stone, watching, waiting.\n\n"
        "you should not be here.\n\n"
        "but you are.\n"
        "you are the one who dared to enter.\n\n"
        "And now you must choose.\n"
        "break the curse... or become part of it.\n"
        "you are the one who decides the fate of those who can no longer choose.\n\n"
        "Press Space to continue.";

    intro.visibleText = "";
    intro.charDelay = 0.05f;
    intro.timer = 0.f;
    intro.index = 0;
    intro.finished = false;

    sf::Text introSfText;
    introSfText.setFont(font);
    introSfText.setCharacterSize(24);
    introSfText.setFillColor(sf::Color(220, 220, 220));
    introSfText.setPosition(80.f, 80.f);
    introSfText.setOrigin(0.f, 0.f);
    introSfText.setLineSpacing(1.1f);
    introSfText.setLetterSpacing(1.05f);

    // inst. pic.
    sf::Texture instructionsTexture;
    instructionsTexture.loadFromFile(assetsPath + "instructions.png");
    sf::Sprite instructionsSprite;
    instructionsSprite.setTexture(instructionsTexture);
    instructionsSprite.setPosition(0.f, 0.f);
    bool spacePressedLast = false;

    // Audio
    sf::SoundBuffer chooseBuffer, enterBuffer, spikeBuffer, swordBuffer;

    if (!spikeBuffer.loadFromFile(assetsPath + "audio/spikeSound.ogg")) cout << "error loading spike sound\n";
    if (!enterBuffer.loadFromFile(assetsPath + "audio/enter2.ogg"))  cout << "error loading enter2.ogg\n";
    if (!chooseBuffer.loadFromFile(assetsPath + "audio/choose.ogg"))  cout << "error loading choose.ogg\n";
    if (!swordBuffer.loadFromFile(assetsPath + "audio/sword_Sound.ogg"))  cout << "error loading sword.ogg\n";

    sf::Sound click_enter, menu_sound, spike_sound, sword_sound;
    click_enter.setBuffer(enterBuffer);
    menu_sound.setBuffer(chooseBuffer);
    spike_sound.setBuffer(spikeBuffer);
    sword_sound.setBuffer(swordBuffer);

    sf::Music backgroundmusic, gameOverMusic, gameMusic;
    if (!backgroundmusic.openFromFile(assetsPath + "audio/Music.wav"))
        cout << "error loading music\n";
    if (!gameOverMusic.openFromFile(assetsPath + "audio/Music.wav"))
        cout << "error loading game over music\n";
    if (!gameMusic.openFromFile(assetsPath + "audio/gameMusic.wav"))
        cout << "error loading game music\n";
    backgroundmusic.setLoop(true);
    gameMusic.setLoop(true);
    gameOverMusic.setLoop(true);
    backgroundmusic.setVolume(100);
    gameMusic.setVolume(100);
    gameOverMusic.setVolume(100);
    backgroundmusic.play();

    // Backgrounds
    sf::Texture bgTextures[3];
    if (!bgTextures[0].loadFromFile(assetsPath + "texture/backgroundd.jpg")) cout << "error bg\n";
    if (!bgTextures[1].loadFromFile(assetsPath + "texture/setting1.jpg"))    cout << "error setting bg\n";
    if (!bgTextures[2].loadFromFile(assetsPath + "texture/playing.jpg"))     cout << "error playing bg\n";

    sf::Sprite background_spr(bgTextures[0]);
    sf::Sprite background_sprit(bgTextures[1]);
    sf::Sprite background_playing(bgTextures[2]);
    background_spr.setScale(1920.f / bgTextures[0].getSize().x, 1080.f / bgTextures[0].getSize().y);
    background_sprit.setScale(1920.f / bgTextures[1].getSize().x, 1080.f / bgTextures[1].getSize().y);
    background_playing.setScale(1920.f / bgTextures[2].getSize().x, 1080.f / bgTextures[2].getSize().y);

    // Cursor sword
    sf::Texture swordTex;
    if (!swordTex.loadFromFile(assetsPath + "texture/sword.png")) cout << "error loading sword\n";
    sf::Sprite sword_mouse(swordTex);

    // Main menu click bounds
    const int   MAIN_ITEM_COUNT = 5;
    const float MAIN_X = 120.f, MAIN_START_Y = 440.f, MAIN_STEP = 106.f;
    const float MAIN_W = 620.f, MAIN_H = 90.f;

    auto mainItemBounds = [&](int i) -> sf::FloatRect {
        return sf::FloatRect(MAIN_X, MAIN_START_Y + i * MAIN_STEP, MAIN_W, MAIN_H);
        };

    // ── Settings click bounds 
    const float SET_X = 1920.f * 0.5f - 360.f, SET_START_Y = 300.f, SET_STEP = 170.f;
    const float SET_W = 720.f, SET_H = 110.f;

    auto settingItemBounds = [&](int i) -> sf::FloatRect {
        return sf::FloatRect(SET_X, SET_START_Y + i * SET_STEP, SET_W, SET_H);
        };

    // ── Character portrait click bounds 
    const float CARD_W = 520.f, CARD_H = 680.f, CARD_Y = 200.f;
    const float CARD_X[2] = { 1920.f * 0.5f - CARD_W - 60.f, 1920.f * 0.5f + 60.f };

    auto cardBounds = [&](int i) -> sf::FloatRect {
        return sf::FloatRect(CARD_X[i], CARD_Y, CARD_W, CARD_H);
        };

    sf::Text           menu[MENU_COUNT];
    sf::RectangleShape menuRects[MENU_COUNT];
    string options[] = { "Play", "Settings", "Leaderboard", "Credits", "Exit" };
    for (int i = 0; i < MENU_COUNT; i++) {
        menuRects[i].setSize(sf::Vector2f(MAIN_W, MAIN_H));
        menuRects[i].setPosition(MAIN_X, MAIN_START_Y + i * MAIN_STEP);
        menu[i].setFont(font);
        menu[i].setString(options[i]);
        menu[i].setCharacterSize(50);
        sf::FloatRect tr = menu[i].getLocalBounds();
        menu[i].setOrigin(tr.left + tr.width / 2.f, tr.top + tr.height / 2.f);
        menu[i].setPosition(menuRects[i].getPosition().x + menuRects[i].getSize().x / 2.f,
            menuRects[i].getPosition().y + menuRects[i].getSize().y / 2.f);
    }

    // Settings 
    sf::Text           setting_txt[3];
    sf::RectangleShape settingRects[3];
    for (int i = 0; i < 3; i++) {
        settingRects[i].setSize(sf::Vector2f(SET_W, SET_H));
        settingRects[i].setPosition(SET_X, SET_START_Y + i * SET_STEP);
        setting_txt[i].setFont(font);
        setting_txt[i].setCharacterSize(45);
    }

    // Pause / Game Over overlays
    sf::RectangleShape pauseOverlay(sf::Vector2f(1920.f, 1080.f));
    pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));

    sf::Text pause_txt;
    pause_txt.setFont(font);
    pause_txt.setString("      Game Paused  \n Press Enter to Resume");
    pause_txt.setCharacterSize(80);
    pause_txt.setFillColor(sf::Color::White);
    {
        sf::FloatRect r = pause_txt.getLocalBounds();
        pause_txt.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        pause_txt.setPosition(1900 / 2.f, 1024 / 2.f);
    }

    sf::Text GameOver_txt;
    GameOver_txt.setFont(font);
    GameOver_txt.setString("   GAME OVER \n Press R to Restart\n Press Escape for Menu");
    GameOver_txt.setCharacterSize(80);
    GameOver_txt.setFillColor(sf::Color::Red);
    {
        sf::FloatRect r = GameOver_txt.getLocalBounds();
        GameOver_txt.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        GameOver_txt.setPosition(1900 / 2.f, 1024 / 2.f);
    }

    // Game state flags
    int    selectedItem = 0;
    bool   isPlaying = false, pressoption = false, isPaused = false;
    bool   isSelectingCharacter = false;
    int    selectedCharacter = 0;
    bool   isEnteringName = false;
    string playerName = "";
    float  gameTimer = 0.f;
    bool   timerRunning = false, isWin = false, showLeaderboard = false;
    bool   isGameOver = false;
    bool   showCredits = false;
    bool gameOverMusicPlayed = false;
    bool lastIsPlaying = false;

    // Character select
    sf::Texture selectBackground;
    if (!selectBackground.loadFromFile(assetsPath + "selectbackground.png"))
        cout << "error loading selectbackground\n";
    sf::Sprite selectBg(selectBackground);
    selectBg.setScale(1920.f / selectBackground.getSize().x,
        1080.f / selectBackground.getSize().y);

    sf::Texture charIdleTex[2];
    if (!charIdleTex[0].loadFromFile(assetsPath + "Shinobi/Idle.png")) cout << "error Shinobi idle\n";
    if (!charIdleTex[1].loadFromFile(assetsPath + "Samurai/Idle.png")) cout << "error Samurai idle\n";

    sf::Sprite charPortrait[2];
    float  portraitX[2] = { 300.f, 1100.f };
    for (int i = 0; i < 2; i++) {
        charPortrait[i].setTexture(charIdleTex[i]);
        charPortrait[i].setTextureRect(sf::IntRect(0, 0, 128, 128));
        charPortrait[i].setScale(3.f, 3.f);
        charPortrait[i].setPosition(portraitX[i], 300.f);
    }

    sf::RectangleShape selectBorder(sf::Vector2f(128.f * 3.f + 20.f, 128.f * 3.f + 20.f));
    selectBorder.setFillColor(sf::Color::Transparent);
    selectBorder.setOutlineThickness(5.f);
    selectBorder.setOutlineColor(sf::Color::White);

    string charNames[2] = { "Shinobi", "Samurai" };
    sf::Text charLabel[2];
    for (int i = 0; i < 2; i++) {
        charLabel[i].setFont(font);
        charLabel[i].setString(charNames[i]);
        charLabel[i].setCharacterSize(45);
        charLabel[i].setFillColor(sf::Color::White);
        sf::FloatRect lb = charLabel[i].getLocalBounds();
        charLabel[i].setOrigin(lb.width / 2.f, 0.f);
        charLabel[i].setPosition(portraitX[i] + (128.f * 3.f) / 2.f, 300.f + 128.f * 3.f + 20.f);
    }

    sf::Text selectPrompt;
    selectPrompt.setFont(font);
    selectPrompt.setString("Press Enter or Click to Select");
    selectPrompt.setCharacterSize(35);
    selectPrompt.setFillColor(sf::Color(200, 200, 200));
    {
        sf::FloatRect r = selectPrompt.getLocalBounds();
        selectPrompt.setOrigin(r.width / 2.f, 0.f);
        selectPrompt.setPosition(1920.f / 2.f, 900.f);
    }

    // Name entry
    sf::Text namePrompt, nameDisplay, nameInstructions;
    namePrompt.setFont(font);
    namePrompt.setString("Enter Your Name");
    namePrompt.setCharacterSize(60);
    namePrompt.setFillColor(sf::Color::White);
    {
        sf::FloatRect r = namePrompt.getLocalBounds();
        namePrompt.setOrigin(r.width / 2.f, 0.f);
        namePrompt.setPosition(1920.f / 2.f, 300.f);
    }
    nameDisplay.setFont(font);
    nameDisplay.setCharacterSize(70);
    nameDisplay.setFillColor(sf::Color::Yellow);

    nameInstructions.setFont(font);
    nameInstructions.setString("Press Enter to confirm   |   Backspace to delete");
    nameInstructions.setCharacterSize(30);
    nameInstructions.setFillColor(sf::Color(200, 200, 200));
    {
        sf::FloatRect r = nameInstructions.getLocalBounds();
        nameInstructions.setOrigin(r.width / 2.f, 0.f);
        nameInstructions.setPosition(1920.f / 2.f, 620.f);
    }

    // HUD
    sf::Font TimerFont;
    if (!TimerFont.loadFromFile(assetsPath + "font/JimNightshade.ttf")) cout << "error loading TimerFont\n";
    sf::Text timerText;
    timerText.setFont(TimerFont);
    timerText.setString("00:00.00");
    timerText.setCharacterSize(60);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(1920.f - 220.f, 10.f);

    sf::Text winText;
    winText.setFont(font);
    winText.setString("YOU WIN!");
    winText.setCharacterSize(120);
    winText.setFillColor(sf::Color::Yellow);
    {
        sf::FloatRect r = winText.getLocalBounds();
        winText.setOrigin(r.width / 2.f, r.height / 2.f);
        winText.setPosition(1920.f / 2.f, 300.f);
    }

    sf::Text winPrompt;
    winPrompt.setFont(font);
    winPrompt.setString("Press Enter to see Leaderboard");
    winPrompt.setCharacterSize(38);
    winPrompt.setFillColor(sf::Color::White);
    {
        sf::FloatRect r = winPrompt.getLocalBounds();
        winPrompt.setOrigin(r.width / 2.f, 0.f);
        winPrompt.setPosition(1920.f / 2.f, 500.f);
    }

    sf::Text lbInstructions;
    lbInstructions.setFont(font);
    lbInstructions.setString("Press Escape to return to Main Menu");
    lbInstructions.setCharacterSize(30);
    lbInstructions.setFillColor(sf::Color(200, 200, 200));
    {
        sf::FloatRect r = lbInstructions.getLocalBounds();
        lbInstructions.setOrigin(r.width / 2.f, 0.f);
        lbInstructions.setPosition(1920.f / 2.f, 980.f);
    }

    // Player spritesheets
    sf::Texture texIdle, texRun, texJump, texHurt, texDead;
    sf::Texture texAttack1, texAttack2, texAttack3;

    int         currentFrame = 0;
    float       animTimer = 0.f;

    enum PlayerState { IDLE, RUN, JUMP, HURT, ATTACK, DEAD };
    PlayerState playerState = IDLE;
    bool isAttacking = false, attackHitDealt = false;

    // Enemy textures
    sf::Texture eTexAttack[E_ATTACK_FRAMES];
    sf::Texture eTexDeath[E_DEATH_FRAMES];
    sf::Texture eTexHurt[E_HURT_FRAMES];
    sf::Texture eTexIdle[E_IDLE_FRAMES];
    sf::Texture eTexWalk[E_WALK_FRAMES];

    sf::Texture* eTexArrays[] = { eTexAttack, eTexDeath, eTexHurt, eTexIdle, eTexWalk };
    int          eTexCounts[] = { E_ATTACK_FRAMES, E_DEATH_FRAMES, E_HURT_FRAMES, E_IDLE_FRAMES, E_WALK_FRAMES };
    string       eTexPrefixes[] = { "Attack", "Death", "Hurt", "Idle", "Walk" };
    for (int a = 0; a < 5; a++)
        for (int f = 0; f < eTexCounts[a]; f++)
            eTexArrays[a][f].loadFromFile(
                assetsPath + "lizard/" + eTexPrefixes[a] + to_string(f + 1) + ".png");

    // Heart texture
    sf::Texture heartTexture;
    if (!heartTexture.loadFromFile(assetsPath + "heart.png")) {
        cerr << "Failed to load heart.png!\n"; return -1;
    }
    sf::Texture* heartTexPtr = &heartTexture;

    // Leaderboard background
    sf::Texture bgLeaderboard;
    if (!bgLeaderboard.loadFromFile(assetsPath + "texture/leaderboard.png"))
        cout << "error leaderboard bg\n";
    sf::Sprite background_leaderboard(bgLeaderboard);
    background_leaderboard.setScale(1920.f / bgLeaderboard.getSize().x,
        1080.f / bgLeaderboard.getSize().y);

    // Load map
    tinyxml2::XMLDocument tmx;
    if (tmx.LoadFile((assetsPath + "map.tmx").c_str()) != tinyxml2::XML_SUCCESS) {
        cerr << "Failed to load map.tmx\n"; return -1;
    }
    XMLElement* mapEl = tmx.FirstChildElement("map");
    int mapWidth = mapEl->IntAttribute("width");
    int mapHeight = mapEl->IntAttribute("height");
    int tileWidth = mapEl->IntAttribute("tilewidth");
    int tileHeight = mapEl->IntAttribute("tileheight");

    vector<TilesetInfo> tilesets;
    for (XMLElement* tsEl = mapEl->FirstChildElement("tileset"); tsEl;
        tsEl = tsEl->NextSiblingElement("tileset"))
    {
        TilesetInfo info;
        info.firstGid = tsEl->IntAttribute("firstgid");
        string tsxFile = tsEl->Attribute("source");
        if (tsxFile.find("BG1") != string::npos) info.type = "background";
        else if (tsxFile.find("obst") != string::npos) info.type = "obstacle";
        else if (tsxFile.find("flag") != string::npos) info.type = "checkpoint";
        else                                            info.type = "ground";

        tinyxml2::XMLDocument tsx;
        tsx.LoadFile((assetsPath + tsxFile).c_str());
        string imgSrc = tsx.FirstChildElement("tileset")
            ->FirstChildElement("image")->Attribute("source");
        size_t lastSlash = imgSrc.find_last_of("/\\");
        string imgFile = (lastSlash != string::npos) ? imgSrc.substr(lastSlash + 1) : imgSrc;
        if (!info.texture.loadFromFile(assetsPath + imgFile))
            cerr << "Failed to load texture: " << assetsPath + imgFile << "\n";
        tilesets.push_back(std::move(info));
    }

    for (size_t i = 0; i < tilesets.size(); ++i)
        tilesets[i].lastGid = tilesets[i].firstGid;

    vector<int> tiles(mapWidth * mapHeight, 0);
    for (XMLElement* layerEl = mapEl->FirstChildElement("layer"); layerEl;
        layerEl = layerEl->NextSiblingElement("layer"))
    {
        XMLElement* dataEl = layerEl->FirstChildElement("data");
        if (!dataEl || !dataEl->GetText()) continue;
        istringstream ss(dataEl->GetText());
        string token;
        for (int idx = 0; getline(ss, token, ',') && idx < (int)tiles.size(); ++idx) {
            token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
            if (!token.empty()) try { int g = stoi(token); if (g) tiles[idx] = g; }
            catch (...) {}
        }
    }

    vector<sf::Sprite> tileSprites;
    for (int i = 0; i < (int)tiles.size(); ++i) {
        int gid = tiles[i];
        if (!gid) continue;
        TilesetInfo* owner = nullptr;
        for (auto& ts : tilesets)
            if (gid >= ts.firstGid && gid <= ts.lastGid) { owner = &ts; break; }
        if (!owner) continue;
        sf::Sprite sprite(owner->texture);
        auto sz = owner->texture.getSize();
        sprite.setScale((float)tileWidth / sz.x, (float)tileHeight / sz.y);
        sprite.setPosition(float(i % mapWidth) * tileWidth, float(i / mapWidth) * tileHeight);
        tileSprites.push_back(sprite);
    }

    

    // Spawn points
    vector<sf::Vector2f> enemySpawnPoints;
    sf::Vector2f bigESpawn(0.f, 0.f);
    for (XMLElement* objGroup = mapEl->FirstChildElement("objectgroup"); objGroup;
        objGroup = objGroup->NextSiblingElement("objectgroup"))
    {
        string layerName = objGroup->Attribute("name") ? objGroup->Attribute("name") : "";
        if (layerName == "enemies") {
            for (XMLElement* obj = objGroup->FirstChildElement("object"); obj;
                obj = obj->NextSiblingElement("object"))
                enemySpawnPoints.push_back({ obj->FloatAttribute("x"), obj->FloatAttribute("y") });
        }
        else if (layerName == "Big E") {
            XMLElement* obj = objGroup->FirstChildElement("object");
            if (obj) { bigESpawn.x = obj->FloatAttribute("x"); bigESpawn.y = obj->FloatAttribute("y"); }
        }
    }

    vector<TilesetInfo*> tilesetPtrs;
    for (auto& ts : tilesets) tilesetPtrs.push_back(&ts);


    // Build enemies
    auto makeEnemy = [&](sf::Vector2f sp) -> Enemy {
        Enemy e;
        e.position = { sp.x, sp.y - 256.f * ENEMY_SCALE };
        e.speed = 80.f;   e.direction = 1;
        e.health = 3;      e.maxHealth = 3;   e.damage = 1;
        e.attackRange = 80.f;   e.canAttack = true;
        e.attackCooldown = 1.5f; e.attackTimer = 0.f;
        e.state = Enemy::IDLE; e.currentFrame = 0;
        e.animTimer = 0.f;    e.animSpeed = 0.12f;
        e.dead = false;  e.dying = false;
        e.flashTimer = 0.f;    e.vy = 0.f; e.onGround = false;
        e.scaleX = ENEMY_SCALE; e.scaleY = ENEMY_SCALE;
        e.sprite.setTexture(eTexIdle[0]);
        e.sprite.setScale(ENEMY_SCALE, ENEMY_SCALE);
        e.sprite.setPosition(e.position);
        return e;
        };
    vector<Enemy> enemies;
    for (auto& sp : enemySpawnPoints) enemies.push_back(makeEnemy(sp));

    // Big E
    BigE bigE;
    if (!loadBigE(bigE, assetsPath)) return -1;
    bigE.x = bigESpawn.x;
    bigE.y = bigESpawn.y - bigE.height;
    BigE* bigEPtr = &bigE;

    // Player
    Player player;
    player.PLAYER_SCALE = 0.7f;
    player.speed = player.moveSpeed = 300.f;
    player.jumpSpeed = 600.f;
    player.gravity = 1200.f;
    player.coyoteTime = 0.1f;
    player.coyoteTimer = player.jumpBuffer = 0.f;
    player.width = FRAME_W * player.PLAYER_SCALE;
    player.height = FRAME_H * player.PLAYER_SCALE;
    player.onGround = player.jumpHeld = player.damaged = false;
    player.vx = player.vy = 0.f;
    player.health = player.maxHealth = 6;
    player.sprite.setTextureRect(sf::IntRect(0, 0, FRAME_W, FRAME_H));
    player.sprite.setScale(player.PLAYER_SCALE, player.PLAYER_SCALE);
    player.x = tileWidth * 2;
    player.y = 10 * tileHeight - player.height;
    player.sprite.setPosition(player.x, player.y);
    const float playerHitboxWidth = 25.f;

    int   playerFacing = 1;
    bool  canTakeDamage = true;
    float damageTimer = 0.f;
    const float damageCooldown = 1.f;

    // Camera
    Camera camera;
    camera.x = player.x; camera.y = player.y;
    camera.width = 1024.f;   camera.height = 750.f;
    camera.smoothSpeed = 6.f;      camera.offsetX = 0.f; camera.offsetY = -80.f;

    sf::View gameView;
    gameView.setSize(camera.width, camera.height);

    float checkpointX = player.x, checkpointY = player.y;

    sf::Clock clock;

    // Character loader
    auto loadCharacter = [&](int charIdx) {
        string folder = assetsPath + (charIdx == 0 ? "Shinobi/" : "Samurai/");
        texIdle.loadFromFile(folder + "Idle.png");
        texRun.loadFromFile(folder + "Run.png");
        texJump.loadFromFile(folder + "Jump.png");
        texHurt.loadFromFile(folder + "Hurt.png");
        texDead.loadFromFile(folder + "Dead.png");
        texAttack1.loadFromFile(folder + "Attack_1.png");
        texAttack2.loadFromFile(folder + "Attack_2.png");
        texAttack3.loadFromFile(folder + "Attack_3.png");
        player.sprite.setTexture(texIdle);
        player.sprite.setTextureRect(sf::IntRect(0, 0, 128, 128));
        player.width = 128.f * player.PLAYER_SCALE;
        player.height = 128.f * player.PLAYER_SCALE;
        };

    // Full reset
    auto resetGame = [&]() {
        isGameOver = false; isAttacking = false; attackHitDealt = false;
        player.health = player.maxHealth;
        player.x = tileWidth * 2;
        player.y = 10 * tileHeight - player.height;
        player.vx = player.vy = 0.f;
        player.sprite.setPosition(player.x, player.y);
        canTakeDamage = true; damageTimer = 0.f;
        currentFrame = 0;   animTimer = 0.f; playerState = IDLE;
        checkpointX = player.x; checkpointY = player.y;
        gameTimer = 0.f; timerRunning = true; isWin = false;
        enemies.clear();
        for (auto& sp : enemySpawnPoints) enemies.push_back(makeEnemy(sp));
        resetBigE(bigEPtr, bigESpawn.x, bigESpawn.y - bigEPtr->height);
        };

    // main loop
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.033f) dt = 0.033f;

        bool spaceNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

        if (currentScene == SCENE_INTRO)
        {
            UpdateIntro(intro, dt);
            if (spaceNow && !spacePressedLast)
                currentScene = SCENE_INSTRUCTIONS;
        }
        else if (currentScene == SCENE_INSTRUCTIONS)
        {
            if (spaceNow && !spacePressedLast)
                currentScene = SCENE_GAME;
        }

        //update music
        if (isPlaying != lastIsPlaying)
        {
            if (isPlaying && !isGameOver)
            {
                backgroundmusic.stop();
                if (gameMusic.getStatus() != sf::Music::Playing)
                    gameMusic.play();
            }
            else if (!isPlaying)
            {
                gameMusic.stop();
                if (backgroundmusic.getStatus() != sf::Music::Playing)
                    backgroundmusic.play();
            }
            lastIsPlaying = isPlaying;
        }
        spacePressedLast = spaceNow;

        updateMainMenu(dt);

        sf::Vector2f mouseUI = window.mapPixelToCoords(
            sf::Mouse::getPosition(window), window.getDefaultView());

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::TextEntered && isEnteringName)
                if (event.text.unicode >= 32 && event.text.unicode < 128 && playerName.size() < 16)
                    playerName += static_cast<char>(event.text.unicode);

            // Mouse move: highlight hovered item 
            if (event.type == sf::Event::MouseMoved)
            {
                sf::Vector2f mp = window.mapPixelToCoords(
                    sf::Vector2i(event.mouseMove.x, event.mouseMove.y), window.getDefaultView());

                if (!isPlaying && !pressoption && !isSelectingCharacter && !isEnteringName
                    && !showLeaderboard && !showCredits)
                {
                    for (int i = 0; i < MENU_COUNT; i++)
                        if (mainItemBounds(i).contains(mp) && selectedItem != i)
                        {
                            selectedItem = i; menu_sound.play();
                        }
                }

                if (pressoption)
                    for (int i = 0; i < 3; i++)
                        if (settingItemBounds(i).contains(mp) && selectedItem != i)
                        {
                            selectedItem = i; menu_sound.play();
                        }

                if (isSelectingCharacter)
                    for (int i = 0; i < 2; i++)
                        if (cardBounds(i).contains(mp) && selectedCharacter != i)
                        {
                            selectedCharacter = i; menu_sound.play();
                        }
            }

            // Mouse click 
            if (event.type == sf::Event::MouseButtonPressed
                && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f mp = window.mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y), window.getDefaultView());

                // Main menu
                if (!isPlaying && !pressoption && !isSelectingCharacter
                    && !isEnteringName && !showLeaderboard && !showCredits)
                {
                    for (int i = 0; i < MENU_COUNT; i++)
                        if (mainItemBounds(i).contains(mp)) {
                            click_enter.play();
                            if (i == 0) { isSelectingCharacter = true; selectedCharacter = 0; }
                            else if (i == 1) { pressoption = true; selectedItem = 0; }
                            else if (i == 2) { showLeaderboard = true; }
                            else if (i == 3) { showCredits = true; }
                            else if (i == 4) window.close();
                        }
                }

                // Settings
                if (pressoption)
                    for (int i = 0; i < 3; i++)
                        if (settingItemBounds(i).contains(mp)) {
                            click_enter.play();
                            if (i == 0) {
                                backgroundmusic.getStatus() == sf::Music::Playing
                                    ? backgroundmusic.pause() : backgroundmusic.play();
                            }
                            else if (i == 2) { pressoption = false; selectedItem = 1; }
                        }

                // Character select — click anywhere on a card
                if (isSelectingCharacter)
                    for (int i = 0; i < 2; i++)
                        if (cardBounds(i).contains(mp)) {
                            click_enter.play();
                            selectedCharacter = i;
                            isSelectingCharacter = false;
                            isEnteringName = true; playerName = "";
                            loadCharacter(selectedCharacter);
                        }
            }

            // Keyboard 
            if (event.type == sf::Event::KeyPressed)
            {
                if (isGameOver) {
                    if (event.key.code == sf::Keyboard::R) {
                        resetGame();
                        gameOverMusic.stop();
                        gameOverMusicPlayed = false;
                        backgroundmusic.stop();
                        if (gameMusic.getStatus() != sf::Music::Playing)
                            gameMusic.play();
                    }
                    else if (event.key.code == sf::Keyboard::Escape) {
                        isGameOver = isPlaying = isAttacking = attackHitDealt = false;
                        selectedItem = 0;
                        gameOverMusicPlayed = false;
                    }
                }
                else if (isWin && isPlaying) {
                    if (event.key.code == sf::Keyboard::Enter)
                    {
                        isPlaying = false; showLeaderboard = true;
                    }
                }
                else if (showLeaderboard) {
                    if (event.key.code == sf::Keyboard::Escape)
                    {
                        showLeaderboard = isPlaying = isWin = false; selectedItem = 0;
                    }
                }
                else if (showCredits) {
                    if (event.key.code == sf::Keyboard::Escape) showCredits = false;
                }
                else if (isEnteringName) {
                    if (event.key.code == sf::Keyboard::Enter && !playerName.empty()) {
                        click_enter.play(); isEnteringName = false; isPlaying = true; resetGame();
                    }
                    else if (event.key.code == sf::Keyboard::BackSpace && !playerName.empty())
                        playerName.pop_back();
                    else if (event.key.code == sf::Keyboard::Escape)
                    {
                        isEnteringName = false; isSelectingCharacter = true;
                    }
                }
                else if (!isPlaying && !pressoption && !isSelectingCharacter) {
                    if (event.key.code == sf::Keyboard::Escape) window.close();
                    else if (event.key.code == sf::Keyboard::Up) {
                        selectedItem = (selectedItem - 1 + MENU_COUNT) % MENU_COUNT;
                        menu_sound.play();
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        selectedItem = (selectedItem + 1) % MENU_COUNT;
                        menu_sound.play();
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        click_enter.play();
                        if (selectedItem == 0) {
                            isSelectingCharacter = true; 
                            selectedCharacter = 0;
                            click_enter.play();
                        }
                        else if (selectedItem == 1) { 
                            pressoption = true; 
                            selectedItem = 0; 
                            click_enter.play();
                        }
                        else if (selectedItem == 2) { 
                            showLeaderboard = true; 
                            click_enter.play();
                        }
                        else if (selectedItem == 3) { 
                            showCredits = true; 
                            click_enter.play();
                        }
                        else if (selectedItem == 4) window.close();
                    }
                }
                else if (pressoption) {
                    if (event.key.code == sf::Keyboard::Escape)
                    {
                        pressoption = false; selectedItem = 1;
                    }
                    // Settings wrap-around navigation 
                    else if (event.key.code == sf::Keyboard::Up) {
                        selectedItem = (selectedItem - 1 + 3) % 3;
                        menu_sound.play();
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        selectedItem = (selectedItem + 1) % 3;
                        menu_sound.play();
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        if (selectedItem == 0) {
                            backgroundmusic.getStatus() == sf::Music::Playing
                                ? backgroundmusic.pause() : backgroundmusic.play();
                            click_enter.play();
                        }
                        else if (selectedItem == 2)
                        {
                            pressoption = false; selectedItem = 1; click_enter.play();
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Right && selectedItem == 1) {
                        float v = backgroundmusic.getVolume();
                        if (v < 95) backgroundmusic.setVolume(v + 5);
                    }
                    else if (event.key.code == sf::Keyboard::Left && selectedItem == 1) {
                        float v = backgroundmusic.getVolume();
                        if (v >= 5) backgroundmusic.setVolume(v - 5);
                    }
                }
                else if (isSelectingCharacter) {
                    if (event.key.code == sf::Keyboard::Left) { selectedCharacter = 0; menu_sound.play(); }
                    else if (event.key.code == sf::Keyboard::Right) { selectedCharacter = 1; menu_sound.play(); }
                    else if (event.key.code == sf::Keyboard::Escape) isSelectingCharacter = false;
                    else if (event.key.code == sf::Keyboard::Enter) {
                        click_enter.play();
                        isSelectingCharacter = false; isEnteringName = true; playerName = "";
                        loadCharacter(selectedCharacter);
                    }
                }
                else if (isPlaying && !isPaused && !isWin) {
                    if (event.key.code == sf::Keyboard::Escape)
                    {
                        isPaused = true; click_enter.play();
                    }
                }
                else if (isPlaying && isPaused) {
                    if (event.key.code == sf::Keyboard::Enter)
                    {
                        isPaused = false; click_enter.play();
                    }
                    else if (event.key.code == sf::Keyboard::Escape)
                    {
                        isPaused = false; isPlaying = false; selectedItem = 0;
                    }
                }
            }
        }

            if (isPlaying && isGameOver && !gameOverMusicPlayed)
            {
                gameMusic.stop();
                if (backgroundmusic.getStatus() != sf::Music::Playing)
                    backgroundmusic.play();
                gameOverMusicPlayed = true;
            }
        // Game Update
        if (isPlaying && !isPaused && !isGameOver && !isWin)
        {
            if (timerRunning) gameTimer += dt;

            InputUpdate(player, playerFacing);
            PlayerPhysicsUpdate(player, dt, tiles, mapWidth, tileWidth, tileHeight, tilesetPtrs, playerHitboxWidth);

            for (auto& e : enemies)
                updateEnemy(e, player, isGameOver, canTakeDamage, damageTimer,
                    dt, mapWidth, tileWidth, tileHeight,
                    tiles, tilesetPtrs,
                    eTexAttack, eTexDeath, eTexHurt, eTexIdle, eTexWalk);

            updateBigE(bigEPtr, player, isGameOver, canTakeDamage, damageTimer, dt,
                mapWidth, tileWidth, tileHeight, tiles, tilesetPtrs);

            bool allEnemiesDead = true;
            for (auto& e : enemies)
                if (!e.dead) { allEnemiesDead = false; break; }

            if (bigEPtr->dead && allEnemiesDead && !isWin) {
                isWin = true; timerRunning = false;
                saveScore(playerName, gameTimer);
            }

            for (int i = 0; i < (int)tiles.size(); ++i) {
                if (!isCheckpoint(tiles[i], tilesetPtrs)) continue;
                int col = i % mapWidth, row = i / mapWidth;
                sf::FloatRect tb(col * tileWidth, row * tileHeight, tileWidth, tileHeight);
                if (isColliding(player, tb)) {
                    checkpointX = float(col * tileWidth) - tileWidth;
                    checkpointY = float(row * tileHeight) - player.height;
                    break;
                }
            }

            CameraUpdate(camera, gameView,
                player.x + player.width / 2, player.y + player.height / 2,
                dt, mapWidth, mapHeight, tileWidth, tileHeight);

            if (!canTakeDamage) {
                damageTimer += dt;
                if (damageTimer >= damageCooldown) { canTakeDamage = true; damageTimer = 0.f; }
            }

            if (canTakeDamage) {
                sf::FloatRect exp(
                    player.x + playerHitboxOffsetX - 10.f,   
                    player.y,
                    playerHitboxWidth + 20.f,               
                    player.height + 10.f
                );
                    for (int i = 0; i < (int)tiles.size(); ++i) {
                        if (!isObstacle(tiles[i], tilesetPtrs)) continue;
                        int col = i % mapWidth, row = i / mapWidth;
                        sf::FloatRect tb(col * tileWidth , row * tileHeight, tileWidth , tileHeight);
                        if (exp.intersects(tb)) {
                            spike_sound.play();
                            damagePlayer(player, 1, isGameOver);
                            if (!isGameOver) {
                                player.x = checkpointX; player.y = checkpointY;
                                player.vx = player.vy = 0.f;
                                player.sprite.setPosition(player.x, player.y);
                            }
                            canTakeDamage = false; damageTimer = 0.f; break;
                        }
                    }
            }

            // Player animation state machine
            PlayerState prevState = playerState;
            if (isGameOver) { playerState = DEAD; isAttacking = false; }
            else if (player.damaged) { playerState = HURT; isAttacking = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X) && !isAttacking)
            {
                playerState = ATTACK; isAttacking = true; attackHitDealt = false;
                sword_sound.play();
            }
            else if (!isAttacking) {
                if (!player.onGround)             playerState = JUMP;
                else if (fabsf(player.vx) > 1.f)  playerState = RUN;
                else                               playerState = IDLE;
            }

            if (playerState != prevState) {
                currentFrame = 0; animTimer = 0.f;
                sf::Texture* stateTex[] = { &texIdle, &texRun, &texJump, &texHurt, &texAttack1, &texDead };
                player.sprite.setTexture(*stateTex[(int)playerState]);
            }

            int frameCounts[] = { FRAMES_IDLE, FRAMES_RUN, FRAMES_JUMP, FRAMES_HURT, FRAMES_ATTACK, FRAMES_DEAD };
            int totalFrames = frameCounts[(int)playerState];
            float currentAnimSpeed = (playerState == ATTACK) ? attackAnimSpeed : animSpeed;

            animTimer += dt;
            if (animTimer >= currentAnimSpeed) {
                animTimer = 0.f;
                if (playerState == DEAD || playerState == HURT) {
                    if (currentFrame < totalFrames - 1) currentFrame++;
                    else if (playerState == HURT) { player.damaged = false; }
                }
                else if (playerState == ATTACK) {
                    if (currentFrame < totalFrames - 1) currentFrame++;
                    else {
                        currentFrame = 0; isAttacking = false; attackHitDealt = false;
                        playerState = IDLE; player.sprite.setTexture(texIdle);
                    }
                }
                else { currentFrame = (currentFrame + 1) % totalFrames; }
            }

            if (playerState == ATTACK) {
                int f = currentFrame;
                if (f < FRAMES_ATK1) {
                    player.sprite.setTexture(texAttack1);
                    player.sprite.setTextureRect(sf::IntRect(f * 128, 0, 128, 128));
                }
                else if (f < FRAMES_ATK1 + FRAMES_ATK2) {
                    player.sprite.setTexture(texAttack2);
                    player.sprite.setTextureRect(sf::IntRect((f - FRAMES_ATK1) * 128, 0, 128, 128));
                }
                else {
                    player.sprite.setTexture(texAttack3);
                    player.sprite.setTextureRect(sf::IntRect((f - FRAMES_ATK1 - FRAMES_ATK2) * 128, 0, 128, 128));
                }
            }
            else {
                player.sprite.setTextureRect(sf::IntRect(currentFrame * 128, 0, 128, 128));
            }

            // Player attack hitbox
            if (playerState == ATTACK && !attackHitDealt)
            {
                static const FrameHitbox swordFrames[] = {
                    { 3, 0.6f,  0.3f,  25.f, 20.f },
                    { 4, 0.9f,  0.3f,  30.f, 20.f },
                    { 5, 0.7f,  0.35f, 25.f, 20.f },
                };
                for (int fi = 0; fi < 3; fi++)
                {
                    const FrameHitbox* fh = &swordFrames[fi];
                    if (currentFrame != fh->frame) continue;

                    float sx = (playerFacing == 1)
                        ? player.x + player.width * fh->offsetX
                        : player.x + player.width * (1.f - fh->offsetX) - fh->w;
                    float sy = player.y + player.height * fh->offsetY;
                    sf::FloatRect swordBox(sx, sy, fh->w, fh->h);

                    for (auto& e : enemies) {
                        if (e.dead || e.dying || e.state == Enemy::HURT) continue;
                        sf::FloatRect enemyBox(
                            e.position.x + 256.f * e.scaleX * 0.38f,
                            e.position.y + 256.f * e.scaleY * 0.25f,
                            256.f * e.scaleX * 0.25f,
                            256.f * e.scaleY * 0.55f);
                        if (swordBox.intersects(enemyBox)) {
                            attackHitDealt = true;
                            e.health--; e.flashTimer = 0.3f;
                            if (e.health <= 0) { e.dying = true; e.state = Enemy::DEAD; e.currentFrame = 0; e.animTimer = 0.f; }
                            else { e.state = Enemy::HURT; e.currentFrame = 0; e.animTimer = 0.f; }
                            break;
                        }
                    }
                    checkPlayerHitsBigE(bigEPtr, currentFrame, playerFacing, player);
                    break;
                }
            }
        }

        // Rendering
        window.clear();

        if (currentScene == SCENE_INTRO)
        {
            drawIntro(window, introSfText, intro);
        }
        else if (currentScene == SCENE_INSTRUCTIONS)
        {
            drawInstructions(window, instructionsSprite);
        }
        else  
        {
            if (showLeaderboard)
            {
                drawLeaderboard(window, font, lbInstructions,
                    background_leaderboard, sword_mouse, mouseUI);
            }
            else if (showCredits)
            {
                drawCredits(window, font, sword_mouse, mouseUI);
            }
            else if (isEnteringName)
            {
                drawNameEntry(window, namePrompt, nameDisplay, nameInstructions,
                    playerName, selectBg, sword_mouse, mouseUI, font);
            }
            else if (!isPlaying && !pressoption && !isSelectingCharacter)
            {
                drawMainMenu(window, menu, menuRects, selectedItem,
                    background_spr, sword_mouse, mouseUI, font, dt);
            }
            else if (pressoption)
            {
                drawSettings(window, setting_txt, settingRects, selectedItem,
                    backgroundmusic, background_sprit, sword_mouse, mouseUI, font);
            }
            else if (isSelectingCharacter)
            {
                drawCharacterSelect(window, charPortrait, charLabel, selectBorder,
                    selectedCharacter, portraitX, selectBg,
                    selectPrompt, sword_mouse, mouseUI, font);
            }
            else if (isPlaying)
            {
                window.draw(background_playing);

                if (isGameOver)
                {
                    window.setView(gameView);
                    for (auto& tile : tileSprites) window.draw(tile);
                    window.draw(player.sprite);
                    for (auto& e : enemies) if (!e.dead) window.draw(e.sprite);
                    if (!bigEPtr->dead) window.draw(bigEPtr->sprite);
                    window.setView(window.getDefaultView());
                    drawGameOver(window, pauseOverlay, GameOver_txt, font);
                }
                else if (isWin)
                {
                    window.setView(gameView);
                    for (auto& tile : tileSprites) window.draw(tile);
                    window.draw(player.sprite);
                    window.setView(window.getDefaultView());
                    drawWinScreen(window, font, pauseOverlay, winText, winPrompt, gameTimer);
                }
                else
                {
                    window.setView(gameView);
                    for (auto& tile : tileSprites) window.draw(tile);
                    window.draw(player.sprite);
                    for (auto& e : enemies) if (!e.dead) window.draw(e.sprite);
                    if (!bigEPtr->dead) window.draw(bigEPtr->sprite);
                    window.setView(window.getDefaultView());

                    drawHearts(window, player, heartTexPtr);
                    timerText.setString(formatTime(gameTimer));
                    window.draw(timerText);
                    drawBigEHealthBar(window, bigEPtr, font, player);

                    int remaining = 0;
                    for (auto& e : enemies)
                        if (!e.dead) remaining++;
                    if (!bigEPtr->dead) remaining++;
                    sf::Text remainingText;
                    remainingText.setFont(font);
                    remainingText.setString("Enemies: " + to_string(remaining));
                    remainingText.setCharacterSize(36);
                    remainingText.setFillColor(sf::Color::White);
                    remainingText.setPosition(1920.f - 220.f, 70.f);
                    window.draw(remainingText);

                    drawBigEHealthBar(window, bigEPtr, font, player);

                    if (isPaused) drawPauseMenu(window, pauseOverlay, pause_txt, font);
                }
            }
        }

        window.display();
    }
    return 0;
}