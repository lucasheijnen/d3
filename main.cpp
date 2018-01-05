#include <iostream>
#include "Automaton.h"
#include "exprtree.h"

//part II
std::set<BitVector> solveFromState(std::set<BitVector> bvset,
					std::map<unsigned,int> pres, int b){
	std::set<BitVector> tempset;
	int temp;
	for(auto i : bvset){
		temp = 0;
		for(auto j : pres) temp += i.find(j.first)->second * j.second;
		if((temp - b) % 2 == 0) tempset.insert(i);
	}
	return tempset;
}

//part II
Automaton build(std::set<BitVector> bvset, std::map<unsigned,int> pres, int b,
								Automaton theAuto){
	std::set<BitVector> tempset = solveFromState(bvset, pres, b);
	BitVector bv;
	int temp;
	for(auto i : tempset){
		temp = b;
		bv = i;
		for(auto j : pres) temp -= bv[j.first] * j.second;
		temp = temp/2;
		theAuto.addTransition(b, bv, temp);
		if(!theAuto.stateInstates(temp)){
			theAuto.addState(temp);
			theAuto = build(bvset, pres, temp, theAuto);
		}
		else theAuto.addState(temp);
	}
	return theAuto;
}

//part II
Automaton automair(node<expr>* root){
	Automaton theAuto;
	ExprTree * tree = new ExprTree();
	std::map<unsigned,int> pres;
	std::set<BitVector> bvset;
	BitVector bv;
	int b = 0, temp;
	tree->createFromNode(root);
	tree->getPresburgerMap(pres, b);
	std::cerr << b << std::endl;
	if(b<0) { //cantor werkt niet met negatieve getallen
	 	b*=-1;
		for(auto i : pres)i.second*=-1;
	}
	theAuto.addState(b);
	theAuto.markInitial(b);
	for(auto i : pres) theAuto.addVar(i.first);
	for(int i = 0; i != pow(2, pres.size()); ++i){
		temp = i;
		for(auto j : pres){
			bv[j.first] = temp&1;
			temp >>= 1;
		}
		bvset.insert(bv);
	}
	theAuto = build(bvset, pres, b, theAuto);
	theAuto.markFinal(0);
	return theAuto;
}

//part II
Automaton createAutomaton(ExprTree * exptree);

//part II
Automaton conjunction(ExprTree * exptree){
	Automaton theAuto, theAuto1, theAuto2;
	ExprTree * exptree1 = new ExprTree();
	ExprTree * exptree2 = new ExprTree();
	exptree1->createFromNode(exptree->getRoot()->getLeft());
	exptree2->createFromNode(exptree->getRoot()->getRight());
	theAuto1 = createAutomaton(exptree1);
	theAuto2 = createAutomaton(exptree2);
	theAuto1.insertFreeVars(theAuto2);
	theAuto2.insertFreeVars(theAuto1);
	theAuto.intersect(theAuto1, theAuto2);
	return theAuto;
}

//part II
Automaton quantification(ExprTree * exptree){
	ExprTree * temptree = new ExprTree();
	temptree->createFromNode(exptree->getRoot()->getRight());
	Automaton theAuto = createAutomaton(temptree);
	theAuto.quant(exptree->getRoot()->getLeft()->getData().variable);
	return theAuto;
}

//part II
Automaton createAutomaton(ExprTree * exptree){
    Automaton theAuto;
    switch(exptree->getRoot()->getData().type) {
    	case expr::NOT: break;
    	case expr::EXISTS: theAuto = quantification(exptree); break;
		case expr::AND: theAuto = conjunction(exptree); break;
		case expr::EQUALS: theAuto = automair(exptree->getRoot()); break;
    	default: break;
    }
    // TODO (voor studenten in deel 2): Bouw de Presburger automaat door de meegegeven syntaxtree exptree van de formule te doorlopen
    return theAuto;
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
	return theAuto.inFinalState();
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
				theAuto.makeDeterministic(theAuto);
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
