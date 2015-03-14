#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "ships.h"

#define LOAD_FACTOR (1)
#define MULTIPLIER (37)
#define INITIAL_SIZE (50)


/*Veena Advani
CPSC 223
HW6
Due: 3/25/2015
CITATION: USED EXAMPLE OF HASH TABLE DICTIONARY USING CHAINING TO AID WRITING THIS PROGRAM*/

struct field {
	size_t shipCount; /*num ships in the field*/
	unsigned int shipSize; /*size of coordinate table*/
	struct shipElem **shipTable;
};

/*element for the coordinate hash table, contains a position struct and a pointer to the next position struct*/
struct shipElem {
	struct ship* shipAddress;
	struct shipElem *next; /*SHOULD THIS BE A DEQUE???*/
};

static struct field *fieldCreateInternal(unsigned int size) { /*IS AN INT LARGE ENOUGH FOR THIS???*/
	struct field* field;
	
	field = malloc(sizeof(struct field));
	assert(field);

	field->shipCount = 0;
	field->shipSize = size;

	field->shipTable = malloc(sizeof(struct shipElem*) * field->shipSize); /*want an array of pointers to coorElems, not coorElems themselves*/
	assert(field->shipTable);

	for (int j = 0; j < field->shipSize; j++) {
		field->shipTable[j] = 0;
	}
	return field;
}

/*this function creates a field*/
struct field *fieldCreate(void) {
	return fieldCreateInternal(INITIAL_SIZE);
}

static unsigned long hashShip(struct ship* s) {
	unsigned long h;

	h=0;
	h = h*MULTIPLIER + s->topLeft.x; /*HOW DO I KNOW THIS WILL WORK WITH VARYING TYPES FOR X AND Y???*/
	h = h*MULTIPLIER + s->topLeft.y;

	return h;
}

/*this function creates a shipElem struct for the hash table of ships used in the field*/
static struct shipElem* shipElemCreate (struct ship* shipAddress) {
	struct shipElem *newElem;
	newElem = malloc(sizeof(struct shipElem));
	assert(newElem);
	newElem->shipAddress = shipAddress;
	newElem->next = 0;
	return newElem;
}

/*NEED TO IMPLEMENT THIS*/
static void growTable (struct field* f) {

}

/*Takes in a pointer to a field and two coordinates and returns a pointer to a ship if that ship has it's topleft coordinate
at the input coordinate pair*/
static struct ship* shipLookUp (struct field *f, coord x, coord y) {
	struct shipElem* e;
	unsigned long h;
	h = 0;
	h = h*MULTIPLIER + x;
	h = h*MULTIPLIER + y;
	h = h % f->shipSize;

	for (e=f->shipTable[h]; e!=0; e=e->next) {
		if (((e->shipAddress->topLeft.x) == x) && ((e->shipAddress->topLeft.y) == y)) {
			break;
		}
	}
	if (e != 0) {
		return e->shipAddress;
	} else {
		return 0;
	}
	
}

/*This function takes a pointer to a ship and a pointer to a field, and removes the shipElem struct for that shipAddress from
the hash table and frees all memory it was using*/
static void freeShip (struct ship* s, struct field* f) {
	struct shipElem* e;
	struct shipElem* prev;
	struct shipElem* next;
	unsigned long h;
	h = hashShip(s) % f->shipSize;

	e=f->shipTable[h]; /*ever going to have problem where value is at coorTable[h]???*/
	/*if coorElem we want is first in list*/
	if (e->shipAddress == s) { /*this compares pointer values, right? if equal, they're pointing to the same thing?*/
		f->shipTable[h] = e->next;
		free(e->shipAddress);
		free(e);
		(f->shipCount)--;
	}
	else {
		prev = f->shipTable[h];
		e = f->shipTable[h]->next;

		while (e!=0) { /*DOES THIS NEED TO RETURN AN ERROR IF IT DOESN'T FIND ANYTHING TO FREE?*/
			next = e->next;
			if (e->shipAddress == s) {
				free(e->shipAddress);
				free (e); /*FREEING FULL ELEM - WILL THIS FREE POSITION STRUCT WITHIN IT???*/
				(f->shipCount)--; /*decrement coordinate counter in the field*/
				prev->next = next; /*set next pointer of prev elem to next elem*/
				break; /*got rid of elem we were looking for, break out of while*/
			}
			/*if we know this isn't the block we need to get rid of, then update previous and e pointers for next loop*/
			prev = e;
			e = next;
		}
	}
}

