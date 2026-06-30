#pragma once
#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include "structs.h"
#include "score.h"

using namespace std;

//  PARTICLE SYSTEM 
struct MenuParticle {
    float x, y, vx, vy, life, maxLife, size;
    sf::Color color;
};

struct MenuState {
    vector<MenuParticle> particles;
    float spawnTimer = 0.f;
    float pulseTimer = 0.f;
    float titleWave = 0.f;
};

static MenuState gMenuState;

static void spawnParticle(float screenW, float screenH) {
    MenuParticle p;
    p.x = (float)(rand() % (int)screenW);
    p.y = screenH + 10.f;
    p.vx = ((rand() % 200) - 100) * 0.003f;
    p.vy = -(0.3f + (rand() % 100) * 0.008f);
    p.maxLife = p.life = 4.f + (rand() % 60) * 0.1f;
    p.size = 1.f + (rand() % 3);
    int r = rand() % 3;
    if (r == 0) p.color = sf::Color(180, 60, 255, 200);   
    else if (r == 1) p.color = sf::Color(80, 160, 255, 180);   
    else  p.color = sf::Color(255, 80, 120, 160);   
    gMenuState.particles.push_back(p);
}

static void updateMenuParticles(float dt, float screenW, float screenH) {
    gMenuState.spawnTimer += dt;
    if (gMenuState.spawnTimer > 0.04f) {
        gMenuState.spawnTimer = 0.f;
        if (gMenuState.particles.size() < 200)
            spawnParticle(screenW, screenH);
    }
    for (auto& p : gMenuState.particles) {
        p.x += p.vx * dt * 60.f;
        p.y += p.vy * dt * 60.f;
        p.life -= dt;
        float alpha = (p.life / p.maxLife);
        p.color.a = (sf::Uint8)(alpha * 220.f);
    }
    gMenuState.particles.erase(
        remove_if(gMenuState.particles.begin(), gMenuState.particles.end(),
            [&](const MenuParticle& p) { return p.life <= 0 || p.y < -20; }),
        gMenuState.particles.end());
    gMenuState.pulseTimer += dt;
    gMenuState.titleWave += dt * 2.f;
}

static void drawMenuParticles(sf::RenderWindow& window) {
    for (auto& p : gMenuState.particles) {
        sf::CircleShape c(p.size);
        c.setFillColor(p.color);
        c.setPosition(p.x, p.y);
        window.draw(c);
    }
}

// Draw a horizontal divider line
static void drawGlowLine(sf::RenderWindow& window, float x, float y, float w,
    sf::Color col = sf::Color(160, 80, 255, 180))
{
    sf::RectangleShape line(sf::Vector2f(w, 1.f));
    line.setFillColor(col); line.setPosition(x, y); window.draw(line);
    sf::RectangleShape glow(sf::Vector2f(w, 5.f));
    glow.setFillColor(sf::Color(col.r, col.g, col.b, 40));
    glow.setPosition(x, y - 2.f); window.draw(glow);
}

// Draw a stylised panel with inner glow
static void drawPanel(sf::RenderWindow& window, float x, float y, float w, float h,
    sf::Color fill = sf::Color(8, 4, 20, 210),
    sf::Color border = sf::Color(140, 60, 255, 200))
{
    // Shadow
    sf::RectangleShape shadow(sf::Vector2f(w + 8.f, h + 8.f));
    shadow.setFillColor(sf::Color(0, 0, 0, 100));
    shadow.setPosition(x + 4.f, y + 4.f);
    window.draw(shadow);

    // Body
    sf::RectangleShape body(sf::Vector2f(w, h));
    body.setFillColor(fill);
    body.setOutlineThickness(2.f);
    body.setOutlineColor(border);
    body.setPosition(x, y);
    window.draw(body);

    // Top inner glow strip
    sf::RectangleShape topGlow(sf::Vector2f(w - 4.f, 2.f));
    topGlow.setFillColor(sf::Color(border.r, border.g, border.b, 80));
    topGlow.setPosition(x + 2.f, y + 2.f);
    window.draw(topGlow);
}

// Full-screen vignette 
static void drawVignette(sf::RenderWindow& window, float W, float H) {
    const float edgeW = W * 0.35f;
    sf::RectangleShape l(sf::Vector2f(edgeW, H));
    sf::RectangleShape overlay(sf::Vector2f(W, H));
    overlay.setFillColor(sf::Color(0, 0, 0, 60));
    window.draw(overlay);
}

static sf::Text makeText(const sf::Font& font, const string& str,
    unsigned int size, sf::Color col,
    float cx = -1, float y = 0, float W = 1920.f)
{
    sf::Text t; t.setFont(font); t.setString(str);
    t.setCharacterSize(size); t.setFillColor(col);
    sf::FloatRect b = t.getLocalBounds();
    if (cx >= 0) { t.setOrigin(b.left + b.width * 0.5f, b.top); t.setPosition(cx, y); }
    return t;
}

