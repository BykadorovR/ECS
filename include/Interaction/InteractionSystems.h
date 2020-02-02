#pragma once
#include "System.h"

class InteractionSystem : public System {
private:
public:
	InteractionSystem();
	// Called every game update
	void update(std::shared_ptr<EntityManager> entityManager);
};