/*First ship is the ship we're checking for a collision, the second ship is the ship that might hit it, the 2 direction inputs
are the directions for each of the ships respectively*/
static int collisionCheck (struct ship* s1, struct ship* s2) {
	int direction1 = s1->direction;
	int direction2 = s2->direction;
	if (direction1 == HORIZONTAL) {
		if (direction2 != direction1) { /*vertical to horizontal collision*/
			if ((s2->topLeft.y + s2->length) > s1->topLeft.y) {
				return 1;
			}
			else { 
				return 0;
			}
		}
		if (direction2 == direction1) { /*horizontal to horizontal collision*/
			if ((s2->topLeft.x + s2->length) > s1->topLeft.x) {
				return 1;
			}
			else {
				return 0;
			}
		}
	}

	if (direction1 == VERTICAL) {
		if (direction2 != direction1) { /*horizontal to vertical collision*/
			if ((s2->topLeft.x + s2->length) > s1->topLeft.x) {
				return 1;
			}
			else {
				return 0;
			}
		}
		if (direction2 == direction1) {
			if ((s2->topLeft.y + s2->length) > s1->topLeft.y) {
				return 1;
			}
			else {
				return 0;
			}
		}
	}
}


static void makeClear (struct field* f, struct ship* s) {
	coord i;
	coord j;
	struct ship* e;
	if (s->direction == VERTICAL) {
		/*checking for vertical ship collisions from above first*/
		if (s->topLeft.y >= (MAX_SHIP_LENGTH - 1)) {
			j = s->topLeft.y - MAX_SHIP_LENGTH + 1;
			i = s->topLeft.x;
		}
		else {
			j = 0;
			i = s->topLeft.x;
		}
		while (j<= (s->topLeft.y + s->length - 1)) {
			e = shipLookUp(f, i, j);
			if ((e != 0) && (e->direction == VERTICAL)) {
				if ((collisionCheck(s, e)) == 1) {
					freeShip(e, f);
				}
			}
			j++;
		}

		/*checking for horizontal ship collisions from side*/

		if (s->topLeft.x >= (MAX_SHIP_LENGTH - 1)) {
			i = s->topLeft.x - MAX_SHIP_LENGTH + 1;
		} else {
			i = 0;
		}

		for (j = s->topLeft.y; j<= (s->topLeft.y + s->length - 1); j++) {
			while (i<= s->topLeft.x) {
				e = shipLookUp(f, i, j);
				if ((e != 0) && (e->direction == HORIZONTAL)) {
					if ((collisionCheck(s, e)) == 1) {
						freeShip(e, f);
					}
				}
				i++;
			}
		}

	}

	if (s->direction == HORIZONTAL) {
		/*checking for vertical ships that could collide from above*/
		if (s->topLeft.y >= (MAX_SHIP_LENGTH - 1)) {
			j = s->topLeft.y - MAX_SHIP_LENGTH + 1;
		} else {
			j = 0;
		}

		for (i = s->topLeft.x; i <= (s->topLeft.x + s->length - 1); i++) {
			while (j<= s->topLeft.y) {
				e = shipLookUp(f, i, j); /*see if ship exists with top left coord at this point*/
				if ((e != 0) && (e->direction == VERTICAL)) {
					if ((collisionCheck(s, e)) == 1) { /*see if ship collides with ship we want to place*/
						freeShip(e, f); /*if ship does collide -> remove it*/
					}
				}
				j++;
			}
		}

		/*checking for horizontal ships that come from sides*/
		if (s->topLeft.x >= (MAX_SHIP_LENGTH - 1)) {
			i = s->topLeft.x - MAX_SHIP_LENGTH + 1;
			j = s->topLeft.y;
		} else {
			i = 0;
			j = s->topLeft.y;
		}

		while (i<= (s->topLeft.x + s->length - 1)) {
			e = shipLookUp(f, i, j);
			if ((e != 0) && (e->direction == HORIZONTAL)) {
				if ((collisionCheck(s, e)) == 1) {
					freeShip(e, f);
				}
			}
			i++;
		}
	}
}



