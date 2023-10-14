#include<iostream>

#include "Game.h"


Game::Game()
{
	init();
}

void Game::init()
{
	settings.antialiasingLevel = 8;

	m_player = { 32, 32, 5, 5, 5, 5, 255, 0, 0, 8, 4 };
	
	m_enemy = { 32, 32, 0, 0, 0, 0, 0, 2, 3, 8, 60, 60};

	m_bullet = { 16, 16, 20, 255, 255, 255, 255, 255, 255, 2, 100, 90 };
	
	m_window.create(sf::VideoMode(m_width, m_height), "Shoot The Shapes", sf::Style::Close, settings);

	sf::Image icon;
	if (icon.loadFromFile("C:/Users/tuana/Desktop/C++ PROJELER/Test Project/SFML Project/clip-477.png"))
	{
		m_window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	}

	m_window.setFramerateLimit(60);

	spawnPlayer();
}

void Game::run()
{
	// TODO: add pause functionality in here
	//		 some systems should function while paused (rendering)
	//		 some systems shouldn't (movement / input)

	while(m_running)
	{
		m_entities.update();

		if(!m_paused)
		{
			sEnemySpawner();
			sMovement();
			sCollision();
			sLifespan();
			sUserInput();
		}

		sRender();

		// increment the current frame
		// may need to be moved when pause implemented
		m_currentFrame++;
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

void Game::spawnPlayer()
{
	// We create every entity by calling EntityManager.addEntity(tag)
	// This returns a std::shared_ptr<Entity>, so we use 'auto' to save typing
	auto entity = m_entities.addEntity("player");

	float mx = m_window.getSize().x / 2;
	float my = m_window.getSize().y / 2;

	entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(1.0f, 1.0f), 0.0f);

	// The entity's shape will have radius 32, 8 sides, dark grey fill, and red outline of thickness 4
	entity->cShape = std::make_shared<CShape>(m_player.SR, m_player.V, sf::Color::Black, sf::Color::Red, 4.0f);

	// Add an input component to the player so that we can use inputs
	entity->cInput = std::make_shared<CInput>();

	entity->cLifespan = std::make_shared<CLifespan>(50);

	// Since we want this Entity to be our player, set our Game's player variable to be this Entity
	// This goes slightly against the EntityManager paradigm, but we use the player so much it's worth it
	m_player_p = entity;
}

void Game::spawnEnemy()
{
	// TODO: make sure the enemy is spawned properly with the m_enemy variables
	//		 the enemy must be spawned completely within the bounds of the window

	auto entity = m_entities.addEntity("enemy");

	float ex = rand() % (m_window.getSize().x - 30);
	float ey = rand() % (m_window.getSize().y - 30);

	entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), Vec2(1.0f, 1.0f), 0.0f);

	entity->cShape = std::make_shared<CShape>(m_enemy.SR, rand() % 7, sf::Color(1 + rand() % 255, 1 + rand() % 255, 1 + rand() % 255), sf::Color(1 + rand() % 255, 1 + rand() % 255, 1 + rand() % 255), 4.0f);

	// record when the most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target) 
{
	// TODO: implement the spawning of a bullet which travels toward target
	//		 - bullet speed is given as a scalar speed
	//		 - you must set the velocity by using formula in notes

	auto bullet = m_entities.addEntity("bullet");
	
	bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, Vec2(0, 0), 0);
	bullet->cShape = std::make_shared<CShape>(m_bullet.SR, m_bullet.CR, sf::Color::Red, sf::Color::Red, 2);
	bullet->cLifespan = std::make_shared<CLifespan>(100);

	// Mermi hýzýný hesapla (baþlangýç pozisyonu ile hedef arasýndaki fark)
	Vec2 direction = target - entity->cTransform->pos;
	direction.normalize();  // Hýzý birim vektör haline getir
	Vec2 velocity = direction * 20.f;

	// Mermi hýzýný ayarla
	bullet->cTransform->velocity = velocity;

	m_lastBulletSpawned = m_currentFrame;

}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	// TODO: implement your own special weapon
	for (int angle = 0; angle < 360; angle += 22.5f) { 

		m_lastBulletSpawned = m_currentFrame;
		float radians = angle * 3.14f / 180.0f;

		auto special_bullet = m_entities.addEntity("bullet");

		special_bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, Vec2(0, 0), 0);

		float bulletSpeed = 20.0f; // Mermi hýzý
		float bulletDirectionX = std::cos(radians);
		float bulletDirectionY = std::sin(radians);

		Vec2 bulletVelocity(bulletDirectionX, bulletDirectionY);
		bulletVelocity.normalize(); 
		bulletVelocity *= bulletSpeed;

		special_bullet->cTransform->velocity = bulletVelocity;

		special_bullet->cShape = std::make_shared<CShape>(16, 16, sf::Color::Magenta, sf::Color::Blue, 2);
		special_bullet->cLifespan = std::make_shared<CLifespan>(100);
	}


}

