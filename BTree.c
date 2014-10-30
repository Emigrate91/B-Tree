/*
Author: Marco Na�
Matrikelnummer: 980188
Last update: 16.10.2014
*/

#include "btree.h"
#include <stdlib.h>
#include <math.h>



enum bool{false, true};

static page* allocatePage(unsigned int k);
static int compareEntries (const entry *_entry, const entry *entryValue);
static entry * lessOrEqual;
page** printLevel(page** levelPages, int level, int k);
page** deleteLevel(page** levelPages, int level, int k);
page *insertOnPage(btree *_btree, page *_page, entry *y);
page *pageSplitting(btree* _btree, page *_page, entry *y);
entry ** mergeEntries(page* left, page *right);

search_variables* search(btree *_btree, entry *y);
int overflow=false;


/** 
*	Erstellt einen neuen, leeren B-Baum
*	@param k die maximale Anzahl neuer Keys pro Page
*	@return einen Pointer, der auf den neu erstellten B-Baum verweist
*/
btree *new_btree(unsigned int k){
	btree *_btree=(btree *)malloc(sizeof (btree));
	
	_btree = malloc(sizeof(*new_btree)); // reserviere Speicher fï¿½r den B-Baum
	_btree->root=NULL;
	_btree->height=0;
	_btree->k=k;	
	return _btree; 
}



/** 
*	Ueberprueft ob der gesuchte Eintrag y im B-Baum _btree vorhanden ist
*	@param  _btree verweist auf den zu durchsuchenden B-Baum 
*	@param  y verweist auf den gesuchten Eintrag
*	@return 0=Eintrag nicht enthalten, 1=Eintrag ist enthalten
*/
int isInBtree(btree *_btree, entry *y){
	
	int returnValue=1;
	
	// Starte Suche
	search_variables* sv=search(_btree, y);	
	
	if(!sv->pos) // Eintrag nicht enthalten
		returnValue=0;
		
	else		// Eintrag enthalten  
		returnValue=1;
}



/** 
*	Fuegt einen Eintrag y dem B-Baum hinzu
*	@param _btree verwei�t auf den B-Baum
*	@param y verweist auf den neuen Eintrag
*	@return 0=Eintrag erfolgreich hinzugefuegt, 1=ein Fehler ist aufgetreten
*/
int insert(btree *_btree, entry *y){
	
	int returnValue=1;
	// Starte Suche
	search_variables* sv=search(_btree, y);	
	
	if(!sv->pos){ // Eintrag nicht enthalten
	page *containsEntry =NULL;
	
		// Baum ist leer
		if(sv->s==NULL) 
			containsEntry=insertInEmptyBTree(_btree, y);
		
		else
			containsEntry=insertOnPage(_btree, sv->s, y);	
			
		if(containsEntry)	
			return 0;			
	}
	
	
	else{		// Eintrag enthalten  
		printf("Entry is already a part of the tree\n");
                return 0;
	}
	freeSearchVariables(sv);
	return returnValue;	
}



