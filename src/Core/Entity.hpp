// Aliases entt::entity as Entity and defines NULL_ENTITY; included by Scene and any code that names entities.
#pragma once
#include <cstdint>
#include "entt/entt.hpp" // The core ECS library

// Using Entity as an alias for entt::entity
using Entity = entt::entity;

// Define a constant for a null/invalid entity
const Entity NULL_ENTITY = entt::null;

