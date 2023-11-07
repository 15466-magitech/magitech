#include "NPCDialogue.hpp"
#include <string>

NPCDialogue::NPCDialogue(std::string filePath) {
	dialogueIdx = 0;
}

void NPCDialogue::AdvanceNPCDialogue() {

	dialogueIdx++;
}