/** 
*	Fuegt den Eintrag y einer Page _page hinzu
*	@param  _btree verweist auf den aktuellen B-Baum 
*	@param  _page verweist auf die Page 
*	@param  y verweist auf den einzufuegenden Eintrag
*	@return ein Pointer, welcher auf die Page mit y verweist
*/
page *insertOnPage(btree *_btree, page *_page, entry *y){
	
	unsigned int k=_btree->k;
	
	// Page noch nicht voll:
	if(_page->NumberOfEntries<2*k){
		
		// Fuege y ein und sortiere die Eintraege:
		_page->pageEntries[_page->NumberOfEntries]=y;
		_page->NumberOfEntries++;                
		qsort(_page->pageEntries, _page->NumberOfEntries, sizeof(entry*), compareEntries);
		return _page;
	}
	
	// Page ist voll -> Page-Splitting oder Overflow:	
	else{
		
		//Overflow:
		if((overflow) && ((_page->brotherLeft)||(_page->brotherRight))){
			page *right=NULL;
			page *left=NULL;
			
			// Ueberpruefen der benachbarten Brueder:            
			if((_page->brotherRight) && ((page*)(_page->brotherRight))->NumberOfEntries <2*k){
				right=_page->brotherRight;
				left=_page;
			}
			
			// bevorzugt mit linken Bruder
			if((_page->brotherLeft) && ((page*)(_page->brotherLeft))->NumberOfEntries<2*k){
				left=_page->brotherLeft;
				right=_page;
			}
			
			// Fuehre Overflow durch:
			if(left && right){
				// Page vergroessern:	
				_page->pageEntries=(entry**)realloc(_page->pageEntries,(_page->NumberOfEntries+1)*sizeof(entry*));
				
				// Eintrag hinzufuegen:
				_page->pageEntries[_page->NumberOfEntries]=y;
				_page->NumberOfEntries++;                
				qsort(_page->pageEntries, _page->NumberOfEntries, sizeof(entry*), compareEntries);
				
				// Underflow durchfuehren:
				underflow(_btree, left, right);			
				return _page;	
			}
// >>>>>			
			else
				return pageSplitting(_btree, _page, y);	
// <<<<<		
		}
		
		else
			return pageSplitting(_btree, _page, y);
	}			
}



/** 
*	Erstellt eine neue Wurzel mit dem Eintrag y
*	@param _btree verweist auf den B-Baum 
*	@param y verweist auf den einzufuegenden Eintrag verweist
*	@return 0=Eintrag erfolgreich hinzugefuegt, 1=ein Fehler ist aufgetreten
*/
int insertInEmptyBTree(btree *_btree, entry *y){
	
	// Erstelle neue Wurzelpage:
	_btree->root=allocatePage(_btree->k);
	
	if(_btree->root){
		page *root=_btree->root;
		root->pageEntries[0]=y;
		root->NumberOfEntries++;
		
		// die Wurzel besitzt keine Parent oder benachbarte Page.
		root->parent = NULL; 	
		root->brotherLeft=NULL;
		root->brotherRight=NULL;
		root->parentsEntry=NULL;
		
		_btree->height++;
		return 0;
	}
	
	else 
		return 1;	
}



/** 
*	Fuegt den Eintrag y einer Page _page hinzu und fuehrt ein Page-Splitting durch.
*	@param _btree verweist auf den aktuellen B-Baum
*	@param _page verweist auf die Page 
*	@param y verwesit auf den einzufuegenden Eintrag
*	@return ein Pointer, welcher auf die Page mit y verweist
*/
page *pageSplitting(btree* _btree, page *_page, entry *y){
	page *returnValue=NULL;
	unsigned int k=_btree->k;	   	
	int i=0;
	
	// temporaeres Array mit den Eintraegen von _page + den neuen Eintrag y:
	entry ** temp =(entry**)malloc((2*k+1)*sizeof(entry*));
	        
	for(i=0;i<2*k;i++){
		temp[i]=_page->pageEntries[i];
		_page->pageEntries[i]=NULL; // Page leeren
	}
	temp[2*k]=y;
        
		
	// Sortiere Eintraege:
	qsort(temp, 2*k+1, sizeof(entry*), compareEntries);
	
	// erstelle eine neue Page: _page2:
	page *_page2=allocatePage(k);
	
	//Verteilen der Eintraege auf _page, _page2 und Elternpage
	for(i=0;i<2*k+1;i++){
	        
	    if(i<k){
	        _page->pageEntries[i]=temp[i];
	        if(temp[i]==y)
	            returnValue=_page;
	    }
	
	    if(i>k){
	        _page2->pageEntries[i-(k+1)]=temp[i];
	        if(temp[i]==y)
	            returnValue=_page2;
	    }
    }
	
	_page->NumberOfEntries=k;
	_page2->NumberOfEntries=k;
	
	// Pointer von k+1 auf _page2 verweisen:
	page *save=temp[k]->pointer;
    temp[k]->pointer=_page2;
    _page2->next=save;
    _page2->parentsEntry=temp[k];
	
	// Falls Wurzel:
	if(!_page->parent){
	    // erstelle eine neue Wurzelpage: root:
	    page *root=allocatePage(k);
	
	    // Verweise auf _page und _page2
	    root->next=_page;
	    root->pageEntries[0]=temp[k];
	
		// Falls neuer Eintrag y
	    if(temp[k]==y)
	        returnValue=root;
	
	    root->NumberOfEntries=1;
	    root->brotherLeft=NULL;
	    root->brotherRight=NULL;
	    _page->parent=root;
	
	    //setze neue Wurzel
	    _btree->root=root;
	    _btree->height++;
	}
	
	else{ // Falls keine neue Wurzel benoetigt wird
        	_page->parent=insertOnPage(_btree, _page->parent, temp[k]);
            if(temp[k]==y)
            	returnValue=_page->parent;
        }   
	
    _page2->parent=_page->parent;

    // setze benachbarte Brueder
    _page->brotherRight=_page2;
    _page2->brotherLeft=_page;


	// temp freigeben (nur ersten Pointer, da die anderen auf Eintraege verweisen:
	free(temp);
   
    return returnValue;
}

