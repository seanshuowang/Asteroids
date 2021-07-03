#include "MoveComponent.h"
#include "../Actor.h"
#include <iostream>
#include <string>

MoveComponent::MoveComponent(Actor *owner, int updateOrder)
	:Component(owner, updateOrder)
	,mAngularSpeed(0.0f)
	,mForwardSpeed(1.0f)
	,mVelocityVector( Vector2(0.0f, 0.0f))
{
}

void MoveComponent::Update(float deltaTime)
{
	// Update rotation
	if (!Math::NearZero(mAngularSpeed))
	{
		mOwner->mRotation += mAngularSpeed * deltaTime;
		/*std::cout << std::to_string(mOwner->mRotation) << std::endl;*/
	}

	// Update position
	if (!Math::NearZero(mVelocityVector.x * mVelocityVector.x + mVelocityVector.y * mVelocityVector.y))
	{
		Vector2 newPos = mOwner->mPosition + mVelocityVector * mForwardSpeed * deltaTime;
		
		// Screen wrapping
		if (newPos.x < 0.0f)
		{
			newPos.x = 1918.0f;
		}
		else if (newPos.x > 1920.0f)
		{
			newPos.x = 2.0f;
		}

		if (newPos.y < 0.0f)
		{
			newPos.y = 1078.0f;
		}
		else if (newPos.y > 1080.0f)
		{
			newPos.y = 2.0f;
		}
		
		mOwner->mPosition = newPos;
	}
}