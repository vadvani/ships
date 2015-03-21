#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "ships.h"

#define LOAD_FACTOR (1)
#define MULTIPLIER (37)
#define INITIAL_SIZE (50)
#define GROWTH_FACTOR (2)


/*Veena Advani
CPSC 223
HW6
Due: 3/25/2015
CITATION: USED EXAMPLE OF HASH TABLE DICTIONARY USING CHAINING TO AID WRITING THIS PROGRAM

THINGS TO DO/FIX/CHECK: 
READ THROUGH SHIPS.H FILE, INSTRUCTIONS ONLINE and PIAZZA QUESTIONS
Finish commenting makeClear and findAndDestroy
check if places where I use int, it will be large enough
turn inputs to const if they should be
INT LARGE ENOUGH FOR SHIPSIZE???
HOW DOES SWAP WORK? - NEVER MALLOCED SPACE FOR IT
WILL ANYTHING CHANGE IF COORD BECOMES U-64INT???
WHAT IF MAX SHIP LENGTH = 0???
SIMPLIFY MAKE CLEAR TO TWO CASES INSTEAD OF FOUR - DO SAME FOR OTHER FUNCTION AS WELL - collisionCheck
make sure whenever iterating through coord values, using type coord
READ SHIPS.H MORE THOROUGHLY TO SEE HOW THIS IMPLEMENTATION NEEDS TO INTERFACE WITH ANY MAIN PROGRAM
THINK OF WAYS HE COULD TRY TO BREAK THIS PROGRAM!!!*/

/*this defines the field struct used for the field of ships, it contains a count with the number of ships in the field, an integer to represent
the size of the ship hash table currently, and a linked list hash table of ships*/
struct field {
	size_t shipCount; /*num ships in the field*/
	size_t shipSize; /*size of coordinate table - IS THIS LARGE ENOUGH?*/
	struct shipElem **shipTable; 
};

/*element for each ship in the hash table, contains a pointer to a ship and a pointer to the next ship struct*/
struct shipElem {
	struct ship* shipAddress;
	struct shipElem *next; 
};

/*this is the internal field Create function, which takes in a size parameter and returns a pointer to a field with a hash table of that size*/
static struct field *fieldCreateInternal(size_t size) { /*changed this from int to size_t*/
	struct field* field;
	
	field = malloc(sizeof(struct field)); /*malloc memory for the field*/
	assert(field); /*make sure malloc worked*/

	field->shipCount = 0;
	field->shipSize = size;

	field->shipTable = malloc(sizeof(struct shipElem*) * field->shipSize); /*malloc memory for an array of pointers to shipElems*/
	assert(field->shipTable); /*make sure malloc worked*/

	for (size_t j = 0; j < field->shipSize; j++) { /*CHANGED J TO SIZE_T AS WELL - THIS ALLOWED???*/
		field->shipTable[j] = 0; /*initialize all pointers to 0, so that they're the end elements of each linked list*/
	}

	return field; /*return pointer to the newly created field*/
}

/*this function creates a field with an initial size of 50*/
struct field *fieldCreate(void) {
	return fieldCreateInternal(INITIAL_SIZE);
}

/*this function takes a pointer to a ship and returns a hash value for the ship based on it's top left coordinate values*/
static unsigned long hashShip(const struct ship* s) {
	unsigned long h;

	h=0;
	h = h*MULTIPLIER + s->topLeft.x;
	h = h*MULTIPLIER + s->topLeft.y;

	return h;
}

/*this function takes in a pointer to a ship and returns a pointer to a shipElem struct with that shipAddress 
for the hash table of ships used in the field*/
static struct shipElem* shipElemCreate (const struct ship* shipAddress) {
	struct shipElem *newElem;
	newElem = malloc(sizeof(struct shipElem)); /*malloc memory for the shipElem struct*/
	assert(newElem); /*make sure malloc worked*/
	newElem->shipAddress = shipAddress; /*set the shipAddress in the new ship struct to the input shipAddress.*/
	newElem->next = 0; /*set next pointer to 0 for now, it'll be reset when element put into a linked list*/
	return newElem; /*return a pointer to this new element*/
}