/** 
*	Durchsucht den B-Baum _btree nach dem Eintrag y
*	@param  _btree verweist auf den zu durchsuchenden B-Baum
*	@param y verweist auf den gesuchten Eintrag
*	@return Pointer der auf die "search_variables" verweist
*/
search_variables* search(btree *_btree, entry *y){
	
	// Initialisierung:
	search_variables *sv=(search_variables*)malloc(sizeof (search_variables));
	sv->r=_btree->root;	// verweist immer auf die Wurzel des Baumes
	sv->p=sv->r;		// verweist immer auf die aktuelle Page
	sv->s=NULL;			// verweist immer auf die "Eltern-Page"
	sv->pos=NULL;  		// verweist auf den Eintrag, falls dieser Teil des B-Baums ist
	
	
	// Solange die aktuelle Page ungleich NULL ist...
	while(sv->p){
		sv->s=sv->p; // sichere aktuelle Page
		
		// Key von y < erster Key der Page:
		if(y->key < sv->p->pageEntries[0]->key){
			sv->p=sv->p->next; // Zeige auf naechste Page	
		}
	
		else{  
			lessOrEqual=NULL; 
			bsearch (&y, sv->p->pageEntries, sv->p->NumberOfEntries, sizeof(entry*), compareEntries);
                      
			
			// Eintrag y gefunden
			if(lessOrEqual->key == y->key){	
				sv->pos=lessOrEqual;
				return sv;			
			}
			
			// Eintrag ist nicht vorhanden -> kleinstes Element wird ausgegeben
			if(lessOrEqual->key < y->key){	
				
				// ist lessOrEqual kleiner als das groesste Indexelement
				if(lessOrEqual->key < sv->p->pageEntries[sv->p->NumberOfEntries-1]->key)
					sv->p=lessOrEqual->pointer;                          
				
				
				// lessOrEqual ist groesser gleich dem groessten Indexelement
				else 
                	sv->p=sv->p->pageEntries[sv->p->NumberOfEntries-1]->pointer;                                  					
			}
		}
	}
	return sv;
}



/** 
*	Compare Methode fuer bsearch, setzt die static Variable "lessOrEqual" auf den gesuchten oder naechst kleineren Eintrag.
*	@param _y verweist auf den gesuchten Key y
*	@param _entryValue verweist auf wechselnde Eintraege des Arrays
*	@return compareEntries welcher Auskunft ueber das Verhaeltnis gibt 
*/
static int compareEntries(const entry *_y, const entry *_entryValue){	


	//Cast:
    entry *entryValue=&(**(entry**)_entryValue);
    entry *y=&(**(entry**)_y);
    
    if(y->key < entryValue->key)
        return -1;
    

    if(y->key  == entryValue->key){
        lessOrEqual=entryValue;
        return 0;
    }
	
    if(y->key  > entryValue->key){
        lessOrEqual=entryValue;
        return 1;
    }	
}