//  MAIN MENU
void updateMainMenu(float dt) {
    updateMenuParticles(dt, 1920.f, 1080.f);
}

void drawMainMenu(sf::RenderWindow& window, sf::Text menu[],
    sf::RectangleShape menuRects[], int selectedItem,
    const sf::Sprite& , sf::Sprite& sword_mouse,
    const sf::Vector2f& mouseUI, const sf::Font& font, float dt)
{
    const float W = 1920.f, H = 1080.f;

    window.clear(sf::Color(4, 2, 12));

    for (int ring = 8; ring >= 1; ring--) {
        float r = ring * 90.f;
        sf::CircleShape glow(r);
        glow.setFillColor(sf::Color(0, 0, 0, 0));
        glow.setOutlineColor(sf::Color(80, 20, 160, (sf::Uint8)(18 * ring)));
        glow.setOutlineThickness(30.f);
        glow.setOrigin(r, r);
        glow.setPosition(W * 0.5f, H * 1.1f);
        window.draw(glow);
    }

    // Particles
    drawMenuParticles(window);

    drawGlowLine(window, 120.f, 200.f, 680.f);
    drawGlowLine(window, 120.f, 201.f, 680.f, sf::Color(80, 20, 160, 60));

    // Game title
    {
        float wave = sinf(gMenuState.titleWave) * 4.f;
        for (int s = 3; s >= 1; s--) {
            sf::Text shadow = makeText(font, "WHISPERING", 110,
                sf::Color(100, 20, 200, (sf::Uint8)(30 * s)),
                W * 0.31f, 90.f + s * 3 + wave);
            window.draw(shadow);
        }
        sf::Text t1 = makeText(font, "WHISPERING", 110, sf::Color(210, 160, 255), W * 0.31f, 90.f + wave);
        window.draw(t1);

        for (int s = 3; s >= 1; s--) {
            sf::Text shadow = makeText(font, "ABYSS", 140,
                sf::Color(180, 40, 255, (sf::Uint8)(40 * s)),
                W * 0.31f, 195.f + s * 3 + wave);
            window.draw(shadow);
        }
        sf::Text t2 = makeText(font, "ABYSS", 140, sf::Color(255, 255, 255), W * 0.31f, 195.f + wave);
        t2.setStyle(sf::Text::Bold);
        window.draw(t2);
    }

    // Subtitle
    sf::Text sub = makeText(font, "break the curse  ·  or become part of it", 22,
        sf::Color(140, 100, 200, 180), W * 0.31f, 360.f);
    window.draw(sub);

    drawGlowLine(window, 120.f, 400.f, 680.f);

    // Menu items
    string labels[] = { "PLAY", "SETTINGS", "LEADERBOARD", "CREDITS", "EXIT" };
    float  startY = 440.f;
    float  stepY = 106.f;

    for (int i = 0; i < 5; i++) {
        bool sel = (i == selectedItem);
        float itemY = startY + i * stepY;
        float pulse = sel ? (sinf(gMenuState.pulseTimer * 4.f) * 0.5f + 0.5f) : 0.f;

        sf::Color fillCol = sel
            ? sf::Color((sf::Uint8)(14 + 20 * pulse), 6, (sf::Uint8)(40 + 30 * pulse), 230)
            : sf::Color(8, 4, 20, 180);
        sf::Color bordCol = sel
            ? sf::Color((sf::Uint8)(200 + 55 * pulse), (sf::Uint8)(100 * pulse),
                (sf::Uint8)(255), (sf::Uint8)(200 + 55 * pulse))
            : sf::Color(80, 40, 120, 120);

        drawPanel(window, 120.f, itemY, 620.f, 90.f, fillCol, bordCol);

        if (sel) {
            sf::RectangleShape bar(sf::Vector2f(5.f, 70.f));
            bar.setFillColor(sf::Color((sf::Uint8)(180 + 75 * pulse), 80, 255, 220));
            bar.setPosition(120.f, itemY + 10.f);
            window.draw(bar);
        }

        // Index number
        sf::Text idx = makeText(font, to_string(i + 1), 22,
            sel ? sf::Color(200, 120, 255, 200) : sf::Color(80, 60, 100, 160));
        idx.setPosition(138.f, itemY + 32.f);
        window.draw(idx);

        // Item label
        sf::Color labelCol = sel ? sf::Color(255, 255, 255) : sf::Color(160, 140, 190);
        sf::Text lbl = makeText(font, labels[i], sel ? 40 : 36, labelCol);
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setOrigin(lb.left, lb.top + lb.height * 0.5f);
        lbl.setPosition(175.f, itemY + 45.f);
        window.draw(lbl);

        // Arrow indicator for selected
        if (sel) {
            float arrowX = 710.f + sinf(gMenuState.pulseTimer * 5.f) * 5.f;
            sf::Text arrow = makeText(font, ">", 38, sf::Color(200, 100, 255, 200));
            arrow.setPosition(arrowX, itemY + 26.f);
            window.draw(arrow);
        }
    }

    // Controls hint bottom-left
    drawGlowLine(window, 120.f, 980.f, 680.f, sf::Color(80, 40, 120, 120));
    sf::Text hint = makeText(font, "Enter / Click Select    Esc Quit", 20,
        sf::Color(100, 80, 140, 160));
    hint.setPosition(120.f, 992.f);
    window.draw(hint);

    //Right-side decorative panel
    drawPanel(window, 900.f, 300.f, 880.f, 600.f,
        sf::Color(6, 3, 16, 160), sf::Color(60, 30, 100, 120));
    drawGlowLine(window, 930.f, 320.f, 820.f, sf::Color(120, 60, 200, 100));

    sf::Text loreTitle = makeText(font, "THE ABYSS AWAITS", 32,
        sf::Color(180, 120, 255, 220), 1340.f, 330.f);
    window.draw(loreTitle);

    string loreLines[] = {
        "Beneath the surface lies",
        "a forgotten city...",
        "",
        "Shadows wander.",
        "Silence hides a terrible truth.",
        "",
        "Break the curse.",
        "Or become part of it."
    };
    for (int i = 0; i < 8; i++) {
        sf::Text ll = makeText(font, loreLines[i], 24,
            sf::Color(160, 140, 200, (sf::Uint8)(180 - i * 8)), 1340.f, 390.f + i * 44.f);
        window.draw(ll);
    }

    sword_mouse.setPosition(mouseUI);
    window.draw(sword_mouse);
}