/*this function takes a pointer to a field and a struct ship and puts the ship in the field if the ship fits the necessary qualifications*/
void fieldPlaceShip(struct field *f, struct ship s) { /*NEED TO ADD THAT IT GETS RID OF OLD SHIP IF THERE'S ALREADY A SHIP IN THAT LOCATION!!!*/
	struct ship* ship;
	struct shipElem* newElem;
	unsigned long h;
	ship = malloc(sizeof(struct ship)); /*mallocing memory to store ship struct*/
	assert(ship);
	*ship = s; /*IS THIS HOW YOU COPY ALL THE CONTENTS OF THE STRUCT OVER???*/

	if ((s.name != NO_SHIP_NAME) && (s.length <= MAX_SHIP_LENGTH) && (s.length > 0)) { /*is it <= or <???*/
		
		/*before place ship, need to make sure there's not already a ship there - STILL NEED TO WRITE THIS CODE*/

		if ((s.direction == VERTICAL) && ((s.topLeft.x < COORD_MAX) && ((s.topLeft.y + s.length - 1) < COORD_MAX))) { /*if dealing with vertical ship*/
				makeClear(f, ship);
				
				if (f->shipCount >= (f->shipSize * LOAD_FACTOR)) { /*If we have too many coordinates --> grow hash table*/
					growTable(f); /*NOT IMPLEMENTED YET*/ 
				}	
				h = hashShip(ship) % f->shipSize;
				newElem = shipElemCreate(ship);
				newElem->next = f->shipTable[h]; /*IS THIS OKAY???, the way I'm pushing things onto the stack?*/
				f->shipTable[h]= newElem;
				(f->shipCount)++;
		/*Now if ship is horizontal, do similar procedure*/
		} else if ((s.direction == HORIZONTAL) && (((s.topLeft.x + s.length - 1) < COORD_MAX) && (s.topLeft.y < COORD_MAX))) {
				/*NEED TO IMPLEMENT THIS*/
				makeClear(f, ship);
				if (f->shipCount >= (f->shipSize * LOAD_FACTOR)) {
					growTable(f); /*NOT IMPLEMENTED YET*/ 
				}	
				h = hashShip(ship) % f->shipSize;
				newElem = shipElemCreate(ship);
				newElem->next = f->shipTable[h];
				f->shipTable[h]= newElem;
				(f->shipCount)++;	
		}
		
	}
	
}

/*this function takes in a pointer to a field and a position struct, and if a ship exists in that location, destroys the ship
and returns the name of the ship*/
char fieldAttack(struct field *f, struct position p) {
	struct ship* e;
	/*NEED TO CHECK ALL POSSIBLE POINTS MAXSHIPLENGTH AWAY FROM POSITION COORDINATE - see notes*/
}

size_t fieldCountShips(const struct field *f) {
	return f->shipCount;
}

void fieldDestroy(struct field *f){
	struct shipElem* e;
	for (int i=0; i<(f->shipSize); i++) {
		for (e = f->shipTable[i]; e != 0; e=e->next) {
			free(e->shipAddress);
			free(e);
		}
	}
	free(f->shipTable);
	free(f);
}