/** 
*	Reserviere speicher fuer eine neue Page:
*	@param k die maximale Anzahl neuer Keys pro neuen Page
*	@return Pointer auf die neue Page
*/
static page *allocatePage(unsigned int k){
	page *_page;
		
	_page = (page*)malloc(sizeof(page));						// reserviere Speicher fuer die Page
	
	_page->pageEntries = (entry**)malloc(2*k*sizeof(entry*)); 	// reserviere Speicher fuer die Eintraege der Page (maximal 2*k)
	
	// Initialisiere alle Eintraege mit NULL
	int i;
	for(i=0; i<2*k; i++)
		_page->pageEntries[i]=NULL;
		
	_page->next = NULL; 										// Eine neue Page wird zunaechst immer als Blatt angesehen
	_page->parent = NULL;										// Eine neue Page hat keinen Elternknoten, da noch nicht im Baum.
	_page->brotherLeft=NULL;
	_page->brotherRight=NULL;
	_page->NumberOfEntries=0;
	return _page;
}   


/** 
*	Zeigt alle Ebenen eines B-Baums _btree an
*	@param  _btree verweist auf den zu anzuzeigenden B-Baum verweist
*/
void print(btree *_btree){
	
	int level=1;
	int h=_btree->height;
	
	page *current = _btree->root;
	
	// Erstelle ein neues Array, welches mit einen Pointer auf die Wurzel initialisiert wird.
	page **levelPages = (page**)malloc(1*sizeof(page*));
	levelPages[0]=current;
	
	// Durchlaufe alle Ebenen:
	for(level=1;level<=h;level++){
		if(levelPages)
        	levelPages=printLevel(levelPages,level, _btree->k);
	}
}



/** 
*	Zeigt alle Pages einer Ebene levelPages an. 
*	@param Pointer-Array levelPages mit den Pages der Ebene
*	@param level bezeichnet die aktuelle Ebene
*	@param k die maximale Anzahl neuer Keys pro neuen Page
*	@return Pointer-Array, welches auf die Pages der naechsten Ebene verweist
*/
page** printLevel(page** levelPages, int level, int k){
	
    int currentPage=0, nextPage=0, i=0; 
    int maxPages = pow((2*k+1),(level-1));
	
	printf("Level %d: ", level);
	
	
	// Erstelle und fuelle Pointer Array auf die naechsten Pages:
	page **nextLevel = (page**)malloc(maxPages*(2*k+1)*sizeof(page*));
		
	for(currentPage=0; currentPage<maxPages; currentPage++){
            
            // Gebe aktuelle Page aus:
            if(levelPages[currentPage])
                printPage(levelPages[currentPage]);
               
            else
                break;
		           
		    // Wenn aktuelle Page Kinder hat, speichere diese in nextLevel
            if(levelPages[currentPage]->next){
                nextLevel[nextPage]=levelPages[currentPage]->next;
                nextPage++;
                for(i=0;i<levelPages[currentPage]->NumberOfEntries;i++){         
                    nextLevel[nextPage]=levelPages[currentPage]->pageEntries[i]->pointer;
                    nextPage++;
                    }
            }
               
            else
                nextLevel=NULL;
	}
 
        // nextLevel mit NULL auffuellen:
        if(nextLevel){
            for(nextPage;nextPage<maxPages*(2*k+1);nextPage++)
                nextLevel[nextPage]=NULL;
        }    
     
	// Speicher freigeben:  
    free(levelPages);
    	
    printf("\n");	
    return nextLevel;        
}



