#include "Mode.hpp"

std::shared_ptr< Mode > Mode::current;
GameState Mode::game_state;

void Mode::set_current(std::shared_ptr< Mode > const &new_current) {
	current = new_current;
	//NOTE: may wish to, e.g., trigger resize events on new current mode.
}


void Mode::set_state(GameState s){
	game_state = s;
}