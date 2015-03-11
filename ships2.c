#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ships.h"
#include "coordinates.h"

#define LOAD_FACTOR (1)
#define MULTIPLIER (37)
#define INITIAL_SIZE (50)


/*Veena Advani
CPSC 223
HW6
Due: 3/25/2015
CITATION: USED EXAMPLE OF HASH TABLE DICTIONARY USING CHAINING TO AID WRITING THIS PROGRAM*/

/*THINGS TO REMEMBER:
WHEN FREEING SHIP - HAVE TO FREE POSITION STRUCT INSIDE IT FIRST??? - ASK SOMEONE
QUESTION: IF YOU'RE HASHING BY COORDINATES --> HOW DO YOU KNOW ALL THE STANDARD MATH STUFF WILL WORK IF TYPE OF COORD NOT GUARANTEED???
CHECK WHETHER ALL VARS ARE LARGE ENOUGH - NEED INTS OR LONGS???
NO HASH TABLE FOR SHIP - NEED AT LEAST AN ARRAY OF SHIP STRUCTS
if ship name is a period --> no ship
if ship length longer than max length --> don't place ship
if ship name is NO_SHIP_NAME or length > MAX_SHIP_LENGTH or any coordinate is > COORD_MAX --> don't place it*/

struct field {
	size_t shipCount; /*num ships in the field*/
	size_t coorCount; /*num coordinates occupied in the field by ships*/
	unsigned int coorSize; /*size of coordinate table*/
	struct coorElem **coorTable;
};

/*element for the coordinate hash table, contains a position struct and a pointer to the next position struct*/
struct coorElem {
	struct ship* shipAddress;
	struct position coor;
	struct position *next; /*SHOULD THIS BE A DEQUE???*/
};

static field *fieldCreateInternal(unsigned int size) { /*IS AN INT LARGE ENOUGH FOR THIS???*/
	struct field* field;
	
	field = malloc(sizeof(struct field));
	assert(field);

	field->shipCount = 0;
	field->coorCount = 0;
	field->coorSize = size;

	field->coorTable = malloc(sizeof(struct coorElem*) * field->coorSize); /*want an array of pointers to coorElems, not coorElems themselves*/
	assert(field->coorTable);

	for (int j = 0; j < field->coorSize; j++) {
		field->coorTable[j] = 0;
	}
}

/*this function creates a field*/
struct field *fieldCreate(void) {
	return fieldCreateInternal(INITIAL_SIZE);
}

static unsigned long hashCoor(coord x, coord y) {
	unsigned long h;

	h=0;
	h = h*MULTIPLIER + x; /*HOW DO I KNOW THIS WILL WORK WITH VARYING TYPES FOR X AND Y???*/
	h = h*MULTIPLIER + y;

	return h;
}

/*this function takes in two coordinates and a field, and returns a pointer to the coorElem
that contains those two points, if it exists in the field, otherwise returns null pointer*/
static struct coorElem* pointLookup (coord x, coord y, field* f) {
	struct coorElem* e;
	unsigned long h;
	h = hashCoor(x, y) % f->coorSize; /*hash point value to find what row it should be in hash table*/

	for (e=f->coorTable[h]; e!=0; e=e->next) { /*now step through linked list to see if the point is there*/
		if ((e->coor.x == x) && (e->coor.y == y)) { /*THIS FUNCTION DOESN'T CHECK THAT THE SHIP ADDRESS ISN'T 0!!!*/
			return e; /*return address of coorElem with that point*/
		}
	}
	else {
		return 0;
	}
}

/*this function doesn't actually destroy the coordinate, just sets the shipaddress to 0
since ship no longer placed in that location*/
static void destroyCoor (struct coorElem* coor) {
	coor->shipAddress = 0;
}