void Game::sMovement()
{
	// TODO: implement all entity movement in this function
	//		 you should read the m_player->cInput component to determine if the player is moving

	m_player_p->cTransform->velocity = { 0, 0 };

	// implement player movement
	if (m_player_p->cInput->up)
	{
		m_player_p->cTransform->velocity.y = -m_player.S;
	}

	if (m_player_p->cInput->down)
	{
		m_player_p->cTransform->velocity.y = +m_player.S;
	}

	if (m_player_p->cInput->right)
	{
		m_player_p->cTransform->velocity.x = +m_player.S;
	}

	if (m_player_p->cInput->left)
	{
		m_player_p->cTransform->velocity.x = -m_player.S;
	}

	for (auto& e : m_entities.getEntities())
	{
		if(e->cTransform->pos.x + e->cShape->circle.getRadius() >= m_window.getSize().x || e->cTransform->pos.x < 0)
		{
			e->cTransform->velocity.x *= -1;
		}

		if (e->cTransform->pos.y + e->cShape->circle.getRadius() >= m_window.getSize().y || e->cTransform->pos.y < 0)
		{
			e->cTransform->velocity.y *= -1;
		}

		e->cTransform->pos.x += e->cTransform->velocity.x;
		e->cTransform->pos.y += e->cTransform->velocity.y;
	}

}

void Game::sLifespan()
{
	// TODO: implement all lifespan functionality
	// 
	// for all entities
	//	   if entity has no lifespan component, skip it
	//	   if entity has > 0 remaining lifespan, substract 1
	//	   if it has lifespan and is alive
	//		  scale its alpha channel properly
	//	   if it has lifespan and its time is up
	//		  destroy the entity
	
	for (auto& entity : m_entities.getEntities("bullet"))
	{
		auto lifespanComponent = entity->cLifespan;

		if (lifespanComponent) 
		{
			if (lifespanComponent->total > 0)
			{
				lifespanComponent->remaining = lifespanComponent->total;

				if (lifespanComponent->remaining > 0) {
					// Azaltma faktörünü belirleyin (0.05 gibi)
					float decreaseFactor = 0.005f;

					// Önceki alfa deðeri
					float previousAlpha = static_cast<float>(entity->cShape->circle.getFillColor().a) / 255.0f;

					// Yeni alfa deðeri
					float newAlpha = previousAlpha - decreaseFactor;

					// Alfa deðerini sýnýrlayýn 0 ile 1 arasýnda
					newAlpha = std::max(0.0f, newAlpha);

					// Alfa kanalýný ayarlayýn
					sf::Color fillColor = entity->cShape->circle.getFillColor();
					fillColor.a = static_cast<sf::Uint8>(newAlpha * 255);
					entity->cShape->circle.setFillColor(fillColor);
					entity->cShape->circle.setOutlineColor(fillColor);

					lifespanComponent->total--;
				}
			}

			else
			{
				entity->destroy(); 
			}	
		}
	}


}


