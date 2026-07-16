// Aliases entt::entity as Entity and defines NULL_ENTITY; included by Scene and any code that names
// entities.
#pragma once
#include "entt/entt.hpp"  // The core ECS library
#include <cstdint>

// Using Entity as an alias for entt::entity
using Entity = entt::entity;

// Define a constant for a null/invalid entity
Entity const NULL_ENTITY = entt::null;
