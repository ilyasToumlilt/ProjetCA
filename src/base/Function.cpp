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

  /**** DEBUT: TP1-2 Exo2 ****/
  
  /* le premier BB commence � la premi�re instruction
     pour illiminer les directives au d�but */
  while( current != _end && !(current->isInst()) )
    current = current->get_next();
  
  while( current != _end ){

    /* pour ignorer les directives entres les blocks 
       ( surtotu � la fin )
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
    current = current->get_next();
  }
  
  /**** FIN: TP1-2 Exo2 ****/

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
   Basic_block *current;
   Instruction *instr;
   Basic_block *succ=NULL;
   // IMPORTANT ne pas enlever la ligne ci-dessous 
   if (BB_pred_succ) return;
   int size= (int) _myBB.size();
   it=_myBB.begin();

   /*** boucle qui permet d'it�rer sur les blocs de la fonction ***/
   for (int i=0; i<size; i++){
     current=*it;
    
     /** A COMPLETER **/

     /**** DEBUT: TP1-2 Exo3 ****/

     instr = (Instruction*)current->get_branch();

     if( instr ){
       /* cas du branchement conditionnel */
       if( instr->is_cond_branch() ){
	 /* on rajoute la cible du branchement */
	 current->set_link_succ_pred(find_label_BB(instr->get_op_label()));
	 /* et �galement le block suivant, si on est pas au dernier */
	 if( i < size - 1 ){
	   current->set_link_succ_pred(get_BB(i+1));
	 }
       }
       /* cas de l'appel de fonction */
       else if( instr->is_call() ){
	 /* block suivant si je ne suis pas dernier ... */
	 if( i < size - 1 ){
	   current->set_link_succ_pred(get_BB(i+1));
	 }
       }
       /* cas des sauts indirects */
       else if( instr->is_indirect_branch() ){
	 /* je ne fais rien */
	 ;
       }
       else {
	 /* que la cible du Br */
	 current->set_link_succ_pred(find_label_BB(instr->get_op_label()));
       }
     } else {
       /* si pas de branchement je me rajoute le block suivant comme 
	  successeur, si je ne suis pas le block de fin */
       if( i < size - 1 ){
	 current->set_link_succ_pred(get_BB(i+1));
       }
     }
     
     it++;

     /**** FIN: TP1-2 Exo3 ****/
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

  /**** BEGIN: TP1-2 Exo5 ****/
  int nbPred, i, j;

  /* la liste des blocks ne doit pas �tre vide ... */
  if( (int)_myBB.size() ){
    /* Je commence par rajouter le premier block � la working list */
    workinglist.push_back(get_BB(0));
    /* parcours de la working list */
    while( workinglist.size() ){
      it = workinglist.begin();
      current = *it;
      
      nbPred = current->get_nb_pred();

      /* si le block n'a pas de pr�d�cesseur c'est que son seul dominant
	 est lui-m�me */
      if( nbPred == 0 ){
	for(i=0; i<(int)_myBB.size(); i++)
	  current->Domin[i] = ( i == current->get_index() ) ? true : false;
      }
      /* sinon s'il a un seul pr�d�cesseur, il prend tous ses dominant
	 union lui-m�me */
      else if( nbPred == 1 ){
	pred = current->get_predecessor(0);
	for(i=0; i<(int)_myBB.size(); i++){
	  if( i == current->get_index()
	      && current->Domin[i] == false ){
	    current->Domin[i] = true;
	    change = true;
	  }
	  else if( i != current->get_index()
		   && current->Domin[i] != pred->Domin[i] ){
	    current->Domin[i] = pred->Domin[i];
	    change = true;
	  }
	}
      }
      /* sinon s'il y a plus d'un pr�d�cesseur on fait l'intersection des
	 dominants des pr�d�cesseurs + moi m�me */
      else {
	for(i=0; i<(int)_myBB.size(); i++){
	  if( i == current->get_index() ) {
	    if( current->Domin[i] == false ){
	      current->Domin[i] = true;
	      change = true;
	    }
	  }
	  else {
	    for(j=0; j<nbPred; j++)
	      if( current->get_predecessor(j)->Domin[i] == false )
		break;
	    if( j != nbPred
		&& current->Domin[i] == true ){
	      current->Domin[i] = false;
	      change = true;
	    }
	    else if ( j == nbPred
		      && current->Domin[i] == false ){
	      current->Domin[i] = true;
	      change = true;
	    }
	  }
	}
      }

      /* si un changement a eu lieu, je rajoute tous les seccesseurs */
      if( change ){
	for(i=0; i<current->get_nb_succ(); i++){
	  if(i==0)
	    workinglist.push_back(current->get_successor1());
	  if(i==1)
	    workinglist.push_back(current->get_successor2());
	}
	change = false;
      }
      
      /* pop */
      workinglist.pop_front();
    }

  }

  /**** END: TP1-2 Exo5 ****/

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