/*This function takes two coordinate values and a pointer to a field, and removes the coorElem struct for that coordinate pair from
the hash table and frees all memory it was using*/
static void freeCoor (coord x, coord y, field* f) {
	struct coorElem* e;
	struct coorElem* prev;
	struct coorElem* oldElem;
	unsigned long h;
	h= hashCoor(x, y) % f->coorSize;

	e=f->coorTable[h]; /*ever going to have problem where value is at coorTable[h]???*/
	/*if coorElem we want is first in list*/
	if ((e->coor.x == x) && (e->coor.y == y)) {
		f->coorTable[h] = e;
		free(e);
	}
	else {
		prev = f->coorTable[h];
		e = f->coorTable[h]->next;

		while (e!=0) { /*DOES THIS NEED TO RETURN AN ERROR IF IT DOESN'T FIND ANYTHING TO FREE?*/
			oldElem = e;
			next = e->next;
			if ((e->coor.x == x) && (e->coor.y == y)) {
				free (oldElem); /*FREEING FULL ELEM - WILL THIS FREE POSITION STRUCT WITHIN IT???*/
				(f->coorSize)--; /*decrement coordinate counter in the field*/
				prev->next = next; /*set next pointer of prev elem to next elem*/
				break; /*got rid of elem we were looking for, break out of while*/
			}
			/*if we know this isn't the block we need to get rid of, then update previous and e pointers for next loop*/
			prev = e;
			e = next;
		}
	}
}



/*this function destroys the field and cleans up all the memory it was using*/
void fieldDestroy(struct field *f) {
	struct coorElem* e;
	struct coorElem* next;
	for (int i=0; i<f->coorSize; i++) {
		e = f->coorTable[i];
		while (e!=0){
			next = e->next;
			/*free(&(e->coor)); /*free position struct within coorElem? - DO WE NEED TO DO THIS?*/
			free(e->shipAddress); /*free ship struct mem that was malloced*/
			free(e); /*free coorElem mem*/
		}
	}
	free(f->coorTable); /*free coorTable*/
	free(f); /*free mem used for field struct*/

}

/*this function creates a coorElem struct for the hash table of coordinates used in the field*/
static struct coorElem* coorElemCreate (coord x, coord y, struct ship* shipAddress) {
	struct coorElem *newElem;
	newElem = malloc(sizeof(struct coorElem));
	assert(newElem);
	newElem->coor.x = x;
	newElem->coor.y = y;
	newElem->shipAddress = shipAddress;
	newElem->next = 0;
	return newElem;
}

/*MESSAGE - NEED TO REHASH EVERYTHING WHEN YOU GROW THE HASH TABLE, this function grows the coordinate table if the number of coordinates occupied becomes too large for it*/
static void growCoors(struct field *f) {
	struct field *f2;
	struct field swap;
	int i;
	struct coorElem *e;
	
	f2 = fieldCreateInternal(f->coorSize*2);

	for (i=0; i < f->coorSize; i++) {
		for (e=f->coorTable[i]; e!=0; e = e->next) {
			fieldPlaceShip(f2, *(e->shipAddress));
		}
	}

	/*NEED TO UNDERSTAND HOW THIS SWAP WORKS BETTER, NEVER MALLOCED ANYTHING FOR SWAP, HOW DOES IT WORK???*/
	swap = *f;
	*f = *f2;
	*f2 = swap;

	fieldDestroy(f2);

	f->coorSize *=2; /*increases size of coorTable to represent increase we just made*/
	
}

