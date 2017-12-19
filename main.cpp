#include <iostream>
#include "Automaton.h"
#include "exprtree.h"

Automaton automair(node<expr>* root){
	Automaton theAuto;
	ExprTree * tree = new ExprTree();
	tree->createFromNode(root);
	int b = 0, temp;	
	std::map<unsigned,int> pres;
	std::set<BitVector> bvset;
	BitVector bv;
	tree->getPresburgerMap(pres, b);
	theAuto.addState(b);
	theAuto.markInitial(b);
	//shit1ALLEBITVECTOREN GENERENREN
	for(int i = 0; i != pow(2, pres.size()); ++i){
		temp = i;
		for(std::map<unsigned,int>::iterator j = pres.begin(); j != pres.end();
			++j){
			bv[j->first] = temp&1;
			temp >>= 1;
		}
		bvset.insert(bv);
	}
	//shit2CONTROELJFE
	//shit3TRANSITITEJOITEN oftewel c
	theAuto.markFinal(0);
	return theAuto;
}
Automaton createAutomaton(ExprTree * exptree){
    Automaton theAuto;
    switch(exptree->getRoot()->getData().type) {
    	case expr::AND: 
    	case expr::NOT: break;
    	case expr::EXISTS: 
    		if(exptree->getRoot()->getRight()->getData().type == expr::EQUALS) {}
					//theAuto = automair(exptree->getRoot()->getRight());
    		else if(exptree->getRoot()->getRight()->getData().type == expr::AND){}
			else if(exptree->getRoot()->getRight()->getData().type == expr::NOT){} 		
    		break;
			case expr::EQUALS: theAuto = automair(exptree->getRoot());
				break;
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
