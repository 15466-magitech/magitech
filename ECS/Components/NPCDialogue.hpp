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
	// TODO: replace the placeholders with actual story events
	enum DialogueEvent {
		Event1,
		Event2
	};

	typedef std::vector< std::string > dialogueLines;

	/* list of all the possible dialogue options (in chronological order) */
	std::vector< dialogueLines > dialogueOptions;

	/* current index for which lines of dialogue to use out of all the dialogue options */
	uint32_t dialogueIdx;

	/* story events that cause the individual NPC's dialogue to progress to the next option */
	std::vector< DialogueEvent > dialogueEvents;

	/* TODO: Constructor should read in file and set the dialogueOptions variable
	 * TODO: Potentially populate the dialogueEvents vector based on the filepath? Or add in another input */
	explicit NPCDialogue(std::string filePath);

	/* TODO: Either this should be a static function that goes through all of the NPC Entities and
	 * calls dialogueIdx++ if they have that particular dialogueEvent, or it should be a function that
	 * every NPC has that just checks if the dialogueEvent is in their dialogueEvents vector and calls dialogueIdx++
	 *
	 * Idk which is better between the static or non-static version */
	void AdvanceNPCDialogue(DialogueEvent dialogueEvent);
};