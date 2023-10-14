#pragma once

#include <SFML/Graphics.hpp>

#include "Entity.h"
#include "EntityManager.h"

struct Player { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct Enemy { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SP; float SMIN, SMAX; };
struct Bullet { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game
{
public:
	Game();

	void run();

private:
	sf::RenderWindow m_window;
	EntityManager m_entities;
	sf::Font m_font;
	sf::Text m_text;
	sf::ContextSettings settings;
	sf::Event event;

	Player m_player;
	Enemy m_enemy;
	Bullet m_bullet;

	int m_score = 0;
	int m_currentFrame = 0;
	int m_lastEnemySpawnTime = 0;
	int m_lastBulletSpawned = 0;
	int m_enemySpawnInterval = 100;
	
	bool m_paused = false;
	bool m_running = true;

	int m_width = 1280;
	int m_height = 720;

	std::shared_ptr<Entity> m_player_p;

	void init();	// config kullanmadýðýmýz için farklý bir iþlem düþün
	void setPaused(bool paused);

	void sMovement();
	void sUserInput();
	void sLifespan();
	void sRender();
	void sEnemySpawner();
	void sCollision();

	void spawnPlayer();
	void spawnEnemy();
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity);
};