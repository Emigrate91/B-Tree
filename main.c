/*
Author: Marco Naﬂ
Matrikelnummer: 980188
Last update: 16.10.2014
*/

#include <stdio.h>
#include <stdlib.h>
#include "BTree.h"

/*
Universale Routine zum Loeschen des Bildschirms
von: http://openbook.galileocomputing.de/c_von_a_bis_z/010_c_praeprozessor_003.htm#mj2f99411b8f80983e466dba2632c02acb
*/
#ifdef __unix__
   #define clrscr() printf("\x1B[2J")
#elif __BORLANDC__ && __MSDOS__
   #include <conio.h>
#elif __WIN32__ || _MSC_VER
   #define clrscr() system("cls")
#else
   #define clrscr() printf("clrscr() - Fehler!!\n")
#endif

typedef enum{false, true}bool;
const char* onOff[2] = {"off","on"};



int main(void) {
	run();
	return 0;
}

void run(){
	btree* _btree=NULL;
    bool printMode=true;
    bool overflows=false;
    bool validCommand=false;
    
	char input[50]="";
	
    //Begruessung
    printf("#################################################################################\n");
    printf("# This program simulates b-tree operations with empty entries: (key, NULL, NULL)#\n");
	printf("#             Real entries can only be used with the BTree.c and BTree.h        #\n");
	printf("#                     commands can be found by typing \'help\'.                   #\n"); 
	printf("#                                %c Have Fun %c                                   #\n",1,1);
	printf("#################################################################################\n");

	// warten bis input="exit"
	while(strcasecmp(input , "exit\n")){
            printf(">");
            validCommand=false;
            fgets(input, sizeof(input), stdin);
        
            // Print
            if(!strcasecmp(input , "print\n")){
            	validCommand=true;
                if(_btree)
                    print(_btree);
                else{
                    printf("no B-Tree was created\n");
                }
            } // Ende Print
			
			
			
            // Help
            if((!strcasecmp(input , "help\n"))||(!strcasecmp(input , "man\n"))){
            	validCommand=true;
            	printf("Commands:\n");
				printf("add [key1] [key2] [stepSize] \t- adds [key] or from [key1] to [key2] with a [stepSize] gap to the b-tree\n");
				printf("clear \t\t\t\t- clears the screen\n");
				printf("del [key] / rmv [key] \t\t- delete [key] from the b-tree\n");
				printf("delTree / rmvTree / reset \t- delete the whole b-tree\n");
				printf("exit \t\t\t\t- exit this program\n");
				printf("help / man \t\t\t- shows this dialog\n");
				printf("newTree [k] \t\t\t- creates a new b-tree with parameter [k]\n");
				printf("overflows [on/off]/[true/false] - adding keys with overflows \n");
				printf("print \t\t\t\t- prints the b-tree\n");
				printf("printmode [on/off]/[true/false] - shows an updated b-tree after del and add\n");
				printf("search [key] \t\t\t- checks if [key] is a part of the b-tree\n");
				printf("settings \t\t\t- displays the current settings\n");    
            } // Ende Help			
			
			        

            // Add
            if(!strncasecmp(input , "add", 3)){
            	validCommand=true;
            	int key1=NULL;
            	int key2=NULL;
            	int stepSize=1;
            	
				// Pruefen ob gueltiger Key/Keys
				if(sscanf( input,  "%*3s%d%", &key1)){	           	
					if(_btree){	

						// Falls Sequenz
						if(sscanf( input,  "%*3s%*d%*[-]%d%d", &key2, &stepSize))
							printf("%d   %d \n", key1, key2);
							
						// Falls keine Sequenz
						else	
							key2=key1;
						
						// Printmode on	:				
						if(printMode){
							clrscr();
							insertSequence(_btree, key1, key2, stepSize);
							print(_btree);
						}
						
						// Printmode off:
						else
							insertSequence(_btree, key1, key2, stepSize);
					
					if(key1>key2)
						printf("invalid - Key1 < Key2\n");
					
					}
					
					else
						printf("no B-Tree was created\n");
				}
				
				else
					printf("invalid key(s)\n");
			} // Ende Add
	
			
	
			// Delete B-Tree
            if((!strncasecmp(input , "delTree", 7))||(!strncasecmp(input , "rmvTree", 7))||(!strncasecmp(input , "reset", 5))){
            	    validCommand=true;  	
					if(_btree){
						deleteBtree(_btree);
						_btree=NULL;
					}
							
					// Kein B-Baum vorhanden:
					else
	                    printf("no B-Tree was created\n");
	                    
	                    
			input[0]=" ";	
            } // Ende Delete B-Tree      				
					
			// Delete
            if((!strncasecmp(input , "del", 3))||(!strncasecmp(input , "rmv", 3))){
            	validCommand=true;
				int key=NULL;
            				
				// Key >=0
				if(sscanf( input,  "%*3s%d", &key )){	           	
					if(_btree){	
						
						// Printmode on	:				
						if(printMode){
							clrscr();
							deleteKey(_btree, key);
							print(_btree);
						}
						
						// Printmode off:
						else
							deleteKey(_btree, key);
					}		
					
					// Kein B-Baum vorhanden:
					else
	                    printf("no B-Tree was created\n");
				}
            	
				else
					printf("invalid key\n");				
				
            } // Ende Delete
            
						
            
			// new B-Tree
            if(!strncasecmp(input , "newTree", 7)){
            	    validCommand=true;
					int k=-1;
            		sscanf( input,  "%*7s%d", &k );   	
            	    
					if(k>0){  
						// loesche alten Baum 	
						if(_btree){
							deleteBtree(_btree);
							_btree=new_btree(k);
						}
								
						// Kein B-Baum vorhanden:
						else
		                    _btree=new_btree(k);
	            	}
	            	
	            	else
						printf("invalid k\n");					
            } // Ende new B-Tree  
			
			
			
			// clear
            if(!strncasecmp(input , "clear", 4)){
				validCommand=true;
				clrscr();
			}// Ende clear
            
            
			
			// show settings
            if(!strncasecmp(input , "settings", 8)){
				validCommand=true;
				clrscr();
				printf("Printmode = %s\n", onOff[printMode]);
				printf("Use Overflows = %s\n", onOff[overflows]);
			
			}
			
            // Ende show setting
			
			
						
			//	search Key
            if(!strncasecmp(input , "search", 6)){
            	    validCommand=true;
					int key=-1, diff;
            		sscanf( input,  "%*6s%d", &key );   	
            	    
					if(key>0){  
						// Baum vorhanden?
						if(_btree){						
							
							if(searchKey(_btree, key))
								printf("%d is part of the tree ", key);
								
							else
								printf("%d is NOT a part of the tree ", key);								
						}
								
						// Kein B-Baum vorhanden:
						else
		                    printf("no B-Tree was created\n");
	            	}
	            	
	            	else
						printf("invalid key\n");					
            } //Ende  search Key 			    			   
            
            
			
			// set Printmode
			if(!strncasecmp(input , "printmode", 9)){
				validCommand=true;
				char str[10]="";
				sscanf( input,  "%*9s%s", &str );
				if((!strcasecmp(str , "on"))||(!strcasecmp(str , "true"))){
					printMode=true;
					clrscr();
					print(_btree);
				}
				
				else{
					if((!strcasecmp(str , "off"))||(!strcasecmp(str , "false")))
						printMode=false;
					
					else
						printf("unknown parameter: %s\n", str);
				}
			}	// Einde Printmode	
			
			
			
			// Overflows
			if(!strncasecmp(input , "overflows", 9)){
				validCommand=true;
				char str[10]="";
				sscanf( input,  "%*9s%s", &str );
				if((!strcasecmp(str , "on"))||(!strcasecmp(str , "true")))
					overflows=true;
				
				
				else{
					if((!strcasecmp(str , "off"))||(!strcasecmp(str , "false")))
						overflows=false;
					
					else
						printf("unknown parameter: %s\n", str);
				}
			setOverflow(overflows);
			}	// Einde Printmode			
			
			if((!validCommand) && (strcasecmp(input , "\n")) && (strcasecmp(input , "exit\n")) )
				printf("invalid/unknown command!\n");	               
	}
}



