#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal.h"
#include "portal_monsties.h"


// Display a series of dialog windows for creating an arbitrary monster chosen by the user
static void create_monstie(char *podName) {
    creature *theMonster;
    pos selectedPosition;
    boolean locationIsValid = false;
    char theMessage[COLS] = "";

    // Generate the monstie
    theMonster = generateMonster(MK_MONSTIE, false, false, podName);
    //initializeMonster(theMonster, false, NULL);

    // Find a random valid location
    for (int i = 0; i < 100; i++) { // Try up to 100 times to find a valid location
        selectedPosition.x = rand_range(0, DCOLS - 1);
        selectedPosition.y = rand_range(0, DROWS - 1);
        if (cellHasTerrainFlag(selectedPosition, T_OBSTRUCTS_PASSABILITY) == false) {
            locationIsValid = true;
            break;
        }
    }

    if (!locationIsValid) {
        sprintf(theMessage, "Failed to create monstie: no valid location found.");
        temporaryMessage(theMessage, REFRESH_SIDEBAR);
        killCreature(theMonster, true);
        removeDeadMonsters();
        return;
    }

    // Place the monstie at the selected location
    theMonster->loc = selectedPosition;
    pmapAt(theMonster->loc)->flags |= HAS_MONSTER;
    theMonster->creatureState = MONSTER_WANDERING;
    fadeInMonster(theMonster);
    refreshSideBar(-1, -1, false);
    refreshDungeonCell(theMonster->loc);

    // Display a message that the monstie was created
    sprintf(theMessage, "Monstie created.");
    temporaryMessage(theMessage, REFRESH_SIDEBAR);
}

/**
 * Calls the get_new_monsties function and creates a monstie for each pod-name in the list.
 */
void update_monsties(void)
{
    char **pod_names = get_new_monsties();
    if (!pod_names)
    {
        fprintf(stdout, "No new monsties to create.\n");
        return;
    }

    for (int i = 0; pod_names[i] != NULL; i++)
    {
        fprintf(stdout, "Creating monstie for pod-name: %s\n", pod_names[i]);
        create_monstie(pod_names[i]);
        free(pod_names[i]);
    }

    free(pod_names);
}