/*this function takes in a pointer to a field and a pointer to a shipElem and then inserts a new, identical ship element 
into the hash table of that field, used for transfering shipElem's over when growing the hash table*/

static void internalShipInsert (struct field* f, const struct shipElem* e) {
	unsigned long h;
	struct shipElem* newElem;
	
	h = hashShip(e->shipAddress) % f->shipSize; /*finds hash value of the shipElem*/
	newElem = shipElemCreate(e->shipAddress); /*creates new shipElem*/
	newElem->next = f->shipTable[h]; /*put new element into front of linked list*/
	f->shipTable[h]= newElem;
	(f->shipCount)++; /*increase shipCount in the field*/
}

/*this function takes in a pointer to a field, and destroys the field and it's shipElems without freeing the memory used to allocate each ship.
This function is used when a new, larger field needs to be constructed, we still want to keep the ships around, we just need to destroy
the memory used for the shipElem structs and field structs*/

static void growFieldDestroy(struct field *f) {
	struct shipElem* e; /*shipElem struct uses to free things*/
	struct shipElem* next; /*shipElem struct used to keep track of next Elem*/
	
	for (size_t i=0; i<(f->shipSize); i++) { /*walk through entire array of linked lists - CHANGED i TO SIZE_T - THIS OKAY???*/
		e = f->shipTable[i];
		while (e != 0) { /*walk through the linked list*/
			next = e->next; 
			free(e); /*free the memory used for that shipElem struct*/
			e = next; /*move on to next one*/
		}
	}

	free(f->shipTable); /*free memory used for array of linked lists*/
	free(f); /*free memory used for field struct*/
}

/*this function takes a pointer to a field and creates a new field with a larger hash table, and replaces the old
one with it*/
static void growTable (struct field* f) {
	struct field* f2; /*new field struct pointer*/
	struct field swap; /*swap field used to switch out contents of new and old fields*/
	size_t i; /*changed i to size_t*/
	struct shipElem *e; /*ship Elem we're using to walk though each linked list*/

	f2 = fieldCreateInternal(f->shipSize * GROWTH_FACTOR); /*create new field with hash table size twice the size of the previous one*/

	for (i = 0; i < f->shipSize; i++) { /*walk through array of linked lists*/
		for (e = f->shipTable[i]; e != 0; e = e->next) { /*walk through linked list*/
			internalShipInsert(f2, e); /*insert a new shipElem identical to e into the new field f2*/
		}
	}
	
	/*swap out contents of old and new fields so pointer f points to new, larger field- HOW DOES THIS WORK, NEVER MALLOCED FOR SWAP???*/
	swap = *f;
	*f = *f2;
	*f2 = swap;
	
	growFieldDestroy(f2); /*destroys old field*/

}


/*Takes in a pointer to a field and two coordinates and returns a pointer to a ship if that ship has it's topleft coordinate
at the input coordinate pair*/
static struct ship* shipLookUp (const struct field *f, const coord x, const coord y) {
	struct shipElem* e; /*variable to store pointer to shipElem as we walk through elements of linked list*/
	unsigned long h; /*variable for hash value*/
	
	/*hash input coordinate*/
	h = 0;
	h = h*MULTIPLIER + x;
	h = h*MULTIPLIER + y;
	h = h % f->shipSize;

	for (e=f->shipTable[h]; e!=0; e=e->next) { /*walk though linked list*/
		if (((e->shipAddress->topLeft.x) == x) && ((e->shipAddress->topLeft.y) == y)) { /*if the shipElem in the list has the same topLeft coordinates*/
			break; /*stop looking through linked list*/
		}
	}

	if (e != 0) { /*if e doesn't = 0 --> a ship was found in for loop above --> return the address of the ship we want*/
		return e->shipAddress; 
	} else { /*if no ship was found, e = 0 --> return 0 (null pointer)*/
		return 0;
	}
	
}

