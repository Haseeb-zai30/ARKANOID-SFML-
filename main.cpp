#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>

// Add sound variables
sf::SoundBuffer levelUpBuffer, gameOverBuffer, backgroundMusicBuffer;
sf::Sound levelUpSound, gameOverSound;
sf::Music backgroundMusic;

// Base class for Brick
class Brick {
protected:
    sf::Sprite sprite;
    int hits;

public:
    Brick(float x, float y, int hits, const sf::Texture& texture) : hits(hits) {
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
    }

    virtual void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    virtual void hit() {
        if (hits > 0) hits--;
    }

    virtual bool isDestroyed() const {
        return hits <= 0;
    }

    virtual sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
};

// Derived classes for different brick types
class BronzeBrick : public Brick {
public:
    BronzeBrick(float x, float y, const sf::Texture& texture)
        : Brick(x, y, 1, texture) {} // 1 hit for bronze bricks
};

class SilverBrick : public Brick {
public:
    SilverBrick(float x, float y, const sf::Texture& texture)
        : Brick(x, y, 2, texture) {} // 2 hits for silver bricks
};

class GoldBrick : public Brick {
public:
    GoldBrick(float x, float y, const sf::Texture& texture)
        : Brick(x, y, 3, texture) {} // 3 hits for gold bricks
};

// Paddle class
class Paddle {
public:
    sf::RectangleShape rect;
    float speed = 6.f;

    Paddle() {
        rect.setSize(sf::Vector2f(100.f, 20.f));
        rect.setFillColor(sf::Color::White);
        rect.setPosition(590.f, 700.f);
    }

    void move() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            rect.move(-speed, 0.f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            rect.move(speed, 0.f);
        }

        // Clamp paddle position
        if (rect.getPosition().x < 0) {
            rect.setPosition(0, rect.getPosition().y);
        }
        if (rect.getPosition().x > 1280 - rect.getSize().x) {
            rect.setPosition(1280 - rect.getSize().x, rect.getPosition().y);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(rect);
    }
};

// Ball class
class Ball {
public:
    sf::CircleShape ball;
    float xVelocity = 3.f;
    float yVelocity = -3.f;

    Ball() {
        ball.setRadius(10.f);
        ball.setFillColor(sf::Color::White);
        ball.setPosition(520.f, 690.f);
    }

    void move() {
        sf::Vector2f position = ball.getPosition();
        position.x += xVelocity;
        position.y += yVelocity;

        // Bounce off walls
        if (position.x <= 0 || position.x >= 1280 - ball.getRadius() * 2) xVelocity *= -1;

        ball.setPosition(position);
    }

    void handleTopPanelCollision(const sf::RectangleShape& topPanel) {
        if (ball.getGlobalBounds().intersects(topPanel.getGlobalBounds())) {
            yVelocity = std::abs(yVelocity); // Ensure it moves downward
        }
    }

    void handlePaddleCollision(Paddle& paddle) {
        if (ball.getGlobalBounds().intersects(paddle.rect.getGlobalBounds())) {
            yVelocity = -std::abs(yVelocity);
            float paddleCenter = paddle.rect.getPosition().x + paddle.rect.getSize().x / 2;
            float ballCenter = ball.getPosition().x + ball.getRadius();
            xVelocity += (ballCenter - paddleCenter) * 0.05f;
        }
    }

    sf::FloatRect getBounds() const {
        return ball.getGlobalBounds();
    }

    void resetPosition() {
        ball.setPosition(520.f, 690.f);
        xVelocity = 3.f;
        yVelocity = -3.f;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(ball);
    }
};

// Initialize bricks for a level
void initializeBricks(std::vector<std::unique_ptr<Brick>>& bricks, int level, const sf::Texture& bronzeTexture,
                      const sf::Texture& silverTexture, const sf::Texture& goldTexture) {
    bricks.clear();
    float brickWidth = 80.f;
    float brickHeight = 40.f;
    float xOffset = 10.f;
    float yOffset = 5.f;

    int rows = 5;
    int cols = (1280 - 2 * xOffset) / (brickWidth + xOffset);

    for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
        float x = xOffset + j * (brickWidth + xOffset);
        float y = yOffset + i * (brickHeight + yOffset) + 40;

        if (level == 1 || (level == 2 && i < 4)) {
            // Level 1: All bronze, Level 2: First 4 rows bronze
            bricks.emplace_back(std::make_unique<BronzeBrick>(x, y, bronzeTexture));
        } else if (level == 2 && i == 4) {
            // Level 2: Fifth row is silver
            bricks.emplace_back(std::make_unique<SilverBrick>(x, y, silverTexture));
        } else if (level == 3) {
            // Level 3: Silver bricks in diagonals and fifth row, others bronze
            if (i == 4 || i == j || i==cols-1-j) {
                bricks.emplace_back(std::make_unique<SilverBrick>(x, y, silverTexture));
            } else {
                bricks.emplace_back(std::make_unique<BronzeBrick>(x, y, bronzeTexture));
            }
        } else if (level == 4) {
            // Level 4: Fifth row bronze, fourth row gold, second row silver
            if (i == 4) {
                bricks.emplace_back(std::make_unique<BronzeBrick>(x, y, bronzeTexture));
            } else if (i == 3) {
                bricks.emplace_back(std::make_unique<GoldBrick>(x, y, goldTexture));
            }else if (i == 1) {
                bricks.emplace_back(std::make_unique<SilverBrick>(x, y, silverTexture));
            }else {
                bricks.emplace_back(std::make_unique<BronzeBrick>(x, y, bronzeTexture));
            }
        } else if (level == 5) {
            // Level 5: Fifth row gold, third and fourth rows silver, others bronze
            if (i == 4) {
                bricks.emplace_back(std::make_unique<GoldBrick>(x, y, goldTexture));
            } else if (i == 3 || i == 2) {
                bricks.emplace_back(std::make_unique<SilverBrick>(x, y, silverTexture));
            } else {
                bricks.emplace_back(std::make_unique<BronzeBrick>(x, y, bronzeTexture));
            }
        }
    }
}

}