/** 
*	Zeigt alle Keys einer einzelnen Page _page an.
*	@param ein Pointer _page auf die auszugebende Page
*/
void printPage(page* _page){
	
		int i=0;
		printf("[ ");
		
		// Gebe Keys aus:
		for(i=0;i<_page->NumberOfEntries;i++){
			printf("%d",_page->pageEntries[i]->key, i);
			
			if(i+1!=_page->NumberOfEntries) 
				printf(", ");
		}
		printf(" ]");
}



/** 
*	loescht einen Eintrag y aus dem B-Baum _btree
*	@param _btree verweist auf den B-Baum 
*	@param y verweist auf den zu loeschenden Eintrags
*	@return 0=Eintrag erfolgreich hinzugefuegt, 1=Eintrag konnte nicht hizugeuegt werden
*/
int rmv(btree *_btree, entry *y){                                 
    int returnValue=1;

    //  Suche
    search_variables* sv=search(_btree, y);	

	// Eintrag ist enthalten
    if(sv->pos){ 

        // der Eintrag befindet sich auf einer Blatt-Page:
        if(!sv->s->next){
        	
            // Falls letztes Element des Baums:
            if((sv->pos==_btree->root->pageEntries[0])&&(_btree->root->NumberOfEntries==1)){
                _btree->height=0;
                _btree->root=NULL;               
            }
            
            // loesche Eintrag:
            rmvEntry(sv->s, sv->pos, _btree->k);
        }

        else{
                // Durchlaufe alle  Pages entlang der P0 Pointer bis zur Blatt Page
                page *tmp=sv->pos->pointer;
                while(tmp->next)
                        tmp=tmp->next;

                // Ersetze y durch den ersten Eintrag y0 der Page:
                tmp->pageEntries[0]->pointer=sv->pos->pointer;
                ((page*)(sv->pos->pointer))->parentsEntry=tmp->pageEntries[0];
              
                rmvEntry(sv->s,sv->pos, _btree->k);
                insertOnPage(_btree, sv->s, tmp->pageEntries[0]);

                // Loesche y0 und ggf. Wiederherstellung der Struktur:
                rmvEntry(tmp, tmp->pageEntries[0], _btree->k);
                return balance(_btree, tmp);
        }
        
        // ggf. Wiederherstellung der Struktur:
        return balance(_btree, sv->s);
    }
	
	// Eintrag ist nicht enthalten 	
	else{		
		returnValue = 1;
		printf("Key ist nicht vorhanden\n");
	}
	
	freeSearchVariables(sv);
	return returnValue;	
}



/** 
*	Stellt, wenn noetig, die B-Baum Struktur wiederher
*	@param _btree verweist auf den B-Baum 
*	@param _page verweist auf die geaenderte Page
*	@return 0=Wiederherstellen erfolgreich, 1=Fehler
*/
int balance(btree *_btree, page *_page){
	// Wiederherstellung noertig?
	if((_page->NumberOfEntries<_btree->k) && (_page!=_btree->root)){
		page *CatenationBrother=NULL;
		page *left=_page->brotherLeft;
		page *right=_page->brotherRight;
		
		// Pruefe linken benachbarten Bruder:
		if(left){

	        if((left->NumberOfEntries+_page->NumberOfEntries)>=2*_btree->k){
	        	return underflow(_btree,left,  _page); // fuehre Underflow durch
	    	}
	
	        else{
	            if((left->NumberOfEntries+_page->NumberOfEntries)<2*_btree->k)	
	                CatenationBrother=left;	
	        }
		}
		
		// Pruefe linken benachbarten Bruder:
		if(right){
	
	        if((right->NumberOfEntries+_page->NumberOfEntries)>=2*_btree->k){
	        	return underflow(_btree, _page , right); // fuehre Underflow durch
			}
	
	        // Bevorzugte Verkettung mit linken benachbarten Bruder
	        else{
	            if(((right->NumberOfEntries+_page->NumberOfEntries)<2*_btree->k) && !CatenationBrother)	
	                    return catenation(_btree, _page, right);	
	        }
		}
		
		// fuehre Catenation aus
		if(CatenationBrother)	
			return catenation(_btree, CatenationBrother,_page);
		
		else
			return 1;
	}
}