/*This function takes a pointer to a ship and a pointer to a field, and removes the shipElem struct for that shipAddress from
the hash table and frees all memory it was using*/
static void freeShip (struct ship* s, struct field* f) {
	struct shipElem* e; /*ship Elem we're using to walk through list and free ships*/
	struct shipElem* prev; /*previous elem to e*/
	struct shipElem* next; /*next element after e*/
	unsigned long h;
	h = hashShip(s) % f->shipSize;

	e=f->shipTable[h]; /*setting e at beginning of linked list*/
	/*if the shipElem we want is first in list --> need to free it differently*/
	if (e->shipAddress == s) { /*if e is pointing to same shipAddress as the ship we're looking for*/
		f->shipTable[h] = e->next; /*set front of linked list to the next element*/
		free(e->shipAddress); /*free the ship memory*/
		free(e); /*free the ship struct we're removing*/
		(f->shipCount)--; /*decrement field shipCounter, ship just removed*/
	}
	else { /*ship is not the first of the linked list --> removing process different*/
		prev = f->shipTable[h];
		e = f->shipTable[h]->next; /*NEED TO FIX??? not a problem currently cause always freeing things we know exist... - make sure when you're calling this, f->shipTable[h] IS NOT A NULL POINTER*/

		while (e!=0) { /*while e isn't at the end of the list*/
			next = e->next; /*set the next element*/
			if (e->shipAddress == s) { /*if this is the ship we want --> free everything and decrement ship count*/
				free(e->shipAddress);
				free (e);
				(f->shipCount)--; /*decrement coordinate counter in the field*/
				prev->next = next; /*set next pointer of prev elem to next elem -> patch hole in linked list*/
				break; /*got rid of elem we were looking for, break out of while loop*/
			}
			/*if we know this isn't the block we need to get rid of, then update previous and e pointers for next loop*/
			prev = e;
			e = next;
		}
	}
}

/*This function takes in two pointers to ship structs, the first ship is the ship we're checking for a collision, the second ship 
is the ship that might hit it.  The function returns one if the ships collide and 0 if they do not*/
/* COULD WE SIMPLIFY THIS TO TWO CASES? -> direction 2 is horizontal --> check x coordinate overlap.  direction 2 is vertical --> check y coordinate overlap*/
static int collisionCheck (const struct ship* s1, const struct ship* s2) {
	int direction1 = s1->direction;
	int direction2 = s2->direction;
	if (direction1 == HORIZONTAL) {
		if (direction2 != direction1) { /*vertical to horizontal collision*/
			if ((s2->topLeft.y + s2->length) > s1->topLeft.y) { /*DO WE NEED >=??? DO WE NEED TO CHECK X COORDS AT ALL? OR ASSUMING INPUT SHIPS ALREADY SATISFY THAT?*/
				return 1;
			}
		}
		if (direction2 == direction1) { /*horizontal to horizontal collision*/
			if ((s2->topLeft.x + s2->length) > s1->topLeft.x) { /*DO WE NEED >=??? DO WE NEED TO CHECK Y COORDINATES?*/
				return 1;
			}
		}
	}

	if (direction1 == VERTICAL) {
		if (direction2 != direction1) { /*horizontal to vertical collision*/
			if ((s2->topLeft.x + s2->length) > s1->topLeft.x) {
				return 1;
			}
		}
		if (direction2 == direction1) { /*vertical to vertical collision*/
			if ((s2->topLeft.y + s2->length) > s1->topLeft.y) {
				return 1;
			}
		}
	}
	return 0;
}

/*This function takes in a pointer to a ship struct and two coordinates and returns 1 if the ship occupies that point and 0 if it does not*/
static int pointOccupied (const struct ship* s, const coord x, const coord y) {
	if (s->direction == VERTICAL) {
		if ((s->topLeft.x == x) && (s->topLeft.y <= y) && ((s->topLeft.y + s->length - 1) >= y)) {
			return 1;
		}
	}

	if (s->direction == HORIZONTAL) {
		if ((s->topLeft.y == y) && (s->topLeft.x <= x) && ((s->topLeft.x + s->length - 1) >= x)) {
			return 1;
		} 
	}

	return 0;
}

/*this function takes in a pointer to a field and a pointer to a ship struct and makes sure that there are no ships that collide with the
position of the input ship.  If there are, it destroys those ships*/

