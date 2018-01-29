//Max Blankestijn s1819658 & Loes Dekker s1850024

#include <iostream>
#include "Automaton.h"
#include "exprtree.h"

//fold
//turnes integer into unique positive integer
int fold(int i){
	if(i >= 0) return 2 * i; //every positive int turns into an even int
	else return -2 * i - 1; //every negative int turns into an odd positive int
}

//solveFromState
//determines which of the given bitVectors in given std::set<BitVector> bvSet
//lead to a non trash state
std::set<BitVector> solveFromState(std::set<BitVector> bvSet,
					std::map<unsigned,int> pres, int b){
	std::set<BitVector> tempSet;
	int temp;
	for(auto i : bvSet){
		temp = 0;
		for(auto j : pres) temp += i.find(j.first)->second * j.second;
		if((temp - b) % 2 == 0) tempSet.insert(i);
	}
	return tempSet;
}

//build
//generates new states from the given state and inserts transitions into
//the given Automaton theAuto
Automaton build(std::set<BitVector> bvSet, std::map<unsigned,int> pres, int b,
								Automaton theAuto){
	std::set<BitVector> tempSet = solveFromState(bvSet, pres, b); //now contains
	BitVector bv;			//only bitVectors that lead to a non trash state
	int temp;
	for(auto i : tempSet){
		temp = b;
		bv = i;
		for(auto j : pres) temp -= bv[j.first] * j.second; //transition
		temp = temp/2; //new state reached by transition with bitvector from b
		theAuto.addTransition(fold(b), bv, fold(temp));
		if(!theAuto.contains(fold(temp))){ //if new state was reached, build all
			theAuto.addState(fold(temp)); //possible transitions from this state
			theAuto = build(bvSet, pres, temp, theAuto); //in recursive way
		}
		else theAuto.addState(fold(temp));
	}
	return theAuto;
}

//automair
//creates automaton corresponding to given root
Automaton automair(node<expr>* root){
	Automaton theAuto;
	ExprTree * tree = new ExprTree();
	std::map<unsigned,int> pres;
	std::set<BitVector> bvSet;
	BitVector bv;
	int b = 0, temp;
	tree->createFromNode(root);
	tree->getPresburgerMap(pres, b); //initialState (b) is found, occurence of
	theAuto.addState(fold(b));       //every free var is counted
	theAuto.markInitial(fold(b));
	for(auto i : pres) theAuto.addVar(i.first); //add free vars to automaton
	for(int i = 0; i != pow(2, pres.size()); ++i){
		temp = i;
		for(auto j : pres){
			bv[j.first] = temp&1;
			temp >>= 1;
		}
		bvSet.insert(bv); //inserts all possible bitVectors in std::set<State> bvSet
	}
	theAuto = build(bvSet, pres, b, theAuto); //builds the automaton from given b
	theAuto.markFinal(0);											//and presburger map pres
	delete tree;
	return theAuto;
}

Automaton createAutomaton(ExprTree * exptree);

//conjunction
//intersects the automaton corresponding to the left subtree of the root of the
//given ExprTree exptree with the automaton corresponding to its right subtree
Automaton conjunction(ExprTree * exptree){
	Automaton theAuto, theAuto1, theAuto2;
	ExprTree * exptree1 = new ExprTree();
	ExprTree * exptree2 = new ExprTree();
	exptree1->createFromNode(exptree->getRoot()->getLeft());
	exptree2->createFromNode(exptree->getRoot()->getRight());
	theAuto1 = createAutomaton(exptree1); //contains automaton corresponding to left child
	theAuto2 = createAutomaton(exptree2); //contains automaton corresponding to right child
	theAuto1.insertFreeVars(theAuto2); //free vars in theAuto2 are added to theAuto1
	theAuto2.insertFreeVars(theAuto1); //free vars in theAuto1 are added to theAuto2
	theAuto.intersect(theAuto1, theAuto2);
	delete exptree1; delete exptree2;
	return theAuto;
}

//quantification
//projects free variable (left child) of root of given ExprTree exptree on the automaton
//created from the formual in right subtree of exptree
Automaton quantification(ExprTree * exptree){
	ExprTree * tempTree = new ExprTree();
	tempTree->createFromNode(exptree->getRoot()->getRight());
	Automaton theAuto = createAutomaton(tempTree);
	theAuto.project(exptree->getRoot()->getLeft()->getData().variable);
	//projects the variable upon the automaton already contained in theAuto
	delete tempTree;
	return theAuto;
}

