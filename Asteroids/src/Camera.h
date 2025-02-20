#pragma once
#include "GameEngine/EntitySystem/Actor.h"

class Camera
	:public Actor
{
public:
	Camera(class Game *game);

	void UpdateActor(float deltaTime) override;
	void ActorInput(const uint8_t *keys) override;
private:
	class MoveComponent *m_moveComp;


};