static void makeClear (struct field* f, struct ship* s) {
	/*coordinates to iterate through all possible topLeft coord pairs*/
	coord i;
	coord j;
	/*initial value of coordinates, so that i and j can be reset at beginning of loop if iterating through multiple rows/columns*/
	coord initialj;
	coord initiali;
	struct ship* e; /*variable to store pointer to colliding ship struct we are searching for*/
	if (s->direction == VERTICAL) { /*if the input ship is vertical - two potential types of collisions, vertical ships from above, or horizontal ships from the side*/
		/*checking for vertical ship collisions from above first*/
		if (s->topLeft.y >= (MAX_SHIP_LENGTH - 1)) { /*if the ship y coordinate is larger than max ship length - 1 --> set initial j coordinate max ship length -1 away from topLeft y*/
			j = s->topLeft.y - MAX_SHIP_LENGTH + 1;
			i = s->topLeft.x; /*looking for vertical ship collision --> i stays at x coordinate of the vertical input ship*/
		}
		else {
			j = 0; /*if topLeft y wasn't large enough --> start j at 0*/
			i = s->topLeft.x; /*still want i at topLeft x coord*/
		}
		while (j<= (s->topLeft.y + s->length - 1)) {
			e = shipLookUp(f, i, j); /*returns a pointer to a ship with topLeft at (i,j) if it exists, otherwise returns 0*/
			if ((e != 0) && (e->direction == VERTICAL)) {
				if ((collisionCheck(s, e)) == 1) {
					freeShip(e, f);
				}
			}
			if (j== COORD_MAX) {
				break;
			}
			j++;
		}

		/*checking for horizontal ship collisions from side*/

		if (s->topLeft.x >= (MAX_SHIP_LENGTH - 1)) {
			initiali = s->topLeft.x - MAX_SHIP_LENGTH + 1;
		} else {
			initiali = 0;
		}

		for (j = s->topLeft.y; j<= (s->topLeft.y + s->length - 1); j++) {
			i = initiali;
			while (i<= s->topLeft.x) {
				e = shipLookUp(f, i, j);
				if ((e != 0) && (e->direction == HORIZONTAL)) {
					if ((collisionCheck(s, e)) == 1) {
						freeShip(e, f);
					}
				}
				if (i == COORD_MAX) {
					break;
				}
				i++;
			}
			if (j == COORD_MAX) {
				break;
			}
		}

	}

	if (s->direction == HORIZONTAL) {
		/*checking for vertical ships that could collide from above*/
		if (s->topLeft.y >= (MAX_SHIP_LENGTH - 1)) {
			initialj = s->topLeft.y - MAX_SHIP_LENGTH + 1;
		} else {
			initialj = 0;
		}

		for (i = s->topLeft.x; i <= (s->topLeft.x + s->length - 1); i++) {
			j = initialj;
			while (j<= s->topLeft.y) {
				e = shipLookUp(f, i, j); /*see if ship exists with top left coord at this point*/
				if ((e != 0) && (e->direction == VERTICAL)) {
					if ((collisionCheck(s, e)) == 1) { /*see if ship collides with ship we want to place*/
						freeShip(e, f); /*if ship does collide -> remove it*/
						/*printf("freed ship");*/
					}
				}
				if (j == COORD_MAX) {
					break;
				}
				j++;
			}
			if (i == COORD_MAX) {
				break;
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
					/*printf("freed ship");*/
				}
			}
			if (i == COORD_MAX) {
				break;
			}
			i++;
		}
	}
}



