/*
Author: Marco Naß
Matrikelnummer: 980188
Last update: 16.10.2014
*/

#ifndef BTREE_H
#define BTREE_H



// Eintrag:
typedef struct {
	int key;
	void *value;
	struct page *pointer;
} entry;


// Page:
typedef struct{
	struct page *next; 				// P0 Pointer auf die naechste Page
	struct page *parent;			// Zeigt auf Elternknoten
	struct page *brotherLeft;		// Zeigt auf den linken benachbarten Bruder
	struct page *brotherRight;		// Zeigt auf den rechten benachbarten Bruder
	unsigned int NumberOfEntries;
	entry **pageEntries;
	entry *parentsEntry;
	
} page;

// BTree:
typedef struct{
	unsigned int height; 
	unsigned int k; 	// "Pagesize" of the Tree
	page *root;
}btree;

// Pointer Variablen p,r,s zum Suchen und einfuegen + pos fï¿½r die Position
typedef struct{
	page *p;
	page *r;
	page *s;
	entry *pos;
} search_variables;

extern btree* new_btree(unsigned int k);	// Erstelle einen neuen B-Baum inkl. Wurzel Page

#endif