/** 
*	Underflow: Gleichmae�ige Verteilung der Keys zweier Pages
*	@param _btree verweist auf den B-Baum 
*	@param _left verweist auf die linke Page
*	@param _reft verweist auf die recht Page
*	@return 0=Underflow erfolgreich, 1=Fehler
*/
int underflow(btree *_btree, page *left, page *right){
    
        int size=left->NumberOfEntries+right->NumberOfEntries+1;
	int half=(int)ceil(size/2);
	int i=0;
	
	// fuege alle Eintraege temporaer in tmp zusammen:
	entry** tmp=mergeEntries(left, right);
	
	// Befuelle left und right. Setze die nicht genutzten Eintraege auf NULL
	for(i=0; i<size; i++){
            
            if(i<2*_btree->k){
                left->pageEntries[i]=NULL;
                right->pageEntries[i]=NULL;
            }
            if(i<half)
                left->pageEntries[i]=tmp[i];					

            if(i>half)
                right->pageEntries[i-(half+1)]=tmp[i];	
	}
	
	left->NumberOfEntries=half;
	right->NumberOfEntries=size-(half+1);
	
	// Loesche den "doppelten" Key
	rmvEntry(right->parent,right->parentsEntry, _btree->k);
    insertOnPage(_btree, right->parent, tmp[half]);
	tmp[half]->pointer=right;
    right->parentsEntry=tmp[half];
	return 0;	
}



/** 
*	catenation: Verbindet zwei Pages zu einer Pages
*	@param _btree verweist auf den B-Baum 
*	@param _left verweist auf die linke Page
*	@param _reft verweist auf die recht Page
*	@return 0=catenation erfolgreich, 1=Fehler
*/
int catenation(btree *_btree, page *left, page *right){
    int size=left->NumberOfEntries+right->NumberOfEntries+1;
	
	// fuege alle Eintraege temporaer in tmp zusammen:
	entry** tmp=mergeEntries(left, right);
	free(right->pageEntries);
	
	// Loesche den doppelten Key und die alten Eintraege der linken Page
	rmvEntry(left->parent,tmp[left->NumberOfEntries], _btree->k);    
		
    // setze neue Eintraege und groesse der "linken" page.
    left->pageEntries=tmp;
    left->NumberOfEntries=size;
    left->brotherRight=right->brotherRight;
    if(right->brotherRight)
    ((page*)(right->brotherRight))->brotherLeft=left;
    
    
    // Falls die Wurzel keine Page mehr enthaelt, muss die Wurzel geaendert werden:
    if(_btree->root->NumberOfEntries==0){
    	_btree->root=_btree->root->next;
        _btree->root->parent=NULL;
        _btree->height--;
        return 0;    
    }
    
    // Da ein Key aus der Elternpage entfernt wurde, muss ggf. die B-Baumstruktur wiederhergestellt werden:
    return balance(_btree, left->parent);	 
}


/** 
*	Verbindet alle Eintraege zweier Pages + der Eintrag der auf die Rechte Page verweist
*	@param _left verweist auf die linke Page
*	@param _reft verweist auf die recht Page
*	@return einen Pointer, der auf die verbundenen Eintraege verweist
*/
entry ** mergeEntries(page* left, page *right){
	
	int i=0;
	int size=left->NumberOfEntries+right->NumberOfEntries+1;
	
	// fuege alle Eintraege temporaer in returnValue zusammen:
	entry ** returnValue=(entry**)malloc((size*sizeof(entry*)));
	
	for(i=0;i<size;i++){
	        
	if(i<left->NumberOfEntries)
		returnValue[i]=left->pageEntries[i];
	
		
	if(i>left->NumberOfEntries)
		returnValue[i]=right->pageEntries[i-((left->NumberOfEntries)+1)];    
	}
	
	// Verbinde next der rechten Page mit dem Key aus dem Eintrag der Elternpage:
    
    returnValue[left->NumberOfEntries]=right->parentsEntry;
    returnValue[left->NumberOfEntries]->pointer=right->next;
    
	return returnValue;    
}