void Game::sCollision()
{
	// TODO: implement all proper collisions between entities
	//		 be sure to use the collision radius, NOT the shape radius

	for (auto& e: m_entities.getEntities("enemy"))
	{
		
		for (auto& b : m_entities.getEntities("bullet"))
		{
			bool isCollided = false;
			
			Vec2 distance = e->cTransform->pos - b->cTransform->pos;
			float distance1 = distance.length();
			
			if(!isCollided && distance1 < e->cShape->circle.getRadius() / 2 + b->cShape->circle.getRadius() / 2)
			{
				m_score += e->cShape->circle.getPointCount() * 25;
				if (m_score % 500 == 0 && m_enemySpawnInterval != 20)
				{
					m_enemySpawnInterval -= 20;
				}
				e->destroy();
				b->destroy();
				isCollided = true;	
			}
		}
	}

	for (auto& p : m_entities.getEntities("player"))
	{
		bool playerCollided = false; // Flag to track player-enemy collision

		for (auto& e : m_entities.getEntities("enemy"))
		{
			Vec2 distance = p->cTransform->pos - e->cTransform->pos;
			float distance1 = distance.length();

			if (distance1 < e->cShape->circle.getRadius() / 2 + p->cShape->circle.getRadius() / 2)
			{
				playerCollided = true; // Set the collision flag
				
				if (playerCollided)
				{
					p->cLifespan->total--;
					if (p->cLifespan->total < 0)
					{
						p->destroy();
						m_window.close();
					}

					e->destroy();
				}
			}
		}
	}
	
}

void Game::sEnemySpawner()
{
	// TODO: code which implements enemy spawning should go here
	//
	//		 (use m_currrentFrame - m_lastEnemySpawnTime) to determine
	//		 how long it has been since the last enemy spawned

	if (m_currentFrame - m_lastEnemySpawnTime > m_enemySpawnInterval)
	{
		spawnEnemy();
	}
}


void Game::sRender()
{
	// TODO: change the code below to draw ALL of the entities
	//		 sample drawing of the player Entity that we have created
	m_window.clear();

	if (!m_font.loadFromFile("C:/Users/tuana/Desktop/C++ PROJELER/Test Project/SFML Project/opensans.ttf"))
	{
		std::cout << "work";

	}
	else
	{
		m_text.setFont(m_font);
		m_text.setString("Score: " + std::to_string(m_score));
		m_text.setCharacterSize(40);
		m_text.setPosition(2.0f, 2.0f);
		m_text.setFillColor(sf::Color::White);
		m_window.draw(m_text);
	}

	for(auto& e: m_entities.getEntities())
	{
		// set the position of the shape based on the entity's transform->pos
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

		// set the rotation of the shape based on the entity's transform->angle
		e->cTransform->angle += 1.0f;
		e->cShape->circle.setRotation(e->cTransform->angle);

		m_window.draw(e->cShape->circle);
	}

	m_window.display();
}


void Game::sUserInput()
{
	// TODO: handle user input here
	//	     note that you should only be setting the player's input component variables here
	//		 you should not implement the player's movement logic here
	//		 the movement system will read the variables you set in this function

	while (m_window.pollEvent(event))
	{
		// this event triggers when the window is closed
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}

		// this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				// TODO: set player's input component 'up' to true
				m_player_p->cInput->up = true;
				break;
			
			case sf::Keyboard::S:
				m_player_p->cInput->down = true;
				break;
			
			case sf::Keyboard::D:
				m_player_p->cInput->right = true;
				break;
			
			case sf::Keyboard::A:
				m_player_p->cInput->left = true;
				break;

			default: break;
			}
		}

		// this event is triggered when a key is released
		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				// TODO: set player's input component 'up' to true
				m_player_p->cInput->up = false;
				break;
			case sf::Keyboard::S:
				m_player_p->cInput->down = false;
				break;

			case sf::Keyboard::D:
				m_player_p->cInput->right = false;
				break;

			case sf::Keyboard::A:
				m_player_p->cInput->left = false;
				break;

			default: break;
			}
		}
		
		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left && m_currentFrame - m_lastBulletSpawned > 20)
			{
				spawnBullet(m_player_p, Vec2(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.mouseButton.button == sf::Mouse::Right && m_currentFrame - m_lastBulletSpawned > 100)
			{
				spawnSpecialWeapon(m_player_p);
			}
		}

	}
}