/** 
*	fuegt die Key von from bis to in aufsteigender Reihenfolge dem B-Baum hinzu
*	@param  from Startpunkt
*	@param  to Endpunkt
*	@param  to Schrittweite
*/
void insertSequence(btree *_btree, int from, int to, int stepSize){
	int i=0;
	int size=to-from+1;

	entry* entries=(entry*)malloc(size*sizeof(entry));
		
	for(i=0; i<size; i+=stepSize){
	entries[i]=(entry){from+i,NULL,NULL};
	insert(_btree, &entries[i]);
	}
}



/** 
*	loescht den Key key, falls dieser Teis des B-Baums ist
*	@param  _btree verweist auf den zu durchsuchenden B-Baum 
*	@param  key verweist auf den zu loeschenden Key 
*/
void deleteKey(btree *_btree, int key){

	entry* delEntry=(entry*)malloc(sizeof(entry));
	delEntry[0]=(entry){key,NULL,NULL};
	rmv(_btree, &delEntry[0]);
        free(delEntry);
}



/** 
*	Ueberprueft ob der gesuchte Eintrag y im B-Baum _btree vorhanden ist
*	@param  _btree verweist auf den zu durchsuchenden B-Baum 
*	@param  key verweist auf den gesuchten Key
*	@return 0=Eintrag nicht enthalten, 1=Eintrag ist enthalten
*/
int searchKey(btree *_btree, int key){

	entry* searchEntry=(entry*)malloc(sizeof(entry));
	searchEntry[0]=(entry){key,NULL,NULL};
	return isInBtree(_btree, &searchEntry[0]);
}