//complement
//creates deterministic automaton corresponding to non-complemented formula
//in left subtree of root of given ExprTree exptree before complementing it
Automaton complement(ExprTree * exptree){
	ExprTree * tempTree = new ExprTree();
	tempTree->createFromNode(exptree->getRoot()->getLeft());
	Automaton temp = createAutomaton(tempTree);	//temp is automaton of non-
	Automaton theAuto, detAuto;									//complemented formula
	detAuto.makeDeterministic(temp); //detAuto is temp made deterministic
	theAuto.complement(detAuto); //theAuto is the complemented automaton
	delete tempTree;
	return theAuto;
}

//createAutomaton
//uses given ExprTree exptree to create a corresponding automaton
Automaton createAutomaton(ExprTree * exptree){
  Automaton theAuto;
  switch(exptree->getRoot()->getData().type) { //root contains operation
  	case expr::NOT: theAuto = complement(exptree); break;
   	case expr::EXISTS: theAuto = quantification(exptree); break;
		case expr::AND: theAuto = conjunction(exptree); break;
		case expr::EQUALS: theAuto = automair(exptree->getRoot()); break;
   	default: break;
  }
  theAuto.restate(); //renames every state so that the biggest state is
  return theAuto;    //equal to the amount of states - 1
}

void addVarToBitVectors(std::list<BitVector> &l, const unsigned index, int val) {
    for(std::list<BitVector>::iterator it = l.begin(); it != l.end(); ++it) {
        bool bit = (val&1);
        (*it)[index] = bit;
        val>>=1;
    }

    // create BitVector in which all variables have a bit value of 0
    BitVector zeroVector;
    if(l.size() > 0)
        zeroVector = *(l.begin());
    for(BitVector::iterator it = zeroVector.begin(); it != zeroVector.end(); ++it) {
            it->second = 0;
    }

    // val requires more bits than l.size(): add new vectors at the end of l
    while(val || l.size() == 0) {
        BitVector b = zeroVector;
        bool bit = (val&1);
        b[index] = bit;
        l.push_back(b);
        val>>=1;
    }
}

/** Utility function for BitVectors. Prints the content all BitVectors in the list to the given output stream
*/
void printBitVectors(std::ostream &out, std::list<BitVector> l) {
    for(BitVector b: l) {
        out << "[";
        for(std::pair<unsigned, bool> elem : b) {
            out << elem.first << ": " << elem.second << ", ";
        }
        out << "]\n";
    }
}

std::list<BitVector> generateBitVectors(std::map<unsigned,unsigned> valueMap){
    std::list<BitVector> l;
    for(auto &var : valueMap){
        addVarToBitVectors(l, var.first, var.second);
    }
    return l;
}

bool verifyAutomaton(Automaton& theAuto, string formula, bool debug, std::ostream &out){
	std::stringstream ss(formula);
	string data;
	int variable;
	int value;
	std::map<unsigned,unsigned> valueMap;
	while(ss >> data){
		if(data[0] == 'x'){
			string substr = data.substr(1,data.find('=')-1);
			variable = std::atoi(substr.c_str());
			if(variable == 0)
				return false;
			string substr2 = data.substr(data.find('=')+1,string::npos);
			value =  std::atoi(substr2.c_str());
			valueMap[variable] = value;
		}
		else
			return false;
	}
	std::list<BitVector> l = generateBitVectors(valueMap);
	theAuto.parseInput(l);
        if(debug) {
            printBitVectors(out,l);
        }
	return theAuto.inFinalState() || theAuto.nulBit();
}


void menu(bool debug, std::istream& inStr, std::ostream &out){
	Automaton theAuto;
	ExprTree * t = new ExprTree();
	while(true){
		string in;
		getline(inStr,in, '\n');
		if(in.substr(0,5) == "read "){
			if(t->create(in.substr(4,string::npos))) {
				theAuto = createAutomaton(t);
        if(debug) {
        	theAuto.print(out);
        }
    	} else if(!debug) {
			out << "Error: invalid formula" <<std::endl;
   		}
		}
		else if(in.substr(0,6) == "check "){
			if(verifyAutomaton(theAuto,in.substr(5,string::npos), debug, out))
				out << "valid" <<std::endl;
			else
				out << "invalid" <<std::endl;
		}
		else if(in.substr(0,3) == "end")
			return;
		else if(!debug)
			out << "Error: invalid input" <<std::endl;
	}
	delete t;
}

int main(int argc, char** argv)
{
    bool debug = false;
    if(argc >= 2 && string(argv[1]).find("d") != string::npos){
        debug = true;
    }
    menu(debug, std::cin, std::cout);
    return 0;
}
