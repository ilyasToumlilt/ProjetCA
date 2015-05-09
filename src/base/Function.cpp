#include <Function.h>

Function::Function(){
  _head = NULL;
  _end = NULL;
  BB_computed = false;
  BB_pred_succ = false;
  dom_computed = false;
}

Function::~Function(){}

void Function::set_head(Line *head){
  _head = head;
}

void Function::set_end(Line *end){
  _end = end;
}

Line* Function::get_head(){
  return _head;
}

Basic_block* Function::get_firstBB(){
   return _myBB.front();
}

Line* Function::get_end(){
  return _end;
}
void Function::display(){
  cout<<"Begin Function"<<endl;
  Line* element = _head;

  if(element == _end)	
    cout << _head->get_content() <<endl;

  while(element != _end){
    cout << element->get_content() <<endl;
		
    if(element->get_next()==_end){
      cout << element->get_next()->get_content() <<endl;
      break;
    }
    else element = element->get_next();

    }
  cout<<"End Function\n\n"<<endl;
	
}

int Function::size(){
  Line* element = _head;
  int lenght=0;
  while(element != _end)
    {
      lenght++;		
      if (element->get_next()==_end)
	break;
      else
	element = element->get_next();
    }
  return lenght;
}	


void Function::restitution(string const filename){
	
  Line* element = _head;
  ofstream monflux(filename.c_str(), ios::app);

  if(monflux){
    monflux<<"Begin"<<endl;
    if(element == _end)	
      monflux << _head->get_content() <<endl;
    while(element != _end)
      {
	if(element->isInst() || 
	   element->isDirective()) 
	  monflux<<"\t";
	
	monflux << element->get_content() ;
	
	if(element->get_content().compare("nop")) 
	  monflux<<endl;
		
	if(element->get_next()==_end){
	  if(element->get_next()->isInst() || 
	     element->get_next()->isDirective())
	    monflux<<"\t";
	  monflux << element->get_next()->get_content()<<endl;
	  break;
	}
	else element = element->get_next();

      }
    monflux<<"End\n\n"<<endl;
		
  }

  else {
    cout<<"Error cannot open the file"<<endl;
  }

  monflux.close();
}

void Function::comput_label(){
  Line* element = _head;

  if(element == _end && element->isLabel())	
    _list_lab.push_back(getLabel(element));
  while(element != _end)
    {

      if(element->isLabel())	
	_list_lab.push_back(getLabel(element));

      if(element->get_next()==_end){
	if(element->isLabel())	
	  _list_lab.push_back(getLabel(element));
	break;
      }
      else element = element->get_next();

    }

}

int Function::nbr_label(){
  return _list_lab.size();

}

Label* Function::get_label(int index){

  list<Label*>::iterator it;
  it=_list_lab.begin();

  int size=(int) _list_lab.size();
  if(index< size){
    for (int i=0; i<index;i++ ) it++;
    return *it;	
  }
  else cout<<"Error get_label : index is bigger than the size of the list"<<endl; 
	
  return _list_lab.back();
}

Basic_block *Function::find_label_BB(OPLabel* label){
  //Basic_block *BB = new Basic_block();
   int size=(int)_myBB.size();
   string str;
   for(int i=0; i<size; i++){		
      if(get_BB(i)->is_labeled()){
	 
	 str=get_BB(i)->get_head()->get_content();
	 if(!str.compare(0, (str.size()-1),label->get_op())){
	    return get_BB(i);
	 }
      }
  }
  return NULL;
}


/* ajoute nouveau BB � la liste de BB de la fonction en le creant */

void Function::add_BB(Line *debut, Line* fin, Line *br, int index){
   Basic_block *b=new Basic_block();
   b->set_head(debut);
   b->set_end(fin);
   b->set_index(index);
   b->set_branch(br);
   _myBB.push_back(b);
}


//Calcule la liste des blocs de base : il faut d�limiter les BB, en parcourant la liste des lignes/instructions � partir de la premiere, il faut s'arreter � chaque branchement (et prendre en compte le delayed slot qui appartient au meme BB, c'est l'instruction qui suit tout branchement) ou � chaque label (on estime que tout label est utilis� par un saut et donc correspond bien � une entete de BB).


void Function::comput_basic_block(){
  Line *debut, *current, *prev;
  current=_head;
  debut=_head;
  prev = NULL;
  int ind=0;
  Line *l=NULL;
  Instruction *i=NULL;
  Line * b;

  cout<< "comput BB" <<endl;
  cout<<"head :"<<_head->get_content()<<endl;
  cout<<"tail :"<<_end->get_content()<<endl;

  //ne pas enlever la ligne ci-dessous 
  if (BB_computed) return;

  
  /**** A COMPLETER ****/

  /* le premier BB commence � la premi�re instruction
     pour eliminer les directives au d�but */
  while( current != _end && !(current->isInst()) )
    current = current->get_next();
  
  while( current != _end ){

    /* pour ignorer les directives entres les blocks 
       ( surtout � la fin )
       si je n'ai pas encore assign� de d�but, je skip */
    if( current == debut
	&& current->isDirective() ){
      current = current->get_next();
      debut = current;
      continue;
    }
    
    /* si la ligne courante est une instruction de branchement */
    if( current->isInst() && (current->get_type() == BR)){
      /* on passe � l'inst suivante ( son delayed slot ) */
      /* je ne boucle pas jusqu'� l'inst suivante vu qu'on a consid�r� 
	 que le delayed slot suit directement le branchement */
      cout << "DEBUG:" << current->get_content() << endl;
      current = current->get_next();

      /* et je add le new BB */
      add_BB(debut, current, current->get_prev(), ind);
      ind++;

      /* et je d�fini le nouveau debut */
      if( current != _end )
	debut = current->get_next();
    }
    /* sinon si c'est un label,
       et que la ligne n'est pas d�j� d�but d'un BB */
    else if( current->isLabel()
	     && current != debut ){
      add_BB(debut, current->get_prev(), NULL, ind);
      ind++;

      debut = current;
    }

    /* mot de fin ? */
    if( current->get_next() == _end
	&& !(current->get_type() == BR )){
      add_BB(debut, current->get_next(), NULL, ind);
      break;
    }
    
    /* incr */
    prev    = current;
    cout << "DEBUG:" << current->get_content() << endl;
    current = current->get_next();
  }
  


  cout<<"end comput Basic Block"<<endl;
  BB_computed = true;
  return;
}


