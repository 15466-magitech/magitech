/*
 * Created by Nellie Tonev on 11/06/23.
 * Author(s): Nellie Tonev
 *
 * Component that contains relevant information for each NPC's dialogue
 */
#pragma once

#include <vector>
#include <string>
#include "../Component.hpp"

struct NPCDialogue : Component<NPCDialogue> {
	typedef std::vector< std::string > dialogueLines;

	/* list of all the possible dialogue options (in chronological order) */
	std::vector< dialogueLines > dialogueOptions;

	/* current index for which lines of dialogue to use out of all the dialogue options */
	uint32_t dialogueIdx;

	explicit NPCDialogue(std::string filePath);

	void AdvanceNPCDialogue();
};