/*this function takes a pointer to a field and a ship struct and puts the ship in the field if the ship fits the necessary qualifications*/
void fieldPlaceShip(struct field *f, struct ship s) { 
	struct ship* ship; /*variable to store pointer to new ship struct that's getting stored and inserted into field*/
	struct shipElem* newElem; /*variable to store pointer to new shipElem for ship that's getting inserted*/
	unsigned long h; /*variable used to store hash value of new ship to place it in appropriate linked list*/

	if ((s.name != NO_SHIP_NAME) && (s.length <= MAX_SHIP_LENGTH) && (s.length > 0)) { /*is it <= or <???*/

		if ((s.direction == VERTICAL) && ((s.topLeft.x <= COORD_MAX) && (s.topLeft.y <= COORD_MAX) && ((s.topLeft.y + s.length - 1) >= s.topLeft.y))) { /*if dealing with vertical ship*/
				ship = malloc(sizeof(struct ship)); /*mallocing memory to store ship struct*/
				assert(ship); /*make sure malloc worked*/
				*ship = s; /*IS THIS HOW YOU COPY ALL THE CONTENTS OF THE STRUCT OVER???*/
				makeClear(f, ship); /*if ships already exist in points where new ship is to be placed --> clear the area, destroy those ships*/
				
				if (f->shipCount >= (f->shipSize * LOAD_FACTOR)) { /*If we have too many coordinates --> grow hash table*/
					growTable(f);
				}	
				/*insert ship into appropriate linked list and increment field shipCounter*/
				h = hashShip(ship) % f->shipSize;
				newElem = shipElemCreate(ship);
				newElem->next = f->shipTable[h]; 
				f->shipTable[h]= newElem;
				(f->shipCount)++;
		
		/*Now if ship is horizontal, do similar procedure as above for vertical ship*/
		} else if ((s.direction == HORIZONTAL) && ((s.topLeft.x <= COORD_MAX) && ((s.topLeft.x + s.length - 1) >= s.topLeft.x) && (s.topLeft.y <= COORD_MAX))) {
				ship = malloc(sizeof(struct ship)); 
				assert(ship);
				*ship = s; 
				makeClear(f, ship);
				if (f->shipCount >= (f->shipSize * LOAD_FACTOR)) {
					growTable(f); 
				}	
				h = hashShip(ship) % f->shipSize;
				newElem = shipElemCreate(ship);
				newElem->next = f->shipTable[h];
				f->shipTable[h]= newElem;
				(f->shipCount)++;	
		}
		
	}
	
}


/*this function takes in a pointer to a field, a position struct and two coordinates, and sees if a ship exists with a topLeft coord
at (i, j), if such a ship exists, it then checks to see if that ship occupies position p, and if it does, it destroys the ship, 
frees all memory the ship was using and returns the name of the ship.  Otherwise it returns NO_SHIP_NAME*/
static char findAndDestroy (struct field* f, struct position p, coord i, coord j) {
	char retVal;
	struct shipElem* e;
	struct shipElem* prev;
	struct shipElem* next;
	unsigned long h;
	retVal = NO_SHIP_NAME;

	h = 0;
	h = h*MULTIPLIER + i;
	h = h*MULTIPLIER + j;
	h = h % f->shipSize;

	e=f->shipTable[h];
	if (e == 0) { /*no ship at this coordinate --> return '.' */
		return retVal;
	}
	/*if shipElem we want is first in list*/
	else if ((e->shipAddress->topLeft.x == i) && (e->shipAddress->topLeft.y == j)) { /*if ship located at correct point*/
		if ((pointOccupied(e->shipAddress, p.x, p.y)) == 1) { /*ship occupies attack position point --> need to free it*/
			f->shipTable[h] = e->next; /*patch hole in linked list*/
			retVal = e->shipAddress->name; /*set return value to shipname*/
			free(e->shipAddress); /*free memory for ship we're destroying*/
			free(e); /*free memory for shipElem struct*/
			(f->shipCount)--; /*decrement field shipCounter*/
			return retVal; /*return ship Name*/
		}
	}
	else {
		prev = f->shipTable[h];
		e = f->shipTable[h]->next;

		while (e!=0) { /*DOES THIS NEED TO RETURN AN ERROR IF IT DOESN'T FIND ANYTHING TO FREE?*/
			next = e->next;
			if ((e->shipAddress->topLeft.x == i) && (e->shipAddress->topLeft.y == j)) { /*if ship located at correct point*/
				if ((pointOccupied(e->shipAddress, p.x, p.y)) == 1) { /*ship occupies attack position point --> need to free it*/
					retVal = e->shipAddress->name; /*setting return value to the shipName*/
					free(e->shipAddress); /*freeing memory the ship used*/
					free (e); /*freeing shipElem struct*/
					(f->shipCount)--; /*decrement coordinate counter in the field*/
					prev->next = next; /*set next pointer of prev elem to next elem*/
					return retVal; /*got rid of elem we were looking for, return shipName*/
				}
			}
		/*if we know this isn't the block we need to get rid of, then update previous and e pointers for next loop*/
			prev = e;
			e = next;
		}
	}

	return retVal; /*if haven't returned anything by now --> reach this line and return unchanged retVal, which is NO_SHIP_NAME, since no ship found, attack missed*/

}