int Function::nbr_BB(){
   return _myBB.size();
}

Basic_block *Function::get_BB(int index){

  list<Basic_block*>::iterator it;
  it=_myBB.begin();
  int size=(int)_myBB.size();

  if(index< size){
    for (int i=0; i<index;i++ ) it++;
    return *it;	
  }
  else 
    return NULL;
}

list<Basic_block*>::iterator Function::bb_list_begin(){
   return _myBB.begin();
}

list<Basic_block*>::iterator Function::bb_list_end(){
   return _myBB.end();
}

/* comput_pred_succ calcule les successeurs et pr�d�cesseur des BB, pour cela il faut commencer par les successeurs */
/* et it�rer sur tous les BB d'une fonction */
/* il faut determiner si un BB a un ou deux successeurs : d�pend de la pr�sence d'un saut pr�sent ou non � la fin */
/* pas de saut ou saut incontionnel ou appel de fonction : 1 successeur (lequel ?)*/
/* branchement conditionnel : 2 successeurs */ 
/* le dernier bloc n'a pas de successeurs - celui qui se termine par jr R31 */
/* les sauts indirects n'ont pas de successeur */


void Function::comput_succ_pred_BB(){
  
   list<Basic_block*>::iterator it, it2;
   Basic_block *current, *current2;
   Instruction *instr;
   Basic_block *succ=NULL;
   // IMPORTANT ne pas enlever la ligne ci-dessous 
   if (BB_pred_succ) return;
   int size= (int) _myBB.size();
   it=_myBB.begin();
   
   for (int i=0; i<size; i++){
     current=*it;
    
     /** A COMPLETER **/
     /*** boucle qui permet d'it�rer sur les blocs de la fonction ***/

     /********************** Partie ajoutee ***********************************/

     /* dernier bloc de base n'a pas de succ */
     if( current->get_end() == _end){
       break;
     }
     
     /* Si la derniere instruction du bloc est un branchement */
     if(current->get_branch()){
       if(getInst(current->get_branch())->is_indirect_branch()){
	 /* pas de successeur pour les sauts indirects */
	 it++;
	 continue;
       }
       else if(getInst(current->get_branch())->is_cond_branch()){
	 /* branchement conditionnel : 2 successeurs */
	 succ = find_label_BB(getInst(current->get_branch())->get_op_label());
	 /* ajout du successeur pointee par 
	    le Label indiquee au branchement */
	 current->set_link_succ_pred(succ);
	 succ = current->get_successor1();
       }
       else{
	 /* 1 seul successeur, recuperer le 
	    bloc de base du Label du saut */
	 succ = find_label_BB(getInst(current->get_branch())->get_op_label());
       }
     }
     else{
       /* pas de saut 1 successeur */
       /* idee parcourir la liste des blocs de bases pour recuperer
	  le bloc de base correspondant au Label */
       it2=_myBB.begin();
       
       for (int i=0; i<size; i++){
	 current2=*it2;

	 if(current2->get_head() == current->get_end()->get_next()){
	   succ = current2;
	   break;
	 }
	 
	 it2++;
       }
     }
     
     /* ajout du successeur du bloc de base courant */
     current->set_link_succ_pred(succ);
     
     /********************** Fin Partie ajoutee *******************************/
     
     it++;
   }
   
   // ne pas enlever la ligne ci-dessous
   BB_pred_succ = true;
   return;
}

void Function::compute_dom(){
 list<Basic_block*>::iterator it, it2;
  list<Basic_block*> workinglist;
  Basic_block *current, *bb, *pred;
  Instruction *instr;
  bool change = true;
  
  // NE pas enlever les 2 ligne ci-dessous
  if (dom_computed) return;
  comput_succ_pred_BB();
 

  // A COMPLETER 

  // ne pas enlever 
  dom_computed = true;
  return;
}

void Function::compute_live_var(){
  
  
  /*** A COMPLETER */

  return;
}
     





/* en implementant la fonction test de la classe BB, permet de tester des choses sur tous les blocs de base d'une fonction par exemple l'affichage de tous les BB d'une fonction ou l'affichage des succ/pred des BBs comme c'est le cas -- voir la classe Basic_block et la m�thode test */

void Function::test(){
  int size=(int)_myBB.size();
   for(int i=0;i<size; i++){
    get_BB(i)->test();
  }
   return;
}