/** 
*	Loescht den Eintrag y aus der page _page 
*	@param _page verweist auf die Page
*	@param y verweist auf den zu loeschenden Eintrag
*	@param 0=loeschen erfolgreich
*/
int rmvEntry(page * _page, entry *y, int k){   
	entry ** pageEntriesNew=(entry**)calloc(2*k,sizeof(entry*));

	int i=0, found=0;
	
	for(i=0;i<_page->NumberOfEntries;i++){
		
		if(_page->pageEntries[i]==y)
			found++;
		
		else
			pageEntriesNew[i-found]=_page->pageEntries[i];	
              
        }
	_page->NumberOfEntries--;
	_page->pageEntries=pageEntriesNew;
	return 0;       
}

int setOverflow(enum bool _overflow){
	overflow=_overflow;
	return 0;
}



/** 
*	Gibt den Speicher einer search_variables wieder frei
*	@param einen Pointer sv auf eine search_variables
*	@return 0=Freigabe erfolgreich
*/
int freeSearchVariables(search_variables *sv){ 
	free(sv->r);
	free(sv->p);
	free(sv->s);
	free(sv->pos);
	free(sv);
	
	return 0;
}



/** 
*	loescht alle Ebenen eines B-Baums _btree
*	@param  _btree verweist auf den zu anzuzeigenden B-Baum verweist
*/
void deleteBtree(btree *_btree){
	
	int level=1;
	int h=_btree->height;
	
	page *current = _btree->root;
	
	// Erstelle ein neues Array, welches mit einen Pointer auf die Wurzel initialisiert wird.
	page **levelPages = (page**)malloc(1*sizeof(page*));
	levelPages[0]=current;
	
	// Durchlaufe alle Ebenen:
	for(level=1;level<=h;level++){
		if(levelPages)
        	levelPages=deleteLevel(levelPages,level, _btree->k);
	}
       
	free(_btree);
}



/** 
*	Loescht alle Pages einer Ebene levelPages. 
*	@param Pointer-Array levelPages mit den Pages der Ebene
*	@param level bezeichnet die aktuelle Ebene
*	@param k die maximale Anzahl neuer Keys pro neuen Page
*	@return Pointer-Array, welches auf die Pages der naechsten Ebene verweist
*/
page** deleteLevel(page** levelPages, int level, int k){
	
    int currentPage=0, nextPage=0, i=0; 
    int maxPages = pow((2*k+1),(level-1));
	
	
	// Erstelle und fuelle Pointer Array auf die naechsten Pages:
	page **nextLevel = (page**)malloc(maxPages*(2*k+1)*sizeof(page*));
		
	for(currentPage=0; currentPage<maxPages; currentPage++){
            
            if(levelPages[currentPage]){
                // Wenn aktuelle Page Kinder hat, speichere diese in nextLevel
                if(levelPages[currentPage]->next){
                    nextLevel[nextPage]=levelPages[currentPage]->next;
                    nextPage++;
                    for(i=0;i<levelPages[currentPage]->NumberOfEntries;i++){         
                        nextLevel[nextPage]=levelPages[currentPage]->pageEntries[i]->pointer;
                        nextPage++;
                        }
                }

                else
                    nextLevel=NULL;
                
                //loesche Page:
                free(levelPages[currentPage]);
            }
            
            else
                break;
	}
 
        // nextLevel mit NULL auffuellen:
        if(nextLevel){
            for(nextPage;nextPage<maxPages*(2*k+1);nextPage++)
                nextLevel[nextPage]=NULL;
        }    
     
	// Speicher freigeben:  
    free(levelPages);
    return nextLevel;        
}