//  SETTINGS MENU
void drawSettings(sf::RenderWindow& window, sf::Text setting_txt[],
    sf::RectangleShape settingRects[], int selectedItem,
    sf::Music& music, const sf::Sprite& ,
    sf::Sprite& sword_mouse, const sf::Vector2f& mouseUI,
    const sf::Font& font)
{
    const float W = 1920.f, H = 1080.f;

    window.clear(sf::Color(4, 2, 12));
    drawMenuParticles(window);

    drawPanel(window, W * 0.5f - 420.f, 160.f, 840.f, 700.f,
        sf::Color(6, 3, 18, 230), sf::Color(120, 50, 220, 180));

    drawGlowLine(window, W * 0.5f - 380.f, 230.f, 760.f);
    sf::Text title = makeText(font, "SETTINGS", 64, sf::Color(220, 180, 255),
        W * 0.5f, 170.f);
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    drawGlowLine(window, W * 0.5f - 380.f, 258.f, 760.f, sf::Color(80, 40, 160, 80));

    string status = (music.getStatus() == sf::Music::Playing) ? "ON" : "OFF";
    int    vol = (int)music.getVolume();

    struct SettingItem { string label; string value; };
    SettingItem items[] = {
        { "MUSIC",  status },
        { "VOLUME", to_string(vol) + " / 100" },
        { "BACK",   "" }
    };

    for (int i = 0; i < 3; i++) {
        bool  sel = (i == selectedItem);
        float itemY = 300.f + i * 170.f;
        float pulse = sel ? (sinf(gMenuState.pulseTimer * 4.f) * 0.5f + 0.5f) : 0.f;

        sf::Color fillCol = sel
            ? sf::Color((sf::Uint8)(14 + 20 * pulse), 6, (sf::Uint8)(40 + 30 * pulse), 240)
            : sf::Color(10, 5, 25, 180);
        sf::Color bordCol = sel
            ? sf::Color(220, (sf::Uint8)(100 * pulse), 255, (sf::Uint8)(220 + 35 * pulse))
            : sf::Color(70, 35, 110, 140);

        drawPanel(window, W * 0.5f - 360.f, itemY, 720.f, 110.f, fillCol, bordCol);

        if (sel) {
            sf::RectangleShape bar(sf::Vector2f(5.f, 90.f));
            bar.setFillColor(sf::Color((sf::Uint8)(160 + 95 * pulse), 60, 255, 220));
            bar.setPosition(W * 0.5f - 360.f, itemY + 10.f);
            window.draw(bar);
        }

        // Label
        sf::Text lbl = makeText(font, items[i].label, 38,
            sel ? sf::Color(255, 255, 255) : sf::Color(160, 140, 200));
        lbl.setPosition(W * 0.5f - 330.f, itemY + 36.f);
        window.draw(lbl);

        if (!items[i].value.empty()) {
            sf::Color valCol = sel ? sf::Color(220, 160, 255) : sf::Color(120, 100, 160);
            sf::Text val = makeText(font, items[i].value, 32, valCol);
            sf::FloatRect vb = val.getLocalBounds();
            val.setOrigin(vb.left + vb.width, vb.top);
            val.setPosition(W * 0.5f + 340.f, itemY + 40.f);
            window.draw(val);
        }

        if (i == 1 && sel) {
            float barX = W * 0.5f - 320.f;
            float barY = itemY + 80.f;
            float barW = 640.f;
            float ratio = vol / 100.f;

            sf::RectangleShape track(sf::Vector2f(barW, 6.f));
            track.setFillColor(sf::Color(40, 20, 70, 200));
            track.setPosition(barX, barY);
            window.draw(track);

            sf::RectangleShape fill(sf::Vector2f(barW * ratio, 6.f));
            fill.setFillColor(sf::Color(180, 80, 255, 220));
            fill.setPosition(barX, barY);
            window.draw(fill);

            sf::CircleShape thumb(8.f);
            thumb.setFillColor(sf::Color(220, 160, 255));
            thumb.setOrigin(8.f, 8.f);
            thumb.setPosition(barX + barW * ratio, barY + 3.f);
            window.draw(thumb);
        }
    }

    drawGlowLine(window, W * 0.5f - 380.f, 920.f, 760.f, sf::Color(80, 40, 120, 100));
    sf::Text hint = makeText(font, "Enter Select    Esc Back", 22,
        sf::Color(100, 80, 140, 160), W * 0.5f, 930.f);
    window.draw(hint);

    sword_mouse.setPosition(mouseUI);
    window.draw(sword_mouse);
}