/*this function takes in a pointer to a field and a position struct, and if a ship exists in that location, destroys the ship
and returns the name of the ship*/
char fieldAttack(struct field *f, struct position p) {
	/*coordinate variable counters to walk through all possible topLeft coordinates ships could have to overlap with the attacked position*/
	coord i;
	coord j;
	char retVal; /*stores function return value*/
	retVal = NO_SHIP_NAME; /*initializes the value to no ship found*/
	
	/*checking vertical coordinates above attacked position, as far away as MAX_SHIP_LENGTH*/
	
	if ((p.x > COORD_MAX) || (p.y > COORD_MAX)) { /*if coordinate being attacked is out of COORD_MAX range --> don't try to look for it, return '.'*/
		return retVal;
	} 
	/*if the y position attacked is greater than or equal to MAX_SHIP_LENGTH - 1 --> initialize the j coord to MAX_SHIP_LENGTH - 1 above y position*/
	if (p.y >= (MAX_SHIP_LENGTH - 1)) {
		j = p.y - MAX_SHIP_LENGTH + 1;
		i = p.x; /*initialize i coord to x position*/
	}
	else { /*otherwise, y position too small --> just start j at 0.*/
		j = 0; 
		i = p.x; /*still start i at x position coordinate*/
	}

	while ((j<= p.y) && (retVal == NO_SHIP_NAME)) { /*test all j values till you reach the y position coordinate or you find a ship that occupies that point --> retVal != NO_SHIP_NAME*/
		
		/*use findAnd Destroy to see if ship exists with topleft at that point, then see if it occupies position p, then free ship if it does and set return value to shipName*/
		retVal = findAndDestroy(f, p, i, j);
		
		if (j == COORD_MAX) { /*if j == COORD_MAX --> incrementing j --> overflow --> loop won't end --> if j reaches this --> break out of while loop instead*/
			break;
		}
		
		j++;
	}

	/*horizontal coordinate check*/
	if (retVal == NO_SHIP_NAME) { /*if retVal still equals NO_SHIP_NAME --> need to check coordinates left of position coordinate up to max ship length away*/
		if (p.x >= (MAX_SHIP_LENGTH - 1)) { /*if p.x greater than max ship length - 1 --> just start i at p.x - max ship length + 1*/
			i = p.x - MAX_SHIP_LENGTH + 1;
			j = p.y; /*start j at y position coordinate*/
		} else { /*otherwise, p.x isn't large enough, just start i at 0*/
			i = 0;
			j = p.y; /*still start j at y position coordinate*/
		}

		while ((i <= p.x) && (retVal == NO_SHIP_NAME)) { /*test all i values till you reach the x attack position coordinate or a ship is found*/
		/*use findAndDestroy see if ship exists with topleft at that point, then see if ship occupies position p, then free ship if needed and set retVal to ship name*/
		retVal = findAndDestroy(f, p, i, j);
		if (i == COORD_MAX) { /*if i reaches coord max --> when incremented it will overflow --> break out of while loop instead*/
			break;
		}
		i++;
		}
	}  
	return retVal;	
}

/*this function takes in a pointer to a field and returns the number of ships in the field*/
size_t fieldCountShips(const struct field *f) {
	return f->shipCount;
}

/*this function takes in a pointer to a field and frees all the memory the field is using*/
void fieldDestroy(struct field *f){
	struct shipElem* e; /*ship elem pointer used to walk though linked lists*/
	struct shipElem* next; /*ship elem pointer used to keep track of next elem to look at after current one is freed*/
	
	for (size_t i=0; i<(f->shipSize); i++) { /*walk through array of linked lists*/
		e = f->shipTable[i]; /*set e to front of list*/
		while (e != 0) { /*while e isn't a null pointer --> at end of list*/
			next = e->next; /*set next elem to check, then free memory for the ship and the shipElem struct*/
			free(e->shipAddress);
			free(e);
			e = next; /*set e to the next element for the next loop*/
		}
	}
	free(f->shipTable); /*free the array of linked lists within the field struct*/
	free(f); /*free the field struct*/
}