// Main function
int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Arkanoid");
    window.setFramerateLimit(60);

    // Load the background texture
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("background.png")) {
        std::cerr << "Error loading background texture!" << std::endl;
        return -1;
    }

    // Load sound buffers
    if (!levelUpBuffer.loadFromFile("level_sound.ogg") ||
        !gameOverBuffer.loadFromFile("game_over.wav") ||
        !backgroundMusic.openFromFile("background_music.ogg")) {
        std::cerr << "Error loading audio files!" << std::endl;
        return -1;
    }

    // Create sound instances
    levelUpSound.setBuffer(levelUpBuffer);
    gameOverSound.setBuffer(gameOverBuffer);

    // Set background music to loop
    backgroundMusic.setLoop(true);
    backgroundMusic.play();

    // Create a sprite for the background
    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale(
        static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
        static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y
    );

    Paddle paddle;
    Ball ball;

    sf::Font font;
    if (!font.loadFromFile("Sansation_Bold.ttf")) {
        std::cerr << "Error loading font!" << std::endl;
        return -1;
    }

    sf::Texture bronzeTexture, silverTexture, goldTexture, gameOverTexture;
    if (!bronzeTexture.loadFromFile("bronze.png") || !silverTexture.loadFromFile("silver.png") ||
        !goldTexture.loadFromFile("gold.png") || !gameOverTexture.loadFromFile("gameover.png")) {
        std::cerr << "Error loading textures!" << std::endl;
        return -1;
    }

    std::vector<std::unique_ptr<Brick>> bricks;
    int score = 0;
    int lives = 3;
    int level = 1;
    bool paused = false;
    bool gameOver = false;

    initializeBricks(bricks, level, bronzeTexture, silverTexture, goldTexture);

    sf::RectangleShape topPanel(sf::Vector2f(1280.f, 25.f));
    topPanel.setFillColor(sf::Color(200, 200, 200)); // Light gray
    topPanel.setPosition(0.f, 0.f);

    sf::Text scoreText, livesText, levelText, pauseText;

    // Initialize score, lives, and level text
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition(10.f, 0.f);

    livesText.setFont(font);
    livesText.setCharacterSize(20);
    livesText.setFillColor(sf::Color::Black);
    livesText.setPosition(1200.f, 0.f);

    levelText.setFont(font);
    levelText.setCharacterSize(20);
    levelText.setFillColor(sf::Color::Black);
    levelText.setPosition(600.f - levelText.getGlobalBounds().width / 2, 0.f);

    pauseText.setFont(font);
    pauseText.setCharacterSize(40);
    pauseText.setFillColor(sf::Color::Green);
    pauseText.setString("Paused\nPress 'Escape' to Resume");
    pauseText.setPosition(1280 / 2.f - pauseText.getGlobalBounds().width / 2.f, 720 / 2.f - pauseText.getGlobalBounds().height / 2.f);

    sf::Sprite gameOverSprite;
    gameOverSprite.setTexture(gameOverTexture);
    gameOverSprite.setPosition(1280 / 2.f - gameOverTexture.getSize().x / 2.f, 720 / 2.f - gameOverTexture.getSize().y / 2.f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape && !gameOver) {
                paused = !paused;
            }
        }

        if (!paused && !gameOver) {
            paddle.move();
            ball.move();

            // Handle ball collisions with the paddle and top panel
            ball.handlePaddleCollision(paddle);
            ball.handleTopPanelCollision(topPanel);

            // Handle ball collisions with bricks
            for (auto it = bricks.begin(); it != bricks.end();) {
                if (ball.getBounds().intersects((*it)->getBounds())) {
                    ball.yVelocity = -ball.yVelocity;
                    (*it)->hit();
                    if ((*it)->isDestroyed()) {
                        it = bricks.erase(it);
                        score += 10;
                    } else {
                        ++it;
                    }
                } else {
                    ++it;
                }
            }

            // Ball goes out of bounds
            if (ball.getBounds().top > 720) {
                ball.resetPosition();
                lives--;
                if (lives == 0) {
                    gameOver = true;
                    gameOverSound.play();  // Play game over sound
                }
            }

            // Check if level is completed
            if (bricks.empty() && !gameOver) {
                level++;
                levelUpSound.play();  // Play level-up sound
                if (level > 5) {
                    std::cout << "Congratulations! You completed all levels!" << std::endl;
                    window.close();
                } else {
                    initializeBricks(bricks, level, bronzeTexture, silverTexture, goldTexture);
                    ball.resetPosition();
                }
            }
        }

        // Update UI text
        std::ostringstream scoreStream, livesStream, levelStream;
        scoreStream << "Score: " << score;
        livesStream << "Lives: " << lives;
        levelStream << "Level: " << level;

        scoreText.setString(scoreStream.str());
        livesText.setString(livesStream.str());
        levelText.setString(levelStream.str());

        // Render the game
        window.clear(sf::Color::Black);

        if (!gameOver) {
            // Draw the background first
            window.draw(backgroundSprite);

            // Then draw other elements
            window.draw(topPanel);
            window.draw(scoreText);
            window.draw(livesText);
            window.draw(levelText);

            for (const auto& brick : bricks) {
                brick->draw(window);
            }

            paddle.draw(window);
            ball.draw(window);

            if (paused) {
                window.draw(pauseText);
            }
        } else {
            // Draw the game over sprite
            window.draw(gameOverSprite);
        }

        // Display everything
        window.display();
    }

    return 0;
}