//  CHARACTER SELECT
void drawCharacterSelect(sf::RenderWindow& window, sf::Sprite charPortrait[],
    sf::Text charLabel[], sf::RectangleShape& ,
    int selectedCharacter, float portraitX[],
    const sf::Sprite& background, const sf::Text& ,
    sf::Sprite& sword_mouse, const sf::Vector2f& mouseUI,
    const sf::Font& font)
{
    const float W = 1920.f, H = 1080.f;

    window.clear(sf::Color(4, 2, 12));
    drawMenuParticles(window);

    drawPanel(window, W * 0.5f - 400.f, 60.f, 800.f, 120.f,
        sf::Color(6, 3, 18, 200), sf::Color(140, 60, 255, 160));
    sf::Text title = makeText(font, "CHOOSE YOUR WARRIOR", 52,
        sf::Color(230, 200, 255), W * 0.5f, 90.f);
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    drawGlowLine(window, W * 0.5f - 380.f, 158.f, 760.f);

    float cardW = 520.f, cardH = 680.f;
    float cardY = 200.f;
    float cardX[2] = { W * 0.5f - cardW - 60.f, W * 0.5f + 60.f };

    string charNames[2] = { "SHINOBI", "SAMURAI" };

    for (int i = 0; i < 2; i++) {
        bool sel = (i == selectedCharacter);
        float pulse = sel ? (sinf(gMenuState.pulseTimer * 3.f) * 0.5f + 0.5f) : 0.f;

        sf::Color fillCol = sel
            ? sf::Color(12, 6, 30, 230)
            : sf::Color(8, 4, 20, 180);
        sf::Color bordCol = sel
            ? sf::Color((sf::Uint8)(200 + 55 * pulse), (sf::Uint8)(80 * pulse), 255,
                (sf::Uint8)(200 + 55 * pulse))
            : sf::Color(60, 30, 90, 140);

        drawPanel(window, cardX[i], cardY, cardW, cardH, fillCol, bordCol);

        if (sel) {
            for (int ring = 3; ring >= 1; ring--) {
                sf::RectangleShape aura(sf::Vector2f(cardW + ring * 16.f, cardH + ring * 16.f));
                aura.setFillColor(sf::Color(0, 0, 0, 0));
                aura.setOutlineThickness(6.f);
                aura.setOutlineColor(sf::Color(160, 60, 255, (sf::Uint8)(25 * ring * pulse)));
                aura.setPosition(cardX[i] - ring * 8.f, cardY - ring * 8.f);
                window.draw(aura);
            }
        }

        sf::Sprite port = charPortrait[i];
        float scale = sel ? 3.8f : 3.2f;
        port.setScale(scale, scale);
        float portW = 128.f * scale;
        port.setPosition(cardX[i] + (cardW - portW) * 0.5f, cardY + 30.f);
        window.draw(port);

        drawGlowLine(window, cardX[i] + 20.f, cardY + 30.f + 128.f * scale + 20.f, cardW - 40.f,
            sel ? sf::Color(180, 80, 255, 180) : sf::Color(60, 30, 90, 100));

        sf::Color nameCol = sel ? sf::Color(255, 255, 255) : sf::Color(160, 140, 200);
        sf::Text name = makeText(font, charNames[i], sel ? 42 : 36, nameCol,
            cardX[i] + cardW * 0.5f, cardY + 30.f + 128.f * scale + 30.f);
        name.setStyle(sf::Text::Bold);
        window.draw(name);

        if (sel) {
            sf::Text badge = makeText(font, "SELECTED", 24,
                sf::Color(220, 160, 255), cardX[i] + cardW * 0.5f, cardY + cardH - 55.f);
            window.draw(badge);
        }
    }

    sf::Text leftHint = makeText(font, "", 36, sf::Color(140, 100, 200, 180));
    leftHint.setPosition(cardX[0] - 80.f, cardY + cardH * 0.5f - 20.f);
    window.draw(leftHint);

    sf::Text rightHint = makeText(font, "", 36, sf::Color(140, 100, 200, 180));
    rightHint.setPosition(cardX[1] + cardW + 20.f, cardY + cardH * 0.5f - 20.f);
    window.draw(rightHint);

    // Bottom hint
    drawGlowLine(window, W * 0.5f - 400.f, 960.f, 800.f, sf::Color(80, 40, 120, 100));
    sf::Text hint = makeText(font, "Switch    Enter / Click  Confirm    Esc  Back", 22,
        sf::Color(100, 80, 140, 160), W * 0.5f, 972.f);
    window.draw(hint);

    sword_mouse.setPosition(mouseUI);
    window.draw(sword_mouse);
}

