#include "common.hpp"
#include "components.hpp"
#include "global.hpp"

class VisibilitySystem {
public:
	std::vector<std::vector<int>> map;
	void restart_map();
	void step(float elapsed_ms);

private:
};