/*this function takes a pointer to a field and a struct ship and puts the ship in the field if the ship fits the necessary qualifications*/
void fieldPlaceShip(struct field *f, struct ship s) { /*NEED TO ADD THAT IT GETS RID OF OLD SHIP IF THERE'S ALREADY A SHIP IN THAT LOCATION!!!*/
	struct ship* ship;
	ship = malloc(sizeof(struct ship)); /*mallocing memory to store ship struct*/
	assert(ship);
	*ship = s; /*IS THIS HOW YOU COPY ALL THE CONTENTS OF THE STRUCT OVER???*/

	if ((s.name != NO_SHIP_NAME) && (s.length <= MAX_SHIP_LENGTH) && (s.length > 0)) { /*is it <= or <???*/
		
		/*before place ship, need to make sure there's not already a ship there - STILL NEED TO WRITE THIS CODE*/

		if ((s.direction == VERTICAL) && ((s.topLeft.x < COORD_MAX) && ((s.topLeft.y + s.length - 1) < COORD_MAX))) { /*if dealing with vertical ship*/
			for (coord i = s.topLeft.y; i < (s.topLeft.y + s.length - 1); i++) { /*HOW DO I KNOW THIS WILL WORK WITH ALL POTENTIAL TYPES FOR COOR???*/
				struct coorElem* newElem;
				unsigned long h;
				h = hashCoor(x, i) % f->coorSize;
				newElem = coorElemCreate(s.topLeft.x, i, ship);
				newElem->next = f->coorTable[h]; /*IS THIS OKAY???, the way I'm pushing things onto the stack?*/
				f->coorTable[h]= newElem;
				(f->coorCount)++;

				if (f->coorCount >= (f->coorSize * LOAD_FACTOR)) {
					growCoors(f); /*STILL NEEDS TO BE IMPLEMENTED*/
				}	
			}
			(f->shipCount)++;

		} else if ((s.direction == HORIZONTAL) && (((s.topLeft.x + s.length - 1) < COORD_MAX) && (s.topLeft.y < COORD_MAX))) {
			for (coord i = s.topLeft.x; i<(s.topLeft.x + s.length - 1); i++) {
				struct coorElem* newElem;
				unsigned long h;
				h = hashCoor(i, y) % f->coorSize;
				newElem = coorElemCreate(i, s.topLeft.y, ship);
				newElem->next = f->coorTable[h];
				f->coorTable[h]= newElem;
				(f->coorCount)++;

				if (f->coorCount >= (f->coorSize * LOAD_FACTOR)) {
					growCoors(f); /*STILL NEEDS TO BE IMPLEMENTED*/
				}	
			}
			(f->shipCount)++;	
		}
		
	}
	
}

/*this function takes in a pointer to a field and a position struct, and if a ship exists in that location, destroys the ship
and returns the name of the ship*/
char fieldAttack(struct field *f, struct position p) {
	struct coorElem* e; /*NEED TO BE CAREFUL, CAN'T FREE e BEFORE FREED OTHER coorElems - if freeing is the way we go later*/
	struct coorElem* oldElem;
	struct ship* s;
	char retVal;
	e = pointLookup(p.x, p.y, f); /*see if that location there exists a ship*/
	if ((e==0) || (e->shipAddress == 0)) { /*WOULDN'T NEED THIS SHIP ADDRESS PART IF I USED A DEQUE*/
		return NO_SHIP_NAME;
	}
	else { /*there is a ship in that location*/
		retVal = e->shipAddress->name; /*set return val to the ship name*/
		s = e->shipAddress; /*keep shipAddress so it can be freed after all coorElem addresses set to 0 since ship destroyed*/
		if (s->direction == VERTICAL) { /*ship exists there --> address to ship is e->shipAddress, now need to set all coorElems for that ship to 0*/
			for (int i=(s->topLeft.y); i<(s->topLeft.y + s->length - 1); i++) {
				/*oldElem = pointLookup(p.x, i, f);
				/*we know there is a ship occupying these points --> don't need to check that oldElem or oldElem->shipAddress == 0*/
				freeCoor(p.x, i, f); /*sets ship Address at the coorElem for that point to 0*/
			}
		}
		else if (s->direction == HORIZONTAL) {
			for (int i=(s->topLeft.x); i<(s->topLeft.x + s->length - 1); i++) {
				/*oldElem = pointLookup(i, p.y, f);*/
				freeCoor(i, p.y, f);
			}
		}
		free(s); /*frees ship struct that we've now destroyed*/
		(f->shipCount)--; /*decrease num ships by one, since destroyed the ship*/
		return retVal;
	}
}

size_t fieldCountShips(const struct field *f) {
	return f->shipCount;
}