//  NAME ENTRY
void drawNameEntry(sf::RenderWindow& window, const sf::Text& ,
    sf::Text& , const sf::Text& ,
    const string& playerName, const sf::Sprite& ,
    sf::Sprite& sword_mouse, const sf::Vector2f& mouseUI,
    const sf::Font& font)
{
    const float W = 1920.f;

    window.clear(sf::Color(4, 2, 12));
    drawMenuParticles(window);

    drawPanel(window, W * 0.5f - 440.f, 260.f, 880.f, 500.f,
        sf::Color(6, 3, 18, 230), sf::Color(140, 60, 255, 180));

    sf::Text title = makeText(font, "ENTER YOUR NAME", 54, sf::Color(220, 180, 255),
        W * 0.5f, 300.f);
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    drawGlowLine(window, W * 0.5f - 400.f, 376.f, 800.f);

    sf::RectangleShape inputBox(sf::Vector2f(640.f, 90.f));
    inputBox.setFillColor(sf::Color(10, 5, 28, 200));
    inputBox.setOutlineThickness(2.f);
    float pulse = sinf(gMenuState.pulseTimer * 4.f) * 0.5f + 0.5f;
    inputBox.setOutlineColor(sf::Color((sf::Uint8)(160 + 95 * pulse), 60, 255,
        (sf::Uint8)(180 + 75 * pulse)));
    inputBox.setPosition(W * 0.5f - 320.f, 420.f);
    window.draw(inputBox);

    string display = playerName + (sinf(gMenuState.pulseTimer * 5.f) > 0 ? "|" : " ");
    sf::Text nameText = makeText(font, display, 52, sf::Color(255, 220, 255),
        W * 0.5f, 433.f);
    window.draw(nameText);

    sf::Text hint = makeText(font, "Letters, numbers and spaces only  ·  Max 16 characters", 22,
        sf::Color(100, 80, 140, 160), W * 0.5f, 540.f);
    window.draw(hint);

    sf::Text hint2 = makeText(font, "Enter  Confirm     Backspace  Delete     Esc  Back", 22,
        sf::Color(100, 80, 140, 140), W * 0.5f, 580.f);
    window.draw(hint2);

    sword_mouse.setPosition(mouseUI);
    window.draw(sword_mouse);
}

