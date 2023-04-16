//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_ECS_H
#define REALTIME_CELL_COLLAPSE_ECS_H

#include <vector>
#include <optional>
#include <tuple>
#include <memory>
#include "renderable.h"

struct Components {
	Renderable* renderable;
};

class ECS {
public:
	void create();

	void destroy();

	uint32_t insert(Components& entityComponents);

	void remove(const uint32_t& index);

	// auto e = [&](std::shared_ptr<Renderable> r) -> bool { return TODO };

	template<typename Evaluator>
	std::vector<Renderable*> requestRenderables(const Evaluator& evaluator);

private:
	void destroyReferences(const uint32_t& index);

	std::vector<bool> isOccupied{};
	std::vector<Renderable*> renderables{};
};

#endif //REALTIME_CELL_COLLAPSE_ECS_H