//  PAUSE MENU
void drawPauseMenu(sf::RenderWindow& window,
    const sf::RectangleShape& ,
    const sf::Text& , const sf::Font& font)
{
    const float W = 1920.f, H = 1080.f;

    sf::RectangleShape overlay(sf::Vector2f(W, H));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    drawPanel(window, W * 0.5f - 320.f, H * 0.5f - 200.f, 640.f, 400.f,
        sf::Color(6, 3, 18, 240), sf::Color(140, 60, 255, 220));

    drawGlowLine(window, W * 0.5f - 280.f, H * 0.5f - 140.f, 560.f);

    sf::Text title = makeText(font, "PAUSED", 72, sf::Color(220, 180, 255),
        W * 0.5f, H * 0.5f - 185.f);
    title.setStyle(sf::Text::Bold);
    window.draw(title);

    drawGlowLine(window, W * 0.5f - 280.f, H * 0.5f - 128.f, 560.f,
        sf::Color(80, 40, 160, 80));

    sf::Text r1 = makeText(font, "Enter to Resume", 32, sf::Color(200, 170, 255),
        W * 0.5f, H * 0.5f - 60.f);
    window.draw(r1);

    sf::Text r2 = makeText(font, "Esc to return to Menu", 32, sf::Color(160, 130, 200),
        W * 0.5f, H * 0.5f + 20.f);
    window.draw(r2);

    drawGlowLine(window, W * 0.5f - 280.f, H * 0.5f + 100.f, 560.f,
        sf::Color(80, 40, 120, 100));

    sf::Text tip = makeText(font, "Your progress is saved at each checkpoint", 20,
        sf::Color(100, 80, 140, 140), W * 0.5f, H * 0.5f + 120.f);
    window.draw(tip);
}

//  GAME OVER
void drawGameOver(sf::RenderWindow& window,
    const sf::RectangleShape& ,
    const sf::Text& , const sf::Font& font)
{
    const float W = 1920.f, H = 1080.f;

    sf::RectangleShape overlay(sf::Vector2f(W, H));
    overlay.setFillColor(sf::Color(0, 0, 0, 190));
    window.draw(overlay);

    for (int ring = 5; ring >= 1; ring--) {
        float r = ring * 100.f;
        sf::CircleShape glow(r);
        glow.setFillColor(sf::Color(0, 0, 0, 0));
        glow.setOutlineColor(sf::Color(180, 20, 20, (sf::Uint8)(20 * ring)));
        glow.setOutlineThickness(30.f);
        glow.setOrigin(r, r);
        glow.setPosition(W * 0.5f, H * 0.5f);
        window.draw(glow);
    }

    drawPanel(window, W * 0.5f - 380.f, H * 0.5f - 230.f, 760.f, 460.f,
        sf::Color(18, 3, 3, 240), sf::Color(200, 30, 30, 220));

    drawGlowLine(window, W * 0.5f - 340.f, H * 0.5f - 170.f, 680.f,
        sf::Color(200, 30, 30, 200));

    // Shadow title
    float wave = sinf(gMenuState.titleWave) * 3.f;
    for (int s = 3; s >= 1; s--) {
        sf::Text sh = makeText(font, "GAME OVER", 90,
            sf::Color(200, 20, 20, (sf::Uint8)(30 * s)), W * 0.5f, H * 0.5f - 220.f + s * 3 + wave);
        window.draw(sh);
    }
    sf::Text title = makeText(font, "GAME OVER", 90, sf::Color(255, 80, 80),
        W * 0.5f, H * 0.5f - 220.f + wave);
    title.setStyle(sf::Text::Bold);
    window.draw(title);

    drawGlowLine(window, W * 0.5f - 340.f, H * 0.5f - 100.f, 680.f,
        sf::Color(120, 20, 20, 120));

    sf::Text sub = makeText(font, "The abyss claimed you.", 30,
        sf::Color(200, 130, 130, 200), W * 0.5f, H * 0.5f - 60.f);
    window.draw(sub);

    sf::Text r1 = makeText(font, "Press R to Try Again", 36, sf::Color(255, 180, 180),
        W * 0.5f, H * 0.5f + 20.f);
    window.draw(r1);

    sf::Text r2 = makeText(font, "Esc to return to Menu", 30, sf::Color(180, 120, 120),
        W * 0.5f, H * 0.5f + 90.f);
    window.draw(r2);

    drawGlowLine(window, W * 0.5f - 340.f, H * 0.5f + 160.f, 680.f,
        sf::Color(100, 20, 20, 100));
}

//  WIN SCREEN
void drawWinScreen(sf::RenderWindow& window, const sf::Font& font,
    const sf::RectangleShape& ,
    const sf::Text& , const sf::Text& ,
    float gameTimer)
{
    const float W = 1920.f, H = 1080.f;

    sf::RectangleShape overlay(sf::Vector2f(W, H));
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    for (int ring = 5; ring >= 1; ring--) {
        float r = ring * 100.f;
        sf::CircleShape glow(r);
        glow.setFillColor(sf::Color(0, 0, 0, 0));
        glow.setOutlineColor(sf::Color(220, 180, 20, (sf::Uint8)(20 * ring)));
        glow.setOutlineThickness(30.f);
        glow.setOrigin(r, r);
        glow.setPosition(W * 0.5f, H * 0.5f);
        window.draw(glow);
    }

    drawPanel(window, W * 0.5f - 400.f, H * 0.5f - 270.f, 800.f, 540.f,
        sf::Color(10, 10, 3, 240), sf::Color(220, 180, 30, 220));

    drawGlowLine(window, W * 0.5f - 360.f, H * 0.5f - 200.f, 720.f,
        sf::Color(220, 180, 30, 200));

    float wave = sinf(gMenuState.titleWave) * 4.f;
    for (int s = 3; s >= 1; s--) {
        sf::Text sh = makeText(font, "VICTORY", 96,
            sf::Color(200, 160, 10, (sf::Uint8)(35 * s)), W * 0.5f, H * 0.5f - 255.f + s * 3 + wave);
        window.draw(sh);
    }
    sf::Text title = makeText(font, "VICTORY", 96, sf::Color(255, 220, 60),
        W * 0.5f, H * 0.5f - 255.f + wave);
    title.setStyle(sf::Text::Bold);
    window.draw(title);

    drawGlowLine(window, W * 0.5f - 360.f, H * 0.5f - 130.f, 720.f,
        sf::Color(140, 110, 20, 120));

    sf::Text sub = makeText(font, "The curse is broken.", 32,
        sf::Color(220, 200, 140, 200), W * 0.5f, H * 0.5f - 90.f);
    window.draw(sub);

    int mins = (int)gameTimer / 60, secs = (int)gameTimer % 60;
    int ms = (int)((gameTimer - (int)gameTimer) * 100);
    char buf[32]; snprintf(buf, sizeof(buf), "%02d:%02d.%02d", mins, secs, ms);

    sf::Text timeLabel = makeText(font, "Your Time", 26, sf::Color(180, 160, 100, 180),
        W * 0.5f, H * 0.5f - 20.f);
    window.draw(timeLabel);
    sf::Text timeVal = makeText(font, string(buf), 64, sf::Color(255, 220, 60),
        W * 0.5f, H * 0.5f + 15.f);
    timeVal.setStyle(sf::Text::Bold);
    window.draw(timeVal);

    drawGlowLine(window, W * 0.5f - 360.f, H * 0.5f + 120.f, 720.f,
        sf::Color(140, 110, 20, 100));

    sf::Text r1 = makeText(font, "Enter to view Leaderboard", 34, sf::Color(255, 230, 120),
        W * 0.5f, H * 0.5f + 145.f);
    window.draw(r1);
}

//  LEADERBOARD
void drawLeaderboard(sf::RenderWindow& window, const sf::Font& font,
    const sf::Text& , const sf::Sprite& ,
    sf::Sprite& sword_mouse, const sf::Vector2f& mouseUI)
{
    const float W = 1920.f, H = 1080.f;

    window.clear(sf::Color(4, 2, 12));
    drawMenuParticles(window);

    drawPanel(window, W * 0.5f - 500.f, 60.f, 1000.f, 920.f,
        sf::Color(5, 3, 14, 230), sf::Color(120, 60, 220, 180));

    sf::Text title = makeText(font, "LEADERBOARD", 64, sf::Color(220, 180, 255),
        W * 0.5f, 90.f);
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    drawGlowLine(window, W * 0.5f - 460.f, 172.f, 920.f);

    vector<ScoreEntry> scores = loadScores();

    if (scores.empty()) {
        sf::Text empty = makeText(font, "No scores yet be the first to win!", 38,
            sf::Color(160, 140, 200, 180), W * 0.5f, 400.f);
        window.draw(empty);
    }
    else {
        // Header row
        sf::Text hRank = makeText(font, "#", 24, sf::Color(120, 100, 160, 160));
        sf::Text hName = makeText(font, "NAME", 24, sf::Color(120, 100, 160, 160));
        sf::Text hTime = makeText(font, "TIME", 24, sf::Color(120, 100, 160, 160));
        hRank.setPosition(W * 0.5f - 440.f, 195.f);
        hName.setPosition(W * 0.5f - 340.f, 195.f);
        hTime.setPosition(W * 0.5f + 160.f, 195.f);
        window.draw(hRank); window.draw(hName); window.draw(hTime);
        drawGlowLine(window, W * 0.5f - 460.f, 228.f, 920.f, sf::Color(80, 40, 120, 100));

        sf::Color rankColors[] = {
            sf::Color(255, 215,   0),   
            sf::Color(192, 192, 192),   
            sf::Color(205, 127,  50),   
        };
        string medals[] = { "🥇", "🥈", "🥉" };

        int shown = min((int)scores.size(), 12);
        for (int i = 0; i < shown; i++) {
            float rowY = 240.f + i * 56.f;
            bool  isTop3 = (i < 3);
            sf::Color col = isTop3 ? rankColors[i] : sf::Color(180, 160, 210);

            if (isTop3) {
                sf::RectangleShape rowBg(sf::Vector2f(920.f, 48.f));
                rowBg.setFillColor(sf::Color(col.r / 6, col.g / 6, col.b / 6, 80));
                rowBg.setPosition(W * 0.5f - 460.f, rowY - 4.f);
                window.draw(rowBg);
            }

            sf::Text rank = makeText(font, to_string(i + 1), 28, col);
            rank.setPosition(W * 0.5f - 440.f, rowY);
            window.draw(rank);

            string name = scores[i].name;
            if (name.size() > 16) name = name.substr(0, 14) + "..";
            sf::Text nameT = makeText(font, name, 28, col);
            nameT.setPosition(W * 0.5f - 340.f, rowY);
            window.draw(nameT);

            sf::Text timeT = makeText(font, formatTime(scores[i].time), 28, col);
            timeT.setPosition(W * 0.5f + 160.f, rowY);
            window.draw(timeT);

            if (i < shown - 1)
                drawGlowLine(window, W * 0.5f - 440.f, rowY + 46.f, 880.f,
                    sf::Color(60, 30, 90, 60));
        }
    }

    drawGlowLine(window, W * 0.5f - 460.f, 940.f, 920.f, sf::Color(80, 40, 120, 100));
    sf::Text hint = makeText(font, "Esc to return to Main Menu", 24,
        sf::Color(100, 80, 140, 160), W * 0.5f, 952.f);
    window.draw(hint);

    sword_mouse.setPosition(mouseUI);
    window.draw(sword_mouse);
}

//  CREDITS
void drawCredits(sf::RenderWindow& window, const sf::Font& font,
    sf::Sprite& sword_mouse, const sf::Vector2f& mouseUI)
{
    const float W = 1920.f, H = 1080.f;

    window.clear(sf::Color(4, 2, 12));
    drawMenuParticles(window);

    drawPanel(window, W * 0.5f - 480.f, 60.f, 960.f, 940.f,
        sf::Color(5, 3, 14, 230), sf::Color(140, 60, 255, 180));

    sf::Text title = makeText(font, "CREDITS", 72, sf::Color(220, 180, 255),
        W * 0.5f, 90.f);
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    drawGlowLine(window, W * 0.5f - 440.f, 180.f, 880.f);

    sf::Text gameName = makeText(font, "Whispering Abyss", 34,
        sf::Color(180, 140, 255, 220), W * 0.5f, 200.f);
    gameName.setStyle(sf::Text::Italic);
    window.draw(gameName);
    drawGlowLine(window, W * 0.5f - 440.f, 248.f, 880.f, sf::Color(80, 40, 120, 80));

    // Team section 
    sf::Text teamLabel = makeText(font, "OUR TEAM MEMBERS", 28,
        sf::Color(160, 120, 220, 180), W * 0.5f, 270.f);
    teamLabel.setStyle(sf::Text::Bold);
    window.draw(teamLabel);

    string teamNames[] = {
        "Mieray Maher",
        "Mina Sami",
        "Bavly Amir",
        "Youssef Emad",
        "Mina Reda",
        "Mina Nader",
        "Mikha Youssef"
    };

    int memberCount = sizeof(teamNames) / sizeof(teamNames[0]);
    for (int i = 0; i < memberCount; i++) {
        float rowY = 330.f + i * 72.f;
        float pulse = sinf(gMenuState.pulseTimer * 2.f + i * 0.8f) * 0.5f + 0.5f;

        sf::RectangleShape rowBg(sf::Vector2f(840.f, 58.f));
        rowBg.setFillColor(sf::Color((sf::Uint8)(8 + 4 * pulse), 4, (sf::Uint8)(22 + 8 * pulse), 160));
        rowBg.setOutlineThickness(1.f);
        rowBg.setOutlineColor(sf::Color(80, 40, 120, 80));
        rowBg.setPosition(W * 0.5f - 420.f, rowY - 6.f);
        window.draw(rowBg);

        sf::Text nameT = makeText(font, teamNames[i], 30,
            sf::Color((sf::Uint8)(200 + 55 * pulse), 160, 255),
            W * 0.5f, rowY + 4.f);
        window.draw(nameT);
    }

    drawGlowLine(window, W * 0.5f - 440.f, 946.f, 880.f, sf::Color(80, 40, 120, 100));
    sf::Text hint = makeText(font, "Esc to return to Main Menu", 24,
        sf::Color(100, 80, 140, 160), W * 0.5f, 958.f);
    window.draw(hint);

    sword_mouse.setPosition(mouseUI);
    window.draw(sword_mouse);
}

#endif